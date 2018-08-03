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

#ifndef REGISTERINODE_P_H
#define REGISTERINODE_P_H

#include "inode_p.h"
#include "pvalue.h"
#include <QVector>

namespace Jgv {

namespace GenICam {

namespace IntSwissKnife {
class Object;
}

namespace InodeRegister {

namespace Endianess {
static const char * BigEndian = "BigEndian";
static const char * LittleEndian = "LittleEndian";
}

namespace Cache {
static const char * NoCache = "NoCache";              // pas de cache
static const char * WriteThrough = "WriteThrough";    // une écriture sur le device implique une écriture sur le cache
static const char * WriteAround = "WriteAround";      // seule la lecture est mise en cache
}

namespace Properties {
static const char * Address = "Address";
static const char * Length = "Length";
static const char * pLength = "pLength";
static const char * AccessMode = "AccessMode";
static const char * pPort = "pPort";
static const char * Cachable = "Cachable";
static const char * PollingTime = "PollingTime";
static const char * pAddress = "pAddress";
static const char * IntSwissKnife = "IntSwissKnife";
static const char * pInvalidator = "pInvalidator";
static const char * pIndex = "pIndex";
static const char * Endianess = "Endianess";
}

namespace Attributes {
static const char * Offset = "Offset";
static const char * pOffset = "pOffset";
}

struct Index
{
    pValueInteger pValue;
    pValueInteger pOffset;
    qint64 value() const { return pValue.value() * pOffset.value(); }
};

class ObjectPrivate : public Inode::ObjectPrivate
{
    using get = qint64 (ObjectPrivate::*) () const;

public:
    ObjectPrivate(QSharedPointer<IPort::Interface> iport)
        : Inode::ObjectPrivate(iport)
    {}
    virtual ~ObjectPrivate() = default;

    QList<qint64> addresses;
    QList<pValueInteger> pAddresses;
    QList<QSharedPointer<IntSwissKnife::Object> > intSwissKnives;
    QList<Index> pIndexes;

    pValueInteger pLength;

    bool canRead = false;
    bool canWrite = false;
    bool readUpdateCache = true;
    bool writeUpdateCache = true;
    bool isLittleEndian = false;

    QString pPort;
    int pollingTime = -1;

    QList<QWeakPointer<Inode::Object> > pInvalidators;
    QVector<quint8> data;
};

} // namespace InodeRegister

} // namespace GenICam

} // namespace Jgv

#endif // REGISTERINODE_P_H
