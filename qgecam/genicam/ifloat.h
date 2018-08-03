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

#ifndef IFLOAT_H
#define IFLOAT_H

#include "iinterface.h"

namespace Jgv {

namespace GenICam {

namespace Float {

class Interface : public GenICam::Interface
{
public:
    virtual ~Interface() = default;

    virtual double getValue() = 0;
    virtual void setValue(double value) = 0;

    virtual double getMin() const;
    virtual double getMax() const;
    virtual double getInc() const;

    Type interfaceType() const override final;
    QString interfaceTypeString() const override final;

}; // class Interface

} // namespace Float

} // namespace GenICam

} // namespace Jgv

#endif // IFLOAT_H
