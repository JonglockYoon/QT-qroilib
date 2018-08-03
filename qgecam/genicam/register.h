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

#ifndef REGISTER_H
#define REGISTER_H

#include "inoderegister.h"
#include "iregister.h"

namespace Jgv {

namespace GenICam {

namespace Register {

static const char * sType = "Register";

class ObjectPrivate;
class Object final :  public InodeRegister::Object, public Register::Interface
{

public:
    Object(QSharedPointer<IPort::Interface> iport);
    virtual ~Object() = default;

    void prepare(InterfaceBuilder &builder, const XMLHelper &helper) override final;
    void invalidate() override final;

    QVariant getVariant() override final;

    void get(quint8 *buffer, qint64 length) override final;
    void set(quint8 *buffer, qint64 length) override final;
    qint64 getAddress() override final;
    qint64 getLength() override final;

    Register::Interface *interface() override final;
    const Register::Interface *constInterface() const override final;

private:
    Q_DECLARE_PRIVATE(Object)

}; // class Object

} // namespace Register

} // namespace GenICam

} // namespace Jgv

#endif // REGISTER_H
