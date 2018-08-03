/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD                             *
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

#include "htmltable.h"

using namespace Qroilib;

const int END_SIZE = 8;

HtmlTable::HtmlTable()
    : QString("<table border=\"1\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" width=\"100%\"></table>")
{}

void HtmlTable::addRow(const QString &first, const QString &second)
{
    const int i = size() - END_SIZE;
    insert(i, QString("<tr>"
                      "<th><span style=\" font-style:italic; font-weight: normal;\">%0<span></th>"
                      "<th><span style=\" font-style:normal; font-weight: bold;\">%1<span></th>"
                      "</tr>").arg(first).arg(second));
}

