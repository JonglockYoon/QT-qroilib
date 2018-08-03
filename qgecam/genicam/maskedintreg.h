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

#ifndef MASKEDINTREG_H
#define MASKEDINTREG_H

#include "inoderegister.h"
#include "iinteger.h"

namespace Jgv {

namespace GenICam {

namespace MaskedIntReg {

static const char * sType = "MaskedIntReg";

class ObjectPrivate;
class Object : public InodeRegister::Object, public Integer::Interface
{


public:
    Object(QSharedPointer<IPort::Interface> iport);
protected:
    Object(ObjectPrivate &dd);
public:
    virtual ~Object() = default;

    virtual void prepare(InterfaceBuilder &builder, const XMLHelper &helper) override;

    void invalidate() override final;

    QVariant getVariant() override final;

    qint64 getValue() override final;
    void setValue(qint64 value) override final;

    Interface *interface() override final;
    const Interface *constInterface() const override final;

private:
    Q_DECLARE_PRIVATE(Object)

}; // class Object

} // namespace MaskedIntReg

} // namespace GenICam

} // namespace Jgv

#endif // MASKEDINTREG_H
