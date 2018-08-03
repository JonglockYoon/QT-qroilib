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

#include "readmemhelper.h"

#ifdef Q_OS_WIN
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

using namespace Jgv::Gvcp;

ReadmemCmdHelper::ReadmemCmdHelper(uint32_t length)
    : HeaderCmdHelper(GVCP_CMD_READMEM, length),
      cmd(reinterpret_cast<READMEM_CMD * const>(data))
{}

void ReadmemCmdHelper::setAddress(uint32_t address)
{
    cmd->address = htonl(address);
}

void ReadmemCmdHelper::setDataCount(uint16_t count)
{
    cmd->count = htons(count);
}

uint32_t ReadmemCmdHelper::address(const READMEM_CMD &cmd)
{
    return ntohl(cmd.address);
}

uint16_t ReadmemCmdHelper::dataCount(const READMEM_CMD &cmd)
{
    return ntohs(cmd.count);
}

ReadmemAckHelper::ReadmemAckHelper(uint16_t lenght)
    : HeaderAckHelper(GVCP_ACK_READMEM, lenght),
      ack(reinterpret_cast<READMEM_ACK * const>(data))
{}

void ReadmemAckHelper::setAddress(uint32_t address)
{
    ack->address = htonl(address);
}

void ReadmemAckHelper::addMem(const uint8_t *mem, uint16_t size)
{
    std::memcpy(ack->datas, mem, size);
}

uint32_t ReadmemAckHelper::address(const READMEM_ACK &ack)
{
    return ntohl(ack.address);
}

const uint8_t *ReadmemAckHelper::dataPtr(const READMEM_ACK &ack)
{
    return ack.datas;
}





