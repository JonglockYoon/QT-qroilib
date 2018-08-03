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
#ifndef FLOATREG_H
#define FLOATREG_H

#include "inoderegister.h"
#include "ifloat.h"

namespace Jgv {

namespace GenICam {

namespace FloatReg {

static const char * sType = "FloatReg";

class ObjectPrivate;
class Object final : public InodeRegister::Object, public Float::Interface
{

public:
    Object(QSharedPointer<IPort::Interface> iport);
    virtual ~Object() = default;

    void prepare(InterfaceBuilder &builder, const XMLHelper &helper) override;
    void invalidate() override;

    QVariant getVariant() override;
    double getValue() override;
    void setValue(double value) override;

    double getMin() const override final;
    double getMax() const override final;

    Interface *interface() override;
    const Interface *constInterface() const override;

private:
    Q_DECLARE_PRIVATE(Object)

}; // class Object

} // namespace FloatReg

} // namespace GenICam

} // namespace Jgv

#endif // FLOATREG_H
