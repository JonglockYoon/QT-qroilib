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

#include "genicamobjectbuilder.h"
#include "genicamobjectbuilder_p.h"
#include "genicamxmlfile.h"
#include "inode.h"
#include "xmlhelper.h"
#include "category.h"
#include "stringreg.h"
#include "boolean.h"
#include "integer.h"
#include "intreg.h"
#include "enumeration.h"
#include "maskedintreg.h"
#include "floatjgv.h"
#include "floatreg.h"
#include "converter.h"
#include "intconverter.h"
#include "intswissknife.h"
#include "swissknife.h"
#include "command.h"
#include "register.h"
#include "structentry.h"
#include "iport.h"


using namespace Jgv::GenICam;

InterfaceBuilderPrivate::InterfaceBuilderPrivate(QSharedPointer<Jgv::GenICam::IPort::Interface> iport)
    : iport(iport)
{}

InterfaceBuilderPrivate::~InterfaceBuilderPrivate()
{}

InterfaceBuilder::InterfaceBuilder(const QString &fileName, const QByteArray &file, QSharedPointer<Jgv::GenICam::IPort::Interface> iport)
    : d_ptr(new InterfaceBuilderPrivate(iport))
{
    Q_D(InterfaceBuilder);
    d->xmlFile.setFileName(fileName);
    if (fileName.endsWith(".zip")) {
        d->xmlFile.setZippedContent(file);
    } else {
       d->xmlFile.setContent(file);
    }
}

InterfaceBuilder::~InterfaceBuilder()
{}

QWeakPointer<Inode::Object> InterfaceBuilder::buildInode(const QString &name)
{
    Q_D(InterfaceBuilder);

    if (name.isEmpty()) {
        qWarning("InterfaceBuilder::buildInode failed: empty inode name");
        return QWeakPointer<Inode::Object>();
    }

    QSharedPointer<Inode::Object> inode = d->inodes.value(name);
    if (inode.isNull()) {
        XMLHelper helper(d->xmlFile.getRootChildNodeByNameAttribute(name));
        inode = buildInode(helper);
        if (inode.isNull()) {
            qWarning("InterfaceBuilder::buildInode(const char * &name) failed: %s", qPrintable(name));
        }
    }

    return inode.toWeakRef();
}

QHash<QString, QSharedPointer<Inode::Object> > InterfaceBuilder::allReferencedInodes() const
{
    Q_D(const InterfaceBuilder);
    return d->inodes;
}


/*!
 * \brief InterfaceBuilder::buildInode
 * \param helper
 * \return
 */

QSharedPointer<Inode::Object> InterfaceBuilder::buildInode(const XMLHelper &helper)
{
    Q_D(InterfaceBuilder);
    QSharedPointer<Inode::Object> inode;

    // Attention, lors de l'initialisation de l'inode,
    // celui-ci peut appeler un nouvel inode manipulant le pointeur
    // de l'appelant, celui-ci n'étant pas initialiser.
    // Il faut donc absolument inserer dans la map l'inode avant de l'initialiser.
    // On évite ainsi les double instances.
    switch (helper.getType()) {
    case Node::Type::Category: {
        QSharedPointer<Category::Object> category(new Category::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), category);
        category->prepare(*this, helper);
        inode = category;
        break;
    }
    case Node::Type::StringReg: {
        QSharedPointer<StringReg::Object> stringreg(new StringReg::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), stringreg);
        stringreg->prepare(*this, helper);
        inode = stringreg;
        break;
    }
    case Node::Type::Boolean: {
        QSharedPointer<Boolean::Object> boolean(new Boolean::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), boolean);
        boolean->prepare(*this, helper);
        inode = boolean;
        break;
    }
    case Node::Type::Integer: {
        QSharedPointer<Integer::Object> integer(new Integer::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), integer);
        integer->prepare(*this, helper);
        inode = integer;
        break;
    }
    case Node::Type::IntReg: {
        QSharedPointer<IntReg::Object> intreg(new IntReg::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), intreg);
        intreg->prepare(*this, helper);
        inode = intreg;
        break;
    }
    case Node::Type::MaskedIntReg: {
        QSharedPointer<MaskedIntReg::Object> maskedintreg(new MaskedIntReg::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), maskedintreg);
        maskedintreg->prepare(*this, helper);
        inode = maskedintreg;
        break;
    }
    case Node::Type::Enumeration: {
        QSharedPointer<Enumeration::Object> enumeration(new Enumeration::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), enumeration);
        enumeration->prepare(*this, helper);
        inode = enumeration;
        break;
    }
    case Node::Type::Float: {
        QSharedPointer<Float::Object> fl(new Float::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), fl);
        fl->prepare(*this, helper);
        inode = fl;
        break;
    }
    case Node::Type::FloatReg: {
        QSharedPointer<FloatReg::Object> floatreg(new FloatReg::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), floatreg);
        floatreg->prepare(*this, helper);
        inode = floatreg;
        break;
    }
    case Node::Type::Converter: {
        QSharedPointer<Converter::Object> converter(new Converter::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), converter);
        converter->prepare(*this, helper);
        inode = converter;
        break;
    }
    case Node::Type::IntConverter: {
        QSharedPointer<IntConverter::Object> intconverter(new IntConverter::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), intconverter);
        intconverter->prepare(*this, helper);
        inode = intconverter;
        break;
    }
    case Node::Type::IntSwissknife: {
        QSharedPointer<IntSwissKnife::Object> intswissknife(new IntSwissKnife::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), intswissknife);
        intswissknife->prepare(*this, helper);
        inode = intswissknife;
        break;
    }
    case Node::Type::Swissknife: {
        QSharedPointer<SwissKnife::Object> swissknife(new SwissKnife::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), swissknife);
        swissknife->prepare(*this, helper);
        inode = swissknife;
        break;
    }
    case Node::Type::Command: {
        QSharedPointer<Command::Object> command(new Command::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), command);
        command->prepare(*this, helper);
        inode = command;
        break;
    }
    case Node::Type::Register: {
        QSharedPointer<Register::Object> reg(new Register::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), reg);
        reg->prepare(*this, helper);
        inode = reg;
        break;
    }
    case Node::Type::StructEntry: {
        QSharedPointer<StructEntry::Object> structEntry(new StructEntry::Object(d->iport));
        d->inodes.insert(helper.nameAttribut(), structEntry);
        structEntry->prepare(*this, helper);
        inode = structEntry;
        break;
    }
    default:
        qWarning("Warning InterfaceBuilder::buildInode(const XMLHelper &helper): %s", qPrintable(helper.typeString()));
        break;
    }
    return inode;
}





