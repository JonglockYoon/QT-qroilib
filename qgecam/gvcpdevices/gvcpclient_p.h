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

#ifndef GVCPCLIENT_P_H
#define GVCPCLIENT_P_H

#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>

#include "gvcp.h"

#ifdef Q_OS_WIN
#include <ws2tcpip.h>
#include<winsock2.h>
#endif

namespace Jgv {

namespace Gvcp {

enum class State {none, controller, monitor};
struct HeaderCmdHelper;

class ClientPrivate
{
    uint16_t id = 1;
    std::mutex proceedMutex;
public:
#ifdef Q_OS_WIN
    SOCKET sd;
#else
    int sd = -1;
#endif

    uint32_t controllerIP = 0;
    uint32_t deviceIP = 0;
    uint32_t heartbeatTimeout = 500;
    uint32_t timestampDateTimeout = 10000;
    uint64_t timestamp = 0;

    std::unique_ptr<std::thread> timerPtr;
    std::unique_ptr<std::thread> timestampPtr;
    volatile bool heartbeatEnable = true;
    std::mutex mutex;
    std::condition_variable cvHB;

    State state = State::none;

    int cmdRetry = 3;

    bool proceed(HeaderCmdHelper &cmd, uint16_t ackType, bool resetHeartbeat = true);
    void doHeartbeat();

};

} // namespace Gvcp

} // namespace Jgv

#endif // GVCPSERVER_P_H
