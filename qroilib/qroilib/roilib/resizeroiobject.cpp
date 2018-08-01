/*
 * resizemapobject.cpp
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

#include "resizeroiobject.h"

//#include "document.h"
#include "roiobject.h"
#include "roiobjectmodel.h"
#include <qroilib/documentview/documentview.h>

#include <QCoreApplication>

using namespace Qroilib;

ResizeRoiObject::ResizeRoiObject(DocumentView *mapDocument,
                                 RoiObject *roiObject,
                                 const QSizeF &oldSize)
    : mRoiDocument(mapDocument)
    , mRoiObject(roiObject)
    , mOldSize(oldSize)
    , mNewSize(roiObject->size())
{
    setText(QCoreApplication::translate("Undo Commands", "Resize Object"));
}

ResizeRoiObject::ResizeRoiObject(DocumentView *mapDocument,
                                 RoiObject *roiObject,
                                 const QSizeF &newSize,
                                 const QSizeF &oldSize)
    : mRoiDocument(mapDocument)
    , mRoiObject(roiObject)
    , mOldSize(oldSize)
    , mNewSize(newSize)
{
    setText(QCoreApplication::translate("Undo Commands", "Resize Object"));
}

//void ResizeRoiObject::undo()
//{
//    mRoiDocument->roiObjectModel()->setObjectSize(mRoiObject, mOldSize);
//}

//void ResizeRoiObject::redo()
//{
//    mRoiDocument->roiObjectModel()->setObjectSize(mRoiObject, mNewSize);
//}
