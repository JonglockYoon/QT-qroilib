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

#ifndef READMEMHELPER_H
#define READMEMHELPER_H

#include "headerhelper.h"

namespace Jgv {

namespace Gvcp {

constexpr uint16_t GVCP_CMD_READMEM                  = 0x0084;
constexpr uint16_t GVCP_ACK_READMEM                  = 0x0085;

struct READMEM_CMD {
    CMD_HEADER header;
    uint32_t address;
    uint16_t reserved;
    uint16_t count;
};

struct READMEM_ACK {
    ACK_HEADER header;
    uint32_t address;
    uint8_t datas[4]; // taille arbitraire pour conformité c++11
};


constexpr int READMEM_CMD_LENGTH = sizeof(READMEM_CMD) - sizeof(CMD_HEADER);
const std::size_t READMEM_ACK_PAYLOAD_MAX_SIZE = 0x200;

class ReadmemCmdHelper final : public HeaderCmdHelper
{
    READMEM_CMD * const cmd;
public:
    ReadmemCmdHelper(uint32_t length);
    virtual ~ReadmemCmdHelper() = default;

    void setAddress(uint32_t address);
    void setDataCount(uint16_t dataCount);

    static uint32_t address(const READMEM_CMD &cmd);
    static uint16_t dataCount(const READMEM_CMD &cmd);
};


class ReadmemAckHelper final : public HeaderAckHelper
{
    READMEM_ACK * const ack;
public:
    ReadmemAckHelper(uint16_t lenght);
    virtual ~ReadmemAckHelper() = default;

    void setAddress(uint32_t address);
    void addMem(const uint8_t *mem, uint16_t size);

    static uint32_t address(const READMEM_ACK &ack);
    static const uint8_t *dataPtr(const READMEM_ACK &ack);

};

} // namespace Gvcp

} // namespace Jgv

#endif // READMEMHELPER_H
