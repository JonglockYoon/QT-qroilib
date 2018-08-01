/*
 * roiobjectitem.cpp
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2008-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "roiobjectitem.h"

#include "roiobject.h"
#include "roiobjectmodel.h"
#include "roirenderer.h"
#include "roiscene.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "resizeroiobject.h"
#include <qroilib/documentview/documentview.h>

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QStyleOptionGraphicsItem>
#include <QVector2D>
#include <QDebug>
#include <qroilib/documentview/rasterimageview.h>

#include <cmath>

using namespace Qroilib;

RoiObjectItem::RoiObjectItem(RoiObject *object, DocumentView *mapDocument,
                             ObjectGroupItem *parent):
    QGraphicsItem(parent),
    mObject(object),
    mRoiDocument(mapDocument)
{
    syncWithRoiObject();
}

void RoiObjectItem::syncWithRoiObject()
{
    //RasterImageView *pView = mapDocument()->imageView();
    const QColor color = objectColor(mObject);

    // Update the whole object when the name, polygon or color has changed
    if (mName != mObject->name() || mPolygon != mObject->polygon() || mColor != color) {
        mName = mObject->name();
        mPolygon = mObject->polygon();
        mColor = color;
        update();
    }

    QString toolTip = mName;
    const QString &type = mObject->type();
    if (!type.isEmpty())
        toolTip += QLatin1String(" (") + type + QLatin1String(")");
    setToolTip(toolTip);

    RoiRenderer *renderer = mRoiDocument->renderer();
    const QPointF pixelPos = renderer->pixelToScreenCoords(mObject->position());
    const QRectF bounds = renderer->boundingRect(mObject);
    //const QRectF bounds = mObject->bounds(); // polygon에서 않됨

    QTransform t;
    // 이동을 할때 offset 과 spt 를 고려해야함.
    // 회전을 할때는 없어야함?
    t.translate(bounds.x(), bounds.y());
    t.rotate(rotation() + mObject->rotation());
    t.translate(-bounds.x(), -bounds.y());
    setTransform(t);

    if (ObjectGroup *objectGroup = mObject->objectGroup())
        if (objectGroup->drawOrder() == ObjectGroup::TopDownOrder)
            setZValue(pixelPos.y());

    if (mBoundingRect != bounds) {
        // Notify the graphics scene about the geometry change in advance
        prepareGeometryChange();
        mBoundingRect = bounds;
        //qDebug() << "RoiObjectItem::syncWithRoiObject()" << bounds;
    }

    setVisible(mObject->isVisible());
    //setVisible(true);
}

QRectF RoiObjectItem::boundingRect() const
{
    if (mObject->objectGroup() == nullptr)
        return QRectF();
    if (!mObject->objectGroup()->isVisible() || !mObject->isVisible())
        return QRectF();
    DocumentView *v = mapDocument();
    if (v->bMultiView)   // view가 두개 이상 display되어 있으면 ROI정보를 나타내지 않는다.
        return QRectF();
    if (!v->isVisible())
        return QRectF();
   return mBoundingRect;
}

//QPainterPath RoiObjectItem::shape() const
//{
//    QPainterPath path = mRoiDocument->renderer()->shape(mObject);
//    //path.translate(-pos());
//    return path;
//}

void RoiObjectItem::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *,
                          QWidget *widget)
{
    if (mObject->objectGroup() == nullptr)
        return;
    if (!mObject->objectGroup()->isVisible() || !mObject->isVisible())
        return;
    DocumentView *v = mapDocument();
    if (v->bMultiView)   // view가 두개 이상 display되어 있으면 ROI정보를 나타내지 않는다.
        return;
    if (!v->isVisible())
        return;

    qreal scale = v->zoom();
    //scale = static_cast<MapView*>(widget->parent())->zoomable()->scale();
    //QPointF pos_ = pos();
    //painter->translate(-pos_);
    v->renderer()->setPainterScale(scale);
    v->renderer()->drawRoiObject(painter, mObject, mColor);
    //qDebug() << "RoiObjectItem::paint()" << scale;
}

void RoiObjectItem::resizeObject(const QRectF &bounds)
{
    // Not using the RoiObjectModel because it is used during object creation,
    // when the object is not actually part of the roimap yet.

    QRectF b = bounds;
    //qDebug() << "resizeObject" << b;
    mObject->setBounds(b);
    syncWithRoiObject();
}

void RoiObjectItem::setPolygon(const QPolygonF &polygon)
{
    // Not using the RoiObjectModel because it is used during object creation,
    // when the object is not actually part of the roimap yet.
    mObject->setPolygon(polygon);
    syncWithRoiObject();
}

QColor RoiObjectItem::objectColor(const RoiObject *object)
{
    const QString effectiveType = object->effectiveType();

//    // See if this object type has a color associated with it
//    for (const ObjectType &type : Preferences::instance()->objectTypes()) {
//        if (type.name.compare(effectiveType, Qt::CaseInsensitive) == 0)
//            return type.color;
//    }

//    // If not, get color from object group
//    const ObjectGroup *objectGroup = object->objectGroup();
//    if (objectGroup && objectGroup->color().isValid())
//        return objectGroup->color();

    // Fallback color
    return Qt::gray;
    //return QColor(Qt::gray, Qt::gray, Qt::gray);
}
