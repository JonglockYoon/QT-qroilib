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

#include "intreg.h"
#include "intreg_p.h"
#include "xmlhelper.h"
#include "genicam.h"

#include <limits>
#include <QVariant>
#include <QtEndian>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::IntReg;

void ObjectPrivate::fakeValue(const uchar *, int)
{
    qWarning("IntReg %s get: Fake Value", qPrintable(featureName));
}

void ObjectPrivate::unsignedLittle(const uchar * data, int size)
{
    if (size==4) {
        const quint32 val = qFromLittleEndian<quint32>(data);
        cache.value = val;
    }
    else if (size==8) {
        const quint64 val = qFromLittleEndian<quint64>(data);
        cache.value = val;
    }

}

void ObjectPrivate::signedLittle(const uchar * data, int size)
{
    if (size==4) {
        const qint32 val = qFromLittleEndian<qint32>(data);
        cache.value = val;
    }
    else if (size==8) {
        const qint64 val = qFromLittleEndian<qint64>(data);
        cache.value = val;
    }
}

void ObjectPrivate::unsignedBig(const uchar * data, int size)
{
    if (size==4) {
        const quint32 val = qFromBigEndian<quint32>(data);
        cache.value = val;
    }
    else if (size==8) {
        const quint64 val = qFromBigEndian<quint64>(data);
        cache.value = val;
    }
}

void ObjectPrivate::signedBig(const uchar * data, int size)
{
    if (size==4) {
        const qint32 val = qFromBigEndian<qint32>(data);
        cache.value = val;
    }
    else if (size==8) {
        const qint64 val = qFromBigEndian<qint64>(data);
        cache.value = val;
    }
}

void ObjectPrivate::setFakeValue(uchar *, int)
{
    qWarning("IntReg %s set: Fake Value", qPrintable(featureName));
}

void ObjectPrivate::setUnsignedLittle(uchar *data, int size)
{
    if (size==4) {
        qToLittleEndian<quint32>(static_cast<quint32>(cache.value), data);
    }
    else if (size==8) {
        qToLittleEndian<quint64>(static_cast<quint64>(cache.value), data);
    }
}

void ObjectPrivate::setSignedLittle(uchar *data, int size)
{
    if (size==4) {
        qToLittleEndian<qint32>(static_cast<qint32>(cache.value), data);
    }
    else if (size==8) {
        qToLittleEndian<qint64>(static_cast<qint64>(cache.value), data);
    }
}

void ObjectPrivate::setUnsignedBig(uchar *data, int size)
{
    if (size==4) {
        qToBigEndian<quint32>(static_cast<quint32>(cache.value), data);
    }
    else if (size==8) {
        qToBigEndian<quint64>(static_cast<quint64>(cache.value), data);
    }
}

void ObjectPrivate::setSignedBig(uchar *data, int size)
{
    if (size==4) {
        qToBigEndian<qint32>(static_cast<qint32>(cache.value), data);
    }
    else if (size==8) {
        qToBigEndian<qint64>(static_cast<qint64>(cache.value), data);
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

    QString prop = helper.getProperty(Properties::Sign);
    if (!prop.isEmpty()) {
        d->isSigned = (prop == Sign::Signed);
    }

    if (d->isSigned) {
        d->updateCache = d->isLittleEndian ? &ObjectPrivate::signedLittle : &ObjectPrivate::signedBig;
        d->writeCache = d->isLittleEndian ? &ObjectPrivate::setSignedLittle : &ObjectPrivate::setSignedBig;
    }
    else {
        d->updateCache = d->isLittleEndian ? &ObjectPrivate::unsignedLittle : &ObjectPrivate::unsignedBig;
        d->writeCache = d->isLittleEndian ? &ObjectPrivate::setUnsignedLittle : &ObjectPrivate::setUnsignedBig;
    }
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
        return Jgv::GenICam::Representation::toString(featureName(), getValue());
    }

    return QVariant(getValue());
}

qint64 Object::getValue()
{
    Q_D(Object);

    if (!d->cache.isValid) {
        const uchar *p = data();
        if (p != nullptr) {
            (d->* (d->updateCache)) (p, dataSize());
        }
        else {
            qWarning("IntReg %s, InodeRegister pointer failed !", qPrintable(featureName()));
        }
    }

    d->cache.isValid = d->readUpdateCache;
    return d->cache.value;
}

void Object::setValue(qint64 value)
{
    Q_D(Object);

    d->cache.value = value;

    uchar *p = data();
    if (p == nullptr) {
        qWarning("IntReg %s, InodeRegister pointer failed !", qPrintable(featureName()));
        return;
    }

    (d->* (d->writeCache)) (p, dataSize());

    // provoque l'écriture sur l'iport
    updateData();
    d->cache.isValid = d->writeUpdateCache;
}

qint64 Object::getMin() const
{
    Q_D(const Object);

    const qint64 size = dataSize();
    if (size == 4) {
        return d->isSigned ? std::numeric_limits<qint32>::lowest() :std::numeric_limits<quint32>::lowest();
    }
    else if (size == 8) {
        return d->isSigned ? std::numeric_limits<qint64>::lowest() :std::numeric_limits<quint64>::lowest();
    }
    return Interface::getMin();

}

qint64 Object::getMax() const
{
    Q_D(const Object);

    const qint64 size = dataSize();
    if (size == 4) {
        return d->isSigned ? std::numeric_limits<qint32>::max() :std::numeric_limits<quint32>::max();
    }
    else if (size == 8) {
        return d->isSigned ? std::numeric_limits<qint64>::max() :std::numeric_limits<quint64>::max();
    }
    return Interface::getMax();
}

Jgv::GenICam::Integer::Interface *Object::interface()
{
    return this;
}

const Jgv::GenICam::Integer::Interface *Object::constInterface() const
{
    return this;
}




