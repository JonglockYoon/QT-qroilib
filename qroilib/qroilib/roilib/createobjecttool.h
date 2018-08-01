/*
 * createobjecttool.h
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "abstractobjecttool.h"

namespace Qroilib {

class RoiObjectItem;
class ObjectGroupItem;

class CreateObjectTool : public AbstractObjectTool
{
    Q_OBJECT

public:
    CreateObjectTool(QObject *parent = nullptr);
    ~CreateObjectTool();

    void activate(RoiScene *scene) override;
    void deactivate(RoiScene *scene) override;

    void keyPressed(QKeyEvent *event) override;
    void mouseEntered() override;
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers) override;
    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

public slots:
    /**
     * Sets the tile that will be used when the creation mode is
     * CreateTileObjects.
     */

protected:
    virtual void mouseMovedWhileCreatingObject(const QPointF &pos,
                                               Qt::KeyboardModifiers modifiers);
    virtual void mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event);


    virtual bool startNewRoiObject(const QPointF &pos, ObjectGroup *objectGroup);
    virtual RoiObject *createNewRoiObject() = 0;
    virtual void cancelNewRoiObject();
    virtual void finishNewRoiObject();

    RoiObject *clearNewRoiObjectItem();
    ObjectGroup *mNewRoiObjectGroup;
    ObjectGroupItem *mObjectGroupItem;
    RoiObjectItem *mNewRoiObjectItem;
    RoiObjectItem *mOverlayPolygonItem;
};

} // namespace Qroilib
