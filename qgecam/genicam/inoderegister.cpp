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

#include "inoderegister.h"
#include "inoderegister_p.h"
#include "genicamobjectbuilder.h"
#include "xmlhelper.h"
#include "iport.h"
#include "iinteger.h"
#include "intswissknife.h"

#include <QDebug>
#include <QStringList>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::InodeRegister;

Object::Object(ObjectPrivate &dd)
    : Inode::Object(dd)
{}

Object::~Object()
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    // prepare l'inode
    Inode::Object::prepare(builder, helper);


    {
        QStringList props = helper.getProperties(Properties::Address);
        auto it = props.constBegin();
        for (; it != props.constEnd(); ++it) {
            bool ok { false };
            qint64 val = (*it).toLongLong(&ok, 0);
            if (ok) {
                d->addresses.append(val);
            }
            else {
                qWarning("InodeRegister: %s failed to parse Address %s", qPrintable(featureName()), qPrintable(*it));
            }
        }
    } // Address


    {
        QStringList props = helper.getProperties(Properties::pAddress);
        auto it = props.constBegin();
        for (; it != props.constEnd(); ++it) {
            d->pAddresses.append(pValueInteger(builder.buildInode(*it)));
        }
    } // pAddress


    {
        ChildrenHelpers helpers = helper.getChildrenHelpers(Properties::IntSwissKnife);
        auto it = helpers.constBegin();
        for (; it != helpers.constEnd(); ++it) {
            QSharedPointer<IntSwissKnife::Object> intSwissKnife(new IntSwissKnife::Object(d->iport));
            intSwissKnife->prepare(builder, (*it));
            d->intSwissKnives.append(intSwissKnife);
        }
    } // IntSwissKnife


    {
        ChildrenHelpers helpers = helper.getChildrenHelpers(Properties::pIndex);
        auto it = helpers.constBegin();
        for (;it != helpers.constEnd(); ++it) {
            pValueInteger inode = pValueInteger(builder.buildInode( (*it).value() ) );
            QString offset = (*it).getAttribute(Attributes::Offset);
            if (!offset.isEmpty()) {
                bool ok;
                qint64 val = offset.toLongLong(&ok, 0);
                if (ok) {
                    d->pIndexes.append( Index{ inode, pValueInteger(val) } );
                }
            }
            else {
                offset = (*it).getAttribute(Attributes::pOffset);
                if (!offset.isEmpty()) {
                    d->pIndexes.append( Index{ inode,  pValueInteger(builder.buildInode(offset)) } );
                }
                else {
                    // l'offset n'est pas déclaré, on le force à 1
                    d->pIndexes.append( Index{ inode,  pValueInteger(1) } );
                }
            }
        }
    } // pIndex


    {
        QString prop = helper.getProperty(Properties::Length);
        if (!prop.isEmpty()) {
            bool ok { false };
            qint64 length = prop.toLongLong(&ok, 0);
            if (ok) {
                d->pLength = pValueInteger(length);
            } else {
                qWarning("InodeRegister: %s failed to parse Length %s", qPrintable(featureName()), qPrintable(prop));
            }
        }
        else {
            prop = helper.getProperty(Properties::pLength);
            if (!prop.isEmpty()) {
                d->pLength = pValueInteger(builder.buildInode(prop));
            }
            else {
                qWarning("InodeRegister: %s no length defined", qPrintable(featureName()));
            }
        }
    } // length


    {
        QStringList props = helper.getProperties(Properties::pInvalidator);
        auto it = props.constBegin();
        for (; it != props.constEnd(); ++it) {
            QWeakPointer<Inode::Object> inode = builder.buildInode(*it);
            QSharedPointer<Inode::Object> p = inode.toStrongRef();
            if (p) {
                d->pInvalidators.append(inode);
                // obtient le weakpointer sur this par le builder
                p->setInodeToInvalidate(builder.buildInode(d->featureName));
            }
        }
    } // invalidators


    {
        QString prop = helper.getProperty(Properties::PollingTime);
        if (!prop.isEmpty()) {
            bool ok { false };
            d->pollingTime = prop.toLongLong(&ok, 0);
            if (!ok) {
                qWarning("InodeRegister: %s failed to parse PollingTime %s", qPrintable(featureName()), qPrintable(prop));
            }
        }
    } // PollingTime


    // si ce n'est pas précisé, le registre est en bigEndian
    d->isLittleEndian = (helper.getProperty(Properties::Endianess) == Endianess::LittleEndian);

    // si la propriété AccesMode n'existe pas,
    // le registre est en accès complet.
    d->canRead = true;
    d->canWrite = true;
    QString accesMode = helper.getProperty(Properties::AccessMode);
    if (accesMode == Inode::AccessMode::ReadOnly) {
        d->canWrite = false;
    }
    else if (accesMode == Inode::AccessMode::WriteOnly) {
        d->canRead = false;
    }

    QString cachable = helper.getProperty(Properties::Cachable);
    // WriteThrough : une écriture provoque la mise à jour du cache (la lecture aussi)
    if (cachable == Cache::WriteThrough) {
        d->readUpdateCache = true;
        d->writeUpdateCache = true;
    }
    // WriteAround : seule une lecture provoque la mise à jour du cache
    else if (cachable == Cache::WriteAround) {
        d->readUpdateCache = true;
        d->writeUpdateCache = false;
    }
    // NoCache : fonctionnement sans cache.
    else if (cachable == Cache::NoCache) {
        d->readUpdateCache = false;
        d->writeUpdateCache = false;
    }

    d->pPort = helper.getProperty(Properties::pPort);
}

QStringList Object::getInvalidatorFeatureNames() const
{
    Q_D(const Object);

    QStringList invalidators;
    auto it = d->pInvalidators.constBegin();
    for (; it != d->pInvalidators.constEnd(); ++it) {
        QSharedPointer<Inode::Object> p = (*it).toStrongRef();
        if (p) {
            invalidators.append(p->featureName());
        }
    }

    return invalidators;
}

int Object::getPollingTime() const
{
    Q_D(const Object);
    return d->pollingTime;
}

bool Object::isWritable() const
{
    Q_D(const Object);
    return d->canWrite;
}

uchar *Object::data()
{
    Q_D(Object);

    if (d->data.isEmpty()) {
        d->data.resize(d->pLength.value());
    }

    if (d->canRead) {
        d->iport->read(d->data.data(), address(), d->data.size());
    }
    return d->data.data();

}

qint64 Object::dataSize() const
{
    Q_D(const Object);

    return d->pLength.value();
}

void Object::updateData()
{
    Q_D(Object);
    if (d->canWrite && (!d->data.isEmpty())) {
        d->iport->write(d->data.data(), address(), d->data.size());
        // invalide les inodes listés
        auto it = d->inodesToInvalidate.constBegin();
        for (; it != d->inodesToInvalidate.constEnd(); ++it) {
            QSharedPointer<Inode::Object> p = (*it).toStrongRef();
            if (p) {
                p->invalidate();
            }
        }
    }
}

qint64 Object::address() const
{
    Q_D(const Object);

    qint64 address = 0;

    {   // adress
        auto it = d->addresses.constBegin();
        for (; it != d->addresses.constEnd(); ++it) {
            address += (*it);
        }
    }

    {   // pAddress
        auto it = d->pAddresses.constBegin();
        for (; it != d->pAddresses.constEnd(); ++it) {
            address += (*it).value();
        }
    }

    {   // IntSwissKnife
        auto it = d->intSwissKnives.constBegin();
        for (; it != d->intSwissKnives.constEnd(); ++it) {
            address += (*it)->getValue();
        }
    }

    {   // pIndex
        auto it = d->pIndexes.constBegin();
        for (; it != d->pIndexes.constEnd(); ++it) {
            address += (*it).value();
        }
    }

    return address;
}








