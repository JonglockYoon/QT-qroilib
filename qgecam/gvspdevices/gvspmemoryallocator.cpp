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

#include "gvspmemoryallocator.h"
#include "gvspmemoryallocator_p.h"

#include "gvspdevices.h"
#include "gvsp.h"

#include <iostream>

#define D(Class) Class##Private * const d = d_func()

using namespace Jgv::Gvsp;

MemoryAllocator::MemoryAllocator()
    : d_ptr(new MemoryAllocatorPrivate)
{}

MemoryAllocator::MemoryAllocator(MemoryAllocatorPrivate &dd)
    : d_ptr(&dd)
{
    dd.geometry = {0,0,0};
}

MemoryAllocator::~MemoryAllocator()
{}

uint8_t *MemoryAllocator::allocate(std::size_t size) const
{
    return new uint8_t [size];
}

void MemoryAllocator::push(GvspImage &image)
{
    trash(image.data);
}

void MemoryAllocator::trash(uint8_t *memory) const
{
    delete [] memory;
    memory = nullptr;
}



