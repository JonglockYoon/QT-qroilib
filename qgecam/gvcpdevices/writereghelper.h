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

#ifndef WRITEREGHELPER_H
#define WRITEREGHELPER_H

#include "headerhelper.h"
#include <vector>

namespace Jgv {

namespace Gvcp {

constexpr uint16_t GVCP_CMD_WRITEREG                 = 0x0082;
constexpr uint16_t GVCP_ACK_WRITEREG                 = 0x0083;

enum class BootstrapAddress: uint32_t;

struct ADDRESS_VALUE {
    uint32_t address;
    uint32_t value;
};

struct WRITEREG_CMD {
    CMD_HEADER header;
    ADDRESS_VALUE address_value[1]; // taille arbitraire pour conformité c++11
};

struct WRITEREG_ACK {
    ACK_HEADER header;
    uint16_t reserved;
    uint16_t index;
};

constexpr std::size_t WRITEREG_ACK_LENGTH = sizeof(WRITEREG_ACK) - sizeof(ACK_HEADER);
using AddrValPairList = std::vector<ADDRESS_VALUE>;

class WriteregCmdHelper final : public HeaderCmdHelper
{
    WRITEREG_CMD * const cmd;

public:
    WriteregCmdHelper(uint16_t length);
    virtual ~WriteregCmdHelper() = default;

    void setRegsValList(const AddrValPairList &values);

    //static RegistersValueList regsValList(const WRITEREG_CMD &cmd);
};

class WriteregAckHelper final : public HeaderAckHelper
{
    WRITEREG_ACK * const ack;

public:
    WriteregAckHelper(uint16_t lenght);
    virtual ~WriteregAckHelper() = default;

    void setIndex(uint16_t index);

};

} // namespace Gvcp

} // namespace Jgv

#endif // WRITEREGHELPER_H
