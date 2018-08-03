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


#include "genicamint64editor.h"
#include "genicamint64editor_p.h"

#include <QLayout>
#include <QSpinBox>
#include <QSlider>
#include <limits>

using namespace Jgv::GenICam;

Int64Editor::Int64Editor(const QString &name, QWidget *parent)
    : IntegerEditor(name, parent),
      d(new Int64EditorPrivate)
{
    d->spinbox = new QSpinBox;
    d->slider = new QSlider(Qt::Horizontal);
    layout()->addWidget(d->spinbox);
    layout()->addWidget(d->slider);
    connect(d->slider, SIGNAL(valueChanged(int)), d->spinbox, SLOT(setValue(int)));
    connect(d->spinbox, SIGNAL(valueChanged(int)), d->slider, SLOT(setValue(int)));
}

Int64Editor::~Int64Editor()
{}

void Int64Editor::setRange(qint64 mini, qint64 maxi, qint64 inc)
{
    const qint64 min = qMax<qint64>(mini , std::numeric_limits<int>::lowest());
    const qint64 max = qMin<qint64>(maxi , std::numeric_limits<int>::max());
    d->spinbox->setRange(min, max);
    d->slider->setRange(min, max);
    d->inc = inc;
}

void Int64Editor::setValue(qint64 value)
{
    d->spinbox->setValue(value);
}

qint64 Int64Editor::value() const
{
    qint64 val = d->spinbox->value();
    if (d->inc > 1) {
        val = qRound64(static_cast<double>(val) / static_cast<double>(d->inc)) * d->inc;
    }
    return val;
}
