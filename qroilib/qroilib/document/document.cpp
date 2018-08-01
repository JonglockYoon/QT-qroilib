/*
Gwenview: an image viewer
Copyright 2007 Aurélien Gâteau <agateau@kde.org>
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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#include "document.h"
#include "document_p.h"

// Qt
#include <QApplication>
#include <QImage>
#include <QUndoStack>
#include <QUrl>
#include <QDebug>

// Local
#include "documentjob.h"
#include <qroilib/document/abstractdocumentimpl.h>
#include "gvdebug.h"

namespace Qroilib
{

#undef ENABLE_LOG
#undef LOG
#define LOG(x) ;
#define LOG_QUEUE(msg, d)

Document::Document(const QUrl &url)
: QObject()
, d(new DocumentPrivate)
{
    d->q = this;
    d->mImpl = 0;
    d->mUrl = url;
    d->mKeepRawData = false;


    reload();

}

Document::~Document()
{
    // We do not want undo stack to emit signals, forcing us to emit signals
    // ourself while we are being destroyed.
    disconnect(&d->mUndoStack, 0, this, 0);

    if (d->mImpl != NULL)
        delete d->mImpl;
    delete d;
}

void Document::reload()
{
    d->mSize = QSize();
    d->mImage = QImage();
    d->mKind = MimeTypeUtils::KIND_UNKNOWN;
    d->mFormat = QByteArray();
    d->mUndoStack.clear();
    d->mErrorString.clear();
    d->mCmsProfile = 0;

    MimeTypeUtils::Kind kind = MimeTypeUtils::KIND_RASTER_IMAGE;
    setKind(kind);
    emitLoaded();

}

const QImage& Document::image() const
{
    return d->mImage;
}

/**
 * invertedZoom is the biggest power of 2 for which zoom < 1/invertedZoom.
 * Example:
 * zoom = 0.4 == 1/2.5 => invertedZoom = 2 (1/2.5 < 1/2)
 * zoom = 0.2 == 1/5   => invertedZoom = 4 (1/5   < 1/4)
 */
inline int invertedZoomForZoom(qreal zoom)
{
    int invertedZoom;
    for (invertedZoom = 1; zoom < 1. / (invertedZoom * 4); invertedZoom *= 2) {}
    return invertedZoom;
}

Document::LoadingState Document::loadingState() const
{
    if (d->mImpl == NULL)
        return Document::Loaded;;
    return d->mImpl->loadingState();
}

void Document::setImageInternal(const QImage& image)
{
    d->mImage = image;
    //d->mDownSampledImageMap.clear();

    // If we didn't get the image size before decoding the full image, set it
    // now
    setSize(d->mImage.size());
}

QUrl Document::url() const
{
    return d->mUrl;
}

QByteArray Document::rawData() const
{
    return d->mImpl->rawData();
}

bool Document::keepRawData() const
{
    return d->mKeepRawData;
}

void Document::setKeepRawData(bool value)
{
    d->mKeepRawData = value;
}

QByteArray Document::format() const
{
    return d->mFormat;
}

void Document::setFormat(const QByteArray& format)
{
    d->mFormat = format;
    emit metaInfoUpdated();
}

MimeTypeUtils::Kind Document::kind() const
{
    return d->mKind;
}

void Document::setKind(MimeTypeUtils::Kind kind)
{
    d->mKind = kind;
    emit kindDetermined(d->mUrl);
}

QSize Document::size() const
{
    return d->mSize;
}

bool Document::hasAlphaChannel() const
{
    if (d->mImage.isNull()) {
        return false;
    } else {
        return d->mImage.hasAlphaChannel();
    }
}

int Document::memoryUsage() const
{
    // FIXME: Take undo stack into account
    int usage = d->mImage.byteCount();
    usage += rawData().length();
    return usage;
}

void Document::setSize(const QSize& size)
{
    if (size == d->mSize) {
        return;
    }
    d->mSize = size;
    emit metaInfoUpdated();
}

bool Document::isModified() const
{
    return !d->mUndoStack.isClean();
}

QString Document::errorString() const
{
    return d->mErrorString;
}

void Document::setErrorString(const QString& string)
{
    d->mErrorString = string;
}

void Document::emitMetaInfoLoaded()
{
    emit metaInfoLoaded(d->mUrl);
}

void Document::emitLoaded()
{
    emit loaded(d->mUrl);
}

QUndoStack* Document::undoStack() const
{
    return &d->mUndoStack;
}

bool Document::isBusy() const
{
    return !d->mJobQueue.isEmpty();
}

void Document::setCmsProfile(Cms::Profile::Ptr ptr)
{
    d->mCmsProfile = ptr;
}

Cms::Profile::Ptr Document::cmsProfile() const
{
    return d->mCmsProfile;
}

} // namespace
