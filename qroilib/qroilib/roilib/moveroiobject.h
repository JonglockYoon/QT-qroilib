/*
 * moveroiobject.h
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

#include <QUndoCommand>
#include <QPointF>

namespace Qroilib {

class DocumentView;
class RoiObject;
//class MapDocument;

class MoveRoiObject : public QUndoCommand
{
public:
    MoveRoiObject(DocumentView *mapDocument,
                  RoiObject *roiObject,
                  const QPointF &oldPos,
                  QUndoCommand *parent = nullptr);

    MoveRoiObject(DocumentView *mapDocument,
                  RoiObject *roiObject,
                  const QPointF &newPos,
                  const QPointF &oldPos,
                  QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    DocumentView *mRoiDocument;
    RoiObject *mRoiObject;
    QPointF mOldPos;
    QPointF mNewPos;
};

} // namespace Qroilib
