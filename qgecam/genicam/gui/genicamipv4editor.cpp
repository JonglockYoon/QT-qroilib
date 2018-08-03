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

#include "genicamipv4editor.h"
#include "genicamipv4editor_p.h"

#include <QLayout>
#include <QLineEdit>
#include <QRegExpValidator>

using namespace Jgv::GenICam;

Ipv4Editor::Ipv4Editor(const QString &name, QWidget *parent)
    : IntegerEditor(name, parent),
      d(new Ipv4EditorPrivate)
{
    d->line = new QLineEdit;
    QRegExp rx( "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}" );
    d->line->setValidator(new QRegExpValidator(rx));
    layout()->addWidget(d->line);
}

Ipv4Editor::~Ipv4Editor()
{}

void Ipv4Editor::setRange(qint64 mini, qint64 maxi, qint64 inc)
{
    Q_UNUSED(mini)
    Q_UNUSED(maxi)
    Q_UNUSED(inc)
}

void Ipv4Editor::setValue(qint64 value)
{
    d->value = value;

    quint32 ip = 0xFFFFFFFF & d->value;
    d->line->setText(QString("%0.%1.%2.%3")
                     .arg((0xFF000000 & ip)>>0x18)
                     .arg((0x00FF0000 & ip)>>0x10)
                     .arg((0x0000FF00 & ip)>>0x8)
                     .arg(0x000000FF & ip));
}

qint64 Ipv4Editor::value() const
{
    QStringList ipv4 = d->line->text().split(QLatin1String("."));
    if (ipv4.count() == 4) {
        quint32 ipv4Address = 0;
        for (int i = 0; i < 4; ++i) {
            uint byteValue = ipv4.at(i).toUInt();
            ipv4Address <<= 8;
            ipv4Address += byteValue;
        }
        d->value = static_cast<qint64>(ipv4Address);
    }
    return d->value;
}
