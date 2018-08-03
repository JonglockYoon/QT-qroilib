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

#include "intconverter.h"
#include "intconverter_p.h"
#include "formula.h"
#include "genicamobjectbuilder.h"
#include "ifloat.h"
#include "xmlhelper.h"
#include "genicam.h"

#include <QStringList>
#include <QVariant>
#include <QDebug>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::IntConverter;

Object::Object(QSharedPointer<IPort::Interface> iport)
    : InodeConverter::Object(*new ObjectPrivate(iport))
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    InodeConverter::Object::prepare(builder, helper);
    d->sType = sType;
}

void Object::invalidate()
{
    Q_D(Object);

    QSharedPointer<Inode::Object> p = (d->* (d->pInode)) ();
    if (p) {
        p->invalidate();
    }
}

bool Object::isWritable() const
{
    Q_D(const Object);

    if (!d->imposedCanWrite) {
        return false;
    }

    QSharedPointer<Inode::Object> p = (d->* (d->pInode)) ();
    return (p) ? p->isWritable() : true;
}

QVariant Object::getVariant()
{
    Q_D(const Object);
    if (!d->namespaceIsCustom) {
        return GenICam::Representation::toString(featureName(), getValue());
    }

    return QVariant(getValue());
}

int Object::getPollingTime() const
{
    Q_D(const Object);

    QSharedPointer<Inode::Object> p = (d->* (d->pInode)) ();
    return (p) ? p->getPollingTime() : -1;
}

qint64 Object::getValue()
{
    Q_D(Object);

    IntegerVariables iVariables;
    FloatVariables fVariables;

    (d->* (d->evaluatePValue)) ("TO", iVariables, fVariables);

    const QStringList variablesList(d->formulaFrom->variablesList());
    auto it = variablesList.constBegin();
    for (; it != variablesList.constEnd(); ++it) {
        if (d->variablesEvals.contains(*it)) {
            (d->* (d->variablesEvals[*it])) (*it, iVariables, fVariables);
        }
    }

    return d->formulaFrom->evaluateAsInteger(iVariables, fVariables);
    //    const qint64 result =  d->formulaFrom->evaluateAsInteger(iVariables, fVariables);
    //    d->formulaFrom->debugOutput("IntConverter " + featureName());
    //    qDebug("RESULT : %lld", result);
    //    return result;
}

void Object::setValue(qint64 value)
{
    Q_D(Object);

    (d->* (d->set)) (value);
}

qint64 Object::getMin() const
{
    Q_D(const Object);

    IntegerVariables iVariables;
    FloatVariables fVariables;

    (d->* (d->evaluatePMin)) ("TO", iVariables, fVariables);

    const QStringList variablesList(d->formulaFrom->variablesList());
    auto it = variablesList.constBegin();
    for (; it != variablesList.constEnd(); ++it) {
        if (d->variablesEvals.contains(*it)) {
            (d->* (d->variablesEvals[*it])) (*it, iVariables, fVariables);
        }
    }

    return d->formulaFrom->evaluateAsInteger(iVariables, fVariables);
}

qint64 Object::getMax() const
{
    Q_D(const Object);

    IntegerVariables iVariables;
    FloatVariables fVariables;

    (d->* (d->evaluatePMax)) ("TO", iVariables, fVariables);

    const QStringList variablesList(d->formulaFrom->variablesList());
    auto it = variablesList.constBegin();
    for (; it != variablesList.constEnd(); ++it) {
        if (d->variablesEvals.contains(*it)) {
            (d->* (d->variablesEvals[*it])) (*it, iVariables, fVariables);
        }
    }

    return d->formulaFrom->evaluateAsInteger(iVariables, fVariables);
}

qint64 Object::getInc() const
{
    Q_D(const Object);

    IntegerVariables iVariables;
    FloatVariables fVariables;

    (d->* (d->evaluatePInc)) ("TO", iVariables, fVariables);

    const QStringList variablesList(d->formulaFrom->variablesList());
    auto it = variablesList.constBegin();
    for (; it != variablesList.constEnd(); ++it) {
        if (d->variablesEvals.contains(*it)) {
            (d->* (d->variablesEvals[*it])) (*it, iVariables, fVariables);
        }
    }

    return d->formulaFrom->evaluateAsInteger(iVariables, fVariables);
}

Jgv::GenICam::Integer::Interface *Object::interface()
{
    return this;
}

const Jgv::GenICam::Integer::Interface *Object::constInterface() const
{
    return this;
}





