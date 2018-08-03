/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD                                  *
 *   cyril.baletaud@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef GENICAMXMLFILE_H
#define GENICAMXMLFILE_H

#include <QDomNode>
#include <QScopedPointer>
//class QDomNode;
class QStringList;
class QByteArray;
class QString;

namespace Jgv {

namespace GenICam {

class GenICamXMLFilePrivate;
class GenICamXMLFile
{
public:
    GenICamXMLFile();
    ~GenICamXMLFile();

    void setFileName(const QString &fileName);
    void setContent(const QByteArray &content);
    void setZippedContent(const QByteArray &zippedContent);
    void saveContent(const QString &path);
    QDomNode getRootChildNodeByNameAttribute(const QString &name) const;

private:
    QScopedPointer<GenICamXMLFilePrivate> d;

}; // class GenICamXMLFile

} // namespace GenICam

} // namespace Jgv

#endif // GENICAMXMLFILE_H
