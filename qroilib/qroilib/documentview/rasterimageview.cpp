// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2011 Aurélien Gâteau <agateau@kde.org>
Qroilib : QT ROI Lib
Copyright 2018 Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
// Self
#include "rasterimageview.h"

// Local
#include <qroilib/imagescaler.h>
#include <qroilib/cms/cmsprofile.h>
#include <qroilib/gvdebug.h>

// Qt
#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QPointer>
#include <QDebug>

// LCMS2
#include <lcms2.h>

#include "roiobject.h"
#include "roiobjectitem.h"
#include "toolmanager.h"
#include "abstracttool.h"

namespace Qroilib
{

struct RasterImageViewPrivate
{
    RasterImageView* q;
    ImageScaler* mScaler;
    QPixmap mBackgroundTexture;
    bool mEmittedCompleted;

    // Config
    RasterImageView::AlphaBackgroundMode mAlphaBackgroundMode;
    QColor mAlphaBackgroundColor;
    bool mEnlargeSmallerImages;
    // /Config

    bool mBufferIsEmpty;
    QPixmap mCurrentBuffer;
    // The alternate buffer is useful when scrolling: existing content is copied
    // to mAlternateBuffer and buffers are swapped. This avoids allocating a new
    // QPixmap every time the image is scrolled.
    QPixmap mAlternateBuffer;

    QTimer* mUpdateTimer;

    //QPointer<AbstractTool> mTool;

    bool mApplyDisplayTransform; // Defaults to true. Can be set to false if there is no need or no way to apply color profile
    cmsHTRANSFORM mDisplayTransform;

    void updateDisplayTransform(QImage::Format format)
    {
        GV_RETURN_IF_FAIL(format != QImage::Format_Invalid);
        mApplyDisplayTransform = false;
        if (mDisplayTransform) {
            cmsDeleteTransform(mDisplayTransform);
        }
        mDisplayTransform = 0;

        Cms::Profile::Ptr profile = q->document()->cmsProfile();
        if (!profile) {
            // The assumption that something unmarked is *probably* sRGB is better than failing to apply any transform when one
            // has a wide-gamut screen.
            profile = Cms::Profile::getSRgbProfile();
        }
        Cms::Profile::Ptr monitorProfile = Cms::Profile::getMonitorProfile();
        if (!monitorProfile) {
            qWarning() << "Could not get monitor color profile";
            return;
        }

        cmsUInt32Number cmsFormat = 0;
        switch (format) {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
            cmsFormat = TYPE_BGRA_8;
            break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        case QImage::Format_Grayscale8:
            cmsFormat = TYPE_GRAY_8;
            break;
#endif
        default:
            qWarning() << "Qroilib can only apply color profile on RGB32 or ARGB32 images";
            return;
        }
        mDisplayTransform = cmsCreateTransform(profile->handle(), cmsFormat,
                                               monitorProfile->handle(), cmsFormat,
                                               INTENT_PERCEPTUAL, cmsFLAGS_BLACKPOINTCOMPENSATION);
        mApplyDisplayTransform = true;
    }

    void createBackgroundTexture()
    {
        mBackgroundTexture = QPixmap(32, 32);
        QPainter painter(&mBackgroundTexture);
        painter.fillRect(mBackgroundTexture.rect(), QColor(128, 128, 128));
        QColor light = QColor(192, 192, 192);
        painter.fillRect(0, 0, 16, 16, light);
        painter.fillRect(16, 16, 16, 16, light);
    }

    void setupUpdateTimer()
    {
        mUpdateTimer = new QTimer(q);
        mUpdateTimer->setInterval(500);
        mUpdateTimer->setSingleShot(true);
        QObject::connect(mUpdateTimer, SIGNAL(timeout()), q, SLOT(updateBuffer()));
    }

    QRectF mapViewportToZoomedImage(const QRectF& viewportRect) const
    {
        return QRectF(viewportRect.topLeft() - q->imageOffset() + q->scrollPos(),
                      viewportRect.size());
    }

    void setScalerRegionToVisibleRect()
    {
        QRectF rect = mapViewportToZoomedImage(q->boundingRect());
        mScaler->setDestinationRegion(QRegion(rect.toRect()));
    }

    void resizeBuffer()
    {
        QSize size = q->visibleImageSize().toSize();
        if (size == mCurrentBuffer.size()) {
            return;
        }
        if (!size.isValid()) {
            mAlternateBuffer = QPixmap();
            mCurrentBuffer = QPixmap();
            return;
        }

        mAlternateBuffer = QPixmap(size);
        mAlternateBuffer.fill(Qt::transparent);
        {
            QPainter painter(&mAlternateBuffer);
            painter.drawPixmap(0, 0, mCurrentBuffer);
        }
        qSwap(mAlternateBuffer, mCurrentBuffer);

        mAlternateBuffer = QPixmap();
    }

    void drawAlphaBackground(QPainter* painter, const QRect& viewportRect, const QPoint& zoomedImageTopLeft)
    {
        if (mAlphaBackgroundMode == RasterImageView::AlphaBackgroundCheckBoard) {
            QPoint textureOffset(
                zoomedImageTopLeft.x() % mBackgroundTexture.width(),
                zoomedImageTopLeft.y() % mBackgroundTexture.height()
            );
            painter->drawTiledPixmap(
                viewportRect,
                mBackgroundTexture,
                textureOffset);
        } else {
            painter->fillRect(viewportRect, mAlphaBackgroundColor);
        }
    }
};

RasterImageView::RasterImageView(QGraphicsItem* parent, int seq)
: AbstractImageView(parent)
, d(new RasterImageViewPrivate)
, mCurrentModifiers(Qt::NoModifier)
, mUnderMouse(false)
{
    bScalerSmooth = false;
    mSeq = seq;
    d->q = this;
    d->mEmittedCompleted = false;
    d->mApplyDisplayTransform = true;
    d->mDisplayTransform = 0;

    d->mAlphaBackgroundMode = AlphaBackgroundCheckBoard;
    d->mAlphaBackgroundColor = Qt::black;
    d->mEnlargeSmallerImages = true;

    d->mBufferIsEmpty = true;
    d->mScaler = new ImageScaler(this);
    connect(d->mScaler, &ImageScaler::scaledRect, this, &RasterImageView::updateFromScaler);

    d->createBackgroundTexture();
    d->setupUpdateTimer();
}

RasterImageView::~RasterImageView()
{
    if (d->mDisplayTransform) {
        cmsDeleteTransform(d->mDisplayTransform);
    }
    delete d;
}

void RasterImageView::setAlphaBackgroundMode(AlphaBackgroundMode mode)
{
    d->mAlphaBackgroundMode = mode;
    if (document() && document()->hasAlphaChannel()) {
        d->mCurrentBuffer = QPixmap();
        updateBuffer();
    }
}

void RasterImageView::setAlphaBackgroundColor(const QColor& color)
{
    d->mAlphaBackgroundColor = color;
    if (document() && document()->hasAlphaChannel()) {
        d->mCurrentBuffer = QPixmap();
        updateBuffer();
    }
}

void RasterImageView::setScalerSmooth(bool b)
{
    bScalerSmooth = b;
}

void RasterImageView::loadFromDocument()
{
    Document::Ptr doc = document();
    if (!doc) {
        return;
    }

    document()->setSize(QSize(640,480));
    finishSetDocument();
}

void RasterImageView::finishSetDocument()
{
    GV_RETURN_IF_FAIL(document()->size().isValid());

    d->mScaler->setDocument(document());
    d->resizeBuffer();
    applyPendingScrollPos();

    connect(document().data(), SIGNAL(imageRectUpdated(QRect)),
            SLOT(updateImageRect(QRect)));

    if (zoomToFit()) {
        // Force the update otherwise if computeZoomToFit() returns 1, setZoom()
        // will think zoom has not changed and won't update the image
        setZoom(computeZoomToFit(), QPointF(-1, -1), ForceUpdate);
    } else {
        updateBuffer();
    }

    update();

    emit finishLoadDocument();
}

void RasterImageView::updateImageRect(const QRect& imageRect)
{
    QRectF viewRect = mapToView(imageRect);
    if (!viewRect.intersects(boundingRect())) {
        return;
    }

    if (zoomToFit()) {
        setZoom(computeZoomToFit());
    }
    d->setScalerRegionToVisibleRect();
    update();
}

void RasterImageView::updateFromScaler(int zoomedImageLeft, int zoomedImageTop, const QImage& image)
{
    if (d->mApplyDisplayTransform) {
        d->updateDisplayTransform(image.format());
        if (d->mDisplayTransform) {
            quint8 *bytes = const_cast<quint8*>(image.bits());
            cmsDoTransform(d->mDisplayTransform, bytes, bytes, image.width() * image.height());
        }
    }

    d->resizeBuffer();
    int viewportLeft = zoomedImageLeft - scrollPos().x();
    int viewportTop = zoomedImageTop - scrollPos().y();
    d->mBufferIsEmpty = false;
    {
        QPainter painter(&d->mCurrentBuffer);
        if (document()->hasAlphaChannel()) {
            d->drawAlphaBackground(
                &painter, QRect(viewportLeft, viewportTop, image.width(), image.height()),
                QPoint(zoomedImageLeft, zoomedImageTop)
            );
        } else {
            painter.setCompositionMode(QPainter::CompositionMode_Source);
        }
        painter.drawImage(viewportLeft, viewportTop, image);
    }
    update();

    if (!d->mEmittedCompleted) {
        d->mEmittedCompleted = true;
        completed();
    }
}

void RasterImageView::onZoomChanged()
{
    // If we zoom more than twice, then assume the user wants to see the real
    // pixels, for example to fine tune a crop operation
    if (bScalerSmooth)
    {
        if (zoom() < 1.)
            d->mScaler->setTransformationMode(Qt::SmoothTransformation);
    } else {
        d->mScaler->setTransformationMode(Qt::FastTransformation);
    }
    if (!d->mUpdateTimer->isActive()) {
        updateBuffer();
    }
}

void RasterImageView::onImageOffsetChanged()
{
    update();
}

void RasterImageView::onScrollPosChanged(const QPointF& oldPos)
{
    QPointF delta = scrollPos() - oldPos;

    // Scroll existing
    {
        if (d->mAlternateBuffer.size() != d->mCurrentBuffer.size()) {
            d->mAlternateBuffer = QPixmap(d->mCurrentBuffer.size());
        }
        QPainter painter(&d->mAlternateBuffer);
        painter.drawPixmap(-delta, d->mCurrentBuffer);
    }
    qSwap(d->mCurrentBuffer, d->mAlternateBuffer);

    // Scale missing parts
    QRegion bufferRegion = QRegion(d->mCurrentBuffer.rect().translated(scrollPos().toPoint()));
    QRegion updateRegion = bufferRegion - bufferRegion.translated(-delta.toPoint());
    updateBuffer(updateRegion);
    update();
}

void RasterImageView::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    QPointF topLeft = imageOffset();
    if (zoomToFit()) {
        // In zoomToFit mode, scale crudely the buffer to fit the screen. This
        // provide an approximate rendered which will be replaced when the scheduled
        // proper scale is ready.
        QSizeF size = documentSize() * zoom();
        painter->drawPixmap(topLeft.x(), topLeft.y(), size.width(), size.height(), d->mCurrentBuffer);
    } else {
        painter->drawPixmap(topLeft, d->mCurrentBuffer);
    }

//    if (d->mTool) {
//        d->mTool.data()->paint(painter);
//    }

    // draw Center line

#if 1

    qreal w = d->mCurrentBuffer.width();
    qreal h = d->mCurrentBuffer.height();

    const QPointF pt = scrollPos();
    QPointF offset = imageOffset();

    QColor gridColor = Qt::lightGray;
    gridColor.setAlpha(128);
    QPen gridPen(gridColor);
    gridPen.setCosmetic(true);
    painter->setPen(gridPen);

    QSizeF visibleSize = documentSize() * zoom();
    const qreal startX = topLeft.x();
    const qreal startY = topLeft.y();
    qreal x = visibleSize.width() / 2;
    gridPen.setDashOffset(startY);
    painter->setPen(gridPen);
    painter->drawLine(offset.x()+x-pt.x(), offset.y(), offset.x()+x-pt.x(), offset.y()+h);
    qreal y = visibleSize.height() / 2;
    gridPen.setDashOffset(startX);
    painter->setPen(gridPen);
    painter->drawLine(offset.x(), offset.y()+y-pt.y(), offset.x()+w, offset.y()+y-pt.y());
#endif


    // Debug
#if 0
    QSizeF visibleSize = documentSize() * zoom();
    painter->setPen(Qt::red);
    painter->drawRect(topLeft.x(), topLeft.y(), visibleSize.width() - 1, visibleSize.height() - 1);

    painter->setPen(Qt::blue);
    painter->drawRect(topLeft.x(), topLeft.y(), d->mCurrentBuffer.width() - 1, d->mCurrentBuffer.height() - 1);
#endif
}

void RasterImageView::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    // If we are in zoomToFit mode and have something in our buffer, delay the
    // update: paint() will paint a scaled version of the buffer until resizing
    // is done. This is much faster than rescaling the whole image for each
    // resize event we receive.
    // mUpdateTimer must be started before calling AbstractImageView::resizeEvent()
    // because AbstractImageView::resizeEvent() will call onZoomChanged(), which
    // will trigger an immediate update unless the mUpdateTimer is active.
    if (zoomToFit() && !d->mBufferIsEmpty) {
        d->mUpdateTimer->start();
    }
    AbstractImageView::resizeEvent(event);
    if (!zoomToFit()) {
        // Only update buffer if we are not in zoomToFit mode: if we are
        // onZoomChanged() will have already updated the buffer.
        updateBuffer();
    }
}

void RasterImageView::updateBuffer(const QRegion& region)
{
    d->mUpdateTimer->stop();
    d->mScaler->setZoom(zoom());
    if (region.isEmpty()) {
        d->setScalerRegionToVisibleRect();
    } else {
        d->mScaler->setDestinationRegion(region);
    }

    //qDebug() << "RasterImageView::updateBuffer()";
}

void RasterImageView::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    updateBuffer();
    AbstractImageView::mousePressEvent(event);
}

void RasterImageView::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    updateBuffer();
    AbstractImageView::mouseMoveEvent(event);
}

void RasterImageView::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    updateBuffer();
    AbstractImageView::mouseReleaseEvent(event);
}

void RasterImageView::wheelEvent(QGraphicsSceneWheelEvent* event)
{
    updateBuffer();
    AbstractImageView::wheelEvent(event);
}

void RasterImageView::keyPressEvent(QKeyEvent* event)
{
    updateBuffer();
    AbstractImageView::keyPressEvent(event);
}

void RasterImageView::keyReleaseEvent(QKeyEvent* event)
{
    updateBuffer();
    AbstractImageView::keyReleaseEvent(event);
}

void RasterImageView::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    updateBuffer();
    AbstractImageView::hoverMoveEvent(event);
}

} // namespace
