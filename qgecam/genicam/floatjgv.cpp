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
#include "floatjgv.h"
#include "floatjgv_p.h"
#include "genicam.h"

#include <QVariant>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::Float;

Object::Object(QSharedPointer<IPort::Interface> iport)
    : InodeValue::Object<double>(*new ObjectPrivate(iport))
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    InodeValue::Object<double>::prepare(builder, helper);
    d->sType = sType;
}

void Object::invalidate()
{
    Q_D(Object);

    QSharedPointer<Inode::Object> p = d->pVal.toStrongRef();
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

    QSharedPointer<Inode::Object> p = d->pVal.toStrongRef();
    return (p) ? p->isWritable() : true;
}

QVariant Object::getVariant()
{
    Q_D(const Object);
    if (!d->namespaceIsCustom) {
        return GenICam::Representation::toString(featureName(), getValue());
    }

    return QVariant(getValue());
}

int Object::getPollingTime() const
{
    Q_D(const Object);

    QSharedPointer<Inode::Object> p = d->pVal.toStrongRef();
    return (p) ? p->getPollingTime() : -1;
}

double Object::getValue()
{
    Q_D(Object);

    return (d->*(d->evalValue))();
}

void Object::setValue(double value)
{
    Q_D(Object);

    (d->* (d->setValue)) (value);

    auto it = d->inodesToInvalidate.constBegin();
    for (; it != d->inodesToInvalidate.constEnd(); ++it) {
        QSharedPointer<Inode::Object> p = (*it).toStrongRef();
        if (p) {
            p->invalidate();
        }
    }

}

double Object::getMin() const
{
    Q_D(const Object);

    return (d->*(d->evalMin))();
}

double Object::getMax() const
{
    Q_D(const Object);

    return (d->*(d->evalMax))();
}

double Object::getInc() const
{
    Q_D(const Object);

    return (d->*(d->evalInc))();
}

Float::Interface *Object::interface()
{
    return this;
}

const Float::Interface *Object::constInterface() const
{
    return this;
}



