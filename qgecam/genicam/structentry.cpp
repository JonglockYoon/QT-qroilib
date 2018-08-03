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
#include "structentry.h"
#include "structentry_p.h"
#include "genicamobjectbuilder.h"
#include "xmlhelper.h"


using namespace Jgv::GenICam;
using namespace Jgv::GenICam::StructEntry;

Object::Object(QSharedPointer<IPort::Interface> iport)
    : MaskedIntReg::Object(*new ObjectPrivate(iport))
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    d->featureName = helper.getAttribute(Inode::Attributes::Name);

    InodeRegister::Object::prepare(builder, helper.getParentHelper());

    d->sType = sType;

    QString prop = helper.getProperty(MaskedIntReg::Properties::Bit);
    if (!prop.isEmpty()) {
        bool ok = false;
        unsigned val = prop.toUInt(&ok, 0);
        if (ok) {
            // le cas bit est equivalent au cas LSB = MSB = BIT
            d->lsb = val;
            d->msb = val;
            if (d->isLittleEndian) {
                d->evaluation32 = &ObjectPrivate::getFrom32LE;
                d->evaluation64 = &ObjectPrivate::getFrom64LE;
                d->setEvaluation32 = &ObjectPrivate::setTo32LE;
                d->setEvaluation64 = &ObjectPrivate::setTo64LE;
            }
            else {
                d->evaluation32 = &ObjectPrivate::getFrom32BE;
                d->evaluation64 = &ObjectPrivate::getFrom64BE;
                d->setEvaluation32 = &ObjectPrivate::setTo32BE;
                d->setEvaluation64 = &ObjectPrivate::setTo64BE;
            }
        }
        else {
            qWarning("MaskedIntReg %s: failed to parse Bit %s", qPrintable(featureName()), qPrintable(prop));
        }
    }
    else {
        prop = helper.getProperty(MaskedIntReg::Properties::LSB);
        QString msbProp = helper.getProperty(MaskedIntReg::Properties::MSB);
        if ((!prop.isEmpty()) && (!msbProp.isEmpty())) {
            bool ok = false;
            unsigned lsb = prop.toUInt(&ok, 0);
            if (ok) {
                unsigned msb = msbProp.toUInt(&ok, 0);
                if (ok) {
                    d->lsb = lsb;
                    d->msb = msb;
                    if (d->isLittleEndian) {
                        d->evaluation32 = &ObjectPrivate::getFrom32LE;
                        d->evaluation64 = &ObjectPrivate::getFrom64LE;
                        d->setEvaluation32 = &ObjectPrivate::setTo32LE;
                        d->setEvaluation64 = &ObjectPrivate::setTo64LE;
                    }
                    else {
                        d->evaluation32 = &ObjectPrivate::getFrom32BE;
                        d->evaluation64 = &ObjectPrivate::getFrom64BE;
                        d->setEvaluation32 = &ObjectPrivate::setTo32BE;
                        d->setEvaluation64 = &ObjectPrivate::setTo64BE;
                    }
                }
                else {
                    qWarning("MaskedIntReg %s: failed to parse MSB %s", qPrintable(featureName()), qPrintable(prop));
                }
            }
            else {
                qWarning("MaskedIntReg %s: failed to parse LSB %s", qPrintable(featureName()), qPrintable(prop));
            }
        }
    }
}

