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

#ifndef HEADERHELPER_H
#define HEADERHELPER_H

#include "packethelper.h"
#include <stdint.h>
#include <string>

namespace Jgv {

namespace Gvcp {

constexpr uint8_t GVCP_HARD_KEY = 0x42;

typedef struct CMD_HEADER
{
    uint8_t hardKey;
    uint8_t flag;
    uint16_t command;
    uint16_t length;
    uint16_t reqId;
} CMD_HEADER;

typedef struct ACK_HEADER
{
    uint16_t status;
    uint16_t acknowledge;
    uint16_t length;
    uint16_t ackId;
} ACK_HEADER;

class HeaderCmdHelper : public PacketHelper
{
    CMD_HEADER *const header;
public:
    HeaderCmdHelper(uint16_t command, uint16_t length);
    virtual ~HeaderCmdHelper() = default;

    void setAcknowledge(bool ack);
    bool acknowledge() const;
    void setReqId(uint16_t id);
    uint16_t reqId() const;

    static bool isValid(const CMD_HEADER &header, uint16_t packetSize);
    static bool acknowledgeIsSet(const CMD_HEADER &header);
    static uint16_t command(const CMD_HEADER &header);
    static uint16_t length(const CMD_HEADER &header);
    static uint16_t reqId(const CMD_HEADER &header);
};

class HeaderAckHelper : public PacketHelper
{
    ACK_HEADER *const header;
public:
    HeaderAckHelper(uint16_t acknowledge, uint16_t length);
    virtual ~HeaderAckHelper() = default;

    void changeStatus(uint16_t newStatus);
    void setAckID(uint16_t id);

    static bool isValid(const ACK_HEADER &header, uint16_t packetSize);
    static uint16_t status(const ACK_HEADER &header);
    static uint16_t acknowledge(const ACK_HEADER &header);
    static uint16_t length(const ACK_HEADER &header);
    static uint16_t ackId(const ACK_HEADER &header);
};

} // namespace Gvcp

} // namespace Jgv

#endif // HEADERHELPER_H
