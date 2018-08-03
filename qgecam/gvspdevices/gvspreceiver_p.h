/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD                                  *
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

#ifndef GVSPRECEIVER_P_H
#define GVSPRECEIVER_P_H

#include "gvspreceiver.h"
#include "gvspblock.h"
#include "gvspdevices.h"
#include "timestampdate.h"

#include "gvspmemoryallocator.h"
#include "gvsp.h"

#include <map>
//#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <time.h>

#ifdef Q_OS_WIN
#include <ws2tcpip.h>
#include <winsock2.h>
#endif

#include <QObject>
#include <QImage>
#include <QThread>

namespace Jgv {

namespace Gvsp {

using BlocksMap = std::map<unsigned, Block>;

class Packet;
//class MemoryAllocator;
struct BlockDesc;
class Receiver;

struct Clock {
    TimestampSource src = TimestampSource::GEVTransmitter;
    uint64_t UtcOffset = UINT64_C(37000000000);
};

class ThreadLoop : public QThread
{
    Q_OBJECT
public:
    explicit ThreadLoop(ReceiverPrivate *parent = 0);

public:
    std::mutex mutex;
    ReceiverPrivate *p;
    void userStack();

private:
    void run();

signals:

public slots:

};

class ReceiverPrivate
{

public:
    Receiver* q;
    //QImage img;
    //QImage convert( Image &image );

    ReceiverPrivate(MemoryAllocator *allocator) : allocatorPtr(allocator)
    {
    }
    virtual ~ReceiverPrivate() = default;

    const std::unique_ptr<MemoryAllocator> allocatorPtr;

    ThreadLoop *pl = nullptr;

#ifdef Q_OS_WIN
    SOCKET sd;
#else
    int sd = -1;
#endif

    volatile bool brun = true;                  // contrôle la sortie de boucle
    //std::unique_ptr<std::thread> threadPtr;

    std::condition_variable cond_variable;

    Clock clock; // gestion de l'horloge

    Parameters params = {0,0,0,0,SocketType::NoType,TimestampSource::GEVTransmitter,1000000000,true,{0,0,0}};
    ReceiverStatistics statistics = {0,0,0,0,0};

    BlocksMap blocksMap;

    TimestampDate datation;

    void ringStack();

    inline void doBuffer(const uint8_t *buffer, std::size_t size);
    inline void doBlock(const BlockDesc *block);
    inline void handleLeader(const Gvsp::Packet &gvsp);
    inline void handlePayload(const Gvsp::Packet &gvsp);
    inline void handleTrailer(const Gvsp::Packet &gvsp);

    inline uint64_t updateTimestamp(const Packet &gvsp);

    inline void callResend(uint16_t blockId, uint32_t firstId, uint32_t lastId);
    inline void doResend(uint16_t blockId, Block &block, uint32_t packetId);


}; // ReceiverPrivate

} // namespace Gvsp

} // namespace Jgv

#endif // GVSPRECEIVER_P_H
