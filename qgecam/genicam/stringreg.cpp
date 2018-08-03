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

#include "stringreg.h"
#include "stringreg_p.h"

#include <QVariant>
#include <cstring>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::StringReg;


Object::Object(QSharedPointer<IPort::Interface> iport)
    : InodeRegister::Object(*new ObjectPrivate(iport))
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    InodeRegister::Object::prepare(builder, helper);
    d->sType = sType;
}

void Object::invalidate()
{
    Q_D(Object);

    d->cache.isValid = false;
}

QVariant Object::getVariant()
{
    return QVariant(getValue());
}

QByteArray Object::getValue()
{
    Q_D (Object);
    if (!d->cache.isValid) {
        const char *p = reinterpret_cast<const char *>(data());
        if (p != 0) {
            d->cache.value = QByteArray(p, dataSize());
            d->cache.isValid = d->readUpdateCache;
        }
        else {
            qWarning("StringReg %s, InodeRegister pointer failed !", qPrintable(featureName()));
        }
    }
    return d->cache.value;
}

void Object::setValue(const QByteArray &value)
{
    Q_D(Object);

    d->cache.value = value;

    const qint64 size = dataSize();
    uchar *p = data();
    // rempli le la tampon de 0
    std::fill(p, p+size, 0);
    // copie dans le tampon la valeur
    std::memcpy(p, d->cache.value.constData(), size);
    // demande l'écriture sur l'iport
    updateData();
    // gère le cache
    d->cache.isValid = d->writeUpdateCache;
}

qint64 Object::getMaxLenght()
{
    return dataSize();
}

Jgv::GenICam::String::Interface *Object::interface()
{
    return this;
}

const Jgv::GenICam::String::Interface *Object::constInterface() const
{
    return this;
}





