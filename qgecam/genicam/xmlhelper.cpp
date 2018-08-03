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

#include "xmlhelper.h"

#include "inode_p.h"


#include "category.h"
#include "stringreg.h"
#include "integer.h"
#include "intreg.h"
#include "intconverter.h"
#include "intswissknife.h"
#include "enumeration.h"
#include "enumentry.h"
#include "maskedintreg.h"
#include "floatjgv.h"
#include "floatreg.h"
#include "converter.h"
#include "swissknife.h"
#include "command.h"
#include "boolean.h"
#include "register.h"
#include "structentry.h"

#include <QObject>
#include <QStringList>

using namespace Jgv::GenICam;



XMLHelper::XMLHelper(const QDomNode &node)
    : m_node(node.toElement())
{}

QString XMLHelper::typeString() const
{
    return m_node.text();
}

Node::Type XMLHelper::getType() const
{
    Node::Type value = Node::Type::None;
    const QString type = m_node.nodeName();

    if (type == Category::sType) value = Node::Type::Category;
    else if (type == StringReg::sType) value = Node::Type::StringReg;
    else if (type == Integer::sType) value = Node::Type::Integer;
    else if (type == IntReg::sType) value = Node::Type::IntReg;
    else if (type == IntConverter::sType) value = Node::Type::IntConverter;
    else if (type == IntSwissKnife::sType) value = Node::Type::IntSwissknife;
    else if (type == Enumeration::sType) value = Node::Type::Enumeration;
    else if (type == Enumentry::sType) value = Node::Type::EnumEntry;
    else if (type == MaskedIntReg::sType) value = Node::Type::MaskedIntReg;
    else if (type == Float::sType) value = Node::Type::Float;
    else if (type == FloatReg::sType) value = Node::Type::FloatReg;
    else if (type == Converter::sType) value = Node::Type::Converter;
    else if (type == SwissKnife::sType) value = Node::Type::Swissknife;
    else if (type == Command::sType) value = Node::Type::Command;
    else if (type == Boolean::sType) value = Node::Type::Boolean;
    else if (type == Register::sType) value = Node::Type::Register;
    else if (type == StructEntry::sType) value = Node::Type::StructEntry;
    return value;
}

QString XMLHelper::value() const
{
    return m_node.text();
}

QString XMLHelper::getAttribute(const QString &attributeName) const
{
    return m_node.attribute(attributeName);
}

QString XMLHelper::getProperty(const QString &propertyName) const
{
    return m_node.firstChildElement(propertyName).text();
}

QString XMLHelper::nameAttribut() const
{
    return getAttribute(Inode::Attributes::Name);
}

QString XMLHelper::namespaceAttribut() const
{
    return getAttribute(Inode::Attributes::NameSpace);
}

QStringList XMLHelper::getProperties(const QString &propertyName) const
{
    QStringList properties;
    QDomNode n = m_node.firstChildElement(propertyName);
    while (!n.isNull()) {
        properties.append(n.toElement().text());
        n = n.nextSiblingElement(propertyName);
    }
    return properties;
}

QMap<QString, QString> XMLHelper::getAttributeValueMap(const QString &nodeName, const QString &attributeName) const
{
    QMap<QString, QString> values;
    QDomNodeList list = m_node.elementsByTagName(nodeName);
    for (int i=0; i<list.size(); ++i) {
        QDomElement element = list.at(i).toElement();
        values.insert(element.attribute(attributeName), element.text());
    }
    return values;
}

XMLHelper XMLHelper::getChildHelper(const QString &childName) const
{
    return XMLHelper(m_node.firstChildElement(childName));
}

ChildrenHelpers XMLHelper::getChildrenHelpers(const QString &childName) const
{
    ChildrenHelpers helpers;
    QDomNodeList list = m_node.elementsByTagName(childName);
    for (int i=0; i<list.size(); ++i) {
        helpers.append(XMLHelper(list.at(i)));
    }
    return helpers;
}

XMLHelper XMLHelper::getParentHelper() const
{
    return XMLHelper(m_node.parentNode());
}








