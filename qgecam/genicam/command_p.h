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
#ifndef COMMAND_P_H
#define COMMAND_P_H

#include "inode_p.h"
#include "pvalue.h"

namespace Jgv {

namespace GenICam {

namespace Command {

namespace Properties {
static const char * CommandValue = "CommandValue";
static const char * pCommandValue = "pCommandValue";
static const char * Value = "Value";
static const char * pValue = "pValue";
static const char * TLParamsLocked = "TLParamsLocked";

} // Properties


class ObjectPrivate : public Inode::ObjectPrivate
{
public:
    ObjectPrivate(QSharedPointer<IPort::Interface> iport)
        : Inode::ObjectPrivate(iport)
    {}

    pValueInteger pValue;
    pValueInteger pCommandValue;
    pValueInteger pTLLocker;

    bool isTLLocker = false;
    bool isTLUnlocker = false;

}; // class ObjectPrivate

} // namespace Command

} // namespace GenICam

} // namespace Jgv

#endif // COMMAND_P_H
