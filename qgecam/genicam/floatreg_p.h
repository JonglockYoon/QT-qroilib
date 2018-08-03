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
#ifndef FLOATREG_P_H
#define FLOATREG_P_H

#include "inoderegister_p.h"

#include <QtEndian>

// float  IEEE-754 32 bits
// double IEEE-754 64 bits
template<>
inline float qFromBigEndian<float>(const void *src)
{
    union {
        quint32 i;
        float f;
    } in;
    in.i = qFromBigEndian<quint32>(src);
    return in.f;
}

template<>
inline double qFromBigEndian<double>(const void *src)
{
    union {
        quint64 i;
        double f;
    } in;
    in.i = qFromBigEndian<quint64>(src);
    return in.f;
}

template<>
inline void qToBigEndian<float>(float src, void *dest)
{
    union {
        quint32 i;
        float f;
    } in;
    in.f = src;
    qToBigEndian<quint32>(in.i, dest);
}

template<>
inline void qToBigEndian<double>(double src, void *dest)
{
    union {
        quint64 i;
        double f;
    } in;
    in.f = src;
    qToBigEndian<quint64>(in.i, dest);
}

template<>
inline float qFromLittleEndian<float>(const void *src)
{
    union {
        quint32 i;
        float f;
    } in;
    in.i = qFromLittleEndian<quint32>(src);
    return in.f;
}

template<>
inline double qFromLittleEndian<double>(const void *src)
{
    union {
        quint64 i;
        double f;
    } in;
    in.i = qFromLittleEndian<quint64>(src);
    return in.f;
}

template<>
inline void qToLittleEndian<float>(float src, void *dest)
{
    union {
        quint32 i;
        float f;
    } in;
    in.f = src;
    qToLittleEndian<quint32>(in.i, dest);
}

template<>
inline void qToLittleEndian<double>(double src, void *dest)
{
    union {
        quint64 i;
        double f;
    } in;
    in.f = src;
    qToLittleEndian<quint64>(in.i, dest);
}

namespace Jgv {

namespace GenICam {

namespace FloatReg {

namespace Properties {
static const char * Sign = "Sign";
}

struct Cache {
    double value;
    bool isValid;
};

class ObjectPrivate : public InodeRegister::ObjectPrivate
{
    using from = void (ObjectPrivate::*) (const uchar *, int size);
    using to = void (ObjectPrivate::*) (uchar *, int size);

public:
    ObjectPrivate(QSharedPointer<IPort::Interface> iport)
        : InodeRegister::ObjectPrivate(iport)
    {
        cache = {0., false};
        updateCache = &ObjectPrivate::fakeValue;
        writeCache = &ObjectPrivate::setFakeValue;
    }

    Cache cache;// {0., false};

    from updateCache;// = &ObjectPrivate::fakeValue;
    to writeCache;// = &ObjectPrivate::setFakeValue;

    void fakeValue(const uchar *, int);
    void little(const uchar * data, int size);
    void big(const uchar * data, int size);
    void setFakeValue(uchar * data, int size);
    void setLittle(uchar *data, int size);
    void setBig(uchar * data, int size);

}; // class ObjectPrivate

} // namespace FloatReg

} // namespace GenICam

} // namespace Jgv

#endif // FLOATREG_P_H
