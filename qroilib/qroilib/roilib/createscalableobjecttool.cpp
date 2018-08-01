/*
 * createscalableobjecttool.cpp
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

#include "createscalableobjecttool.h"

//#include "mapdocument.h"
#include "roiobject.h"
#include "roiobjectitem.h"
#include "roirenderer.h"
//#include "snaphelper.h"
#include "utils.h"
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>
#include <QDebug>

using namespace Qroilib;

CreateScalableObjectTool::CreateScalableObjectTool(QObject *parent)
    : CreateObjectTool(parent)
{
}

bool CreateScalableObjectTool::startNewRoiObject(const QPointF &posIn, ObjectGroup *objectGroup)
{
    QPointF pos = posIn;
//    RasterImageView *pView = mapDocument()->imageView();
//    QPointF offset = pView->imageOffset();
//    QPointF scroll = pView->scrollPos();
//    const qreal scale = mapDocument()->zoom();
//    pos.setX(pos.x()/scale - offset.x() + scroll.x()/scale);
//    pos.setY(pos.y()/scale - offset.y() + scroll.y()/scale);

    mStartPos = pos;
    return CreateObjectTool::startNewRoiObject(pos, objectGroup);
}

static qreal sign(qreal value)
{
    return value < 0 ? -1 : 1;
}

void CreateScalableObjectTool::mouseMovedWhileCreatingObject(const QPointF &posIn, Qt::KeyboardModifiers modifiers)
{
    //const RoiRenderer *renderer = mapDocument()->renderer();
    //const QPointF pixelCoords = renderer->screenToPixelCoords(pos);

    QPointF pos = posIn;
//    RasterImageView *pView = mapDocument()->imageView();
//    QPointF offset = pView->imageOffset();
//    QPointF scroll = pView->scrollPos();
//    const qreal scale = mapDocument()->zoom();
//    pos.setX(pos.x()/scale - offset.x() + scroll.x()/scale);
//    pos.setY(pos.y()/scale - offset.y() + scroll.y()/scale);

    QRectF objectArea(mStartPos, pos);

//    RasterImageView *pView = mapDocument()->imageView();
//    QPointF offset = pView->imageOffset();
//    QPointF spt = pView->scrollPos();
//    objectArea.translate(offset-spt);

    // Holding shift creates circle or square
    if (modifiers & Qt::ShiftModifier) {
        //qreal max = qMax(qAbs(objectArea.width()), qAbs(objectArea.height()));
        //objectArea.setWidth(max * sign(objectArea.width()));
        //objectArea.setHeight(max * sign(objectArea.height()));
    }

    // Update the position and size of the new roimap object
    //QPointF snapSize(objectArea.width(), objectArea.height());

    //SnapHelper(renderer, modifiers).snap(snapSize);
    //objectArea.setWidth(snapSize.x());
    //objectArea.setHeight(snapSize.y());

//    objectArea.setLeft(objectArea.left() * scale);
//    objectArea.setRight(objectArea.right() * scale);
//    objectArea.setTop(objectArea.top() * scale);
//    objectArea.setBottom(objectArea.bottom() * scale);


    mNewRoiObjectItem->resizeObject(objectArea.normalized());

    //qDebug() << "CreateScalableObjectTool::mouseMovedWhileCreatingObject" << objectArea << mNewRoiObjectItem->roiObject()->bounds();
}

void CreateScalableObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        cancelNewRoiObject();
}

void CreateScalableObjectTool::mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        finishNewRoiObject();
}
