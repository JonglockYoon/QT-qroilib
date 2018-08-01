/*
 * addremovemapobject.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "addremoveroiobject.h"

//#include "document.h"
#include "roiobject.h"
#include "objectgroup.h"
#include "roiobjectmodel.h"
#include <qroilib/documentview/documentview.h>
//#include "recipedata.h"

#include <QCoreApplication>

using namespace Qroilib;

AddRemoveRoiObject::AddRemoveRoiObject(DocumentView *mapDocument,
                                       ObjectGroup *objectGroup,
                                       RoiObject *roiObject,
                                       bool ownObject,
                                       QUndoCommand *parent)
    : QUndoCommand(parent)
    , mRoiDocument(mapDocument)
    , mRoiObject(roiObject)
    , mObjectGroup(objectGroup)
    , mIndex(-1)
    , mOwnsObject(ownObject)
{
    if (ownObject == true)
    {
        if (mRoiObject->mInspectType == 0)
        {
            mRoiObject->m_vecParams.clear();

            if (mRoiObject->shape() == RoiObject::Rectangle)
                mRoiObject->mInspectType = _INSPACT_ROI_START;//_Inspect_Roi_Start;
            else if (mRoiObject->shape() == RoiObject::Pattern)
                mRoiObject->mInspectType = _INSPECT_PATT_START;//_Inspect_Patt_Start;
            else if (mRoiObject->shape() == RoiObject::Point)
                mRoiObject->mInspectType = _INSPACT_POINT_START;//_Inspect_Point_Start;

            const ParamTable *pParamTable = mRoiDocument->getParamTable();

            for (int n = 0;; n++) // nInspectType 의 default parameter를 설정한다.
            {
                if (pParamTable->nInspectType == _INSPACT_TYPE_END) //_Inspect_Type_End
                    break;
                if (pParamTable->nInspectType == mRoiObject->mInspectType)
                    mRoiObject->m_vecParams.push_back(*pParamTable);
                pParamTable++;
            }
        }
    }
}

AddRemoveRoiObject::~AddRemoveRoiObject()
{
    if (mOwnsObject)
        delete mRoiObject;
}

void AddRemoveRoiObject::addObject()
{
    mRoiDocument->roiObjectModel()->insertObject(mObjectGroup, mIndex,
                                                 mRoiObject);
    mOwnsObject = false;
}

void AddRemoveRoiObject::removeObject()
{
    mIndex = mRoiDocument->roiObjectModel()->removeObject(mObjectGroup,
                                                          mRoiObject);
    mOwnsObject = true;
}


AddRoiObject::AddRoiObject(DocumentView *mapDocument, ObjectGroup *objectGroup,
                           RoiObject *roiObject, QUndoCommand *parent)
    : AddRemoveRoiObject(mapDocument,
                         objectGroup,
                         roiObject,
                         true,
                         parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Object"));
}


RemoveRoiObject::RemoveRoiObject(DocumentView *mapDocument,
                                 RoiObject *roiObject,
                                 QUndoCommand *parent)
    : AddRemoveRoiObject(mapDocument,
                         roiObject->objectGroup(),
                         roiObject,
                         false,
                         parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Object"));
}
