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

#ifndef GENICAMOBJECTBUILDER_P_H
#define GENICAMOBJECTBUILDER_P_H

#include <QHash>
#include <QSharedPointer>

#include "genicamxmlfile.h"

namespace IPort {
class Interface;
}

namespace Jgv {

namespace GenICam {

namespace Inode {
class Object;
}

class InterfaceBuilderPrivate
{
public:
    InterfaceBuilderPrivate(QSharedPointer<IPort::Interface> iport);
    ~InterfaceBuilderPrivate();

    QSharedPointer<IPort::Interface> iport;
    QHash<QString, QSharedPointer<Inode::Object> > inodes;

    GenICamXMLFile xmlFile;

}; // class InterfaceBuilderPrivate

} // namespace GenICam

} // namespace Jgv

#endif // GENICAMOBJECTBUILDER_P_H
