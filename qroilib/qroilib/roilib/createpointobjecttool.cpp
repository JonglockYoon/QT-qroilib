/*
 * createpointobjecttool.cpp
 *Qroilib : QT ROI Lib
 * Copyright 2018 Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "createpointobjecttool.h"

#include "roiobject.h"
#include "utils.h"

using namespace Qroilib;

CreatePointObjectTool::CreatePointObjectTool(QObject *parent)
    : CreateScalableObjectTool(parent)
{
    setIcon(QIcon(QLatin1String(":images/insert-point.png")));
    setThemeIcon(this, "insert-point");
    languageChanged();
}

void CreatePointObjectTool::languageChanged()
{
    setName(tr("Insert Point"));
    setShortcut(QKeySequence(tr("P")));
}

RoiObject *CreatePointObjectTool::createNewRoiObject()
{
    RoiObject *newRoiObject = new RoiObject;
    newRoiObject->setShape(RoiObject::Point);
    return newRoiObject;
}
