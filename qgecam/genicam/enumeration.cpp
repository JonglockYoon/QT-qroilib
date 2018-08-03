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
#include "enumeration.h"
#include "enumeration_p.h"
#include "enumentry.h"
#include "iinteger.h"
#include "genicamobjectbuilder.h"
#include "xmlhelper.h"

#include <QVariant>
#include <QStringList>
#include <QDomNodeList>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::Enumeration;

Object::Object(QSharedPointer<IPort::Interface> iport)
    : Inode::Object(*new ObjectPrivate(iport))
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    Inode::Object::prepare(builder, helper);

    d->sType = sType;

    QString prop = helper.getProperty(Properties::Value);
    if (!prop.isEmpty()) {
        bool ok;
        qint64 val = prop.toLongLong(&ok, 0);
        if (ok) {
            d->pValue = pValueInteger(val);
        }
        else {
            qWarning("Enumeration: %s failed to parse Value %s", qPrintable(featureName()), qPrintable(prop));
        }
    }
    else {
        prop = helper.getProperty(Properties::pValue);
        if (!prop.isEmpty()) {
            d->pValue = pValueInteger(builder.buildInode(prop));
        }
    }

    ChildrenHelpers entryList = helper.getChildrenHelpers(Enumentry::sType);
    auto it = entryList.constBegin();
    for (; it != entryList.constEnd(); ++it) {
        QSharedPointer<Enumentry::Object> entry(new Enumentry::Object(d->iport));
        d->entries.append(entry);
        entry->prepare(builder, *it);
    }
}

void Object::invalidate()
{
    Q_D(Object);

    QSharedPointer<Inode::Object> p = d->pValue.toStrongRef();
    if (p) {
        p->invalidate();
    }
}

bool Object::isWritable() const
{
    Q_D(const Object);

    if (!d->imposedCanWrite) {
        return false;
    }

    QSharedPointer<Inode::Object> p = d->pValue.toStrongRef();
    return (p) ? p->isWritable() : true;
}

QVariant Object::getVariant()
{
    return QVariant(getStringValue());
}

QString Object::getStringValue()
{
    Q_D(const Object);

    // obtient l'index courant
    const qint64 index = getIntValue();
    // obtient le texte lié à l'index
    auto it = d->entries.constBegin();
    for (; it != d->entries.constEnd(); ++it) {
        if ((*it)->getIntValue() == index) {
            return (*it)->featureName();
        }
    }

    return QString();
}

void Object::setStringValue(const QString &value)
{
    Q_UNUSED(value);
    qDebug("TODO Interface::setStringValue");
}


qint64 Object::getIntValue() const
{
    Q_D(const Object);

    return d->pValue.value();
}

void Object::setIntValue(qint64 value)
{
    Q_D(Object);

    d->pValue.set(value);

    auto it = d->inodesToInvalidate.constBegin();
    for (; it != d->inodesToInvalidate.constEnd(); ++it) {
        QSharedPointer<Inode::Object> p = (*it).toStrongRef();
        if (p) {
            p->invalidate();
        }
    }
}
/*!
 * \brief Interface::getEntries
 * Obtient les liste des entrées disponibles
 * \return La liste des entrées valides
 */
QList<Jgv::GenICam::Enumentry::Object *> Object::getEntries() const
{
    Q_D(const Object);

    QList<Enumentry::Object *> entries;
    auto it = d->entries.constBegin();
    for (; it != d->entries.constEnd(); ++it) {
        if ((*it)->isImplemented() && (*it)->isAvailable()) {
            entries.append((*it).data());
        }
    }

    return entries;
}

Enumeration::Interface *Object::interface()
{
    return this;
}

const Enumeration::Interface *Object::constInterface() const
{
    return this;
}







