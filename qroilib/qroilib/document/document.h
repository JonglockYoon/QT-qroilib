/*
Gwenview: an image viewer
Copyright 2007 Aurélien Gâteau <agateau@kde.org>

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
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "properties.h"

#include <roilib_export.h>

#include <string.h>

// Qt
#include <QObject>
#include <QSharedData>
#include <QSize>

// Local
#include <qroilib/mimetypeutils.h>
#include <qroilib/cms/cmsprofile.h>

class QImage;
class QRect;
class QSize;
class QUndoStack;

class KJob;
class QUrl;

#include "objectgroup.h"

namespace Qroilib
{

class AbstractDocumentImpl;
class DocumentJob;
class DocumentFactory;
struct DocumentPrivate;

class Object;
class RoiMap;
class RoiObject;
class RoiRenderer;
class RoiObjectModel;

/**
 * This class represents an image.
 *
 * It handles loading and saving the image, applying operations and maintaining
 * the document undo stack.
 *
 * It is capable of loading down sampled versions of an image using
 * prepareDownSampledImageForZoom() and downSampledImageForZoom(). Down sampled
 * images load much faster than the full image but you need to load the full
 * image to manipulate it (use startLoadingFullImage() to do so).
 *
 * To get a Document instance for url, ask for one with
 * DocumentFactory::instance()->load(url);
 */
class ROIDSHARED_EXPORT Document : public QObject, public QSharedData
{
    Q_OBJECT
public:
    /**
     * Document won't produce down sampled images for any zoom value higher than maxDownSampledZoom().
     *
     * Note: We can't use the enum {} trick to declare this constant, that's
     * why it's defined as a static method
     */
    static qreal maxDownSampledZoom();

    enum LoadingState {
        Loading,        ///< Image is loading
        KindDetermined, ///< Image is still loading, but kind has been determined
        MetaInfoLoaded, ///< Image is still loading, but meta info has been loaded
        Loaded,         ///< Full image has been loaded
        LoadingFailed   ///< Image loading has failed
    };

    typedef QExplicitlySharedDataPointer<Document> Ptr;
    ~Document();

    /**
     * Returns a message for the last error which happened
     */
    QString errorString() const;

    void reload();

    LoadingState loadingState() const;

    MimeTypeUtils::Kind kind() const;

    bool isModified() const;

    const QImage& image() const;

    QUrl url() const;

    QByteArray format() const;

    QSize size() const;

    int width() const
    {
        return size().width();
    }

    int height() const
    {
        return size().height();
    }

    bool hasAlphaChannel() const;

    QUndoStack* undoStack() const;

    void setKeepRawData(bool);

    bool keepRawData() const;

    /**
     * Returns how much bytes the document is using
     */
    int memoryUsage() const;

    /**
     * Returns the compressed version of the document, if it is still
     * available.
     */
    QByteArray rawData() const;

    Cms::Profile::Ptr cmsProfile() const;

    /**
     * Returns true if there are queued tasks for this document.
     */
    bool isBusy() const;

Q_SIGNALS:
    void imageRectUpdated(const QRect&);
    void kindDetermined(const QUrl&);
    void metaInfoLoaded(const QUrl&);
    void loaded(const QUrl&);
    void metaInfoUpdated();
    void allTasksDone();

private Q_SLOTS:
    void emitMetaInfoLoaded();
    void emitLoaded();

private:
    friend class AbstractDocumentImpl;
    friend class DocumentFactory;
    friend struct DocumentPrivate;

    void setKind(MimeTypeUtils::Kind);
    void setFormat(const QByteArray&);
    void setErrorString(const QString&);
    void setCmsProfile(Cms::Profile::Ptr);

    Document(const QUrl&);
    DocumentPrivate * const d;
public:
    void setSize(const QSize&);
    void setImageInternal(const QImage&);

};


} // namespace

#endif /* DOCUMENT_H */
