/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD                             *
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

#include "floatreg.h"
#include "floatreg_p.h"
#include "genicamobjectbuilder.h"
#include "xmlhelper.h"
#include "genicam.h"

#include <QVariant>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::FloatReg;

void ObjectPrivate::fakeValue(const uchar *, int)
{
    qWarning("FloatReg %s get: Fake Value", qPrintable(featureName));
}

void ObjectPrivate::little(const uchar * data, int size)
{
    if (size==4) {
        const float val = qFromLittleEndian<float>(data);
        cache.value = val;
    }
    else if (size==8) {
        const double val = qFromLittleEndian<double>(data);
        cache.value = val;
    }

}

void ObjectPrivate::big(const uchar * data, int size)
{
    if (size==4) {
        const float val = qFromBigEndian<float>(data);
        cache.value = val;
    }
    else if (size==8) {
        const double val = qFromBigEndian<double>(data);
        cache.value = val;
    }
}

void ObjectPrivate::setFakeValue(uchar *, int)
{
    qWarning("FloatReg %s set: Fake Value", qPrintable(featureName));
}

void ObjectPrivate::setLittle(uchar *data, int size)
{
    if (size==4) {
        qToLittleEndian<float>(static_cast<float>(cache.value), data);
    }
    else if (size==8) {
        qToLittleEndian<double>(static_cast<double>(cache.value), data);
    }
}

void ObjectPrivate::setBig(uchar *data, int size)
{
    if (size==4) {
        qToBigEndian<float>(static_cast<float>(cache.value), data);
    }
    else if (size==8) {
        qToBigEndian<double>(static_cast<double>(cache.value), data);
    }
}

Object::Object(QSharedPointer<IPort::Interface> iport)
    : InodeRegister::Object(*new ObjectPrivate(iport))
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    InodeRegister::Object::prepare(builder, helper);
    d->sType = sType;



    d->updateCache = d->isLittleEndian ? &ObjectPrivate::little : &ObjectPrivate::big;
    d->writeCache = d->isLittleEndian ? &ObjectPrivate::setLittle : &ObjectPrivate::setBig;

}

void Object::invalidate()
{
    Q_D(Object);

    d->cache.isValid = false;
}

QVariant Object::getVariant()
{
    Q_D(const Object);
    if (!d->namespaceIsCustom) {
        return GenICam::Representation::toString(featureName(), getValue());
    }

    return QVariant(getValue());
}

double Object::getValue()
{
    Q_D(Object);

    if (!d->cache.isValid) {
        const uchar *p = data();
        if (p != nullptr) {
            (d->* (d->updateCache)) (p, dataSize());
        }
        else {
            qWarning("FloatReg %s, InodeRegister pointer failed !", qPrintable(featureName()));
        }
    }

    d->cache.isValid = d->readUpdateCache;
    return d->cache.value;
}

void Object::setValue(double value)
{
    Q_D(Object);

    d->cache.value = value;

    uchar *p = data();
    if (p == nullptr) {
        qWarning("FloatReg %s, InodeRegister pointer failed !", qPrintable(featureName()));
        return;
    }

    (d->* (d->writeCache)) (p, dataSize());

    updateData();
    d->cache.isValid = d->writeUpdateCache;

}

double Object::getMin() const
{
    Q_D(const Object);

    const qint64 size = dataSize();
    if (size == 4) {
        return std::numeric_limits<float>::lowest();
    }
    else if (size == 8) {
        return std::numeric_limits<double>::lowest();
    }
    return Interface::getMin();
}

double Object::getMax() const
{
    Q_D(const Object);

    const qint64 size = dataSize();
    if (size == 4) {
        return std::numeric_limits<float>::max();
    }
    else if (size == 8) {
        return std::numeric_limits<double>::max();
    }
    return Interface::getMax();
}

Jgv::GenICam::Float::Interface *Object::interface()
{
    return this;
}

const Jgv::GenICam::Float::Interface *Object::constInterface() const
{
    return this;
}



