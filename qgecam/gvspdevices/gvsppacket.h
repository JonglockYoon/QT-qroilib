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

#ifndef GVSPPACKET_H
#define GVSPPACKET_H

#include <stdint.h>

namespace Jgv
{

namespace Gvsp
{

struct Header;
struct DataLeader;
struct DataLeaderImage;
struct PayloadImage;
struct DATATRAILER;
struct DataTrailerImage;

typedef union {
    const uint8_t *p;
    const Header *header;
    const DataLeader *leader;
    const DataLeaderImage *leaderImage;
    const PayloadImage *payload;
    const DATATRAILER *trailer;
    const DataTrailerImage *trailerImage;
} pData; // union pData

const uint32_t GVSP_HEADER_SIZE = 8;

class Packet final
{
    pData m_data;
    const uint32_t m_size = 0;

public:
    explicit Packet(const uint8_t *const data, uint32_t size);
    Packet(const Packet &) = delete;
    Packet & operator=(const Packet &) = delete;

    uint16_t headerStatus() const ;
    uint16_t headerBlockId() const ;
    uint8_t headerPacketFormat() const ;
    uint32_t headerPacketId() const ;
    uint16_t leaderPayloadType() const ;
    uint32_t leaderTimestampHigh() const ;
    uint32_t leaderTimestampLow() const ;
    uint32_t leaderImagePixelFormat() const ;
    uint32_t leaderImageSizeX() const ;
    uint32_t leaderImageSizeY() const ;
    uint16_t leaderImagePaddingX() const ;
    uint16_t leaderImagePaddingY() const ;
    const uint8_t *payloadImageData() const ;
    uint32_t imageDataSize() const ;

    uint16_t trailerPayloadType() const ;
    uint32_t trailerImageSizeY() const ;

}; // class Packet

} // namespace Gvsp

} // namespace Jgv
#endif // GVSPPACKET_H
