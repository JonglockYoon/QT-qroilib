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
#ifndef CATEGORY_H
#define CATEGORY_H

#include "inode.h"
#include "icategory.h"

namespace Jgv {

namespace GenICam {

namespace Inode {
class Object;
}

namespace Category {

static const char * sType = "Category";

class ObjectPrivate;
class Object final : public Inode::Object, public Category::Interface
{

public:
    Object(QSharedPointer<IPort::Interface> iport);
    virtual ~Object() = default;

    void prepare(InterfaceBuilder &builder, const XMLHelper &helper) override;

    void invalidate() override;
    bool isWritable() const override;

    QVariant getVariant() override;

    QList<QWeakPointer<Inode::Object> > getFeatures() const override;
    QWeakPointer<Inode::Object> getChild(int raw) const override;
    bool haveChild() const override;
    int childCount() const override;

    Category::Interface *interface() override;
    const Category::Interface *constInterface() const override;

private:
    Q_DECLARE_PRIVATE(Object)

};

} // namespace Category

} // namespace GenICam

} // namespace Jgv

#endif // CATEGORY_H
