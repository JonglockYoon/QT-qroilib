/*
 * orthogonalrenderer.cpp
 * Copyright 2009-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *Qroilib : QT ROI Lib
 * Copyright 2018 Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
 *
 * This file is part of qroilib.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "orthogonalrenderer.h"

#include "roimap.h"
#include "roiobject.h"
#include "qpatternroif.h"

#include <QtCore/qmath.h>
#include <QDebug>

#include "roiscene.h"
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>

using namespace Qroilib;

QSize OrthogonalRenderer::mapSize() const
{
    DocumentView *v = mapDocument();
    const qreal scale = v->zoom();
    return QSize(roimap()->width() * roimap()->roiWidth() * scale,
                 roimap()->height() * roimap()->roiHeight() * scale);
}

QRect OrthogonalRenderer::boundingRect(const QRect &rect) const
{
    //return QRect();

    DocumentView *v = mapDocument();
    RasterImageView *pView = v->imageView();
    QPointF scroll = pView->scrollPos();
    const qreal scale = v->zoom();
//    const int roiWidth = roimap()->roiWidth();
//    const int roiHeight = roimap()->roiHeight();

    QRect orect = QRect(QPoint(rect.x()*scale - scroll.x(), rect.y()*scale - scroll.y()),
           QSize(rect.size())*scale);

//    return QRect(rect.x() * roiWidth*scale,
//                 rect.y() * roiHeight*scale,
//                 rect.width() * roiWidth*scale,
//                 rect.height() * roiHeight*scale);
    return  orect;
}

QRectF OrthogonalRenderer::boundingRect(const RoiObject *object) const
{
    //return QRect();
    const QRectF bounds = object->bounds();

    QRectF boundingRect;
    DocumentView *v = mapDocument();
    const qreal scale = v->zoom();

    {
        qreal extraSpace = qMax(objectLineWidth(), qreal(1));

        switch (object->shape()) {
        case RoiObject::Ellipse:
        case RoiObject::Rectangle:
        case RoiObject::Pattern:
        case RoiObject::Point:
            if (bounds.isNull()) {
                boundingRect = bounds.adjusted(-10 - extraSpace,
                                               -10 - extraSpace,
                                               10 + extraSpace + 1,
                                               10 + extraSpace + 1);
            } else {
                boundingRect = bounds.adjusted(-extraSpace,
                                               -extraSpace,
                                               extraSpace + 1,
                                               extraSpace + 1);


            }

            break;

        case RoiObject::Polygon:
        case RoiObject::Polyline: {
            // Make some more room for the starting dot
            extraSpace += objectLineWidth() * 4;

            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon();//.translated(pos);
            QPolygonF screenPolygon = pixelToScreenCoords(polygon);
            boundingRect = screenPolygon.boundingRect().adjusted(-extraSpace,
                                                                 -extraSpace,
                                                                 extraSpace + 1,
                                                                 extraSpace + 1);
            break;
        }

        case RoiObject::Text:
            boundingRect = object->bounds();
            break;
        }
    }

    RasterImageView *pView = v->imageView();
    //QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();

    QRectF rect = QRectF(QPointF(boundingRect.x()*scale - scroll.x(), boundingRect.y()*scale - scroll.y()),
           QSizeF(boundingRect.size())*scale);
    //qDebug() << "OrthogonalRenderer" << rect << scroll;
    return  rect;
}

void OrthogonalRenderer::drawRoiObject(QPainter *painter,
                                       const RoiObject *object,
                                       const QColor &color) const
{
    painter->save();

    const QRectF bounds = object->bounds();
    //qDebug() << "bounds" << bounds;
    QRectF rect(bounds);
    //qDebug() << "rectIn: " << rect;

    //painter->translate(rect.topLeft());
    //rect.moveTopLeft(QPointF(0, 0));

    //RoiObject *mObject = (RoiObject *)object;

    DocumentView *v = mapDocument();

    const qreal lineWidth = objectLineWidth();
    //const qreal scale = painterScale();
    const qreal scale = v->zoom();
    const qreal shadowDist = (lineWidth == 0 ? 1 : lineWidth) / scale;
    const QPointF shadowOffset = QPointF(shadowDist * 0.5,
                                         shadowDist * 0.5);

    QPen linePen(color, lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QPen dotPen(color, lineWidth, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
    linePen.setCosmetic(true);
    dotPen.setCosmetic(true);
    QPen shadowPen(linePen);
    shadowPen.setColor(Qt::black);

    QColor brushColor = color;
    brushColor.setAlpha(50);
    const QBrush fillBrush(brushColor);

    painter->setRenderHint(QPainter::Antialiasing);

    // Trying to draw an ellipse with 0-width is causing a hang in
    // CoreGraphics when drawing the path requested by the
    // QCoreGraphicsPaintEngine. Draw them as rectangle instead.
    RoiObject::Shape shape = object->shape();
    if (shape == RoiObject::Ellipse &&
            ((rect.width() == qreal(0)) ^ (rect.height() == qreal(0)))) {
        shape = RoiObject::Rectangle;
    }

    RasterImageView *pView = v->imageView();
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();

    switch (shape) {
        case RoiObject::Pattern: {
            if (rect.isNull()) {
                QPointF lpt = v->mRoiScene->getLastMousePos();
                rect = QRectF(QPointF(lpt.x()-10, lpt.y()-10), QSizeF(20, 20));
            } else {
                rect.setLeft(rect.left() * scale - scroll.x() + offset.x());
                rect.setRight(rect.right() * scale - scroll.x() + offset.x());
                rect.setTop(rect.top() * scale - scroll.y() + offset.y());
                rect.setBottom(rect.bottom() * scale - scroll.y() + offset.y());
            }

            painter->setPen(dotPen);
            painter->setBrush(fillBrush);
            painter->drawRect(rect);

            QRectF recti;
            painter->setPen(linePen);
            if (object->mPattern == nullptr) {
                float fw = rect.width() * 0.4;
                float fh = rect.height() * 0.4;
                recti.setLeft(rect.left()+fw);
                recti.setRight(rect.right()-fw);
                recti.setTop(rect.top()+fh);
                recti.setBottom(rect.bottom()-fh);
                painter->drawRect(recti);
            }

            break;
        }

        case RoiObject::Rectangle: {
            if (rect.isNull()) {
                QPointF lpt = v->mRoiScene->getLastMousePos();
                rect = QRectF(QPointF(lpt.x()-10, lpt.y()-10), QSizeF(20, 20));
            } else {
                rect.setLeft(rect.left()*scale - scroll.x() + offset.x());
                rect.setRight(rect.right()*scale - scroll.x() + offset.x());
                rect.setTop(rect.top()*scale - scroll.y() + offset.y());
                rect.setBottom(rect.bottom()*scale - scroll.y() + offset.y());
            }

            // Draw the shadow
            painter->setPen(shadowPen);
            painter->drawRect(rect.translated(shadowOffset));

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawRect(rect);

            break;
        }

        case RoiObject::Point: {
            if (rect.isNull()) {
                QPointF lpt = v->mRoiScene->getLastMousePos();
                rect = QRectF(QPointF(lpt.x()-10, lpt.y()-10), QSizeF(20, 20));
            } else {
                rect.setLeft(rect.left()*scale - scroll.x() + offset.x());
                rect.setRight(rect.right()*scale - scroll.x() + offset.x());
                rect.setTop(rect.top()*scale - scroll.y() + offset.y());
                rect.setBottom(rect.bottom()*scale - scroll.y() + offset.y());
            }


            int w = rect.width() / 2;
            int h = rect.height() / 2;
            QLine lines[] = {
                QLine(rect.left(),rect.top()+h, rect.right(),rect.top()+h),
                QLine(rect.left()+w, rect.top(), rect.left()+w,rect.bottom()),
            };

            painter->setPen(linePen);
            //painter->setPen(shadowPen);
            painter->drawLines(lines, sizeof(lines) / sizeof(lines[0]));

            break;
        }

        case RoiObject::Polyline: {
            QPolygonF screenPolygon = pixelToScreenCoords(object->polygon());
            int size = screenPolygon.size();
            for (int i=0; i<size; i++) {
                QPointF* p = &screenPolygon[i];
                p->setX(p->x() * scale - scroll.x() + offset.x());
                p->setY(p->y() * scale - scroll.y() + offset.y());
            }

            QPen thickShadowPen(shadowPen);
            QPen thickLinePen(linePen);
            thickShadowPen.setWidthF(thickShadowPen.widthF() * 4);
            thickLinePen.setWidthF(thickLinePen.widthF() * 4);

            painter->setPen(shadowPen);
            painter->drawPolyline(screenPolygon.translated(shadowOffset));
            painter->setPen(thickShadowPen);
            painter->drawPoint(screenPolygon.first() + shadowOffset);

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawPolyline(screenPolygon);
            painter->setPen(thickLinePen);
            painter->drawPoint(screenPolygon.first());
            break;
        }

        case RoiObject::Polygon: {
            QPolygonF screenPolygon = pixelToScreenCoords(object->polygon());
            int size = screenPolygon.size();
            for (int i=0; i<size; i++) {
                QPointF* p = &screenPolygon[i];
                p->setX(p->x() * scale - scroll.x() + offset.x());
                p->setY(p->y() * scale - scroll.y() + offset.y());
            }

            QPen thickShadowPen(shadowPen);
            QPen thickLinePen(linePen);
            thickShadowPen.setWidthF(thickShadowPen.widthF() * 4);
            thickLinePen.setWidthF(thickLinePen.widthF() * 4);

            painter->setPen(shadowPen);
            painter->drawPolygon(screenPolygon.translated(shadowOffset));
            painter->setPen(thickShadowPen);
            painter->drawPoint(screenPolygon.first() + shadowOffset);

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawPolygon(screenPolygon);
            painter->setPen(thickLinePen);
            painter->drawPoint(screenPolygon.first());
            break;
        }

        case RoiObject::Ellipse: {
            if (rect.isNull()) {
                QPointF lpt = v->mRoiScene->getLastMousePos();
                rect = QRectF(QPointF(lpt.x()-10, lpt.y()-10), QSizeF(20, 20));
            } else {
                rect.setLeft(rect.left() * scale - scroll.x() + offset.x());
                rect.setRight(rect.right() * scale - scroll.x() + offset.x());
                rect.setTop(rect.top() * scale - scroll.y() + offset.y());
                rect.setBottom(rect.bottom() * scale - scroll.y() + offset.y());
            }
            // Draw the shadow
            painter->setPen(shadowPen);
            painter->drawEllipse(rect.translated(shadowOffset));

            painter->setPen(linePen);
            painter->setBrush(fillBrush);
            painter->drawEllipse(rect);
            break;
        }

        case RoiObject::Text: {


            rect.setLeft(rect.left() * scale - scroll.x() + offset.x());
            rect.setRight(rect.right() * scale - scroll.x() + offset.x());
            rect.setTop(rect.top() * scale - scroll.y() + offset.y());
            rect.setBottom(rect.bottom() * scale - scroll.y() + offset.y());

            const auto& textData = object->textData();
            painter->setFont(textData.font);
            painter->setPen(textData.color);
            painter->drawText(rect, textData.text, textData.textOption());
            break;
        }
    }

    painter->restore();
}

QPointF OrthogonalRenderer::pixelToRoiCoords(qreal x, qreal y) const
{
    return QPointF(x / roimap()->roiWidth(),
                   y / roimap()->roiHeight());
}

QPointF OrthogonalRenderer::roiToPixelCoords(qreal x, qreal y) const
{
    return QPointF(x * roimap()->roiWidth(),
                   y * roimap()->roiHeight());
}

QPointF OrthogonalRenderer::screenToRoiCoords(qreal x, qreal y) const
{
    return QPointF(x / roimap()->roiWidth(),
                   y / roimap()->roiHeight());
}

QPointF OrthogonalRenderer::roiToScreenCoords(qreal x, qreal y) const
{
    return QPointF(x * roimap()->roiWidth(),
                   y * roimap()->roiHeight());
}

QPointF OrthogonalRenderer::screenToPixelCoords(qreal x, qreal y) const
{
    return QPointF(x, y);
}

QPointF OrthogonalRenderer::pixelToScreenCoords(qreal x, qreal y) const
{
    return QPointF(x, y);
}
