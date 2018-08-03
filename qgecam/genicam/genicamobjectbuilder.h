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

#ifndef GENICAMOBJECTBUILDER_H
#define GENICAMOBJECTBUILDER_H

#include "iinterface.h"
#include <QScopedPointer>
#include <QHash>
#include <QWeakPointer>


class QString;

namespace Jgv {

namespace GenICam {

namespace IPort {
class Interface;
}

namespace Inode {
class Object;
}

class XMLHelper;

class InterfaceBuilderPrivate;
class InterfaceBuilder
{
public:
    InterfaceBuilder(const QString &fileName, const QByteArray &file, QSharedPointer<IPort::Interface> iport);
    ~InterfaceBuilder();

    QWeakPointer<Inode::Object> buildInode(const QString &name);
    QHash<QString, QSharedPointer<Inode::Object> > allReferencedInodes() const;

private:
    QSharedPointer<Inode::Object> buildInode(const XMLHelper &helper);
    const QScopedPointer<InterfaceBuilderPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(InterfaceBuilder)

}; // class InterfaceBuilder

} // namespace GenICam

} // namespace Jgv


#endif // GENICAMOBJECTBUILDER_H
