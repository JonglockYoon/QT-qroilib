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

#include "writereghelper.h"

#ifdef Q_OS_WIN
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

using namespace Jgv::Gvcp;

WriteregCmdHelper::WriteregCmdHelper(uint16_t length)
    : HeaderCmdHelper(GVCP_CMD_WRITEREG, length),
      cmd(reinterpret_cast<WRITEREG_CMD * const>(data))
{}

void WriteregCmdHelper::setRegsValList(const AddrValPairList &values)
{
    for (std::size_t i=0; i<values.size(); ++i) {
        cmd->address_value[i].address = htonl(values.at(i).address);
        cmd->address_value[i].value = htonl(values.at(i).value);
    }
}

WriteregAckHelper::WriteregAckHelper(uint16_t lenght)
    : HeaderAckHelper(GVCP_ACK_WRITEREG, lenght),
      ack(reinterpret_cast<WRITEREG_ACK * const>(data))
{}

void WriteregAckHelper::setIndex(uint16_t index)
{
    ack->index = htons(index);
}

