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

#include <sys/types.h>

#include "gvspreceiver.h"
#include "gvspreceiver_p.h"
#include "gvsppacket.h"

#include "bootstrapregisters.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>


#ifndef Q_OS_WIN
#include <unistd.h>
#include <cap-ng.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <fcntl.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/filter.h>
#include <sys/mman.h>
#include <poll.h>
#include <assert.h>
#include <pthread.h>
#include <vector>
#endif

#include <QtCore>
#include <QDebug>
#include <QDateTime>
#include <QtWidgets>

#define D(Class) Class##Private * const d = d_func()
#define GVSP_BUFFER_SIZE 10000
#define IP_HEADER_SIZE 20
#define UDP_HEADER_SIZE 8
#define PTP_DEVICE "/dev/ptp0"

#include "ColorConversion.h"
#include "bayer.h"

using namespace Jgv::Gvsp;

//enum class PacketFormat:uint8_t
enum PacketFormat
{
    DATA_LEADER = 1,
    DATA_TRAILER = 2,
    DATA_PAYLOAD = 3,
    ALL_IN = 4
};

uint16_t nextBlockId(uint16_t blockID)
{
    return (blockID == UINT16_MAX) ? 1 : ++blockID;
}

uint16_t previousBlockId(uint16_t blockID)
{
    return (blockID == 1) ? UINT16_MAX : --blockID;
}

uint64_t timestamp(uint64_t high, uint64_t low)
{
    return low | (high << 32);
}

inline std::size_t dataSizeFromGeometry(const Geometry &geometry)
{
    return (geometry.width * geometry.height * GVSP_PIX_PIXEL_SIZE(geometry.pixelFormat)) / 8;
}


void setFilter(int sd, uint32_t srcIP, uint32_t srcPort, uint32_t dstIP, uint32_t dstPort)
{
#ifndef Q_OS_WIN
    // tcpdump udp and src host <srcIP> and src port <srcPort> dst host <dstIP> and dst port <dstPort> -dd
    sock_filter bpf_filter[18] = {
        { 0x28, 0, 0, 0x0000000c },
        { 0x15, 15, 0, 0x000086dd },
        { 0x15, 0, 14, 0x00000800 },
        { 0x30, 0, 0, 0x00000017 },
        { 0x15, 0, 12, 0x00000011 },
        { 0x20, 0, 0, 0x0000001a },
        { 0x15, 0, 10, srcIP },
        { 0x28, 0, 0, 0x00000014 },
        { 0x45, 8, 0, 0x00001fff },
        { 0xb1, 0, 0, 0x0000000e },
        { 0x48, 0, 0, 0x0000000e },
        { 0x15, 0, 5, srcPort },
        { 0x20, 0, 0, 0x0000001e },
        { 0x15, 0, 3, dstIP },
        { 0x48, 0, 0, 0x00000010 },
        { 0x15, 0, 1, dstPort },
        { 0x6, 0, 0, 0x00040000 },
        { 0x6, 0, 0, 0x00000000 }
    };

    const sock_fprog bpf_prog = {sizeof(bpf_filter) / sizeof(struct sock_filter), bpf_filter};
    if (setsockopt(sd, SOL_SOCKET, SO_ATTACH_FILTER, &bpf_prog, sizeof(bpf_prog)) != 0) {
        std::perror("GvspSocket failed to set filter");
    }
#endif
}

void setRealtime()
{
#ifndef Q_OS_WIN
    if (capng_have_capability(CAPNG_EFFECTIVE, CAP_SYS_NICE)) {
        sched_param param;
        param.sched_priority = 20;
        if (sched_setscheduler(0, SCHED_FIFO, &param) < 0) {
            std::perror("GvspSocket failed to set socket thread realtime");
        }
    }
#endif

}

void threadToLastCore() {
    const unsigned concurentThreadsSupported = std::thread::hardware_concurrency();
    if (concurentThreadsSupported == 0) {
        std::cerr << "GvspSocket failed to get hardware concurrency" << std::endl;
        return;
    }
}

struct PACKETRESEND_CMD {
    uint8_t hardKey;
    uint8_t flag;
    uint16_t command;
    uint16_t length;
    uint16_t reqId;
    uint16_t streamChannelIndex;
    uint16_t blockId;
    uint32_t firstPacketId;
    uint32_t lastPacketId;
};

void ReceiverPrivate::callResend(uint16_t blockId, uint32_t firstId, uint32_t lastId)
{

    if (sd > 0) {
        PACKETRESEND_CMD resendCmd {
            0x42, 0x00, htons(0x0040),
                    htons(0x000C), htons(0x00FF),
                    htons(0x0000), htons(blockId),
                    htonl(0x00FFFFFF & firstId),
                    htonl(0x00FFFFFF & lastId) };

        sockaddr_in dest = {};
        dest.sin_family = AF_INET;
        dest.sin_port = htons(3956); //GVCP port
        dest.sin_addr.s_addr = htonl(params.transmitterIP);
        int destlen = sizeof(dest);

        int sendlen = sendto(sd, (const char *)&resendCmd, sizeof(resendCmd), 0, (struct sockaddr*)&dest, destlen);
        // l'envoi n'est pas passé
        if (sendlen < 0) {
            std::perror("GvcpClient callresend failed");
        }
    }
}

/*!
  * \ Brief ReceiverPrivate :: doResend
  * Applies the packet request rules for missing packets on block \ a blockId.
  *
  * \ Param blockId The block number on which the rules are to be applied.
  * \ Param block A reference on the block, allowing the updating of statistics.
  * \ Param packetId The packet number from which to apply the rules.
  */
void ReceiverPrivate::doResend(uint16_t blockId, Block &block, uint32_t packetId)
{
    // The number that preceding blocks to reissue
    const int count = block.emptySegmentsCount(packetId);
    if (count>0 && sd>0) {
        uint32_t firstId = packetId - count;
        uint32_t lastId =  packetId - 1;

        callResend(blockId, firstId, lastId);
        // informe le block
        block.setSegmentsAskedAgain(firstId, lastId);
    }
}

void ThreadLoop::userStack()
{
    p->params.socketType = SocketType::Classic;

    // préparation du polling sur le descripteur de socket
    pollfd pfd {p->sd,POLLIN,0};
    sockaddr_in from = {};
    socklen_t fromSize = sizeof(from);
    std::vector<uint8_t> buffer(GVSP_BUFFER_SIZE, 0);


//    union Buffer {
//        char data[GVSP_BUFFER_SIZE];
//    };
//    Buffer buffer = {};
    int read;
//    struct sockaddr_in from;
//    socklen_t fromSize = sizeof(from);

    while (p->brun) {
#ifndef Q_OS_WIN
        int pollResult = poll(&pfd, 1, 200);
        if (pollResult > 0)    // oki, on lit
#endif
        {
            //try to receive some data, this is a blocking call
            if ((read = recvfrom(p->sd, (char *)buffer.data(), GVSP_BUFFER_SIZE, 0, (struct sockaddr *) &from, &fromSize)) < 0)
            {
                qDebug() << "recvfrom() failed with error code";
                break;
            }
            else
            {
                if (p->params.transmitterIP == 0 ||  ntohl(from.sin_addr.s_addr) == p->params.transmitterIP) {
                    if (p->params.transmitterPort == 0 || ntohs(from.sin_port) == p->params.transmitterPort) {
                        p->doBuffer((const uint8_t *)buffer.data(), read);
                    }
                } else {
                    std::clog << "GvspSocket recv: 0 byte readed" << std::endl;
                }

            }
        }
    }
}

ThreadLoop::ThreadLoop(ReceiverPrivate *parent) :
    QThread()
{
    p = (ReceiverPrivate*)parent;
}

void ThreadLoop::run()
{

    // The lock is requested (blocked by the caller)
    std::unique_lock<std::mutex> lock(mutex);

    // UDP socket descriptor

    //Create a socket
#ifndef Q_OS_WIN
    p->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
    p->sd = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
#endif
    if(p->sd < 0)
    {
        qDebug() << "GvspSocket: failed to create socket";
        return;
    }

    //Prepare the sockaddr_in structure
    sockaddr_in localAddress = {};
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(0);   // port aléatoire
    localAddress.sin_addr.s_addr = INADDR_ANY;//htonl(params.receiverIP);

    //Bind
    if( bind(p->sd ,(struct sockaddr *)&localAddress , sizeof(localAddress)) < 0)
    {
        qDebug() << "GvspSocket: failed to bind socket";
        return;
    }

    // on récupère le numéro de port affecté
    struct sockaddr_in bindAddress;
    socklen_t bindAddressSize = sizeof(struct sockaddr_in);
    getsockname(p->sd, reinterpret_cast<sockaddr *>(&bindAddress), &bindAddressSize);
    p->params.receiverPort = ntohs(bindAddress.sin_port);

    // We notify the caller that the socket is mode bind and that we know the port number
    p->cond_variable.notify_all();
    // The latch
    lock.unlock();

    // Sets the affinity on the last cpu
    threadToLastCore();

    // Obtains real-time scheduling
    setRealtime();

    userStack();

#ifdef Q_OS_WIN
    closesocket(p->sd);
#else
    close(p->sd);
#endif

    // purge
    for(auto it = p->blocksMap.begin(); it != p->blocksMap.end(); ++it) {
        p->allocatorPtr->trash((*it).second.image().data);
    }
    p->blocksMap.clear();

    std::clog << "GvspSocket closes listener" << std::endl;
}

void ReceiverPrivate::handleLeader(const Packet &gvsp)
{
    statistics.lastTimestamp = updateTimestamp(gvsp);

    auto pair = blocksMap.emplace(std::piecewise_construct, std::forward_as_tuple(gvsp.headerBlockId()), std::forward_as_tuple());

    // If the insertion is effective, the memory is allocated (the key did not exist)
    if (pair.second) {
        pair.first->second.changeGeometry(Geometry {gvsp.leaderImageSizeX(), gvsp.leaderImageSizeY(), gvsp.leaderImagePixelFormat()});
        pair.first->second.image().data = allocatorPtr->allocate(pair.first->second.image().dataSize);
    }
    // Otherwise it is checked that the geometry already allocated is the correct one
    else if (pair.first->second.changeGeometry(Geometry {gvsp.leaderImageSizeX(), gvsp.leaderImageSizeY(), gvsp.leaderImagePixelFormat()})) {
        // destroy
        allocatorPtr->trash(pair.first->second.image().data);
        // Reallocates with the new data (the mapping will be done to the payload because we do not know the new segment size)
        pair.first->second.image().data = allocatorPtr->allocate(pair.first->second.image().dataSize);
    }

    // The leader has fulfilled his role
    pair.first->second.setLeaderIsFilled();

    // Informs the metadata
    pair.first->second.image().timestamp = statistics.lastTimestamp;

}

void ReceiverPrivate::handlePayload(const Packet &gvsp)
{
    // If the block stack is empty, it is not processed
    if (blocksMap.size() == 0) {
        return;
    }

    // We need the last known geometry
    const Geometry &geometry = blocksMap.crbegin()->second.geometry();

    const unsigned currentId = gvsp.headerBlockId();
    const unsigned packetId = gvsp.headerPacketId();

//    int sz = gvsp.imageDataSize();
//    memcpy(&fixbuff[(packetId-1)*sz], gvsp.payloadImageData(), sz);


    auto pair = blocksMap.emplace(std::piecewise_construct, std::forward_as_tuple(gvsp.headerBlockId()), std::forward_as_tuple());
    // If the insertion is effective, the memory
    if (pair.second) {
        pair.first->second.changeGeometry(geometry);
        pair.first->second.image().data = allocatorPtr->allocate(pair.first->second.image().dataSize);
    }

    // on insert le segment
    pair.first->second.insertSegment(packetId, gvsp.payloadImageData(), gvsp.imageDataSize());

    // Activate the resend policy if enabled
    if (params.resend) {
        // We take care of the previous block if it exists
        const unsigned previousId = previousBlockId(currentId);
        if (blocksMap.count(previousId) > 0) {
            Block &previous = blocksMap.at(previousId);
            // It is since the last Id that we look for the missing segments
            doResend(previousId, previous, previous.endID());
        }

        // we are now dealing with the current block
        // if the leader is not present we ask for a resend
        if (!pair.first->second.leaderIsFilled()) {
            callResend(currentId, 0, 0);
            pair.first->second.setLeaderAskedAgain();
        }
        // The missing segments are searched from the current segment
        doResend(currentId, pair.first->second, packetId);
    }
}

void ReceiverPrivate::handleTrailer(const Packet &gvsp)
{
    const uint16_t currentId = gvsp.headerBlockId();
    const uint16_t prevId = previousBlockId(currentId);
    //qDebug() << "handleTrailer: " << currentId;

    // If the block n-1 exists, it is injected even if it is not full
    if (blocksMap.count(prevId) != 0) {
        Block &block = blocksMap.at(prevId);

        q->qimage(&block.image());
        allocatorPtr->push(block.image());

        statistics.imagesCount++;
        if (params.resend) {
            statistics.segmentsLostCount += block.segmentState.empty;
            statistics.segmentsResendCount += block.segmentState.askedAgain;
        }

        blocksMap.erase(prevId);
    }

    // if the block does not exist, we stop
    // (at the level of the trailer if we still have nothing, it is hard to miss trying to get the missing segments)
    if (blocksMap.count(currentId) == 0) {
        return;
    }

    // The reference on the current block
    Block & block = blocksMap.at(currentId);

    // Saving of block characteristics for the reallocation of the following
    const Geometry geometry = block.geometry();
    const unsigned segmentSize = block.segmentSize();

    // MapSize describes the size that the block map should have
    size_t mapSize = 3;
    // If the block is full, it is consumed
    if (block.segmentState.empty == 0) {

        q->qimage(&block.image());
        // We consume the block
        allocatorPtr->push(block.image());

        statistics.imagesCount++;
        params.geometry = geometry;
        if (params.resend) {
            statistics.segmentsResendCount += block.segmentState.askedAgain;
        }
        blocksMap.erase(currentId);
        // The size of the map will be only 2
        mapSize = 2;
    }
    else if (params.resend) { // si gestion des resends
        doResend(currentId, block, block.endID());
    }

    auto prealloc = [this, &geometry, &segmentSize] (unsigned id) {
        auto next = blocksMap.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
        // If the insertion is effective, the memory is allocated and the map
        if (next.second) {
            next.first->second.changeGeometry(geometry);
            next.first->second.image().data = allocatorPtr->allocate(next.first->second.image().dataSize);
            next.first->second.mapMemory(segmentSize);
        }
    };

    // Preallocates the next 2 images
    const uint16_t nextId = nextBlockId(currentId);
    const uint16_t nextNextId = nextBlockId(nextId);
    prealloc(nextId);
    prealloc(nextNextId);

    //We clean the map
    auto it = blocksMap.begin();
    while (blocksMap.size() > mapSize) {
        if (it->first == currentId || it->first == nextId || it->first == nextNextId/*(mapSize>2)?it->first==nextNextId:true*/) {
            ++it;
        }
        else {
            allocatorPtr->trash(it->second.image().data);
            it = blocksMap.erase(it);

             qDebug() << "image trash: " << statistics.imagesLostCount;

            statistics.imagesLostCount++;
        }
    }
}

uint64_t ReceiverPrivate::updateTimestamp(const Packet &gvsp)
{

    if (clock.src == TimestampSource::GEVTransmitter) {
        uint64_t t = timestamp(gvsp.leaderTimestampHigh(), gvsp.leaderTimestampLow());
        // Applies the time base factor if the clock is not 1 GHz
        if (params.timestampFrequency != 1000000000) {
            t *= static_cast<double>(1000000000./params.timestampFrequency);
        }
         // The camera time is in TAI
        return t - clock.UtcOffset;
    }
    else if (clock.src == TimestampSource::TimestampDate) {
        return datation.getDate(timestamp(gvsp.leaderTimestampHigh(), gvsp.leaderTimestampLow()));
    }

    uint64_t msecs = QDateTime::currentMSecsSinceEpoch();
    return static_cast<uint64_t>(msecs * UINT64_C(10000));

//    timespec start;
//    if (clock_gettime(CLOCK_REALTIME, &start) >= 0) {
//        return static_cast<uint64_t>((start.tv_sec) * UINT64_C(1000000000) + start.tv_nsec);
//    }

    return 0;
}

void ReceiverPrivate::doBuffer(const uint8_t *buffer, std::size_t size)
{
    Packet gvsp(buffer, size);
    const uint16_t status = gvsp.headerStatus();
    if  (status == enumType(Status::Succes)) {
        switch (gvsp.headerPacketFormat()) {
        case (PacketFormat::DATA_PAYLOAD):
            //qDebug() << "ReceiverPrivate doBuffer: Payload: " << size << gvsp.headerPacketId();
            handlePayload(gvsp);
            break;
        case (PacketFormat::DATA_LEADER):
            //qDebug() << "ReceiverPrivate doBuffer: Leader: " << size;
            handleLeader(gvsp);
            break;
        case (PacketFormat::DATA_TRAILER):
            //qDebug() << "ReceiverPrivate doBuffer: Trailer: " << size;
            handleTrailer(gvsp);

            break;
        default:
            qDebug() << "GvspSocket: packet format not handled";// << std::endl;
        }
    }

    else if (status == enumType(Status::Resend)) {
        //blockPtr->insertSegment(packetID, gvsp.imageData(), gvsp.imageDataSize());
        qDebug() << "GvspSocket: resend";// << std::endl;

    }

    else if (status == 0x4100) {
        // This is apparently a PLEORA bug, should be 0x0100 (STATUS_PACKET_RESEND)
        //blockPtr->insertSegment(packetID, gvsp.imageData(), gvsp.imageDataSize());
        qDebug() << "GvspSocket: 0x4100";
    }

    else {
        //qDebug() << "GvspSocket: unsucces " << status;//Status::toString(status);// << std::endl;
    }
}

Receiver::Receiver()
    : d_ptr(new ReceiverPrivate(new MemoryAllocator))
{
    qcnt = 0;
    memset(dst, 0, sizeof(dst));
    //qRegisterMetaType<GvspImage>();
    d_ptr->q = this;

    //image.data = nullptr;
}

Receiver::Receiver(MemoryAllocator *allocator)
    : d_ptr(new ReceiverPrivate(allocator))
{
    qcnt = 0;
    memset(dst, 0, sizeof(dst));
    d_ptr->q = this;
    //image.data = nullptr;
}

Receiver::Receiver(ReceiverPrivate &dd)
    : d_ptr(&dd)
{
    qcnt = 0;
    memset(dst, 0, sizeof(dst));
    d_ptr->q = this;
    //image.data = nullptr;
}

Receiver::~Receiver()
{
    D(Receiver);
    d->brun = false;
    d->pl->exit();

    for (int i=0; i<MAXQ; i++)
    {
        if (dst[i]) free(dst[i]);
    }
}


/*!
 * \ Brief Receiver :: listenOn
 * Puts the receiver in the listening position.
 * \ Param bindAddress The IPV4 address used by the receiver.
 */
void Receiver::listenOn(uint32_t bindAddress, uint32_t transAddress)
{

    D(Receiver);

    // on bloque le mutex

    d->params.resend = false;
    //The receiver's ip
    d->params.receiverIP = bindAddress;

    d->pl = new ThreadLoop(d);
    std::unique_lock<std::mutex> lock(d->pl->mutex);
    d->pl->start(QThread::TimeCriticalPriority);

    // on débloque le mutex et on attends
    d->cond_variable.wait(lock);
}

void Receiver::acceptFrom(uint32_t transmitterIP, uint16_t transmitterPort)
{
    D(Receiver);

    d->params.transmitterIP = transmitterIP;
    d->params.transmitterPort = transmitterPort;

//    if (d->params.socketType == SocketType::RingBuffer) {
//        setFilter(d->sd, d->params.transmitterIP, d->params.transmitterPort, d->params.receiverIP, d->params.receiverPort);
//    }
}

void Receiver::preallocImages(const Geometry &geometry, uint32_t packetSize)
{
    D(Receiver);

    if (packetSize==0) {
        return;
    }

    const int segmentSize = packetSize - IP_HEADER_SIZE - UDP_HEADER_SIZE - GVSP_HEADER_SIZE;
    if (segmentSize < 1) {
        std::clog << "GvspReceiver failed to prealloc images: bad packetSize" << std::endl;
        return;
    }

    auto insert = [d, &geometry, segmentSize] (unsigned id) {
        auto pair = d->blocksMap.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple());
        // If the insertion is effective, the memory is allocated and mapped
        if (pair.second) {
            pair.first->second.changeGeometry(geometry);
            pair.first->second.image().data = d->allocatorPtr->allocate(pair.first->second.image().dataSize);
            pair.first->second.mapMemory(segmentSize);
        }
    };

    insert(1);
    insert(2);

    d->params.geometry = geometry;

}

void Receiver::setTimestampSource(TimestampSource source)
{
    D(Receiver);
    d->clock.src = source;
    d->params.timestampSrc = source;
}

void Receiver::setTransmitterTimestampFrequency(uint64_t frequency)
{
    D(Receiver);

    // on bloque le mutex
    std::unique_lock<std::mutex> lock(d->pl->mutex);
    d->params.timestampFrequency = frequency;
}

void Receiver::setResendActive(bool active)
{
    D(Receiver);
    d->params.resend = active;
    d->statistics.segmentsLostCount = 0;
    d->statistics.segmentsResendCount = 0;
}

void Receiver::pushDatation(uint64_t timestamp, uint64_t dateMin, uint64_t dateMax)
{
    D(Receiver);

    d->datation.push(timestamp, dateMin, dateMax);
}

const Parameters &Receiver::parameters() const
{
    D(const Receiver);
    return d->params;
}

const ReceiverStatistics &Receiver::statistics() const
{
    D(const Receiver);
    return d->statistics;
}


void Receiver::qimage(const GvspImage* image)
{
    const Geometry geometry = image->geometry;
    dc1394color_filter_t first_color = DC1394_COLOR_FILTER_RGGB;
    dc1394bayer_method_t method = DC1394_BAYER_METHOD_BILINEAR;//DC1394_BAYER_METHOD_SIMPLE;

    if (!dst[qcnt])
        dst[qcnt] = (char *)malloc(geometry.width * geometry.height * 3);

    if (geometry.pixelFormat == GVSP_PIX_MONO8) {
        for( int y = 0; y < geometry.height; ++y ){
            // each row pixels data starts at row*width position (if flipped, we count rows from h-1 to 0)
            const int srcRow = y;// : (geometry.height-y-1);
            const int dstRow = y;
            const int widthStep = geometry.width;
            for( int x = 0; x < geometry.width; ++x ){
                const int srcPixel = x+(srcRow*widthStep);
                const int dstPixel = x*3+(dstRow*geometry.width*3);
                // copy pixel value
                dst[qcnt][ dstPixel ] = image->data[ srcPixel ];
                dst[qcnt][ dstPixel+1 ] = image->data[ srcPixel ];
                dst[qcnt][ dstPixel+2 ] = image->data[ srcPixel ];
            }
        }
        qimg = QImage(reinterpret_cast<unsigned char*>(dst[qcnt]), geometry.width, geometry.height,  QImage::Format_RGB888);
        //qDebug() << "MainWindow::updatePlayerGvspUI";
    } else if (geometry.pixelFormat == GVSP_PIX_BAYRG8) {
        first_color = DC1394_COLOR_FILTER_RGGB;
        dc1394_bayer_decoding_8bit((const uint8_t*)image->data, (uint8_t*)dst[qcnt], geometry.width, geometry.height, first_color, method);
        qimg = QImage(reinterpret_cast<unsigned char*>(dst[qcnt]), geometry.width, geometry.height,  QImage::Format_RGB888);
    } else if (geometry.pixelFormat == GVSP_PIX_BAYGR8) {
        first_color = DC1394_COLOR_FILTER_GRBG;
        dc1394_bayer_decoding_8bit((const uint8_t*)image->data, (uint8_t*)dst[qcnt], geometry.width, geometry.height, first_color, method);
        qimg = QImage(reinterpret_cast<unsigned char*>(dst[qcnt]), geometry.width, geometry.height,  QImage::Format_RGB888);
    } else if (geometry.pixelFormat == GVSP_PIX_BAYGB8) {
        first_color = DC1394_COLOR_FILTER_GBRG;
        dc1394_bayer_decoding_8bit((const uint8_t*)image->data, (uint8_t*)dst[qcnt], geometry.width, geometry.height, first_color, method);
        qimg = QImage(reinterpret_cast<unsigned char*>(dst[qcnt]), geometry.width, geometry.height,  QImage::Format_RGB888);
    } else if (geometry.pixelFormat == GVSP_PIX_BAYBG8) {
        first_color = DC1394_COLOR_FILTER_BGGR;
        dc1394_bayer_decoding_8bit((const uint8_t*)image->data, (uint8_t*)dst[qcnt], geometry.width, geometry.height, first_color, method);
        qimg = QImage(reinterpret_cast<unsigned char*>(dst[qcnt]), geometry.width, geometry.height,  QImage::Format_RGB888);
    }
    else return;

    qcnt++;
    if (qcnt >= MAXQ)
        qcnt = 0;
    //if (qcnt == 0)
        emit processedGvspImage((const QImage &)qimg);
}
