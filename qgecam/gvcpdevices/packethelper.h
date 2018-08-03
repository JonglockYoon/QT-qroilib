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

#ifndef PACKETHELPER_H
#define PACKETHELPER_H

#include <cstring>

namespace Jgv {

namespace Gvcp {

const std::size_t GVCP_PACKET_MAX_SIZE = 548;


class PacketHelper
{
public:
    inline PacketHelper(std::size_t size = GVCP_PACKET_MAX_SIZE)
        : size(size)
    {}
    virtual ~PacketHelper() = default;
    const std::size_t size;
    char data[GVCP_PACKET_MAX_SIZE] = {};
};

} // namespace Gvcp

} // namespace Jgv

#endif // PACKETHELPER_H
