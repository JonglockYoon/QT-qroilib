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

#include "readreghelper.h"

#ifdef Q_OS_WIN
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

using namespace Jgv::Gvcp;

ReadregCmdHelper::ReadregCmdHelper(uint16_t length)
    : HeaderCmdHelper(GVCP_CMD_READREG, length),
      cmd(reinterpret_cast<READREG_CMD * const>(data))
{}

void ReadregCmdHelper::setAddresses(const std::vector<uint32_t> &addresses)
{
    for (std::size_t i=0; i < addresses.size(); ++i) {
        cmd->addresses[i] = htonl(addresses.at(i));
    }
}


std::vector<uint32_t> ReadregCmdHelper::addresses(const READREG_CMD &cmd)
{
    std::vector<uint32_t> result;
    for (uint16_t i = 0; i < (HeaderCmdHelper::length(cmd.header)/4); ++i) {
        result.push_back(ntohl(cmd.addresses[i]));
    }
    return result;
}


ReadregAckHelper::ReadregAckHelper(uint16_t lenght)
    : HeaderAckHelper(GVCP_ACK_READREG, lenght),
      ack(reinterpret_cast<READREG_ACK * const>(data))
{}

void ReadregAckHelper::setAnswers(const std::vector<uint32_t> &answers)
{
    for (std::size_t i=0; i<answers.size(); ++i) {
        ack->registerData[i] = htonl(answers.at(i));
    }
}

std::vector<uint32_t> ReadregAckHelper::aswers(const READREG_ACK &ack)
{
    std::vector<uint32_t> result;
    for (uint16_t i = 0; i < (HeaderAckHelper::length(ack.header)/4); ++i) {
        result.push_back(ntohl(ack.registerData[i]));
    }
    return result;
}




