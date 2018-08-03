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

#include "genicamdoubleeditor.h"
#include "genicamdoubleeditor_p.h"

#include <QDoubleSpinBox>
#include <QSlider>
#include <QLayout>
#include <limits>

using namespace Jgv::GenICam;

DoubleEditor::DoubleEditor(const QString &name, QWidget *parent)
    : FloatEditor(name, parent),
      d(new DoubleEditorPrivate)
{
    d->spinbox = new QDoubleSpinBox;
    d->slider = new QSlider(Qt::Horizontal);
    layout()->addWidget(d->spinbox);
    layout()->addWidget(d->slider);
//    connect(d->slider, SIGNAL(valueChanged(int)), d->spinbox, SLOT(setValue(int)));
//    connect(d->spinbox, SIGNAL(valueChanged(int)), d->slider, SLOT(setValue(int)));
}

DoubleEditor::~DoubleEditor()
{}

void DoubleEditor::setRange(double minimum, double maximum)
{
    d->spinbox->setRange(minimum, maximum);
}

void DoubleEditor::setValue(double value)
{
    d->spinbox->setValue(value);
}

double DoubleEditor::value() const
{
    return d->spinbox->value();
}
