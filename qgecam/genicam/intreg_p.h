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

#ifndef INTREG_P_H
#define INTREG_P_H

#include "inoderegister_p.h"

namespace Jgv {

namespace GenICam {

namespace IntReg {

namespace Sign {
static const char * Signed = "Signed";
static const char * Unsigned = "Unsigned";
}

namespace Properties {
static const char * Sign = "Sign";
}

struct Cache {
    qint64 value;
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
        cache = {0, false};
        updateCache = &ObjectPrivate::fakeValue;
        writeCache = &ObjectPrivate::setFakeValue;
    }

    Cache cache;// {0, false};
    bool isSigned = true;

    from updateCache;// = &ObjectPrivate::fakeValue;
    to writeCache;// = &ObjectPrivate::setFakeValue;

    void fakeValue(const uchar *, int);
    void unsignedLittle(const uchar * data, int size);
    void signedLittle(const uchar * data, int size);
    void unsignedBig(const uchar * data, int size);
    void signedBig(const uchar * data, int size);
    void setFakeValue(uchar * data, int size);
    void setUnsignedLittle(uchar *data, int size);
    void setSignedLittle(uchar *data, int size);
    void setUnsignedBig(uchar * data, int size);
    void setSignedBig(uchar *data, int size);




}; // class ObjectPrivate

} // namespace IntReg

} // namespace GenICam

} // namespace Jgv

#endif // INTREG_P_H
