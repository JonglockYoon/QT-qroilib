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

#ifndef FORCEIPHELPER_H
#define FORCEIPHELPER_H

#include "headerhelper.h"

namespace Jgv {

namespace Gvcp {

struct FORCEIP_CMD {
    CMD_HEADER header;
    uint16_t reserved1;
    uint16_t MACAddressHigh;
    uint32_t MACAddressLow;
    uint32_t reserved2[3];
    uint32_t staticIP;
    uint32_t reserved3[3];
    uint32_t staticSubnetMask;
    uint32_t reserved4[3];
    uint32_t staticDefaultGateway;
};

struct FORCEIP_ACK {
    ACK_HEADER header;
};

constexpr uint16_t GVCP_CMD_FORCEIP                  = 0x0004;
constexpr uint16_t GVCP_ACK_FORCEIP                  = 0x0005;
constexpr int FORCEIP_CMD_LENGTH = sizeof(FORCEIP_CMD) - sizeof(CMD_HEADER);
constexpr int FORCEIP_ACK_LENGTH = sizeof(FORCEIP_ACK) - sizeof(ACK_HEADER);


class ForceipCmdHelper final : public HeaderCmdHelper
{
    FORCEIP_CMD * const cmd;

public:
    ForceipCmdHelper(uint16_t length);
    virtual ~ForceipCmdHelper() = default;

    void setMacHigh(uint16_t mac);
    void setMacLow(uint32_t mac);
    void setStaticIP(uint32_t ip);
    void setStaticNetmask(uint32_t netmask);
    void setStaticDefaultGateway(uint32_t gateway);

    static uint16_t macHigh(const FORCEIP_CMD &cmd);
    static uint32_t macLow(const FORCEIP_CMD &cmd);
    static uint32_t staticIP(const FORCEIP_CMD &cmd);
    static uint32_t staticNetmask(const FORCEIP_CMD &cmd);
    static uint32_t staticDefaultGateway(const FORCEIP_CMD &cmd);

};

class ForceipAckHelper final : public HeaderAckHelper
{
    FORCEIP_ACK * const ack;

public:
    ForceipAckHelper(uint16_t length);
    virtual ~ForceipAckHelper() = default;
};

} // namespace Gvcp

} // namespace Jgv

#endif // FORCEIPHELPER_H
