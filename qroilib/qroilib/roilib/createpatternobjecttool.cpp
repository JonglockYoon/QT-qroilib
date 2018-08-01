/*
 * createpatternobjecttool.cpp
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

#include "createpatternobjecttool.h"

#include "roiobject.h"
#include "utils.h"

using namespace Qroilib;

CreatePatternObjectTool::CreatePatternObjectTool(QObject *parent)
    : CreateScalableObjectTool(parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-patternroi.png")));
    setThemeIcon(this, "insert-patternroi");
    languageChanged();
}

void CreatePatternObjectTool::languageChanged()
{
    setName(tr("Insert PatternROI"));
    setShortcut(QKeySequence(tr("P")));
}

RoiObject *CreatePatternObjectTool::createNewRoiObject()
{
    RoiObject *newRoiObject = new RoiObject;
    newRoiObject->setShape(RoiObject::Pattern);
    return newRoiObject;
}
