// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2008 Aurélien Gâteau <agateau@kde.org>

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
#include "rasterimageviewadapter.h"

// Local
#include <qroilib/document/documentfactory.h>
#include <qroilib/documentview/rasterimageview.h>

// Qt
#include <QUrl>
#include <QCursor>

namespace Qroilib
{

//// RasterImageViewAdapter ////
struct RasterImageViewAdapterPrivate
{
    RasterImageViewAdapter* q;
    RasterImageView* mView;
};

RasterImageViewAdapter::RasterImageViewAdapter(int seq)
: d(new RasterImageViewAdapterPrivate)
{
    mSeq = seq;
    d->q = this;
    d->mView = new RasterImageView(nullptr, seq);

    connect(d->mView, &RasterImageView::zoomChanged, this, &RasterImageViewAdapter::zoomChanged);
    connect(d->mView, &RasterImageView::zoomToFitChanged, this, &RasterImageViewAdapter::zoomToFitChanged);
    connect(d->mView, &RasterImageView::zoomInRequested, this, &RasterImageViewAdapter::zoomInRequested);
    connect(d->mView, &RasterImageView::zoomOutRequested, this, &RasterImageViewAdapter::zoomOutRequested);
    connect(d->mView, &RasterImageView::scrollPosChanged, this, &RasterImageViewAdapter::scrollPosChanged);
    connect(d->mView, &RasterImageView::completed, this, &RasterImageViewAdapter::completed);
    setWidget(d->mView);
}

RasterImageViewAdapter::~RasterImageViewAdapter()
{
    delete d;
}

QCursor RasterImageViewAdapter::cursor() const
{
    return d->mView->cursor();
}

void RasterImageViewAdapter::setCursor(const QCursor& cursor)
{
    d->mView->setCursor(cursor);
}

void RasterImageViewAdapter::setDocument(Document::Ptr doc)
{
    d->mView->setDocument(doc);

//    if (doc->loadingState() == Document::LoadingFailed) {
//        slotLoadingFailed();
//    }
}

qreal RasterImageViewAdapter::zoom() const
{
    return d->mView->zoom();
}

void RasterImageViewAdapter::setZoomToFit(bool on)
{
    d->mView->setZoomToFit(on);
}

bool RasterImageViewAdapter::zoomToFit() const
{
    return d->mView->zoomToFit();
}

void RasterImageViewAdapter::setZoom(qreal zoom, const QPointF& center)
{
    d->mView->setZoom(zoom, center);
}

qreal RasterImageViewAdapter::computeZoomToFit() const
{
    return d->mView->computeZoomToFit();
}

Document::Ptr RasterImageViewAdapter::document() const
{
    return d->mView->document();
}

//void RasterImageViewAdapter::slotLoadingFailed()
//{
//    d->mView->setDocument(Document::Ptr());
//}

void RasterImageViewAdapter::loadConfig()
{
    d->mView->setAlphaBackgroundMode(RasterImageView::AlphaBackgroundCheckBoard);
    d->mView->setAlphaBackgroundColor(QColor( "#ffffff" ));
    d->mView->setEnlargeSmallerImages(true);
}

RasterImageView* RasterImageViewAdapter::rasterImageView() const
{
    return d->mView;
}

QPointF RasterImageViewAdapter::scrollPos() const
{
    return d->mView->scrollPos();
}

void RasterImageViewAdapter::setScrollPos(const QPointF& pos)
{
    d->mView->setScrollPos(pos);
}

QRectF RasterImageViewAdapter::visibleDocumentRect() const
{
    return QRectF(d->mView->imageOffset(), d->mView->visibleImageSize());
}


} // namespace
