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

#ifndef ICATEGORY_H
#define ICATEGORY_H

#include "iinterface.h"
#include <QList>
#include <QWeakPointer>

namespace Jgv {

namespace GenICam {

namespace Inode {class Object;}

namespace Category {

class Interface : public GenICam::Interface
{
public:
    virtual ~Interface() = default;
    virtual QList<QWeakPointer<Inode::Object> >  getFeatures() const = 0;

    Type interfaceType() const override final;
    QString interfaceTypeString() const override final;
};

} // namespace Category

} // namespace GenICam

} // namespace Jgv

#endif // IINTEGER_H
