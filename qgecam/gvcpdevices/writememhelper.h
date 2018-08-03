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

#ifndef WRITEMEMHELPER_H
#define WRITEMEMHELPER_H

#include "headerhelper.h"

namespace Jgv {

namespace Gvcp {

constexpr uint16_t GVCP_CMD_WRITEMEM                 = 0x0086;
constexpr uint16_t GVCP_ACK_WRITEMEM                 = 0x0087;

struct WRITEMEM_CMD {
    CMD_HEADER header;
    uint32_t address;
    uint8_t data[4]; // taille arbitraire pour conformité c++11
};

struct WRITEMEM_ACK {
    ACK_HEADER header;
    uint16_t reserved;
    uint16_t index;
};

class WritememCmdHelper final : public HeaderCmdHelper
{
    WRITEMEM_CMD * const cmd;

public:
    WritememCmdHelper(uint16_t length);
    virtual ~WritememCmdHelper() = default;

    void setAddress(uint32_t address);
    void setCmdData(const uint8_t *datas, uint32_t size);

    static uint32_t address(const WRITEMEM_CMD &cmd);
    static uint16_t count(const WRITEMEM_CMD &cmd);
    static const uint8_t *cmdData(const WRITEMEM_CMD &cmd);

};

constexpr int WRITEMEM_ACK_LENGTH = sizeof(WRITEMEM_ACK) - sizeof(ACK_HEADER);

class WritememAckHelper final : public HeaderAckHelper
{
    WRITEMEM_ACK * const ack;

public:
    WritememAckHelper(uint16_t lenght);
    virtual ~WritememAckHelper() = default;

    void setIndex(uint16_t index);

};

} // namespace Gvcp

} // namespace Jgv

#endif // WRITEMEMHELPER_H
