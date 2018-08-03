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

#include "intswissknife.h"
#include "intswissknife_p.h"
#include "formula.h"
#include "ienumeration.h"
#include "iboolean.h"
#include "enumentry.h"
#include "genicamobjectbuilder.h"
#include "xmlhelper.h"
#include "genicam.h"

#include <QStringList>
#include <QVariant>
#include <QDebug>

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::IntSwissKnife;


Object::Object(QSharedPointer<IPort::Interface> iport)
    : InodeSwissKnife::Object(*new ObjectPrivate(iport))
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    InodeSwissKnife::Object::prepare(builder, helper);
    d->sType = sType;

}

void Object::invalidate()
{}

bool Object::isWritable() const
{
    return false;
}

QVariant Object::getVariant()
{
    Q_D(const Object);

    if (!d->namespaceIsCustom) {
        return GenICam::Representation::toString(featureName(), getValue());
    }

    return QVariant(getValue());
}

qint64 Object::getValue()
{
    Q_D(Object);

    IntegerVariables iVariables;
    FloatVariables fVariables;

    const QStringList variablesList(d->formula->variablesList());
    auto it = variablesList.constBegin();
    for (; it != variablesList.constEnd(); ++it) {
        if (d->variablesEvals.contains(*it)) {
            (d->* (d->variablesEvals[*it])) (*it, iVariables, fVariables);
        }
    }

    return d->formula->evaluateAsInteger(iVariables, fVariables);

}

void Object::setValue(qint64 value)
{
    Q_UNUSED(value)
    qWarning("IntSwissknife %s setValue : Not implemented (no sense) !", qPrintable(featureName()));
}

Jgv::GenICam::Integer::Interface *Object::interface()
{
    return this;
}

const Jgv::GenICam::Integer::Interface *Object::constInterface() const
{
    return this;
}


