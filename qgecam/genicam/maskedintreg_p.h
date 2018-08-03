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

#ifndef MASKEDINTREG_P_H
#define MASKEDINTREG_P_H

#include "inoderegister_p.h"

namespace Jgv {

namespace GenICam {

namespace MaskedIntReg {

namespace Properties {
static const char * LSB = "LSB";
static const char * MSB = "MSB";
static const char * Bit = "Bit";
} // Properties

struct Cache {
    qint64 value;
    bool isValid;
    Cache() : isValid(false) {}
};

class ObjectPrivate : public InodeRegister::ObjectPrivate
{
    using get = void (ObjectPrivate::*) (const uchar *registerValues);
    using set = void (ObjectPrivate::*) (uchar *registerValues);

public:
    ObjectPrivate(QSharedPointer<IPort::Interface> iport)
        : InodeRegister::ObjectPrivate(iport)
    {
        evaluation32 = &ObjectPrivate::fakeGet;
        evaluation64 = &ObjectPrivate::fakeGet;
        setEvaluation32 = &ObjectPrivate::fakeSet;
        setEvaluation64 = &ObjectPrivate::fakeSet;

    }

    unsigned msb = 0;
    unsigned lsb = 0;

    Cache cache;

    get evaluation32;// = &ObjectPrivate::fakeGet;
    get evaluation64;// = &ObjectPrivate::fakeGet;
    set setEvaluation32;// = &ObjectPrivate::fakeSet;
    set setEvaluation64;// = &ObjectPrivate::fakeSet;

    void fakeGet(const uchar *);
    void getFrom32LE(const uchar *registerValues);
    void getFrom32BE(const uchar *registerValues);
    void getFrom64LE(const uchar *registerValues);
    void getFrom64BE(const uchar *registerValues);
    void fakeSet(uchar *);
    void setTo32LE(uchar *registerValues);
    void setTo32BE(uchar *registerValues);
    void setTo64LE(uchar *registerValues);
    void setTo64BE(uchar *registerValues);

}; // class ObjectPrivate

} // namespace MaskedIntReg

} // namespace GenICam

} // namespace Jgv

#endif // MASKEDINTREG_P_H
