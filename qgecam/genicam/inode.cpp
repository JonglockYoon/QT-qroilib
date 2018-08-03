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

#include "inode.h"
#include "inode_p.h"

#include "xmlhelper.h"
#include "genicamobjectbuilder.h"
#include "iinteger.h"
#include "boolean.h"
#include "enumeration.h"

#include <QStringList>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::Inode;

bool ObjectPrivate::trueResponse() const
{
    return true;
}

bool ObjectPrivate::falseResponse() const
{
    return false;
}

bool ObjectPrivate::isImplementedByPValue() const
{
    return pIsImplemented.value();
}

bool ObjectPrivate::isAvailableByPValue() const
{
    return pIsAvailable.value();
}

bool ObjectPrivate::isLockedByPValue() const
{
    return pIsLocked.value();
}

qint64 ObjectPrivate::fakeInteger() const
{
    qWarning("Inode %s: fake evaluation integer", qPrintable(featureName));
    return 0;
}

qint64 ObjectPrivate::noError() const
{
    return 0;
}

qint64 ObjectPrivate::errorNum() const
{
    const QSharedPointer<Object> p = pError.toStrongRef();
    if (p) {
        return JGV_CONST_IENUMERATION(p.data())->getIntValue();
    }
    else {
        qWarning("Inode %s: pError pointer is not valid", qPrintable(featureName));
    }
    return 0;
}

Object::Object(QSharedPointer<Jgv::GenICam::IPort::Interface> iport)
    : d_ptr(new ObjectPrivate(iport))
{}

Object::Object(ObjectPrivate &dd)
    : d_ptr(&dd)
{}

Object::~Object()
{
    //qDebug("Deleting inode %s", qPrintable(featureName()));
}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    QString prop = helper.getAttribute(Attributes::Name);
    if (!prop.isEmpty()){
        d->featureName = prop;
    }

    prop = helper.getProperty(Properties::ToolTip);
    if (!prop.isEmpty()) {
        d->toolTip = prop;
    }

    prop = helper.getProperty(Properties::Description);
    if (!prop.isEmpty()) {
        d->description = prop;
    }

    prop = helper.getProperty(Properties::DisplayName);
    if (!prop.isEmpty()) {
        d->displayName = prop;
    }
    else {
        d->displayName = d->featureName;
    }

    prop = helper.getProperty(Properties::Visibility);
    if (!prop.isEmpty()) {
        d->visibility = prop;
    }

    prop = helper.getProperty(Properties::EventID);
    if (!prop.isEmpty()) {
        bool ok = false;
        const quint64 val = prop.toULongLong(&ok, 16);
        if (ok) {
            d->eventID = val;
        }
        else {
            qWarning("Inode %s: failed to parse EventID %s", qPrintable(featureName()), qPrintable(prop));
        }
    }

    prop = helper.getProperty(Properties::pIsImplemented);
    if (!prop.isEmpty()) {
        d->pIsImplemented = pValueInteger(builder.buildInode(prop));
        d->isImplemented = &ObjectPrivate::isImplementedByPValue;
    }

    prop = helper.getProperty(Properties::pIsAvailable);
    if (!prop.isEmpty()) {
        d->pIsAvailable = pValueInteger(builder.buildInode(prop));
        d->isAvailable = &ObjectPrivate::isAvailableByPValue;
    }

    prop = helper.getProperty(Properties::pIsLocked);
    if (!prop.isEmpty()) {
        d->pIsLocked = pValueInteger(builder.buildInode(prop));
        d->isLocked = &ObjectPrivate::isLockedByPValue;
    }
    // si la propriété ImposedAccesMode n'existe pas,
    // l'inode est en accès complet.
    prop = helper.getProperty(Properties::ImposedAccessMode);
    if (prop == Inode::AccessMode::ReadOnly) {
        d->imposedCanWrite = false;
    }
    else if (prop == Inode::AccessMode::WriteOnly) {
        d->imposedCanRead = false;
    }

    prop = helper.getProperty(Properties::pBlockPolling);
    if (!prop.isEmpty()) {
        d->pBlockingPolling = builder.buildInode(prop);
    }

    prop = helper.getProperty(Properties::pAlias);
    if (!prop.isEmpty()) {
        d->pAlias = builder.buildInode(prop);
    }

    prop = helper.getProperty(Properties::pCastAlias);
    if (!prop.isEmpty()) {
        d->pCastAlias = builder.buildInode(prop);
    }

    prop = helper.getProperty(Properties::Streamable);
    if (!prop.isEmpty()) {
        d->isStreamable = prop == "Yes";
    }

    prop = helper.getProperty(Properties::pError);
    if (!prop.isEmpty()) {
        QWeakPointer<Object> pError = builder.buildInode(prop);
        QSharedPointer<Object> p = pError.toStrongRef();
        if (p) {
            if (p->interface()->interfaceType() == Type::IEnumeration) {
                 d->pError = pValueInteger(pError);
            }
            else {
                d->pError = pValueInteger(0);
                qWarning("Inode %s: pError %s doesn't have IEnumeration interface", qPrintable(displayName()), qPrintable(prop));
            }
        }

    }
    else {
        d->pError = pValueInteger(0);
    }

    d->namespaceIsCustom = helper.namespaceAttribut() == "Custom";
}

const QString &Object::typeString() const
{
    Q_D(const Object);
    return d->sType;
}

const QString &Object::toolTip() const
{
    Q_D(const Object);
    return d->toolTip;
}

const QString &Object::displayName() const
{
    Q_D(const Object);
    return d->displayName;
}

const QString &Object::featureName() const
{
    Q_D(const Object);
    return d->featureName;
}

const QString &Object::description() const
{
    Q_D(const Object);
    return d->description;
}

const QString &Object::visibility() const
{
    Q_D(const Object);
    return d->visibility;
}

void Object::setInodeToInvalidate(QWeakPointer<Object> inode)
{
    Q_D(Object);

    if (!d->inodesToInvalidate.contains(inode)) {
        d->inodesToInvalidate.append(inode);
    }
}

QStringList Object::invalidateNames() const
{
    Q_D(const Object);

    QStringList list;

    auto it = d->inodesToInvalidate.constBegin();
    for (; it != d->inodesToInvalidate.constEnd(); ++it) {
        QSharedPointer<Inode::Object> p = (*it).toStrongRef();
        if (p) {
            list.append(p->featureName());
        }
    }

    return list;
}

QStringList Object::getInvalidatorFeatureNames() const
{
    // un simple inode n'a pas d'invalidateur
    return QStringList();
}

void Object::setHierarchy(int row, QWeakPointer<Inode::Object> parent)
{
    Q_D(Object);
    d->row = row;
    d->parent = parent;
}

int Object::row() const
{
    Q_D(const Object);

    return d->row;
}

QWeakPointer<Object> Object::parent() const
{
    Q_D(const Object);

    return d->parent;
}

QWeakPointer<Object> Object::getChild(int raw) const
{
    Q_UNUSED(raw)
    // par défaut un inode n'a pas d'enfant.
    return QWeakPointer<Inode::Object>();
}

bool Object::haveChild() const
{
    // par défaut un inode n'a pas d'enfant.
    return false;
}

int Object::childCount() const
{
    // par défaut un inode n'a pas d'enfant.
    return 0;
}

int Object::getPollingTime() const
{
    // par défaut un inode n'a pas de temps de polling.
    return -1;
}

bool Object::isImplemented() const
{
    Q_D(const Object);

    return (d->*(d->isImplemented)) ();
}

bool Object::isAvailable() const
{
    Q_D(const Object);

    return (d->*(d->isAvailable)) ();
}


bool Object::isLocked() const
{
    Q_D(const Object);

    return (d->*(d->isLocked)) ();
}

qint64 Object::error() const
{
    Q_D(const Object);

    return d->pError.value();
}

QString Object::errrorString(qint64 error) const
{
    Q_D(const Object);

    if (error == 0) {
        return "No error";
    }

    QSharedPointer<Object> p = d->pError.toStrongRef();
    if (p) {
        QList<Enumentry::Object *> entries = JGV_CONST_IENUMERATION(p)->getEntries();
        auto it = entries.constBegin();
        for (; it != entries.constEnd(); ++it) {
            if ((*it)->getIntValue() == error) {
                return QString("%1 (%2)").arg((*it)->displayName()).arg((*it)->toolTip());
            }
        }
    }
    return "Error string failed";
}










