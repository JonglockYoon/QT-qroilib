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

#include "genicamcomboboxeditor.h"
#include "genicamcomboboxeditor_p.h"

#include <QLayout>
#include <QComboBox>


using namespace Jgv::GenICam;

ComboboxEditor::ComboboxEditor(const QString &name, QWidget *parent)
    : EnumerationEditor(name, parent),
      d(new ComboboxEditorPrivate)
{
    d->combobox = new QComboBox;
    layout()->addWidget(d->combobox);
    connect(d->combobox, SIGNAL(currentIndexChanged(int)),
            this, SIGNAL(editingFinished()));
}

ComboboxEditor::~ComboboxEditor()
{}

void ComboboxEditor::addItem(const QString &text, const qint64 value)
{
    d->combobox->addItem(text, QVariant(value));
}

void ComboboxEditor::setCurrentIndex(int index)
{
    d->combobox->setCurrentIndex(index);
}

qint64 ComboboxEditor::value() const
{
    return d->combobox->itemData(d->combobox->currentIndex()).toLongLong();
}

