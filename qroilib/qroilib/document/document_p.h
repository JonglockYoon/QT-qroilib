// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2013 Aurélien Gâteau <agateau@kde.org>

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
#ifndef DOCUMENT_P_H
#define DOCUMENT_P_H

// Local
#include <document/documentjob.h>

// Qt
#include <QUrl>
#include <QImage>
#include <QQueue>
#include <QUndoStack>
#include <QPointer>
#include <QAbstractItemModel>

namespace Qroilib
{

typedef QQueue<DocumentJob*> DocumentJobQueue;
struct DocumentPrivate
{
    Document* q;
    AbstractDocumentImpl* mImpl;
    QUrl mUrl;
    bool mKeepRawData;
    QPointer<DocumentJob> mCurrentJob;
    DocumentJobQueue mJobQueue;

    /**
     * @defgroup imagedata should be reset in reload()
     * @{
     */
    QSize mSize;
    QImage mImage;
    MimeTypeUtils::Kind mKind;
    QByteArray mFormat;
    QUndoStack mUndoStack;
    QString mErrorString;
    Cms::Profile::Ptr mCmsProfile;

};

} // namespace

#endif /* DOCUMENT_P_H */
