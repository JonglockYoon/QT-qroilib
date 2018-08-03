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

#ifndef GVSPBLOCK_H
#define GVSPBLOCK_H

#include "gvspdevices.h"

#include <vector>

namespace Jgv {

namespace Gvsp
{

struct PtrInfos;

enum class SegmentState {empty,askedAgain,filled};

struct BlockStatistics {
    unsigned empty = 0;
    unsigned askedAgain = 0;
};

class Block final
{

public:
    Block();
    ~Block();

    // pas d'affectation
    Block & operator=(const Block &) = delete;

    GvspImage & image() ;
    const GvspImage & image() const ;
    const Geometry & geometry() const ;
    BlockStatistics segmentState;

    uint32_t segmentSize() const ;
    uint32_t endID() const ;

    bool changeGeometry(const Geometry &geometry) ;
    void mapMemory(uint32_t segmentSize) ;

    void insertSegment(unsigned packetId, const uint8_t * const segment, uint32_t segmentSize) ;

    void setLeaderIsFilled()  ;
    bool leaderIsFilled() const ;

    void setLeaderAskedAgain()  ;
    void setSegmentsAskedAgain(std::size_t startID, std::size_t stopID) ;

    int emptySegmentsCount(unsigned packetId) const ;

    //void dump() const;

private:
    std::vector<PtrInfos> m_memoryMapping;  // le tableau des addresses des pointeurs de segments
    GvspImage m_image;                          // structure contenant les données images
    SegmentState m_leaderState = SegmentState::empty;             // l'état du pointeur du Leader GVSP

}; // class Block

} // namespace Gvsp

} // namespace Jgv

#endif // GVSPBLOCK_H
