/*
 * createobjecttool.cpp
 * Copyright 2010-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "createobjecttool.h"

#include "addremoveroiobject.h"
#include "roimap.h"
#include "roiobject.h"
#include "roiobjectitem.h"
#include "roirenderer.h"
#include "roiscene.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "utils.h"
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>
#include "qpatternroif.h"

#include <QApplication>
#include <QKeyEvent>
#include <QPalette>
#include <QThread>
#include <QDebug>
#include <QDateTime>

using namespace Qroilib;

CreateObjectTool::CreateObjectTool(QObject *parent)
    : AbstractObjectTool(QString(),
                         QIcon(QLatin1String(":images/24x24/insert-rectangle.png")),
                         QKeySequence(tr("O")),
                         parent)
    , mNewRoiObjectGroup(new ObjectGroup)
    , mObjectGroupItem(new ObjectGroupItem(mNewRoiObjectGroup))
    , mNewRoiObjectItem(nullptr)
    , mOverlayPolygonItem(nullptr)
    //, mRoi(nullptr)
{
    mObjectGroupItem->setZValue(10000); // same as the BrushItem
}

CreateObjectTool::~CreateObjectTool()
{
    delete mObjectGroupItem;
    //delete mNewRoiObjectGroup;
}

void CreateObjectTool::activate(RoiScene *scene)
{
    AbstractObjectTool::activate(scene);
    scene->addItem(mObjectGroupItem);
}

void CreateObjectTool::deactivate(RoiScene *scene)
{
    if (mNewRoiObjectItem)
        cancelNewRoiObject();

    scene->removeItem(mObjectGroupItem);
    AbstractObjectTool::deactivate(scene);
}

void CreateObjectTool::keyPressed(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (mNewRoiObjectItem) {
            finishNewRoiObject();
            return;
        }
        break;
    case Qt::Key_Escape:
        if (mNewRoiObjectItem) {
            cancelNewRoiObject();
            return;
        }
        break;
    }

    AbstractObjectTool::keyPressed(event);
}

void CreateObjectTool::mouseEntered()
{
}

void CreateObjectTool::mouseMoved(const QPointF &posIn,
                                  Qt::KeyboardModifiers modifiers)
{
    QPointF pos;
    RasterImageView *pView = mapDocument()->imageView();
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = mapDocument()->zoom();
    pos.setX((posIn.x() - offset.x())/scale + scroll.x()/scale);
    pos.setY((posIn.y() - offset.y())/scale + scroll.y()/scale);

    //qDebug() << "mouseMoved" << posIn << scroll <<  offset << scale << pos;

    AbstractObjectTool::mouseMoved(pos, modifiers);

    if (mNewRoiObjectItem) {
        QPointF goffset = mNewRoiObjectItem->roiObject()->objectGroup()->totalOffset();
        mouseMovedWhileCreatingObject(pos - goffset, modifiers);
    }
}

void CreateObjectTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mNewRoiObjectItem) {
        mousePressedWhileCreatingObject(event);
        return;
    }

    if (event->button() != Qt::LeftButton) {
        AbstractObjectTool::mousePressed(event);
        return;
    }

    ObjectGroup *objectGroup = currentObjectGroup();
    if (!objectGroup || !objectGroup->isVisible())
        return;

    const RoiRenderer *renderer = mapDocument()->renderer();
    const QPointF posIn = event->scenePos() - objectGroup->totalOffset();

    QPointF pos;
    DocumentView *v = mapDocument();
    RasterImageView *pView = v->imageView();
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = v->zoom();
    pos.setX((posIn.x() - offset.x())/scale + scroll.x()/scale);
    pos.setY((posIn.y() - offset.y())/scale + scroll.y()/scale);

    //QPointF pixelCoords = renderer->screenToPixelCoords(pos);
    //SnapHelper(renderer, event->modifiers()).snap(pixelCoords);

    if (startNewRoiObject(pos, objectGroup))
        mouseMovedWhileCreatingObject(pos, event->modifiers());
}

void CreateObjectTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (mNewRoiObjectItem)
        mouseReleasedWhileCreatingObject(event);
}

bool CreateObjectTool::startNewRoiObject(const QPointF &pos,
                                         ObjectGroup *objectGroup)
{
    Q_ASSERT(!mNewRoiObjectItem);

    RoiObject *newRoiObject = createNewRoiObject();
    if (!newRoiObject)
        return false;

    newRoiObject->setPosition(pos);

    mNewRoiObjectGroup->addObject(newRoiObject);

    mNewRoiObjectGroup->setColor(objectGroup->color());
    mNewRoiObjectGroup->setOffset(objectGroup->totalOffset());

    mObjectGroupItem->setPos(mNewRoiObjectGroup->offset());

    DocumentView *v = mapDocument();
    mNewRoiObjectItem = new RoiObjectItem(newRoiObject, v, mObjectGroupItem);
    //mNewRoiObjectItem->setPos(0,0);

    return true;
}

RoiObject *CreateObjectTool::clearNewRoiObjectItem()
{
    Q_ASSERT(mNewRoiObjectItem);

    RoiObject *newRoiObject = mNewRoiObjectItem->roiObject();

    mNewRoiObjectGroup->removeObject(newRoiObject);

    delete mNewRoiObjectItem;
    mNewRoiObjectItem = nullptr;

    delete mOverlayPolygonItem;
    mOverlayPolygonItem = nullptr;

    return newRoiObject;
}

void CreateObjectTool::cancelNewRoiObject()
{
    RoiObject *newRoiObject = clearNewRoiObjectItem();
    delete newRoiObject;
}

void CreateObjectTool::finishNewRoiObject()
{
    Q_ASSERT(mNewRoiObjectItem);
    //qDebug() << "CreateObjectTool::finishNewRoiObject enter" << mNewRoiObjectItem->roiObject()->bounds();

    ObjectGroup *objectGroup = currentObjectGroup();
    if (!objectGroup) {
        cancelNewRoiObject();
        return;
    }

    DocumentView *v = mapDocument();

    RoiObject *newRoiObject = mNewRoiObjectItem->roiObject();
    const QRectF bnd = newRoiObject->bounds();
    if (bnd.isNull() && mNewRoiObjectItem->roiObject()->polygon().size() < 3)
    {
        //qDebug() << "CreateObjectTool::finishNewRoiObject cancelNewRoiObject()";
        cancelNewRoiObject();
        return;
    }
    clearNewRoiObjectItem();

    const qreal scale = v->zoom();
    const QImage *pimg = v->image();
    RasterImageView *pView = v->imageView();
    QPointF scroll = pView->scrollPos();
    double x = scroll.x();
    double y = scroll.y();
    QRectF vrect = v->boundingRect();
    vrect.setLeft((vrect.left()+x)/scale);
    vrect.setTop((vrect.top()+y)/scale);
    vrect.setRight((vrect.right()+x)/scale);
    vrect.setBottom((vrect.bottom()+y)/scale);
    if (vrect.right() > pimg->width())
        vrect.setRight(pimg->width());
    if (vrect.bottom() > pimg->height())
        vrect.setBottom(pimg->height());
    const QRectF bounds = vrect.intersected(bnd);
    newRoiObject->setBounds(bounds);
    //qDebug() << "CreateObjectTool::finishNewRoiObject" << vrect << bounds;

    if (bounds.isNull())
    {
        //qDebug() << "CreateObjectTool::finishNewRoiObject cancelNewRoiObject() #2";
        return;
    }

    QString time = QDateTime::currentDateTime().toString("MMddhhmmss");
    newRoiObject->setName("noname" + time);

    RoiObject *PatternObject = nullptr;
    RoiObject::Shape shape = newRoiObject->shape();
    if (shape == RoiObject::Pattern)
    {
        if (PatternObject == nullptr)
        {
            float fw = bounds.width() * 0.4;
            float fh = bounds.height() * 0.4;
            QRectF recti;
            recti.setLeft(bounds.left()+fw);
            recti.setRight(bounds.right()-fw);
            recti.setTop(bounds.top()+fh);
            recti.setBottom(bounds.bottom()-fh);

            QPointF pos = QPointF(recti.left(), recti.top());
            PatternObject = new RoiObject;
            PatternObject->mParent = newRoiObject;
            PatternObject->mInspectType = 0; //InspectType  must be 0...
            PatternObject->setShape(RoiObject::Rectangle);
            PatternObject->setPosition(pos);
            PatternObject->setBounds(recti);
            mNewRoiObjectGroup->addObject(PatternObject);
			newRoiObject->mPattern = PatternObject;
            QThread::msleep(1);
        }
    }
    v->undoStack()->push(new AddRoiObject(v, objectGroup, newRoiObject));

    v->setSelectedObjects(QList<RoiObject*>() << newRoiObject);

    if (shape == RoiObject::Pattern)
    {
        v->undoStack()->push(new AddRoiObject(v, objectGroup, PatternObject));

        v->setSelectedObjects(QList<RoiObject*>() << newRoiObject << PatternObject);
    }
    emit v->finishNewRoiObject();
}

void CreateObjectTool::mouseMovedWhileCreatingObject(const QPointF &, Qt::KeyboardModifiers)
{
    // optional override
}

void CreateObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *)
{
    // optional override
}

void CreateObjectTool::mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *)
{
    // optional override
}
