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

#include "genicameditor.h"
#include "genicamint64editor.h"
#include "genicamdoubleeditor.h"
#include "genicambuttoneditor.h"
#include "genicamcomboboxeditor.h"
#include "genicambooleditor.h"
#include "genicamlineeditor.h"
#include "genicamipv4editor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QPushButton>

using namespace Jgv::GenICam;

Editor::Editor(const QString &name, QWidget *parent)
    : QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::AlternateBase);
    setAttribute(Qt::WA_NoMousePropagation);
    QVBoxLayout *layout = new QVBoxLayout;

    QLabel *label = new QLabel(name);
    label->setAlignment(Qt::AlignCenter);
    QFont font = QFrame::font();
    font.setBold(true);
    label->setFont(font);
    QPushButton *abord = new QPushButton;
    QPushButton *accept = new QPushButton;
    abord->setFixedSize(label->sizeHint().height(), label->sizeHint().height());
    accept->setFixedSize(label->sizeHint().height(), label->sizeHint().height());
    abord->setStyleSheet("background-color: red");
    accept->setStyleSheet("background-color: green");
    QHBoxLayout *l = new QHBoxLayout;
    l->addWidget(abord);
    l->addWidget(label, 1);
    l->addWidget(accept);
    layout->addLayout(l);
    setLayout(layout);
    setToolTip(trUtf8("Input valid, Esc ignore"));

    connect(abord, &QPushButton::clicked, this, &Editor::deleteLater);
    connect(accept, SIGNAL(clicked()), this, SIGNAL(editingFinished()));
}

Jgv::GenICam::IntegerEditor *EditorFactory::createIntegerEditor(const QString &featureName, const QString &displayName, QWidget *parent)
{
    if (featureName == "GevCurrentIPAddress"
            || featureName == "GevCurrentSubnetMask"
            || featureName == "GevCurrentDefaultGateway"
            || featureName == "GevPersistentIPAddress"
            || featureName == "GevPersistentSubnetMask"
            || featureName == "GevPersistentDefaultGateway"
            || featureName == "GevPrimaryApplicationIPAddress"
            || featureName == "GevMCDA"
            || featureName == "GevSCDA") {
        return new GenICam::Ipv4Editor(displayName, parent);
    }
    return new GenICam::Int64Editor(displayName, parent);
}

FloatEditor *EditorFactory::createDoubleEditor(const QString &featureName, const QString &displayName, QWidget *parent)
{
    Q_UNUSED(featureName);
    return new GenICam::DoubleEditor(displayName, parent);
}

CommandEditor *EditorFactory::createCommandEditor(const QString &featureName, const QString &displayName, QWidget *parent)
{
    Q_UNUSED(featureName);
    return new GenICam::ButtonEditor(displayName, parent);
}

EnumerationEditor *EditorFactory::createEnumerationEditor(const QString &featureName, const QString &displayName, QWidget *parent)
{
    Q_UNUSED(featureName);
    return new GenICam::ComboboxEditor(displayName, parent);
}

BooleanEditor *EditorFactory::createBooleanEditor(const QString &featureName, const QString &displayName, QWidget *parent)
{
    Q_UNUSED(featureName);
    return new GenICam::BoolEditor(displayName, parent);
}

StringEditor *EditorFactory::createStringEditor(const QString &featureName, const QString &displayName, QWidget *parent)
{
    Q_UNUSED(featureName);
    return new GenICam::LineEditor(displayName, parent);
}



