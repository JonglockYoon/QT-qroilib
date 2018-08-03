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

#include "gvspblock.h"
#include "gvsp.h"

#include <cstring>
#include <iostream>

using namespace Jgv::Gvsp;

struct Jgv::Gvsp::PtrInfos
{
    uint8_t *ptr = nullptr;
    uint32_t size = 0;
    SegmentState state = SegmentState::empty;
};

Block::Block()
    //: m_image {0,0,0, 0, nullptr, 0}
{
    m_image = {0,0,0, 0, nullptr, 0};
}

Block::~Block()
{}

/*!
 * \brief Block::image
 * Fournit une référence modifiable sur les données d'image.
 *
 * \return Une référence modifiable sur Jgv::Gvsp::Image
 */
GvspImage &Block::image()
{
    return m_image;
}

/*!
 * \brief Block::image
 * Fournit une référence constante sur les données d'image.
 *
 * \return Une référence non modifiable sur Jgv::Gvsp::Image
 */
const GvspImage & Block::image() const
{
    return m_image;
}

/*!
 * \brief Block::geometry
 * Fournit une référence constante sur les données de géomtrie de l'image.
 * \return Une référence non modifiable sur Jgv::Gvsp::Geometry
 */
const Geometry &Block::geometry() const
{
    return m_image.geometry;
}

/*!
 * \brief Block::segmentSize
 * Obtient la taille du segment d'image courant.
 * \return La taille du segment image.
 */
uint32_t Block::segmentSize() const
{
    return (m_memoryMapping.empty()) ? 0 : m_memoryMapping.front().size;
}

/*!
 * \brief GvspBlock::endID
 * \return Le numéro de segment suivant le dernier numéro de segment valide.
 */
uint32_t Block::endID() const
{
    return static_cast<uint32_t>(m_memoryMapping.size()) + 1;
}

/*!
 * \brief Block::changeGeometry
 * Change la géométrie de l'image portée par le bloc.
 * Si le changement est effectif, invalide la cartographie mémoire.
 *
 * \param geometry La nouvelle géométrie de l'image.
 * \return Vrai si la géométrie a effectivement été changé.
 */
bool Block::changeGeometry(const Geometry &geometry)
{
    if (m_image.geometry != geometry) {
        m_image.geometry = geometry;
        m_image.dataSize = (geometry.width * geometry.height * GVSP_PIX_PIXEL_SIZE(geometry.pixelFormat)) / 8;
        m_memoryMapping.clear();
        return true;
    }
    return false;
}

/*!
 * \brief Block::mapMemory
 * Construit la cartographie de la mémoire allouée à l'image.
 * Chaque index de segment se voit affecter un pointeur sur la mémoire image.
 *
 * \param segmentSize La taille de chaque segment de mémoire.
 */
void Block::mapMemory(uint32_t segmentSize)
{
    if (segmentSize == 0) {
        std::cerr << "GvspBlock failed to map memory, segmentSize = 0" << std::endl;
        return;
    }

    if (m_image.data == nullptr) {
        std::cerr << "GvspBlock failed to map memory on nullptr memory" << std::endl;
        return;
    }

    // calcul de la taille du dernier segment
    const uint32_t lastSize = m_image.dataSize % segmentSize;

    // calcul du nombre de segments
    const std::size_t segmentsCount = (lastSize==0) ? m_image.dataSize/segmentSize : 1 + m_image.dataSize/segmentSize;

    // contruit la cartographie
    m_memoryMapping.resize(segmentsCount);

     uint8_t *p = m_image.data;
    // initialise la cartographie mémoire
    for (std::size_t index = 0; index < segmentsCount; ++index) {
        m_memoryMapping[index].ptr = p;
        m_memoryMapping[index].size = segmentSize;
        p += segmentSize;
    }

    // on corrige la taille du dernier segment
    m_memoryMapping.back().size = lastSize==0 ? segmentSize : lastSize;

    // met à jour les statistiques
    segmentState.empty = segmentsCount;
    segmentState.askedAgain = 0;
}

void Block::insertSegment(unsigned packetId, const uint8_t * const segment, uint32_t segmentSize)
{
    // rejette les segments hors plage
    if (packetId == 0) {
        std::clog << "GvspBlock cant insert packet Id 0" << std::endl;
        return;
    }

    // l'initialisation n'a pas encore eu lieu, on map la mémoire
    if (m_memoryMapping.size() == 0) {
        mapMemory(segmentSize);
        if (m_memoryMapping.empty()) {
            std::cerr << "GvspBlock can't insert segment on empty memory map" << std::endl;
            return;
        }
    }

    // si le numéro de segment est en dehors de la plage, on rejette
    if (packetId > m_memoryMapping.size()) {
        std::cerr << "GvspBlock: segnum out of range(1, " << m_memoryMapping.size() << ") " << packetId << std::endl;
        return;
    }

    PtrInfos &ptrInfos = m_memoryMapping[packetId - 1];

    // on rejette les segments dont la taille ne corresponds pas à la carte calculée
    if (segmentSize != ptrInfos.size) {
        // si on a affaire au dernier segment, il est possible que la caméra fait du bourrage,
        // du coup il peut être égal à la taille des autres segments
        if (segmentSize != m_memoryMapping.front().size) {
            std::cerr << "GvspBlock: segmentSize != mapped size " << segmentSize << " " << ptrInfos.size << std::endl;
            return;
        }
    }

    // on copie seulement si cela n'a pas déjà été fait.
    if (ptrInfos.state != SegmentState::filled) {
        std::memcpy(ptrInfos.ptr, segment, ptrInfos.size);
        ptrInfos.state = SegmentState::filled;
        // mets à jour les statistiques
        segmentState.empty--;
    }
}

void Block::setLeaderIsFilled()
{
    m_leaderState = SegmentState::filled;
}

bool Block::leaderIsFilled() const
{
    return m_leaderState == SegmentState::filled;
}

void Block::setLeaderAskedAgain()
{
    m_leaderState = SegmentState::askedAgain;
}

void Block::setSegmentsAskedAgain(std::size_t startID, std::size_t stopID)
{

    // on convertit les numeros de segments en index
    for (std::size_t index = startID - 1; index <= stopID -1; ++index) {
        m_memoryMapping[index].state = SegmentState::askedAgain;
        // met à jourles statistiques
        segmentState.askedAgain++;
    }
}

int Block::emptySegmentsCount(unsigned packetId) const
{
    // index du segment précédent packedId
    int index = packetId - 2;

    if (index < 0) {
        return 0;
    }

    int count = 0;

    for (; index >= 0; --index) {
        if (m_memoryMapping[index].state == SegmentState::empty)
            count++;
        else break;
    }
    return count;
}


//void Block::dump() const
//{
//    std::clog << "# DUMP #" << std::endl;
//    for (int i=0; i<m_memoryMapping.size(); ++i) {
//        std::clog << i << " " << (int)m_memoryMapping[i].state << std::endl;
//    }
//    std::clog << "########" << std::endl;
//}








