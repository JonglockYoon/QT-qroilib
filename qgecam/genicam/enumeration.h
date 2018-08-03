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
#ifndef ENUMERATION_H
#define ENUMERATION_H

#include "inode.h"
#include "ienumeration.h"

namespace Jgv {

namespace GenICam {

namespace Enumentry {
class Object;
}

namespace Enumeration {

static const char * sType = "Enumeration";

class ObjectPrivate;
class Object final : public Inode::Object, public Enumeration::Interface
{

public:
    Object(QSharedPointer<IPort::Interface> iport);
    virtual ~Object() = default;

    void prepare(InterfaceBuilder &builder, const XMLHelper &helper) override;

    void invalidate() override;
    bool isWritable() const override;

    QVariant getVariant() override;

    QString getStringValue() override;
    void setStringValue(const QString &value) override;
    qint64 getIntValue() const override;
    void setIntValue(qint64 value) override;
    QList<Enumentry::Object *> getEntries() const override;

    Enumeration::Interface *interface() override;
    const Enumeration::Interface *constInterface() const override;

private:
    Q_DECLARE_PRIVATE(Object)

}; // class Object

} // namespace Enumeration

} // namespace GenICam

} // namespace Jgv


#endif // ENUMERATION_H
