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
#ifndef FORMULA_P_H
#define FORMULA_P_H

#include "formula.h"
#include <QMap>
#include <QQueue>

namespace Jgv {

namespace GenICam {

namespace Token {
class Object;
}

class FormulaPrivate
{
public:
    FormulaPrivate(const QString &formula);
    virtual ~FormulaPrivate();

    const QString formula;
    QMap<QString, qint64> intVariables;
    FloatVariables floatVariables;
    QQueue<Token::Object> output;

    Token::Object getNextToken(int fromPosition) const;
    Token::Object getVariable(int fromPosition) const;
    QByteArray getNumber(int fromPosition) const;
    bool predecendenceIsEqual(const Token::Object &left, const Token::Object &right) const;
    bool predecendenceIsLess(const Token::Object &left, const Token::Object &right) const;
    void debugOutput(const QString &additionnalInfos) const;

}; // class FormulaPrivate

} // namespace GenICam

} // namespace Jgv

#endif // FORMULA_P_H
