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

#ifndef INODESWISSKNIFE_H
#define INODESWISSKNIFE_H

#include "inode.h"

namespace Jgv {

namespace GenICam {

namespace InodeSwissKnife {

class ObjectPrivate;
class Object : public Inode::Object
{

protected:
    Object(ObjectPrivate &dd);

public:
    virtual ~Object() = default;

    virtual void prepare(InterfaceBuilder &builder, const XMLHelper &helper) override;

private:
    Q_DECLARE_PRIVATE(Object)

}; // class Object

} // namespace InodeSwissKnife

} // namespace GenICam

} // namespace Jgv

#endif // INODESWISSKNIFE_H
