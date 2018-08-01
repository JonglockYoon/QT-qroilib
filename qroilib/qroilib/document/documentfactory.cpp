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
#include "documentfactory.h"

// Qt
#include <QByteArray>
#include <QDateTime>
#include <QMap>
#include <QUndoGroup>
#include <QUrl>
#include <QDebug>

// Local
#include <gvdebug.h>

namespace Qroilib
{

#undef ENABLE_LOG
#undef LOG
//#define ENABLE_LOG
#ifdef ENABLE_LOG
#define LOG(x) ;//qDebug() << x
#else
#define LOG(x) ;
#endif

inline int getMaxUnreferencedImages()
{
    int defaultValue = 3;
    QByteArray ba = qgetenv("GV_MAX_UNREFERENCED_IMAGES");
    if (ba.isEmpty()) {
        return defaultValue;
    }
    LOG("Custom value for max unreferenced images:" << ba);
    bool ok;
    int value = ba.toInt(&ok);
    return ok ? value : defaultValue;
}

static const int MAX_UNREFERENCED_IMAGES = getMaxUnreferencedImages();

/**
 * This internal structure holds the document and the last time it has been
 * accessed. This access time is used to "garbage collect" the loaded
 * documents.
 */
struct DocumentInfo
{
    Document::Ptr mDocument;
    QDateTime mLastAccess;
};

/**
 * Our collection of DocumentInfo instances. We keep them as pointers to avoid
 * altering DocumentInfo::mDocument refcount, since we rely on it to garbage
 * collect documents.
 */
typedef QMap<QUrl, DocumentInfo*> DocumentMap;

struct DocumentFactoryPrivate
{
    DocumentMap mDocumentMap;
//    QUndoGroup mUndoGroup;

//    /**
//     * Removes items in a roimap if they are no longer referenced elsewhere
//     */
//    void garbageCollect(DocumentMap& roimap)
//    {
//        // Build a roimap of all unreferenced images. We use a MultiMap because in
//        // rare cases documents may get accessed at the same millisecond.
//        // See https://bugs.kde.org/show_bug.cgi?id=296401
//        typedef QMultiMap<QDateTime, QUrl> UnreferencedImages;
//        UnreferencedImages unreferencedImages;

//        DocumentMap::Iterator it = roimap.begin(), end = roimap.end();
//        for (; it != end; ++it) {
//            DocumentInfo* info = it.value();
//            if (info->mDocument->ref == 1 && !info->mDocument->isModified()) {
//                unreferencedImages.insert(info->mLastAccess, it.key());
//            }
//        }

//        // Remove oldest unreferenced images. Since the roimap is sorted by key,
//        // the oldest one is always unreferencedImages.begin().
//        for (
//            UnreferencedImages::Iterator unreferencedIt = unreferencedImages.begin();
//            unreferencedImages.count() > MAX_UNREFERENCED_IMAGES;
//            unreferencedIt = unreferencedImages.erase(unreferencedIt))
//        {
//            QUrl url = unreferencedIt.value();
//            LOG("Collecting" << url);
//            it = roimap.find(url);
//            Q_ASSERT(it != roimap.end());
//            delete it.value();
//            roimap.erase(it);
//        }

//#ifdef ENABLE_LOG
//        logDocumentMap(roimap);
//#endif
//    }

//    void logDocumentMap(const DocumentMap& roimap)
//    {
//        LOG("roimap:");
//        DocumentMap::ConstIterator
//        it = roimap.constBegin(),
//        end = roimap.constEnd();
//        for (; it != end; ++it) {
//            LOG("-" << it.key()
//                << "refCount=" << it.value()->mDocument.count()
//                << "lastAccess=" << it.value()->mLastAccess);
//        }
//    }

//    QList<QUrl> mModifiedDocumentList;
};

DocumentFactory::DocumentFactory()
: d(new DocumentFactoryPrivate)
{
}

DocumentFactory::~DocumentFactory()
{
    qDeleteAll(d->mDocumentMap);
    delete d;
}

DocumentFactory* DocumentFactory::instance()
{
    static DocumentFactory factory;
    return &factory;
}


Document::Ptr DocumentFactory::load(const QUrl &url)
{
    GV_RETURN_VALUE_IF_FAIL(!url.isEmpty(), Document::Ptr());
    DocumentInfo* info = 0;

    DocumentMap::Iterator it = d->mDocumentMap.find(url);

    if (it != d->mDocumentMap.end()) {
        LOG(url.fileName() << "url in mDocumentMap");
        info = it.value();
        info->mLastAccess = QDateTime::currentDateTime();
        return info->mDocument;
    }

    // At this point we couldn't find the document in the roimap

    // Start loading the document
    LOG(url.fileName() << "loading");
    Document* doc = new Document(url);

    // Create DocumentInfo instance
    info = new DocumentInfo;
    Document::Ptr docPtr(doc);
    info->mDocument = docPtr;
    info->mLastAccess = QDateTime::currentDateTime();

    // Place DocumentInfo in the roimap
    d->mDocumentMap[url] = info;

    //d->garbageCollect(d->mDocumentMap);

    return docPtr;
}

//QList<QUrl> DocumentFactory::modifiedDocumentList() const
//{
//    return d->mModifiedDocumentList;
//}











} // namespace
