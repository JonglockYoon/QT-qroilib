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

#include "genicambooleditor.h"
#include "genicambooleditor_p.h"

#include <QComboBox>
#include <QLayout>

using namespace Jgv::GenICam;

BoolEditor::BoolEditor(const QString &name, QWidget *parent)
    : BooleanEditor(name, parent),
      d(new BoolEditorPrivate)
{
    d->combobox = new QComboBox;
    d->combobox->addItems(QStringList() << "False" << "True");
    layout()->addWidget(d->combobox);
    connect(d->combobox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(editingFinished()));
}

BoolEditor::~BoolEditor()
{}

bool BoolEditor::value() const
{
    return (d->combobox->currentIndex() == 1);
}

void BoolEditor::setValue(bool value)
{
    d->combobox->blockSignals(true);
    d->combobox->setCurrentIndex(value?1:0);
    d->combobox->blockSignals(false);
}

