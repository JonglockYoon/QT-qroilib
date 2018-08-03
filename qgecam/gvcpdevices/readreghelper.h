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

#ifndef READREGHELPER_H
#define READREGHELPER_H


#include "headerhelper.h"
#include <vector>

namespace Jgv {

namespace Gvcp {

constexpr uint16_t GVCP_CMD_READREG                  = 0x0080;
constexpr uint16_t GVCP_ACK_READREG                  = 0x0081;

struct READREG_CMD {
    CMD_HEADER header;
    uint32_t addresses[1]; // taille arbitraire pour conformité c++11
};

struct READREG_ACK {
    ACK_HEADER header;
    uint32_t registerData[1]; // taille arbitraire pour conformité c++11
};

class ReadregCmdHelper final : public HeaderCmdHelper
{
    READREG_CMD * const cmd;

public:
    ReadregCmdHelper(uint16_t length);
    virtual ~ReadregCmdHelper() = default;

    void setAddresses(const std::vector<uint32_t> &addresses);
    static std::vector<uint32_t> addresses(const READREG_CMD &cmd);
};

class ReadregAckHelper final : public HeaderAckHelper
{
    READREG_ACK * const ack;

public:
    ReadregAckHelper(uint16_t lenght);
    virtual ~ReadregAckHelper() = default;

    void setAnswers(const std::vector<uint32_t> &answers);
    static std::vector<uint32_t> aswers(const READREG_ACK &ack);
};

} // namespace Gvcp

} // namespace Jgv

#endif // READREGHELPER_H
