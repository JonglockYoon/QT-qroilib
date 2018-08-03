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

#ifndef GVSPRECEIVER_H
#define GVSPRECEIVER_H

#include <memory>
#include "pimpl.hpp"
#include <QObject>
#include <QImage>

namespace Jgv
{

namespace Gvsp
{

enum class SocketType
{
    NoType,
    Classic,
    RingBuffer,
};

enum class TimestampSource : int
{
    GEVTransmitter = 0,
    TimestampDate = 1,
    SystemClock = 2
};

struct Geometry
{
    uint32_t width;
    uint32_t height;
    uint32_t pixelFormat;
    inline bool operator !=(const Geometry &other) const  {return (other.width!=width)||(other.height!=height)||(other.pixelFormat!=pixelFormat);}
};

struct GvspImage
{
    Geometry geometry;
    uint64_t timestamp;
    uint8_t *data;
    std::size_t dataSize;
};

struct ReceiverStatistics
{
    uint64_t imagesCount;
    uint64_t imagesLostCount;
    uint64_t segmentsResendCount;
    uint64_t segmentsLostCount;
    uint64_t lastTimestamp;
};

struct Parameters {
    uint32_t receiverIP;
    uint32_t transmitterIP;
    uint16_t receiverPort;
    uint16_t transmitterPort;
    SocketType socketType;
    TimestampSource timestampSrc;
    uint64_t timestampFrequency;
    bool resend;
    Geometry geometry;
};

class MemoryAllocator;
class ReceiverPrivate;
class Receiver : public QObject
{
    Q_OBJECT
signals:
    void processedGvspImage(const QImage &qimg);

public:
    QImage qimg;
    //GvspImage image;
    //std::vector<GvspImage> vecImage;
    void qimage(const GvspImage* image);

public:
    Receiver();
    explicit Receiver(MemoryAllocator *allocator);
    virtual ~Receiver();

    void listenOn(uint32_t bindAddress, uint32_t transAddress);
    void acceptFrom(uint32_t transmitterIP, uint16_t transmitterPort);
    void preallocImages(const Geometry &geometry, uint32_t packetSize);
    void setTimestampSource(TimestampSource source);
    void setTransmitterTimestampFrequency(uint64_t frequency);
    void setResendActive(bool active);
    void pushDatation(uint64_t timestamp, uint64_t dateMin, uint64_t dateMax);

    const Parameters &parameters() const;
    const ReceiverStatistics & statistics() const;

protected:
    Receiver(ReceiverPrivate &dd);
    const std::unique_ptr<ReceiverPrivate> d_ptr;

private:
    inline ReceiverPrivate *d_func() { return d_ptr.get(); }
    inline const ReceiverPrivate *d_func() const { return d_ptr.get(); }

#define MAXQ 20
    int qcnt;
    char *dst[MAXQ];

}; // class Receiver


} // namespace Gvsp

} // namespace Jgv
//Q_DECLARE_METATYPE(Jgv::Gvsp::GvspImage)

#endif // GVSPRECEIVER_H
