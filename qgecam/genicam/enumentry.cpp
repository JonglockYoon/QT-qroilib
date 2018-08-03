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
#include "enumentry.h"
#include "enumentry_p.h"
#include "genicamobjectbuilder.h"
#include "xmlhelper.h"

#include <QVariant>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::Enumentry;

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
        bool ok = false;
        qint64 val = prop.toLongLong(&ok, 0);
        if (ok) {
            d->value = val;
        }
        else {
            qWarning("EnumEntry: %s failed to parse Value %s", qPrintable(featureName()), qPrintable(prop));
        }
    }
}

void Object::invalidate()
{}

bool Object::isWritable() const
{
    return false;
}

qint64 Object::getIntValue() const
{
    Q_D(const Object);

    return d->value;
}

QVariant Object::getVariant()
{
    return QVariant();
}

Jgv::GenICam::Interface *Object::interface()
{
    return 0;
}

const Jgv::GenICam::Interface *Object::constInterface() const
{
    return 0;
}

bool Jgv::GenICam::Enumentry::operator==(const Object *object, const QString &objectName)
{
    return object->featureName() == objectName;
}
