/*
 * selectionrectangle.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * *Qroilib : QT ROI Lib
 * Copyright 2018 Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
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

#include "selectionrectangle.h"

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QDebug>
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>

using namespace Qroilib;

//class DocumentView;

SelectionRectangle::SelectionRectangle(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setZValue(10000);
    //zoom = 1.0;
}

void SelectionRectangle::setRectangle(const QRectF &rectangle)
{
    prepareGeometryChange();
    mRectangle = rectangle;
}

QRectF SelectionRectangle::boundingRect() const
{
    RasterImageView *pView = mapDocument()->imageView();
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = mapDocument()->zoom();

    QRectF rect = mRectangle;
    rect.setLeft(mRectangle.left()/scale + offset.x() - scroll.x()/scale);
    rect.setRight(mRectangle.right()/scale + offset.x() - scroll.x()/scale);
    rect.setTop(mRectangle.top()/scale + offset.y() - scroll.y()/scale);
    rect.setBottom(mRectangle.bottom()/scale + offset.y() - scroll.y()/scale);
    //qDebug() << "boundingRect" << rect << scroll;
    return rect;
}

void SelectionRectangle::paint(QPainter *painter,
                               const QStyleOptionGraphicsItem *, QWidget *)
{
    if (mRectangle.isNull())
        return;

    RasterImageView *pView = mapDocument()->imageView();
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = mapDocument()->zoom();
    QRectF rect;
    rect.setLeft(mRectangle.left()/scale + offset.x() - scroll.x()/scale);
    rect.setRight(mRectangle.right()/scale + offset.x() - scroll.x()/scale);
    rect.setTop(mRectangle.top()/scale + offset.y() - scroll.y()/scale);
    rect.setBottom(mRectangle.bottom()/scale + offset.y() - scroll.y()/scale);
    // Draw a shadow
    QColor black(Qt::black);
    black.setAlpha(128);
    QPen pen(black, 2, Qt::DotLine);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->drawRect(rect.translated(1, 1));

    // Draw a rectangle in the highlight color
    QColor highlight = QApplication::palette().highlight().color();
    pen.setColor(highlight);
    highlight.setAlpha(32);
    painter->setPen(pen);
    painter->setBrush(highlight);
    painter->drawRect(rect);

}
