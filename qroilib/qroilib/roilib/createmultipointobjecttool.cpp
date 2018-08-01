/*
 * createmultipointobjecttool.cpp
 * Copyright 2014, Martin Ziel <martin.ziel.com>
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

#include "createmultipointobjecttool.h"

//#include "mapdocument.h"
#include "roiobject.h"
#include "roiobjectitem.h"
#include "roirenderer.h"
#include "roiscene.h"
#include "objectgroup.h"
#include "snaphelper.h"
#include "utils.h"
#include <qroilib/documentview/documentview.h>

#include <QApplication>
#include <QPalette>

using namespace Qroilib;

CreateMultipointObjectTool::CreateMultipointObjectTool(QObject *parent)
    : CreateObjectTool(parent)
    , mOverlayPolygonObject(new RoiObject)
    , mOverlayObjectGroup(new ObjectGroup)
{
    mOverlayObjectGroup->addObject(mOverlayPolygonObject);

    QColor highlight = QApplication::palette().highlight().color();
    mOverlayObjectGroup->setColor(highlight);
}

CreateMultipointObjectTool::~CreateMultipointObjectTool()
{
    delete mOverlayObjectGroup;
}

void CreateMultipointObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos,
                                                               Qt::KeyboardModifiers modifiers)
{
    const RoiRenderer *renderer = mapDocument()->renderer();
    QPointF pixelCoords = renderer->screenToPixelCoords(pos);

    //SnapHelper(renderer, modifiers).snap(pixelCoords);

    //pixelCoords -= mNewRoiObjectItem->roiObject()->position();

    QPolygonF polygon = mOverlayPolygonObject->polygon();
    polygon.last() = pixelCoords;
    mOverlayPolygonItem->setPolygon(polygon);
}

void CreateMultipointObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        finishNewRoiObject();
    } else if (event->button() == Qt::LeftButton) {
        QPolygonF current = mNewRoiObjectItem->roiObject()->polygon();
        QPolygonF next = mOverlayPolygonObject->polygon();

        // If the last position is still the same, ignore the click
        if (next.last() == current.last())
            return;

        // Assign current overlay polygon to the new object
        mNewRoiObjectItem->setPolygon(next);

        // Add a new editable point to the overlay
        next.append(next.last());
        mOverlayPolygonItem->setPolygon(next);
    }
}

bool CreateMultipointObjectTool::startNewRoiObject(const QPointF &pos, ObjectGroup *objectGroup)
{
    CreateObjectTool::startNewRoiObject(pos, objectGroup);
    RoiObject *newRoiObject = mNewRoiObjectItem->roiObject();
    QPolygonF polygon;
    polygon.append(QPointF(pos));
    newRoiObject->setPolygon(polygon);

    polygon.append(QPointF(pos)); // The last point is connected to the mouse
    mOverlayPolygonObject->setPolygon(polygon);
    mOverlayPolygonObject->setShape(newRoiObject->shape());
    mOverlayPolygonObject->setPosition(pos);

    mOverlayPolygonItem = new RoiObjectItem(mOverlayPolygonObject,
                                            mapDocument(),
                                            mObjectGroupItem);

    return true;
}
