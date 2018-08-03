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

#include "inodeconverter.h"
#include "inodeconverter_p.h"

#include "genicamobjectbuilder.h"
#include "xmlhelper.h"

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::InodeConverter;

QSharedPointer<Jgv::GenICam::Inode::Object> ObjectPrivate::pValueFromInteger() const
{
    return pIValue.toStrongRef();
}

QSharedPointer<Jgv::GenICam::Inode::Object> ObjectPrivate::pValueFromFloat() const
{
    return pFValue.toStrongRef();
}

void ObjectPrivate::setToInteger(qint64 value)
{
    IntegerVariables iVariables;
    FloatVariables fVariables;

    const QStringList variablesList(formulaTo->variablesList());
    auto it = variablesList.constBegin();
    for (; it != variablesList.constEnd(); ++it) {
        if (variablesEvals.contains(*it)) {
            (this->* (variablesEvals[*it])) (*it, iVariables, fVariables);
        }
    }

    iVariables.insert("FROM", value);
    const qint64 val = formulaTo->evaluateAsInteger(iVariables, fVariables);
    pIValue.set(val);
}

void ObjectPrivate::setToFloat(qint64 value)
{
    IntegerVariables iVariables;
    FloatVariables fVariables;

    const QStringList variablesList(formulaTo->variablesList());
    auto it = variablesList.constBegin();
    for (; it != variablesList.constEnd(); ++it) {
        if (variablesEvals.contains(*it)) {
            (this->* (variablesEvals[*it])) (*it, iVariables, fVariables);
        }
    }

    iVariables.insert("FROM", value);
    const double val = formulaTo->evaluateAsFloat(iVariables, fVariables);
    pFValue.set(val);
}

void ObjectPrivate::fillFake(const QString &name, IntegerVariables &, FloatVariables &) const
{
    qWarning("IntConverter %s: fake fill (%s)", qPrintable(featureName), qPrintable(name));
}

void ObjectPrivate::fillPValueAsInteger(const QString &name, Jgv::GenICam::IntegerVariables &integers, Jgv::GenICam::FloatVariables &floats) const
{
    Q_UNUSED(floats)
    integers.insert(name, pIValue.value());
}

void ObjectPrivate::fillPValueAsFloat(const QString &name, Jgv::GenICam::IntegerVariables &integers, Jgv::GenICam::FloatVariables &floats) const
{
    Q_UNUSED(integers)
    floats.insert(name, pFValue.value());
}

void ObjectPrivate::fillPMinAsInteger(const QString &name, Jgv::GenICam::IntegerVariables &integers, Jgv::GenICam::FloatVariables &floats) const
{
    Q_UNUSED(floats)
    integers.insert(name, pIValue.min());
}

void ObjectPrivate::fillPMinAsFloat(const QString &name, Jgv::GenICam::IntegerVariables &integers, Jgv::GenICam::FloatVariables &floats) const
{
    Q_UNUSED(integers)
    floats.insert(name, pFValue.min());
}

void ObjectPrivate::fillPMaxAsInteger(const QString &name, Jgv::GenICam::IntegerVariables &integers, Jgv::GenICam::FloatVariables &floats) const
{
    Q_UNUSED(floats)
    integers.insert(name, pIValue.max());
}

void ObjectPrivate::fillPMaxAsFloat(const QString &name, Jgv::GenICam::IntegerVariables &integers, Jgv::GenICam::FloatVariables &floats) const
{
    Q_UNUSED(integers)
    floats.insert(name, pFValue.max());
}

void ObjectPrivate::fillPIncAsInteger(const QString &name, Jgv::GenICam::IntegerVariables &integers, Jgv::GenICam::FloatVariables &floats) const
{
    Q_UNUSED(floats)
    integers.insert(name, pIValue.inc());
}

void ObjectPrivate::fillPIncAsFloat(const QString &name, Jgv::GenICam::IntegerVariables &integers, Jgv::GenICam::FloatVariables &floats) const
{
    Q_UNUSED(integers)
    floats.insert(name, pFValue.inc());
}

Object::Object(ObjectPrivate &dd)
    : InodeSwissKnife::Object(dd)
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    InodeSwissKnife::Object::prepare(builder, helper);

    const QString slope = helper.getProperty(Properties::Slope);

    QString prop = helper.getProperty(Properties::pValue);
    if (!prop.isEmpty()) {
        QWeakPointer<Inode::Object> inode = builder.buildInode(helper.getProperty(Properties::pValue));
        QSharedPointer<Inode::Object> p = inode.toStrongRef();
        if (p) {
            if (p->interface()->interfaceType() == Type::IFloat) {
                d->pFValue = pValueFloat(inode);

                d->set = &ObjectPrivate::setToFloat;

                d->pInode = &ObjectPrivate::pValueFromFloat;
                d->evaluatePValue = &ObjectPrivate::fillPValueAsFloat;
                d->evaluatePMin = (slope == Slope::Decreasing) ? &ObjectPrivate::fillPMaxAsFloat : &ObjectPrivate::fillPMinAsFloat;
                d->evaluatePMax = (slope == Slope::Decreasing) ? &ObjectPrivate::fillPMinAsFloat : &ObjectPrivate::fillPMaxAsFloat;;
                d->evaluatePInc = &ObjectPrivate::fillPIncAsFloat;
            }
            else {
                d->pIValue = pValueInteger(inode);

                d->set = &ObjectPrivate::setToInteger;

                d->pInode = &ObjectPrivate::pValueFromInteger;
                d->evaluatePValue = &ObjectPrivate::fillPValueAsInteger;
                d->evaluatePMin = (slope == Slope::Decreasing) ? &ObjectPrivate::fillPMaxAsInteger : &ObjectPrivate::fillPMinAsInteger;
                d->evaluatePMax = (slope == Slope::Decreasing) ? &ObjectPrivate::fillPMinAsInteger : &ObjectPrivate::fillPMaxAsInteger;;
                d->evaluatePInc = &ObjectPrivate::fillPIncAsInteger;
            }
        }
    }


    prop = helper.getProperty(Properties::FormulaFrom);
    QScopedPointer<Formula> pFrom(new Formula(prop));
    d->formulaFrom.swap(pFrom);

    prop = helper.getProperty(Properties::FormulaTo);
    QScopedPointer<Formula> pTo(new Formula(prop));
    d->formulaTo.swap(pTo);

}






