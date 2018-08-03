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

#ifndef SWISSKNIFE_H
#define SWISSKNIFE_H

#include "inodeswissknife.h"
#include "ifloat.h"

namespace Jgv {

namespace GenICam {

namespace SwissKnife {

static const char * sType = "SwissKnife";

class ObjectPrivate;
class Object final : public InodeSwissKnife::Object, public Float::Interface
{

public:
    Object(QSharedPointer<IPort::Interface> iport);
    virtual ~Object() = default;

    void prepare(InterfaceBuilder &builder, const XMLHelper &helper) override final;

    void invalidate() override final;

    bool isWritable() const override final;

    QVariant getVariant() override final;

    double getValue() override final;
    void setValue(double value) override final;

    Interface *interface() override final;
    const Interface *constInterface() const override final;

private:
    Q_DECLARE_PRIVATE(Object)

}; // class Interface

} // namespace SwissKnife

} // namespace GenICam

} // namespace Jgv

#endif // SWISSKNIFE_H
