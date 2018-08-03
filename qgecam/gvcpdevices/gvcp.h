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

#ifndef GVCP_H
#define GVCP_H

#include <stdint.h>
#include <string>
#include <type_traits>

#define GVCP_PORT 3956

namespace Jgv {

namespace Gvcp {

enum class Status: uint16_t {
    SUCCESS                                 = 0x0000,
    PACKET_RESEND                           = 0x0100,
    NOT_IMPLEMENTED                         = 0x8001,
    INVALID_PARAMETER                       = 0x8002,
    INVALID_ADDRESS                         = 0x8003,
    WRITE_PROTECT                           = 0x8004,
    BAD_ALIGNMENT                           = 0x8005,
    ACCESS_DENIED                           = 0x8006,
    PACKET_UNAVAILABLE                      = 0x800C,
    DATA_OVERUN                             = 0x800D,
    INVALID_HEADER                          = 0x800E,
    PACKET_NOT_YET_AVAILABLE                = 0x8010,
    PACKET_AND_PREV_REMOVED_FROM_MEMORY     = 0x8011,
    PACKET_REMOVED_FROM_MEMORY              = 0x8012,
    NO_REF_TIME                             = 0x8013,
    PACKET_TEMPORARILY_UNAVAILABLE          = 0x0814,
    STATUS_OVERFLOW                         = 0x0815,
    ACTION_LATE                             = 0x0816,
    SERROR                                   = 0x8FFF
};

template <typename T>
constexpr typename std::underlying_type<T>::type enumType(T enumerator) noexcept
{return static_cast<typename std::underlying_type<T>::type>(enumerator);}



} // namespace Gvcp

} // namespace Jgv

#endif // GVCP_H
