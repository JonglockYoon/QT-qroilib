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

#ifndef TOKEN_H
#define TOKEN_H

#include <QByteArray>

namespace Jgv {

namespace GenICam {

namespace Token {

enum class Type {
    UNKNOW,
    LEFT_BRACKET, RIGHT_BRACKET,
    ADDITION, SUBSTRACTION, MULTIPLICATION, DIVISION,
    REMAINDER,
    POWER,
    BITWISE_AND, BITWISE_OR, BITWISE_XOR, BITWISE_NOT,
    LOGICAL_NOT_EQUAL, LOGICAL_EQUAL, LOGICAL_GREATER, LOGICAL_LESS, LOGICAL_LESS_OR_EQUAL, LOGICAL_GREATER_OR_EQUAL,
    LOGICAL_AND, LOGICAL_OR,
    SHIFT_LEFT, SHIFT_RIGHT,
    TERNARY_QUESTION_MARK, TERNARY_COLON,
    SGN, NEG,
    ATAN, COS, SIN, TAN, ABS, EXP, LN, SQRT, TRUNC, FLOOR, CEIL, ROUND, ASIN, ACOS,
    E, PI,
    INT64, DOUBLE,
    VARIABLE, VARIABLE_INT64, VARIABLE_DOUBLE
};

class Object final
{
    Type m_type = Type::UNKNOW;
    QByteArray m_genicamToken;
    union Value {
        double floatValue;
        qint64 intValue;
    };
    Value m_value = {};

public:
    void setType(Type type, const QByteArray &token = QByteArray());
    Type getType() const;
    QByteArray getGenicamToken() const;
    int getSize() const;
    bool isNumber() const;
    bool isVariable() const;
    bool isINT64() const;
    bool isDOUBLE() const;
    bool isOperator() const;
    bool isLeftParenthesis() const;
    bool isRightParenthesis() const;
    bool isLeftAssociative() const;
    int operandsCount() const;
    int precedence() const;
    void setFloat(double value);
    double toFloat() const;
    void setInteger(qint64 value);
    qint64 toInteger() const;

}; // class Object

} // namespace Token

} // namespace GenICam

} // namespace Jgv

#endif // TOKEN_H
