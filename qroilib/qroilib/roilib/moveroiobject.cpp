/*
 * movemapobject.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "moveroiobject.h"

//#include "mapdocument.h"
#include "roiobject.h"
#include "roiobjectmodel.h"
#include <qroilib/documentview/documentview.h>

#include <QCoreApplication>

using namespace Qroilib;

MoveRoiObject::MoveRoiObject(DocumentView *mapDocument,
                             RoiObject *roiObject,
                             const QPointF &oldPos,
                             QUndoCommand *parent)
    : QUndoCommand(parent)
    , mRoiDocument(mapDocument)
    , mRoiObject(roiObject)
    , mOldPos(oldPos)
    , mNewPos(roiObject->position())
{
    setText(QCoreApplication::translate("Undo Commands", "Move Object"));
}

MoveRoiObject::MoveRoiObject(DocumentView *mapDocument,
                             RoiObject *roiObject,
                             const QPointF &newPos,
                             const QPointF &oldPos,
                             QUndoCommand *parent)
    : QUndoCommand(parent)
    , mRoiDocument(mapDocument)
    , mRoiObject(roiObject)
    , mOldPos(oldPos)
    , mNewPos(newPos)
{
    setText(QCoreApplication::translate("Undo Commands", "Move Object"));
}

void MoveRoiObject::undo()
{
    mRoiDocument->roiObjectModel()->setObjectPosition(mRoiObject, mOldPos);
}

void MoveRoiObject::redo()
{
    mRoiDocument->roiObjectModel()->setObjectPosition(mRoiObject, mNewPos);
}
