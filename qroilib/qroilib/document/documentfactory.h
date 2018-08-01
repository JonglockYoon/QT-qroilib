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
#ifndef DOCUMENTFACTORY_H
#define DOCUMENTFACTORY_H

// Qt
#include <QObject>

#include <qroilib/document/document.h>

class QUndoGroup;

class QUrl;

namespace Qroilib
{

struct DocumentFactoryPrivate;

/**
 * This class holds all instances of Document.
 *
 * It keeps a cache of recently accessed documents to avoid reloading them.
 * To do so it keeps a last-access timestamp, which is updated to the
 * current time every time DocumentFactory::load() is called.
 */
class ROIDSHARED_EXPORT DocumentFactory : public QObject
{
    Q_OBJECT
public:
    static DocumentFactory* instance();
    ~DocumentFactory();

    /**
     * Loads the document associated with url, or returns an already cached
     * instance of Document::Ptr if there is any.
     * This method updates the last-access timestamp.
     */
    Document::Ptr load(const QUrl &url);

    /**
     * Returns a document if it has already been loaded once with load().
     * This method does not update the last-access timestamp.
     */

    //QList<QUrl> modifiedDocumentList() const;




    /**
     * Do not keep document whose url is @url in cache even if it has been
     * modified
     */

Q_SIGNALS:

private Q_SLOTS:


private:
    DocumentFactory();

    DocumentFactoryPrivate* const d;
};

} // namespace
#endif /* DOCUMENTFACTORY_H */
