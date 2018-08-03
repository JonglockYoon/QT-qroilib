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

#ifndef GENICAMIPV4EDITOR_H
#define GENICAMIPV4EDITOR_H

#include "genicameditor.h"
#include <QScopedPointer>

namespace Jgv {

namespace GenICam {

class Ipv4EditorPrivate;
class Ipv4Editor : public IntegerEditor
{
public:
    Ipv4Editor(const QString &name, QWidget * parent = nullptr);
    ~Ipv4Editor();

    void setRange(qint64 minimum, qint64 maximum, qint64 inc);
    void setValue(qint64 value);
    qint64 value() const;

private:
    const QScopedPointer<Ipv4EditorPrivate> d;
    Q_DISABLE_COPY(Ipv4Editor)

};

} // namespace GenICam

} // namespace Jgv

#endif // GENICAMIPV4EDITOR_H
