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

#include "gvsppacket.h"

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <arpa/inet.h>
#endif

using namespace Jgv::Gvsp;

struct Jgv::Gvsp::Header {
    uint16_t status;
    uint16_t blockId;
    uint32_t packetFormat_packetId;
};

//struct GVSP_HEADER_EXT {
//    unsigned char blockId64HighPart[4];
//    unsigned char blockId64LowPart[4];
//    unsigned char packetId32[4];
//};

struct Jgv::Gvsp::DataLeader {
    Header header;
    uint16_t reserved;
    uint16_t payloadType;
    uint32_t timestampHighPart;
    uint32_t timestampLowPart;
};

struct Jgv::Gvsp::DataLeaderImage {
    DataLeader leader;
    uint32_t pixelFormat;
    uint32_t sizeX;
    uint32_t sizeY;
    uint32_t offsetX;
    uint32_t offsetY;
    uint16_t paddingX;
    uint16_t paddingY;
};

struct Jgv::Gvsp::PayloadImage {
    Header header;
    unsigned char data[4];  // Fixed the size arbitrarily (conforming c ++ 2011)
};

struct Jgv::Gvsp::DATATRAILER {
    Header header;
    uint16_t reserved;
    uint16_t payloadType;
};

struct Jgv::Gvsp::DataTrailerImage {
    DATATRAILER trailer;
    uint32_t sizeY;
};

Packet::Packet(const uint8_t *const data, uint32_t size)
    : m_size{size}
{
    m_data.p = data;
}

uint16_t Packet::headerStatus() const
{
    return ntohs(m_data.header->status);
}

uint16_t Packet::headerBlockId() const
{
    return ntohs(m_data.header->blockId);
}

uint8_t Packet::headerPacketFormat() const
{
    return UINT8_C(0xFF) & (ntohl(m_data.header->packetFormat_packetId)>>24);
}

uint32_t Packet::headerPacketId() const
{
    return UINT32_C(0x00FFFFFF) & ntohl(m_data.header->packetFormat_packetId);
}

uint16_t Packet::leaderPayloadType() const
{
    return ntohs(m_data.leader->payloadType);
}

uint32_t Packet::leaderTimestampHigh() const
{
    return ntohl(m_data.leader->timestampHighPart);
}

uint32_t Packet::leaderTimestampLow() const
{
    return ntohl(m_data.leader->timestampLowPart);
}

uint32_t Packet::leaderImagePixelFormat() const
{
    return ntohl(m_data.leaderImage->pixelFormat);
}

uint32_t Packet::leaderImageSizeX() const
{
    return ntohl(m_data.leaderImage->sizeX);
}

uint32_t Packet::leaderImageSizeY() const
{
    return ntohl(m_data.leaderImage->sizeY);
}

uint16_t Packet::leaderImagePaddingX() const
{
    return ntohs(m_data.leaderImage->paddingX);
}

uint16_t Packet::leaderImagePaddingY() const
{
    return ntohs(m_data.leaderImage->paddingY);
}

const uint8_t *Packet::payloadImageData() const
{
    return m_data.payload->data;
}

uint32_t Packet::imageDataSize() const
{
    return m_size - sizeof(Header);
}

uint16_t Packet::trailerPayloadType() const
{
    return ntohs(m_data.trailer->payloadType);
}

uint32_t Packet::trailerImageSizeY() const
{
    return ntohl(m_data.trailerImage->sizeY);
}

