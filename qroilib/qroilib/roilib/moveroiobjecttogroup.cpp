/*
 * moveroiobjecttogroup.cpp
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "moveroiobjecttogroup.h"

//#include "mapdocument.h"
#include "roiobject.h"
#include "objectgroup.h"
#include "roiobjectmodel.h"
#include <qroilib/documentview/documentview.h>

#include <QCoreApplication>

using namespace Qroilib;

MoveRoiObjectToGroup::MoveRoiObjectToGroup(DocumentView *mapDocument,
                                           RoiObject *roiObject,
                                           ObjectGroup *objectGroup)
    : mRoiDocument(mapDocument)
    , mRoiObject(roiObject)
    , mOldObjectGroup(roiObject->objectGroup())
    , mNewObjectGroup(objectGroup)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Move Object to Layer"));
}

void MoveRoiObjectToGroup::undo()
{
    mRoiDocument->roiObjectModel()->removeObject(mNewObjectGroup, mRoiObject);
    mRoiDocument->roiObjectModel()->insertObject(mOldObjectGroup, -1, mRoiObject);
}

void MoveRoiObjectToGroup::redo()
{
    mRoiDocument->roiObjectModel()->removeObject(mOldObjectGroup, mRoiObject);
    mRoiDocument->roiObjectModel()->insertObject(mNewObjectGroup, -1, mRoiObject);
}
