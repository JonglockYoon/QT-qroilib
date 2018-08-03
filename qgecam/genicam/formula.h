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
#ifndef FORMULA_H
#define FORMULA_H

#include <QScopedPointer>
#include <QString>
#include <QMap>

class QByteArray;

namespace Jgv {

namespace GenICam {

namespace Token {
class Object;
}

using IntegerVariables = QMap<QString, qint64>;
using FloatVariables = QMap<QString, double>;

class FormulaPrivate;
class Formula final
{
public:
    Formula(const QString &formula);
    ~Formula();

    QStringList variablesList() const;

    qint64 evaluateAsInteger(const IntegerVariables &intVariables, const FloatVariables &floatVariables);
    double evaluateAsFloat(const IntegerVariables &intVariables, const FloatVariables &floatVariables);
    void debugOutput(const QString &additionnalInfos) const;

private:
    const QScopedPointer<FormulaPrivate> d_ptr;
    Q_DECLARE_PRIVATE(Formula)

}; // class Formula

} // namespace GenICam

} // namespace Jgv

#endif // FORMULA_H
