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

#ifndef INODECONVERTER_P_H
#define INODECONVERTER_P_H

#include "inodeswissknife_p.h"
#include "pvalue.h"

namespace Jgv {

namespace GenICam {


namespace InodeConverter {

namespace Properties {
static const char * pValue = "pValue";
static const char * FormulaTo = "FormulaTo";
static const char * FormulaFrom = "FormulaFrom";
static const char * Slope = "Slope";
}

namespace Slope {
static const char * Increasing = "Increasing";
static const char * Decreasing = "Decreasing";
static const char * Varying = "Varying";
static const char * Automatic = "Automatic";
}

class ObjectPrivate : public InodeSwissKnife::ObjectPrivate
{
    using evaluateVariable = void (ObjectPrivate::*) (const QString &, IntegerVariables &, FloatVariables &) const;
    using pValue = QSharedPointer<Inode::Object> (ObjectPrivate::*) () const;
    using set_pValue = void (ObjectPrivate::*) (qint64);

public:
    ObjectPrivate(QSharedPointer<IPort::Interface> iport)
        : InodeSwissKnife::ObjectPrivate(iport)
    {
        pInode = &ObjectPrivate::pValueFromInteger;
        set = &ObjectPrivate::setToInteger;

        evaluatePValue = &ObjectPrivate::fillFake;
        evaluatePMin = &ObjectPrivate::fillFake;
        evaluatePMax = &ObjectPrivate::fillFake;
        evaluatePInc = &ObjectPrivate::fillFake;
    }

    pValueInteger pIValue;
    pValueFloat pFValue;

    QScopedPointer<Formula> formulaFrom;
    QScopedPointer<Formula> formulaTo;

    QSharedPointer<Inode::Object> pValueFromInteger() const;
    QSharedPointer<Inode::Object> pValueFromFloat() const;
    void setToInteger(qint64 value);
    void setToFloat(qint64 value);

    pValue pInode;// = &ObjectPrivate::pValueFromInteger;
    set_pValue set;// = &ObjectPrivate::setToInteger;

    void fillFake(const QString &, IntegerVariables &, FloatVariables &) const;
    void fillPValueAsInteger(const QString &name, IntegerVariables &integers, FloatVariables &floats) const;
    void fillPValueAsFloat(const QString &name, IntegerVariables &integers, FloatVariables &floats) const;
    void fillPMinAsInteger(const QString &name, IntegerVariables &integers, FloatVariables &floats) const;
    void fillPMinAsFloat(const QString &name, IntegerVariables &integers, FloatVariables &floats) const;
    void fillPMaxAsInteger(const QString &name, IntegerVariables &integers, FloatVariables &floats) const;
    void fillPMaxAsFloat(const QString &name, IntegerVariables &integers, FloatVariables &floats) const;
    void fillPIncAsInteger(const QString &name, IntegerVariables &integers, FloatVariables &floats) const;
    void fillPIncAsFloat(const QString &name, IntegerVariables &integers, FloatVariables &floats) const;

    evaluateVariable evaluatePValue;// = &ObjectPrivate::fillFake;
    evaluateVariable evaluatePMin;// = &ObjectPrivate::fillFake;
    evaluateVariable evaluatePMax;// = &ObjectPrivate::fillFake;
    evaluateVariable evaluatePInc;// = &ObjectPrivate::fillFake;

}; // class ObjetcPrivate

} // namespace InodeConverter

} // namespace GenICam

} // namespace Jgv

#endif // INODECONVERTER_P_H
