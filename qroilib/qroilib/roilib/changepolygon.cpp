/*
 * changepolygon.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changepolygon.h"

//#include "mapdocument.h"
#include "roiobject.h"
#include "roiobjectmodel.h"
#include <qroilib/documentview/documentview.h>

#include <QCoreApplication>

using namespace Qroilib;

ChangePolygon::ChangePolygon(DocumentView *mapDocument,
                             RoiObject *roiObject,
                             const QPolygonF &oldPolygon)
    : mRoiDocument(mapDocument)
    , mRoiObject(roiObject)
    , mOldPolygon(oldPolygon)
    , mNewPolygon(roiObject->polygon())
{
    setText(QCoreApplication::translate("Undo Commands", "Change Polygon"));
}

ChangePolygon::ChangePolygon(DocumentView *mapDocument,
                             RoiObject *roiObject,
                             const QPolygonF &newPolygon,
                             const QPolygonF &oldPolygon)
    : mRoiDocument(mapDocument)
    , mRoiObject(roiObject)
    , mOldPolygon(oldPolygon)
    , mNewPolygon(newPolygon)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Polygon"));
}

void ChangePolygon::undo()
{
    mRoiDocument->roiObjectModel()->setObjectPolygon(mRoiObject, mOldPolygon);
}

void ChangePolygon::redo()
{
    mRoiDocument->roiObjectModel()->setObjectPolygon(mRoiObject, mNewPolygon);
}
