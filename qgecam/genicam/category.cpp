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
#include "category.h"
#include "category_p.h"
#include "genicamobjectbuilder.h"
#include "xmlhelper.h"

#include <QStringList>
#include <QVariant>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::Category;

Object::Object(QSharedPointer<IPort::Interface> iport)
    : Inode::Object(*new ObjectPrivate(iport))
{}

void ObjectPrivate::setVisibility(const QString &childVisibility)
{
    if (visibility.startsWith(Inode::Visibility::Invisible)) {
        if (!childVisibility.startsWith(Inode::Visibility::Invisible)) {
            visibility = childVisibility;
            return;
        }
    }
    if (visibility.startsWith(Inode::Visibility::Guru)) {
        if (childVisibility.startsWith(Inode::Visibility::Expert)
                || childVisibility.startsWith(Inode::Visibility::Beginner)) {
            visibility = childVisibility;
            return;
        }
    }
    if (visibility.startsWith(Inode::Visibility::Expert)) {
        if (childVisibility.startsWith(Inode::Visibility::Beginner)) {
            visibility = childVisibility;
        }
    }
}


void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    Inode::Object::prepare(builder, helper);
    d->sType = sType;

    // par défaut, category n'a pas d'enfant, son status est donc invisible
    d->visibility = Inode::Visibility::Invisible;

    int raw = 0;

    QStringList features = helper.getProperties(Properties::pFeature);
    auto it = features.constBegin();
    for (; it != features.constEnd(); ++it) {
        QWeakPointer<Inode::Object> inode = builder.buildInode(*it);
        QSharedPointer<Inode::Object> p = inode.toStrongRef();
        if (p) {
            p->setHierarchy(raw, builder.buildInode(featureName()));
            d->setVisibility(p->visibility());
            d->childs.append(inode);
            raw++;
        }
        else {
            qWarning("Category %s: failed to construct child %s", qPrintable(d->featureName), qPrintable(*it));
        }
    }
}

void Object::invalidate()
{}

bool Object::isWritable() const
{
    return false;
}

QList<QWeakPointer<Jgv::GenICam::Inode::Object> > Object::getFeatures() const
{
    Q_D(const Object);

    return d->childs;
}

QWeakPointer<Jgv::GenICam::Inode::Object> Object::getChild(int raw) const
{
    Q_D(const Object);

    return ( (raw >= 0) && (raw < d->childs.count()) )
            ? d->childs.at(raw)
            : QWeakPointer<Inode::Object>();
}

bool Object::haveChild() const
{
    Q_D(const Object);

    return !d->childs.isEmpty();
}

int Object::childCount() const
{
    Q_D(const Object);

    return d->childs.count();
}

QVariant Object::getVariant()
{
    return QVariant();
}

Category::Interface *Object::interface()
{
    return this;
}

const Category::Interface *Object::constInterface() const
{
    return this;
}




