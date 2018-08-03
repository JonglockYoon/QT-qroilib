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

#include "maskedintreg.h"
#include "maskedintreg_p.h"
#include "xmlhelper.h"
#include "genicam.h"

#include <QVariant>
#include <QtEndian>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::MaskedIntReg;

namespace {
const quint64 MASK[64] = {
    0x0000000000000001,0x0000000000000003,0x0000000000000007,0x000000000000000F,0x000000000000001F,0x000000000000003F,0x000000000000007F,0x00000000000000FF,
    0x00000000000001FF,0x00000000000003FF,0x00000000000007FF,0x0000000000000FFF,0x0000000000001FFF,0x0000000000003FFF,0x0000000000007FFF,0x000000000000FFFF,
    0x000000000001FFFF,0x000000000003FFFF,0x000000000007FFFF,0x00000000000FFFFF,0x00000000001FFFFF,0x00000000003FFFFF,0x00000000007FFFFF,0x0000000000FFFFFF,
    0x0000000001FFFFFF,0x0000000003FFFFFF,0x0000000007FFFFFF,0x000000000FFFFFFF,0x000000001FFFFFFF,0x000000003FFFFFFF,0x000000007FFFFFFF,0x00000000FFFFFFFF,
    0x00000001FFFFFFFF,0x00000003FFFFFFFF,0x00000007FFFFFFFF,0x0000000FFFFFFFFF,0x0000001FFFFFFFFF,0x0000003FFFFFFFFF,0x0000007FFFFFFFFF,0x000000FFFFFFFFFF,
    0x000001FFFFFFFFFF,0x000003FFFFFFFFFF,0x000007FFFFFFFFFF,0x00000FFFFFFFFFFF,0x00001FFFFFFFFFFF,0x00003FFFFFFFFFFF,0x00007FFFFFFFFFFF,0x0000FFFFFFFFFFFF,
    0x0001FFFFFFFFFFFF,0x0003FFFFFFFFFFFF,0x0007FFFFFFFFFFFF,0x000FFFFFFFFFFFFF,0x001FFFFFFFFFFFFF,0x003FFFFFFFFFFFFF,0x007FFFFFFFFFFFFF,0x00FFFFFFFFFFFFFF,
    0x01FFFFFFFFFFFFFF,0x03FFFFFFFFFFFFFF,0x07FFFFFFFFFFFFFF,0x0FFFFFFFFFFFFFFF,0x1FFFFFFFFFFFFFFF,0x3FFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFF,
};
} // anonymous namespace

void ObjectPrivate::fakeGet(const uchar *)
{
    qWarning("MaskedIntReg %s: fake get", qPrintable(featureName));
}

void ObjectPrivate::getFrom32LE(const uchar *registerValues)
{
    const quint32 val = qFromLittleEndian<quint32>(registerValues);
    // on décale du LSB puis applique le masque
    cache.value = MASK[(msb-lsb)] & (val >> lsb);
}

void ObjectPrivate::getFrom32BE(const uchar *registerValues)
{
    // conversion BE -> LE
    const unsigned m = 31-msb;
    const unsigned l = 31-lsb;

    const quint32 val = qFromBigEndian<quint32>(registerValues);
    // on décale du LSB puis applique le masque
    cache.value = MASK[m-l] & (val >> l);
}

void ObjectPrivate::getFrom64LE(const uchar *registerValues)
{
    const quint64 val = qFromLittleEndian<quint64>(registerValues);
    // on décale du LSB puis applique le masque
    cache.value = MASK[(msb-lsb)] & (val >> lsb);
}

void ObjectPrivate::getFrom64BE(const uchar *registerValues)
{
    // conversion BE -> LE
    const unsigned m = 63-msb;
    const unsigned l = 63-lsb;

    const quint64 val = qFromBigEndian<quint64>(registerValues);
    // on décale du LSB puis applique le masque
    cache.value = MASK[m-l] & (val >> l);
}

void ObjectPrivate::fakeSet(uchar *)
{
    qWarning("MaskedIntReg %s: fake set", qPrintable(featureName));
}

void ObjectPrivate::setTo32LE(uchar *registerValues)
{
    // calcul du masque
    const quint32 mask = MASK[msb-lsb] << lsb;
    // extraction du registre
    quint32 regVal = qFromLittleEndian<quint32>(registerValues);
    // mise à zéro des bits concernés
    regVal &= (~mask);
    // aligne la valeur du champs de bits
    quint32 alignedValue = cache.value << lsb;
    // applique au registre
    regVal |= alignedValue;
    // diffuse sur le tableau
    qToLittleEndian<quint32>(regVal, registerValues);

}

void ObjectPrivate::setTo32BE(uchar *registerValues)
{
    // conversion BE -> LE
    const unsigned m = 31-msb;
    const unsigned l = 31-lsb;

    // calcul du masque
    const quint32 mask = MASK[m-l] << l;
    // extraction du registre
    quint32 regVal = qFromBigEndian<quint32>(registerValues);
    // mise à zéro des bits concernés
    regVal &= (~mask);
    // aligne la valeur du champs de bits
    quint32 alignedValue = cache.value << l;
    // applique au registre
    regVal |= alignedValue;
    // diffuse sur le tableau
    qToBigEndian<quint32>(regVal, registerValues);
}

void ObjectPrivate::setTo64LE(uchar *registerValues)
{
    // calcul du masque
    const quint64 mask = MASK[msb-lsb] << lsb;
    // extraction du registre
    quint64 regVal = qFromLittleEndian<quint64>(registerValues);
    // mise à zéro des bits concernés
    regVal &= (~mask);
    // aligne la valeur du champs de bits
    quint64 alignedValue = cache.value << lsb;
    // applique au registre
    regVal |= alignedValue;
    // diffuse sur le tableau
    qToLittleEndian<quint64>(regVal, registerValues);
}

void ObjectPrivate::setTo64BE(uchar *registerValues)
{
    // conversion BE -> LE
    const unsigned m = 63-msb;
    const unsigned l = 63-lsb;

    // calcul du masque
    const quint64 mask = MASK[m-l] << l;
    // extraction du registre
    quint64 regVal = qFromBigEndian<quint64>(registerValues);
    // mise à zéro des bits concernés
    regVal &= (~mask);
    // aligne la valeur du champs de bits
    quint64 alignedValue = cache.value << l;
    // applique au registre
    regVal |= alignedValue;
    // diffuse sur le tableau
    qToBigEndian<quint64>(regVal, registerValues);
}


Object::Object(QSharedPointer<IPort::Interface> iport)
    : InodeRegister::Object(*new ObjectPrivate(iport))
{}

Object::Object(ObjectPrivate &dd)
    : InodeRegister::Object(dd)
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    InodeRegister::Object::prepare(builder, helper);

    d->sType = sType;

    QString prop = helper.getProperty(Properties::Bit);
    if (!prop.isEmpty()) {
        bool ok = false;
        unsigned val = prop.toUInt(&ok, 0);
        if (ok) {
            // le cas bit est equivalent au cas LSB = MSB = BIT
            d->lsb = val;
            d->msb = val;
            if (d->isLittleEndian) {
                d->evaluation32 = &ObjectPrivate::getFrom32LE;
                d->evaluation64 = &ObjectPrivate::getFrom64LE;
                d->setEvaluation32 = &ObjectPrivate::setTo32LE;
                d->setEvaluation64 = &ObjectPrivate::setTo64LE;
            }
            else {
                d->evaluation32 = &ObjectPrivate::getFrom32BE;
                d->evaluation64 = &ObjectPrivate::getFrom64BE;
                d->setEvaluation32 = &ObjectPrivate::setTo32BE;
                d->setEvaluation64 = &ObjectPrivate::setTo64BE;
            }
        }
        else {
            qWarning("MaskedIntReg %s: failed to parse Bit %s", qPrintable(featureName()), qPrintable(prop));
        }
    }
    else {
        prop = helper.getProperty(Properties::LSB);
        QString msbProp = helper.getProperty(Properties::MSB);
        if ((!prop.isEmpty()) && (!msbProp.isEmpty())) {
            bool ok = false;
            unsigned lsb = prop.toUInt(&ok, 0);
            if (ok) {
                unsigned msb = msbProp.toUInt(&ok, 0);
                if (ok) {
                    d->lsb = lsb;
                    d->msb = msb;
                    if (d->isLittleEndian) {
                        d->evaluation32 = &ObjectPrivate::getFrom32LE;
                        d->evaluation64 = &ObjectPrivate::getFrom64LE;
                        d->setEvaluation32 = &ObjectPrivate::setTo32LE;
                        d->setEvaluation64 = &ObjectPrivate::setTo64LE;
                    }
                    else {
                        d->evaluation32 = &ObjectPrivate::getFrom32BE;
                        d->evaluation64 = &ObjectPrivate::getFrom64BE;
                        d->setEvaluation32 = &ObjectPrivate::setTo32BE;
                        d->setEvaluation64 = &ObjectPrivate::setTo64BE;
                    }
                }
                else {
                    qWarning("MaskedIntReg %s: failed to parse MSB %s", qPrintable(featureName()), qPrintable(prop));
                }
            }
            else {
                qWarning("MaskedIntReg %s: failed to parse LSB %s", qPrintable(featureName()), qPrintable(prop));
            }
        }
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
        return GenICam::Representation::toString(featureName(), getValue());
    }

    return QVariant(getValue());
}

qint64 Object::getValue()
{
    Q_D(Object);

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    qCritical("MaskedIntReg::getValue error: machine en BigEndian !");
    return 0;
#endif

    // ne met à jour le cache que si il est invalide
    if (!d->cache.isValid) {

        const uchar *p = data();
        if (p == nullptr) {
            qWarning("MaskedIntreg %s get: invalid pointer", qPrintable(featureName()));
            return d->cache.value;
        }

        const qint64 pSize = dataSize();

        switch (pSize) {
        case 4:
            (d->* (d->evaluation32)) (p);
            break;
        case 8:
            (d->* (d->evaluation64)) (p);
            break;
        default:
            qWarning("MaskedIntReg %s getValue: bad data size %lld", qPrintable(featureName()), pSize);
        }

        d->cache.isValid = d->readUpdateCache;

    }

    return d->cache.value;
}



void Object::setValue(qint64 value)
{
    Q_D(Object);

    // met à jour le cache
    d->cache.value = value;

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    qCritical("MaskedIntReg::setValue error: machine en BigEndian !");
    return;
#endif

    uchar *p = data();
    if (p == 0) {
        qWarning("MaskedIntreg %s, InodeRegister pointer failed !", qPrintable(featureName()));
        return;
    }

    const qint64 size = dataSize();

    switch (size) {
    case 4:
        (d->* (d->setEvaluation32)) (p);
        break;
    case 8:
        (d->* (d->setEvaluation64)) (p);
        break;
    default:
        qWarning("MaskedIntReg %s setValue: bad data size %lld", qPrintable(featureName()), size);
    }

    updateData();
    d->cache.isValid = d->writeUpdateCache;
}

Jgv::GenICam::Integer::Interface *Object::interface()
{
    return this;
}

const Jgv::GenICam::Integer::Interface *Object::constInterface() const
{
    return this;
}






