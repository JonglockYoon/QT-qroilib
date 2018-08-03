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

#ifndef INODESWISSKNIFE_P_H
#define INODESWISSKNIFE_P_H

#include "inode_p.h"
#include "pvalue.h"
#include "formula.h"

#include <QMap>
#include <QScopedPointer>

namespace Jgv {

namespace GenICam {

namespace InodeSwissKnife {

namespace Properties {
static const char * pVariable = "pVariable";
static const char * Constant = "Constant";
static const char * Expression = "Expression";
static const char * Formula = "Formula";
}

class ObjectPrivate : public Inode::ObjectPrivate
{
    using evaluateVariable = void (ObjectPrivate::*) (const QString &, IntegerVariables &, FloatVariables &) const;

public:
    ObjectPrivate(QSharedPointer<IPort::Interface> iport)
        : Inode::ObjectPrivate(iport)
    {}

    QMap<QString, qint64> constantsInteger;
    QMap<QString, double> constantsFloat;
    QMap<QString, pValueInteger> pVariablesInteger;
    QMap<QString, pValueFloat> pVariablesFloat;
    QMap<QString, QSharedPointer<Formula> > expressions;

    QMap<QString, evaluateVariable> variablesEvals;

    void evaluateConstantInteger(const QString &name, IntegerVariables &integerList, FloatVariables &floatList) const;
    void evaluateConstantFloat(const QString &name, IntegerVariables &integerList, FloatVariables &floatList) const;
    void evaluatePValueInteger(const QString &name, IntegerVariables &integerList, FloatVariables &floatList) const;
    void evaluatePValueFloat(const QString &name, IntegerVariables &integerList, FloatVariables &floatList) const;
    void evaluateSubVariableInteger(const QString &name, IntegerVariables &integerList, FloatVariables &floatList) const;
    void evaluateSubVariableFloat(const QString &name, IntegerVariables &integerList, FloatVariables &floatList) const;
    void evaluateEnumeration(const QString &name, IntegerVariables &integerList, FloatVariables &floatList) const;
    void evaluateExpressionInteger(const QString &name, IntegerVariables &integerList, FloatVariables &floatList) const;

    QScopedPointer<Formula> formula;

}; // class ObjectPrivate

} // namespace InodeSwissKnife

} // namespace GenICam

} // namespace Jgv

#endif // INODESWISSKNIFE_P_H
