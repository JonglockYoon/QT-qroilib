/*
 * addremoveroiobject.h
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

#pragma once

#include <roilib_export.h>
#include <QUndoCommand>

namespace Qroilib {

class RoiObject;
class ObjectGroup;

class DocumentView;

/**
 * Abstract base class for AddRoiObject and RemoveRoiObject.
 */
class AddRemoveRoiObject : public QUndoCommand
{
public:
    AddRemoveRoiObject(DocumentView *mapDocument,
                       ObjectGroup *objectGroup,
                       RoiObject *roiObject,
                       bool ownObject,
                       QUndoCommand *parent = nullptr);
    ~AddRemoveRoiObject();

protected:
    void addObject();
    void removeObject();

private:
    DocumentView *mRoiDocument;
    RoiObject *mRoiObject;
    ObjectGroup *mObjectGroup;
    int mIndex;
    bool mOwnsObject;
};

/**
 * Undo command that adds an object to a roimap.
 */
class ROIDSHARED_EXPORT AddRoiObject : public AddRemoveRoiObject
{
public:
    AddRoiObject(DocumentView *mapDocument, ObjectGroup *objectGroup,
                 RoiObject *roiObject, QUndoCommand *parent = nullptr);

    void undo() override
    { removeObject(); }

    void redo() override
    { addObject(); }
};

/**
 * Undo command that removes an object from a roimap.
 */
class ROIDSHARED_EXPORT RemoveRoiObject : public AddRemoveRoiObject
{
public:
    RemoveRoiObject(DocumentView *mapDocument, RoiObject *roiObject,
                    QUndoCommand *parent = nullptr);

    void undo() override
    { addObject(); }

    void redo() override
    { removeObject(); }
};

} // namespace Qroilib
