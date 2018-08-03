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

#include "genicambuttoneditor.h"
#include "genicambuttoneditor_p.h"

#include <QPushButton>
#include <QLayout>

using namespace Jgv::GenICam;

ButtonEditor::ButtonEditor(const QString &name, QWidget *parent)
    : CommandEditor(name, parent),
      d(new ButtonEditorPrivate)
{
    d->button = new QPushButton(trUtf8("Execute"));
    d->button->setMinimumHeight(d->button->sizeHint().height() * 2);
    layout()->addWidget(d->button);
    d->clicked = false;
    connect(d->button, &QPushButton::clicked, [this] () {
        d->clicked = true;
        emit editingFinished();
    });

    //connect(d->button, SIGNAL(clicked()), this, SLOT(onClicked()));
    setToolTip(trUtf8("Click Run, Escape Cancels"));
}

ButtonEditor::~ButtonEditor()
{}

bool ButtonEditor::isClicked() const
{
    return d->clicked;
}



