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

#include <iostream>
#include <memory>

#include "gvcpdiscoverer.h"
#include "gvcpdiscoverer_p.h"

#include "gvcp.h"
#include "forceiphelper.h"
#include "discoveryhelper.h"

#include <QtWidgets>

#ifndef Q_OS_WIN
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>
//#include <pthread.h>
//#include <poll.h>
//#include <arpa/inet.h>
#endif

using namespace Jgv::Gvcp;

#define D(Class) Class##Private * const d = d_func()

void GvcpDiscovererPrivate::sendFrom(int sd, const PacketHelper &cmd, uint32_t from, uint32_t to)
{
    sockaddr_in dest = {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(GVCP_PORT); //GVCP port
    //if (to == INADDR_BROADCAST)
    //    dest.sin_addr.s_addr = htonl(INADDR_ANY);
    //else
        dest.sin_addr.s_addr = htonl(to);
#ifdef Q_OS_WIN
    int destlen = sizeof(dest);

    int sendlen = sendto(sd, (const char *)&cmd.data, cmd.size, 0, (struct sockaddr*)&dest, destlen);
    // l'envoi n'est pas passé
    if (sendlen < 0) {
        qDebug() << "GvcpDiscovererPrivate sendto failed";
    }
#else
    iovec iov = {const_cast<char *>(cmd.data), cmd.size};

    // construction du tampon des données de service
    char buffer[CMSG_SPACE(sizeof(in_pktinfo))];
    memset(&buffer, 0, sizeof(buffer));

    // construction du message
    msghdr msg;
    msg.msg_name = &dest;
    msg.msg_namelen = sizeof(dest);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = &buffer;
    msg.msg_controllen = sizeof(buffer);
    msg.msg_flags = 0;

    // le pointeur sur les données de service
    cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_IP;
    cmsg->cmsg_type = IP_PKTINFO;
    cmsg->cmsg_len = CMSG_LEN(sizeof(in_pktinfo));
    in_pktinfo *pktinfo = reinterpret_cast<in_pktinfo *>(CMSG_DATA(cmsg));
    pktinfo->ipi_ifindex = 0;
    pktinfo->ipi_spec_dst.s_addr = htonl(from); // on ne traite que les paquets vers notre controleur

    if (sendmsg(sd, &msg, 0) < 0) {
        std::perror("GvcpDiscoverer::discover sendmsg");
    }
#endif
}

void GvcpDiscovererPrivate::listenSocket()
{

    int size = 0;
    QTime time;

    struct sockaddr_in from;
    socklen_t fromSize = sizeof(from);

    time.start();
    while (listen) {
        Packet packet;

        size = recvfrom(sd, packet.buffer, sizeof(packet.buffer), 0, (struct sockaddr *) &from, &fromSize);

        if (size >= 0)
        {
            time.start();

            qDebug() << "Received datagram: " << size;

            if (HeaderAckHelper::status(packet.headerAck) != enumType(Status::SUCCESS)) {
                qDebug() << "GvcpDiscovererPrivate::listenSocket() : Status unsucces";// << std::endl;
                continue;
            }

            if (HeaderAckHelper::ackId(packet.headerAck) != id) {
                continue;
            }

            if (HeaderAckHelper::acknowledge(packet.headerAck) == GVCP_ACK_DISCOVERY) {
                if ( (HeaderAckHelper::length(packet.headerAck) != DISCOVERY_ACK_LENGTH) || (size != sizeof(DISCOVERY_ACK)) ) {
                    std::clog << "GvcpDiscovererPrivate::listenSocket() : Discover mal formatted packet !" << std::endl;
                    continue;
                }

                auto it = listeners.begin();
                auto end = listeners.end();

                while (it != end) {
                    auto listener = (*it).lock();
                    if (listener) {
                        listener->deviceDiscovered(packet.discoveryAck);
                        ++it;
                    }
                    else {
                        it = listeners.erase(it);
                    }
                }
            }
            else if (HeaderAckHelper::acknowledge(packet.headerAck) == GVCP_ACK_FORCEIP) {
                if ( (HeaderAckHelper::length(packet.headerAck) != FORCEIP_ACK_LENGTH) || (size != sizeof(FORCEIP_ACK)) ) {
                    qDebug() << "GvcpDiscovererPrivate::listenSocket() : Discover mal formatted packet !";// << std::endl;
                    continue;
                }

                auto it = listeners.begin();
                auto end = listeners.end();

                while (it != end) {
                    auto listener = (*it).lock();
                    if (listener) {
                        listener->forceIpSucces();
                        ++it;
                    }
                    else {
                        it = listeners.erase(it);
                    }
                }
            }

        }

    }

    //std::clog << "GvcpDiscover stops listening on " << saddr << std::endl;
}

GvcpDiscoverer::GvcpDiscoverer()
    : d_ptr(new GvcpDiscovererPrivate)
{
}

GvcpDiscoverer::~GvcpDiscoverer()
{
    stop();
}

bool GvcpDiscoverer::listen(uint32_t srcIP)
{
    D(GvcpDiscoverer);


#ifdef Q_OS_WIN
    WSADATA wsaData;    // Windows socket

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsaData) == SOCKET_ERROR) {
        qDebug() << "WinSock Error";
        return false;
    }
#endif

    //if (d->threadPtr && d->threadPtr->joinable() && d->srcIP == srcIP) {
//        in_addr addr = {};
//        addr.s_addr = htonl(srcIP);
//        char saddr[INET_ADDRSTRLEN];
//        inet_ntop(AF_INET, &addr.s_addr, saddr, INET_ADDRSTRLEN);

//        std::clog << "GvcpDiscoverer: allready listening " << saddr << std::endl;
    //    return true;
    //}

    d->srcIP = srcIP;

#ifndef Q_OS_WIN
    d->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
    d->sd = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
#endif
    if(d->sd < 0)
    {
        qDebug() << "GvcpDiscoverer: failed to create socket";
        return false;
    }

    // ancillary data
    int on = 1;
#ifndef Q_OS_WIN
    setsockopt(d->sd, SOL_IP, IP_PKTINFO, &on, sizeof(on));
    // autorise le broadcast
    setsockopt(d->sd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
#else
    if (setsockopt(d->sd, IPPROTO_IP, IP_PKTINFO, (const char *)&on, sizeof(on)) == SOCKET_ERROR) {
        qDebug() << "Setsockopt fail - IPPROTO_IP IP_PKTINFO";
    }
    if (setsockopt(d->sd, SOL_SOCKET, SO_BROADCAST, (const char *)&on, sizeof(on)) == SOCKET_ERROR) {
        qDebug() << "Setsockopt fail - SOL_SOCKET SO_BROADCAST";
    }
#endif

    //Prepare the sockaddr_in structure
    sockaddr_in localAddress = {};
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(0);   // port aléatoire
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY); // écoute sur tous les NIC

    //Bind
    if( bind(d->sd ,(struct sockaddr *)&localAddress , sizeof(localAddress)) < 0)
    {
        qDebug() << "GvcpDiscoverer: failed to bind socket";
        return false;
    }

    // socket non bloquant
#ifndef Q_OS_WIN
    if (fcntl(d->sd, F_SETFL, O_NONBLOCK) < 0) {
        qDebug() << "GvcpDiscoverer: failed to ioctlsocket nonblock";
        return false;
    }
#else
    unsigned long arg = 1;
    if (ioctlsocket(d->sd, FIONBIO, &arg) < 0) {
        qDebug() << "GvcpDiscoverer: failed to ioctlsocket nonblock";
        return false;
    }
#endif

    // Our descriptor listens, we start the polling thread
    d->listen = true;
    d->threadPtr = std::unique_ptr<std::thread>(new std::thread(&GvcpDiscovererPrivate::listenSocket, d));
    //pthread_setname_np(d->threadPtr->native_handle(), "GvcpDiscoverer");


    return true;
}

void GvcpDiscoverer::stop()
{
    D(GvcpDiscoverer);

    if (d->threadPtr && d->threadPtr->joinable()) {
        d->listen = false;
        d->threadPtr->join();
    }
    if (d->sd >= 0) {
#ifdef Q_OS_WIN
        closesocket(d->sd);
#else
        close(d->sd);
#endif
        d->sd = -1;
    }
}

void GvcpDiscoverer::discover(uint32_t peerIP)
{
    D(GvcpDiscoverer);

    d->id = (d->id == 0xFFFFu) ? 1 : d->id + 1;
    DiscoveryCmdHelper cmd(DISCOVERY_CMD_LENGTH);
    cmd.allowBroadcastAck(peerIP == INADDR_BROADCAST);
    cmd.setReqId(d->id);

    d->sendFrom(d->sd, cmd, d->srcIP, peerIP);
}

void GvcpDiscoverer::forceIP(uint64_t mac, uint32_t newIP, uint32_t newNetmask, uint32_t newGateway)
{
    D(GvcpDiscoverer);

    d->id = (d->id == 0xFFFF) ? 1 : d->id + 1;
    ForceipCmdHelper cmd(FORCEIP_CMD_LENGTH);
    cmd.setReqId(d->id);
    cmd.setMacHigh(0xFFFF & (mac>>32));
    cmd.setMacLow(0xFFFFFFFF & mac);
    cmd.setStaticIP(newIP);
    cmd.setStaticNetmask(newNetmask);
    cmd.setStaticDefaultGateway(newGateway);

    d->sendFrom(d->sd, cmd, d->srcIP, INADDR_BROADCAST);

}

void GvcpDiscoverer::addListener(std::weak_ptr<Discoverer::IListener> listener)
{
    D(GvcpDiscoverer);

    d->listeners.push_front(listener);
}


