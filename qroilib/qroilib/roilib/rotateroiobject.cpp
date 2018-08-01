/*
 * rotatemapobject.cpp
 * Copyright 2012, Przemysław Grzywacz <nexather@gmail.com>
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

#include "rotateroiobject.h"

//#include "mapdocument.h"
#include "roiobject.h"
#include "roiobjectmodel.h"
#include <qroilib/documentview/documentview.h>

#include <QCoreApplication>

using namespace Qroilib;

RotateRoiObject::RotateRoiObject(DocumentView *mapDocument,
                                 RoiObject *roiObject,
                                 qreal oldRotation)
    : mRoiDocument(mapDocument)
    , mRoiObject(roiObject)
    , mOldRotation(oldRotation)
    , mNewRotation(roiObject->rotation())
{
    setText(QCoreApplication::translate("Undo Commands", "Rotate Object"));
}

RotateRoiObject::RotateRoiObject(DocumentView *mapDocument,
                                 RoiObject *roiObject,
                                 qreal newRotation,
                                 qreal oldRotation)
    : mRoiDocument(mapDocument)
    , mRoiObject(roiObject)
    , mOldRotation(oldRotation)
    , mNewRotation(newRotation)
{
    setText(QCoreApplication::translate("Undo Commands", "Rotate Object"));
}

void RotateRoiObject::undo()
{
    mRoiDocument->roiObjectModel()->setObjectRotation(mRoiObject, mOldRotation);
}

void RotateRoiObject::redo()
{
    mRoiDocument->roiObjectModel()->setObjectRotation(mRoiObject, mNewRotation);
}
