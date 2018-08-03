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

#include "writememhelper.h"

#ifdef Q_OS_WIN
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

using namespace Jgv::Gvcp;

WritememCmdHelper::WritememCmdHelper(uint16_t length)
    : HeaderCmdHelper(GVCP_CMD_WRITEMEM, length),
      cmd(reinterpret_cast<WRITEMEM_CMD * const>(data))
{}

void WritememCmdHelper::setAddress(uint32_t address)
{
    cmd->address = htonl(address);
}

void WritememCmdHelper::setCmdData(const uint8_t *datas, uint32_t size)
{
    std::memcpy(cmd->data, datas, size);
}

uint32_t WritememCmdHelper::address(const WRITEMEM_CMD &cmd)
{
    return ntohl(cmd.address);
}

uint16_t WritememCmdHelper::count(const WRITEMEM_CMD &cmd)
{
    return HeaderCmdHelper::length(cmd.header) - sizeof(cmd.address);
}

const uint8_t *WritememCmdHelper::cmdData(const WRITEMEM_CMD &cmd)
{
    return cmd.data;
}

WritememAckHelper::WritememAckHelper(uint16_t lenght)
    : HeaderAckHelper(GVCP_ACK_WRITEMEM, lenght),
      ack(reinterpret_cast<WRITEMEM_ACK * const>(data))
{}

void WritememAckHelper::setIndex(uint16_t index)
{
    ack->index = htons(index);
}




