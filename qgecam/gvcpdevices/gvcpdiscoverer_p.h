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

#ifndef GVCPDISCOVERER_P_H
#define GVCPDISCOVERER_P_H

#include "discoveryhelper.h"

#include <list>
#include <thread>
#include <QList>

#ifdef Q_OS_WIN
#include <ws2tcpip.h>
#include<winsock2.h>
#endif

namespace Jgv {

namespace Gvcp {

union Packet {
    char buffer[sizeof(DISCOVERY_ACK)];
    ACK_HEADER headerAck;
    DISCOVERY_ACK discoveryAck;
};

class GvcpDiscovererPrivate
{
public:
    void sendFrom(int sd, const PacketHelper &cmd, uint32_t from, uint32_t to);

    uint16_t id = 0;

#ifdef Q_OS_WIN
    SOCKET sd;
#else
    int sd = -1;
#endif

    uint32_t srcIP = 0;
    std::unique_ptr<std::thread> threadPtr;
    std::list<std::weak_ptr<Discoverer::IListener> > listeners;

    volatile bool listen = true;
    void listenSocket();

};

} // namespace Gvcp

} // namespace Jgv

#endif // GVCPDISCOVERER_P_H
