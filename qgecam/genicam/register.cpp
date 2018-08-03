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

#include "register.h"
#include "register_p.h"
#include "genicamobjectbuilder.h"
#include "xmlhelper.h"
#include "iinteger.h"
#include "iport.h"

#include <QtEndian>
#include <QStringList>
#include <QVariant>
#include <cstring>


using namespace Jgv::GenICam;
using namespace Jgv::GenICam::Register;

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
    return QVariant(QByteArray(reinterpret_cast<const char*>(data()), dataSize()));
}

void Object::get(quint8 *buffer, qint64 length)
{
    Q_D(const Object);
    const uchar *p = data();
    if (p != 0) {
        if ((buffer!=0) && (length<=dataSize())) {
            std::memcpy(buffer, data(), length);
        }
    }
    else {
        qWarning("Register %s, InodeRegister pointer failed !", qPrintable(featureName()));
    }

}

void Object::set(quint8 *buffer, qint64 length)
{
    Q_D(Object);

    qint64 size = dataSize();
    if (buffer && length <= size) {
        uchar *p = data();
        std::fill(p, p + size, 0);
        std::memcpy(p, buffer, length);
        updateData();
    }
}

qint64 Object::getAddress()
{
    return InodeRegister::Object::address();
}

qint64 Object::getLength()
{
    return dataSize();
}

Register::Interface *Object::interface()
{
    return this;
}

const Register::Interface *Object::constInterface() const
{
    return this;
}




