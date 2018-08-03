/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD                             *
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

#ifndef GVSPRECEIVERALLOCATOR_H
#define GVSPRECEIVERALLOCATOR_H


#include <memory>
#include "gvspreceiver.h"

namespace Jgv
{

namespace Gvsp
{

class MemoryAllocatorPrivate;
class MemoryAllocator
{
public:
    MemoryAllocator();
protected:
    MemoryAllocator(MemoryAllocatorPrivate &dd);
public:
    virtual ~MemoryAllocator();

    virtual uint8_t *allocate(std::size_t size) const ;
    virtual void push(GvspImage &image);
    virtual void trash(uint8_t *memory) const ;

protected:
    const std::unique_ptr<MemoryAllocatorPrivate> d_ptr;
private:
    inline MemoryAllocatorPrivate *d_func() { return d_ptr.get(); }
    inline const MemoryAllocatorPrivate *d_func() const { return d_ptr.get(); }
};

} // namespace Gvsp

} // namespace Jgv

#endif // GVSPRECEIVERALLOCATOR_H
