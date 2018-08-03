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

#ifndef XMLHELPER_H
#define XMLHELPER_H


#include <QString>
#include <QMap>
#include <QDomElement>

//class QDomNodeList;
class QStringList;
//class QDomNode;

namespace Jgv {

namespace GenICam {

namespace Node {
enum class Type {
    Inode,
    Category,
    StringReg,
    Integer,
    IntReg,
    IntConverter,
    IntSwissknife,
    Enumeration,
    EnumEntry,
    MaskedIntReg,
    Float,
    FloatReg,
    Converter,
    Swissknife,
    Command,
    Boolean,
    Register,
    StructEntry,
    None
};
}

class XMLHelper;
using ChildrenHelpers = QList<XMLHelper>;

class XMLHelper
{
    QDomElement m_node;

public:
    XMLHelper(const QDomNode &m_node);
    ~XMLHelper() = default;

    QString typeString() const;
    Node::Type getType() const;

    QString value() const;

    QString getAttribute(const QString &attributeName) const;

    QString getProperty(const QString &propertyName) const;
    QString nameAttribut() const;
    QString namespaceAttribut() const;

    QStringList getProperties(const QString &propertyName) const;
    QMap<QString, QString> getAttributeValueMap(const QString &nodeName, const QString &attributeName) const;

    XMLHelper getChildHelper(const QString &childName) const;
    ChildrenHelpers getChildrenHelpers(const QString &childName) const;
    XMLHelper getParentHelper() const;


}; // XMLHelper

} // namespace GenICam

} // namespace Jgv

#endif // XMLHELPER_H
