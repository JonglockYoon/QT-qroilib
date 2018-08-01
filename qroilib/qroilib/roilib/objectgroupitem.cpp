/*
 * objectgroupitem.cpp
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

#include "objectgroupitem.h"
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>

using namespace Qroilib;

ObjectGroupItem::ObjectGroupItem(ObjectGroup *objectGroup, QGraphicsItem *parent)
    : LayerItem(objectGroup, parent)
    , mRoiDocument(nullptr)
{
    // Since we don't do any painting, we can spare us the call to paint()
    setFlag(QGraphicsItem::ItemHasNoContents);
}

QRectF ObjectGroupItem::boundingRect() const
{
    return QRectF();
//    DocumentView *roimap = mapDocument();
//    if (roimap == nullptr)
//        return QRectF();
//    QSizeF size = roimap->size();
//    //const qreal zoom = roimap->zoom();
//    return QRectF(QPointF(0,0), size);
}

void ObjectGroupItem::paint(QPainter *,
                            const QStyleOptionGraphicsItem *,
                            QWidget *)
{
}
