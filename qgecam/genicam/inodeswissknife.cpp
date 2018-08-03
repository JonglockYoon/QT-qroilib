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

#include "inodeswissknife.h"
#include "inodeswissknife_p.h"

#include "xmlhelper.h"
#include "genicamobjectbuilder.h"

using namespace Jgv::GenICam;
using namespace Jgv::GenICam::InodeSwissKnife;

void ObjectPrivate::evaluateConstantInteger(const QString &name, Jgv::GenICam::IntegerVariables &integerList, Jgv::GenICam::FloatVariables &floatList) const
{
    Q_UNUSED(floatList)
    integerList.insert(name, constantsInteger[name]);
}

void ObjectPrivate::evaluateConstantFloat(const QString &name, Jgv::GenICam::IntegerVariables &integerList, Jgv::GenICam::FloatVariables &floatList) const
{
    Q_UNUSED(integerList)
    floatList.insert(name, constantsFloat[name]);
}

void ObjectPrivate::evaluatePValueInteger(const QString &name, Jgv::GenICam::IntegerVariables &integerList, Jgv::GenICam::FloatVariables &floatList) const
{
    Q_UNUSED(floatList)
    integerList.insert(name, pVariablesInteger[name].value());
}

void ObjectPrivate::evaluatePValueFloat(const QString &name, Jgv::GenICam::IntegerVariables &integerList, Jgv::GenICam::FloatVariables &floatList) const
{
    Q_UNUSED(integerList)
    floatList.insert(name, pVariablesFloat[name].value());
}

void ObjectPrivate::evaluateSubVariableInteger(const QString &name, Jgv::GenICam::IntegerVariables &integerList, Jgv::GenICam::FloatVariables &floatList) const
{
    Q_UNUSED(floatList)
    QSharedPointer<Inode::Object> object = pVariablesInteger[name].toStrongRef();
    if (!object) {
        qWarning("IntSwissKnife evaluateSubVariableInteger: weak pointer is not valid");
        return;
    }

    QStringList split = name.split('.');
    if (split.at(1) == "Value") integerList.insert(name, JGV_IINTEGER(object)->getValue());
    else if (split.at(1) == "Min") integerList.insert(name, JGV_IINTEGER(object)->getMin());
    else if (split.at(1) == "Max") integerList.insert(name, JGV_IINTEGER(object)->getMax());
    else integerList.insert(name, JGV_IINTEGER(object)->getInc());
}

void ObjectPrivate::evaluateSubVariableFloat(const QString &name, Jgv::GenICam::IntegerVariables &integerList, Jgv::GenICam::FloatVariables &floatList) const
{
    Q_UNUSED(integerList)
    QSharedPointer<Inode::Object> object = pVariablesFloat[name].toStrongRef();
    if (!object) {
        qWarning("IntSwissKnife evaluateSubVariableInteger: weak pointer is not valid");
        return;
    }

    QStringList split = name.split('.');
    if (split.at(1) == "Value") floatList.insert(name, JGV_IFLOAT(object)->getValue());
    else if (split.at(1) == "Min") floatList.insert(name, JGV_IFLOAT(object)->getMin());
    else if (split.at(1) == "Max") floatList.insert(name, JGV_IFLOAT(object)->getMax());
    else floatList.insert(name, JGV_IFLOAT(object)->getInc());
}

void ObjectPrivate::evaluateEnumeration(const QString &name, Jgv::GenICam::IntegerVariables &integerList, Jgv::GenICam::FloatVariables &floatList) const
{
    Q_UNUSED(floatList)
    QSharedPointer<Inode::Object> object = pVariablesInteger[name].toStrongRef();
    if (!object) {
        qWarning("IntSwissKnife evaluateEnumeration: weak pointer is not valid");
        return;
    }

    QStringList split = name.split('.');

    QList<Enumentry::Object *> entries = JGV_IENUMERATION(object)->getEntries();
    auto it = qFind(entries, split.at(2));
    if (it != entries.constEnd()) {
        integerList.insert(name, (*it)->getIntValue());
    }
}

void ObjectPrivate::evaluateExpressionInteger(const QString &name, Jgv::GenICam::IntegerVariables &integerList, Jgv::GenICam::FloatVariables &floatList) const
{
    Q_UNUSED(floatList)

    // la formule de l'expression
    const QSharedPointer<Formula> formula = expressions[name];
    // la liste des variables de la formule
    const QStringList variablesList(formula->variablesList());

    IntegerVariables iVariables;
    FloatVariables fVariables;

    auto it = variablesList.constBegin();
    for (;it!=variablesList.constEnd();++it) {
        if (variablesEvals.contains(*it)) {
            (this->* (variablesEvals[*it])) (*it, iVariables, fVariables);
        }
    }

    integerList.insert(name, formula->evaluateAsInteger(iVariables, fVariables));
}

Object::Object(ObjectPrivate &dd)
    : Inode::Object(dd)
{}

void Object::prepare(InterfaceBuilder &builder, const XMLHelper &helper)
{
    Q_D(Object);

    Inode::Object::prepare(builder, helper);

    // parsing des constantes
    {
        QMap<QString, QString> constants = helper.getAttributeValueMap(Properties::Constant, Inode::Attributes::Name);
        auto it = constants.constBegin();
        for (; it != constants.constEnd(); ++it) {
            bool ok;
            const qint64 I = (*it).toLongLong(&ok, 0);
            if (ok) {
                d->constantsInteger.insert(it.key(), I);
                d->variablesEvals.insert(it.key(), &ObjectPrivate::evaluateConstantInteger);
            }
            else {
                const double D = (*it).toDouble(&ok);
                if (ok) {
                    d->constantsFloat.insert(it.key(), D);
                    d->variablesEvals.insert(it.key(), &ObjectPrivate::evaluateConstantFloat);
                }
                else {
                    qWarning("IntSwissKnife prepare(): failed to evaluate constante %s - %s in node %s", qPrintable(it.key()), qPrintable(it.value()), qPrintable(featureName()));
                }
            }
        }
    }



    // parsing des expressions
    {
        QMap<QString, QString> expressions = helper.getAttributeValueMap(Properties::Expression, Inode::Attributes::Name);
        auto it = expressions.constBegin();
        for (; it != expressions.constEnd(); ++it) {
            d->expressions.insert(it.key(), QSharedPointer<Formula>(new Formula(it.value())));
            d->variablesEvals.insert(it.key(), &ObjectPrivate::evaluateExpressionInteger);
        }
    }

    // parsing des pVariables
    {
        QMap<QString, QString> pVariables = helper.getAttributeValueMap(Properties::pVariable, Inode::Attributes::Name);
        auto it = pVariables.constBegin();
        for (; it != pVariables.constEnd(); ++it) {
            QSharedPointer<Inode::Object> pVariable = builder.buildInode(it.value());
            if (!pVariable) {
                qWarning("IntSwissKnife %s prepare(): pVariable %s failed ", qPrintable(featureName()), qPrintable(it.value()));
            }



            // pour la détection des noms particuliers
            QStringList split = it.key().split('.');

            if (pVariable->interface()->interfaceType() == Type::IFloat) {
                d->pVariablesFloat.insert(it.key(), pValueFloat(pVariable));
                // simple pValue
                if (split.count() == 1) {
                    d->variablesEvals.insert(it.key(), &ObjectPrivate::evaluatePValueFloat);
                }
                // Value, Min, Max, Inc
                else if (split.count() == 2) {
                    if (split.at(1) == "Value" || split.at(1) == "Min" || split.at(1) == "Max" || split.at(1) == "Inc") {
                        d->variablesEvals.insert(it.key(), &ObjectPrivate::evaluateSubVariableFloat);
                    }
                }
            }
            // si la pVariable est une énumération son accès est particulier
            else if (pVariable->interface()->interfaceType() == Type::IEnumeration) {
                d->pVariablesInteger.insert(it.key(), pValueInteger(pVariable));
                // si le nom est en une seule partie, c'est un accès normal
                if (split.count() == 1) {
                    d->variablesEvals.insert(it.key(), &ObjectPrivate::evaluatePValueInteger);
                }
                // si le nom est en 3 parties
                else if (split.count()==3 && split.at(1)=="Entry") {
                    d->variablesEvals.insert(it.key(), &ObjectPrivate::evaluateEnumeration);
                }
                else {
                    qWarning("InodeIntSwissKnife %s: variable bad IEnumeration name %s", qPrintable(featureName()), qPrintable(it.key()));
                }
            }
            else {
                d->pVariablesInteger.insert(it.key(), pValueInteger(pVariable));
                // simple pValue
                if (split.count() == 1) {
                    d->variablesEvals.insert(it.key(), &ObjectPrivate::evaluatePValueInteger);
                }
                // Value, Min, Max, Inc
                else if (split.count() == 2) {
                    if (split.at(1) == "Value" || split.at(1) == "Min" || split.at(1) == "Max" || split.at(1) == "Inc") {
                        d->variablesEvals.insert(it.key(), &ObjectPrivate::evaluateSubVariableInteger);
                    }
                }
            }

        }
    }

    // parsing de la formule
    QScopedPointer<Formula> pFormula(new Formula(helper.getProperty(Properties::Formula)));
    d->formula.swap(pFormula);
}



