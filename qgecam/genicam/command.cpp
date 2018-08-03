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
#include "command.h"
#include "command_p.h"
#include "icommand.h"
#include "iinteger.h"
#include "genicamobjectbuilder.h"
#include "xmlhelper.h"

#include <QVariant>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::Command;


Object::Object(QSharedPointer<IPort::Interface> iport)
    : Inode::Object(*new ObjectPrivate(iport))
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    Inode::Object::prepare(builder, helper);
    d->sType = sType;

    {
        QString prop = helper.getProperty(Properties::Value);
        if (!prop.isEmpty()) {
            bool ok = false;
            qint64 val = prop.toLongLong(&ok, 0);
            if (ok) {
                d->pValue = pValueInteger(val);
            }
        }
        else {
            prop = helper.getProperty(Properties::pValue);
            if (!prop.isEmpty()) {
                d->pValue = pValueInteger(builder.buildInode(prop));
            }
        }
    } // pValue

    {
        QString prop = helper.getProperty(Properties::CommandValue);
        if (!prop.isEmpty()) {
            bool ok = false;
            qint64 val = prop.toLongLong(&ok, 0);
            if (ok) {
                d->pCommandValue = pValueInteger(val);
            }
        }
        else {
            prop = helper.getProperty(Properties::pCommandValue);
            if (!prop.isEmpty()) {
                d->pCommandValue = pValueInteger(builder.buildInode(prop));
            }
        }
    } // pCommandValue

    {
        // gestion de l'acquisition
        // SNFC : Standard Features Naming Convention
        // Le SNFC définit le node TLParamsLocked comme étant un verrou
        // qui interdit les changements critiques sur le device.
        // Devient un locker, une commande qui lance le stream,
        // un unlocker une commande qui stop le flux.
        d->pTLLocker = pValueInteger(builder.buildInode(Properties::TLParamsLocked));

        if (d->featureName == "AcquisitionStart") {
            d->isTLLocker = true;
        }
        else if (d->featureName == "AcquisitionStop" || d->featureName == "AcquisitionAbord")
        {
            d->isTLUnlocker = true;
        }

        //d->inodesToInvalidate.append(d->pTLLocker);
    } // TLParamsLocked
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

    QSharedPointer<Inode::Object> p = d->pValue.toStrongRef();
    return (p) ? p->isWritable() : true;
}

QVariant Object::getVariant()
{
    return QVariant("<command>");
}

void Object::execute()
{
    Q_D(Object);

    const qint64 commandValue = d->pCommandValue.value();
    d->pValue.set(commandValue);
    if (d->isTLLocker) {
        d->pTLLocker.set(1);
    }
    else if (d->isTLUnlocker) {
        d->pTLLocker.set(0);
    }
}

bool Object::isDone() const
{
    qDebug("%s",qPrintable("TODO command isDone"));
    return true;
}

Command::Interface *Object::interface()
{
    return this;
}

const Command::Interface *Object::constInterface() const
{
    return this;
}



