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
#ifndef FLOAT_H
#define FLOAT_H

#include "inodevalue.h"
#include "ifloat.h"

namespace Jgv {

namespace GenICam {

namespace Float {

static const char * sType = "Float";

class ObjectPrivate;
class Object final : public InodeValue::Object<double>, public Float::Interface
{

public:
    Object(QSharedPointer<IPort::Interface> iport);
    virtual ~Object() = default;

    void prepare(InterfaceBuilder &builder, const XMLHelper &helper) override final;

    void invalidate() override final;
    virtual bool isWritable() const override final;

    virtual QVariant getVariant() override final;
    int getPollingTime() const override final;


    double getValue() override final;
    void setValue(double value) override final;
    double getMin() const override final;
    double getMax() const override final;
    double getInc() const override final;

    Float::Interface *interface() override final;
    const Float::Interface *constInterface() const override final;

private:
    Q_DECLARE_PRIVATE(Object)

}; // class Object

} // namespace Float

} // namespace GenICam

} // namespace Jgv

#endif // FLOAT_H
