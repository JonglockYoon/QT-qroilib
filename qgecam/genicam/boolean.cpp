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
#include "boolean.h"
#include "boolean_p.h"
#include "iinteger.h"
#include "genicamobjectbuilder.h"
#include "xmlhelper.h"

#include <QVariant>
#include <QSharedPointer>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::Boolean;

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
        qint64 value = prop.toLongLong(&ok, 0);
        if (ok) {
            d->pValue = pValueInteger(value);
        }
        else {
            qWarning("Boolean: %s failed to parse Value %s", qPrintable(featureName()), qPrintable(prop));
        }

    }
    else {
        prop = helper.getProperty(Properties::pValue);
        if (!prop.isEmpty()) {
            d->pValue = pValueInteger(builder.buildInode(prop));
        }
    }

    prop = helper.getProperty(Properties::OnValue);
    if (!prop.isEmpty()) {
        bool ok;
        d->onValue = prop.toLongLong(&ok, 0);
        if (!ok) {
            qWarning("Boolean: %s failed to parse onValue %s", qPrintable(featureName()), qPrintable(prop));
            d->onValue = 1;
        }
    }

    prop = helper.getProperty(Properties::OffValue);
    if (!prop.isEmpty()) {
        bool ok;
        d->offValue = prop.toLongLong(&ok, 0);
        if (!ok) {
            qWarning("Boolean: %s failed to parse offValue %s", qPrintable(featureName()), qPrintable(prop));
            d->offValue = 0;
        }
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
    Q_D(Object);

    return getValue() ? QVariant("True") : QVariant("False");
}

bool Object::getValue()
{
    Q_D(Object);

    return d->pValue.value() != d->offValue;
}

void Object::setValue(bool value)
{
    Q_D(Object);

    d->pValue.set(value ? d->onValue : d->offValue);
}

Boolean::Interface *Object::interface()
{
    return (Boolean::Interface *)this;
}

const Boolean::Interface *Object::constInterface() const
{
    return (const Boolean::Interface *)this;
}






