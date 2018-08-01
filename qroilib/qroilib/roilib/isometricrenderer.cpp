/*
 * isometricrenderer.cpp
 * Copyright 2009-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of qroilib.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "isometricrenderer.h"

#include "roimap.h"
#include "roiobject.h"
#include <qroilib/documentview/documentview.h>

#include <cmath>

using namespace Qroilib;

QSize IsometricRenderer::mapSize() const
{
    // RoiMap width and height contribute equally in both directions
    const int side = roimap()->height() + roimap()->width();
    return QSize(side * roimap()->roiWidth() / 2,
                 side * roimap()->roiHeight() / 2);
}

QRect IsometricRenderer::boundingRect(const QRect &rect) const
{
    const int roiWidth = roimap()->roiWidth();
    const int roiHeight = roimap()->roiHeight();

    const int originX = roimap()->height() * roiWidth / 2;
    const QPoint pos((rect.x() - (rect.y() + rect.height()))
                     * roiWidth / 2 + originX,
                     (rect.x() + rect.y()) * roiHeight / 2);

    const int side = rect.height() + rect.width();
    const QSize size(side * roiWidth / 2,
                     side * roiHeight / 2);

    return QRect(pos, size);
}

QRectF IsometricRenderer::boundingRect(const RoiObject *object) const
{
    if (object->shape() == RoiObject::Text) {
        const QPointF topLeft = pixelToScreenCoords(object->position());
        return QRectF(topLeft, object->size());
    } else if (!object->polygon().isEmpty()) {
        qreal extraSpace = qMax(objectLineWidth(), qreal(1));

        // Make some more room for the starting dot
        extraSpace += objectLineWidth() * 4;

        const QPointF &pos = object->position();
        const QPolygonF polygon = object->polygon().translated(pos);
        const QPolygonF screenPolygon = pixelToScreenCoords(polygon);
        return screenPolygon.boundingRect().adjusted(-extraSpace,
                                                     -extraSpace - 1,
                                                     extraSpace,
                                                     extraSpace);
    } else {
        // Take the bounding rect of the projected object, and then add a few
        // pixels on all sides to correct for the line width.
        const QRectF base = pixelRectToScreenPolygon(object->bounds()).boundingRect();
        const qreal extraSpace = qMax(objectLineWidth() / 2, qreal(1));

        return base.adjusted(-extraSpace,
                             -extraSpace - 1,
                             extraSpace, extraSpace);
    }
}

void IsometricRenderer::drawRoiObject(QPainter *painter,
                                      const RoiObject *object,
                                      const QColor &color) const
{
    painter->save();

    QPen pen(Qt::black);
    pen.setCosmetic(true);

    if (object->shape() == RoiObject::Text) {
        const QPointF pos = pixelToScreenCoords(object->position());
        const auto& textData = object->textData();

        painter->setFont(textData.font);
        painter->setPen(textData.color);
        painter->drawText(QRectF(pos, object->size()),
                          textData.text,
                          textData.textOption());
    } else {
        const qreal lineWidth = objectLineWidth();
        //const qreal scale = painterScale();
        const qreal scale = mapDocument()->zoom();
        const qreal shadowOffset = (lineWidth == 0 ? 1 : lineWidth) / scale;

        QColor brushColor = color;
        brushColor.setAlpha(50);
        QBrush brush(brushColor);

        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidth(lineWidth);

        QPen colorPen(pen);
        colorPen.setColor(color);

        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing);

        // TODO: Do something sensible to make null-sized objects usable

        switch (object->shape()) {
        case RoiObject::Ellipse: {
            QPolygonF polygon = pixelRectToScreenPolygon(object->bounds());

            float tw = roimap()->roiWidth();
            float th = roimap()->roiHeight();
            QPointF transformScale(1, 1);
            if (tw > th)
                transformScale = QPointF(1, th/tw);
            else
                transformScale = QPointF(tw/th, 1);

            QPointF l1 = polygon.at(1) - polygon.at(0);
            QPointF l2 = polygon.at(3) - polygon.at(0);
            QTransform trans;
            trans.scale(transformScale.x(), transformScale.y());
            trans.rotate(45);
            QTransform iTrans = trans.inverted();
            QPointF l1x = iTrans.map(l1);
            QPointF l2x = iTrans.map(l2);
            QSizeF ellipseSize(l1x.manhattanLength(), l2x.manhattanLength());

            if (ellipseSize.width() > 0 && ellipseSize.height() > 0) {
                painter->save();
                painter->setPen(pen);
                painter->translate(polygon.at(0));
                //ainter->scale(transformScale.x(), transformScale.y());
                painter->rotate(45);
                painter->drawEllipse(QRectF(QPointF(0, 0), ellipseSize));
                painter->restore();
            }

            painter->setBrush(Qt::NoBrush);
            painter->drawPolygon(polygon);

            painter->setPen(colorPen);
            painter->setBrush(Qt::NoBrush);
            painter->translate(QPointF(0, -shadowOffset));
            painter->drawPolygon(polygon);

            painter->setBrush(brush);
            if (ellipseSize.width() > 0 && ellipseSize.height() > 0) {
                painter->save();
                painter->translate(polygon.at(0));
                //painter->scale(transformScale.x(), transformScale.y());
                painter->rotate(45);
                painter->drawEllipse(QRectF(QPointF(0, 0), ellipseSize));
                painter->restore();
            }
            break;
        }
        case RoiObject::Rectangle: {
            QPolygonF polygon = pixelRectToScreenPolygon(object->bounds());
            painter->drawPolygon(polygon);

            painter->setPen(colorPen);
            painter->setBrush(brush);
            polygon.translate(0, -shadowOffset);
            painter->drawPolygon(polygon);
            break;
        }
        case RoiObject::Point: {
            QRectF r = object->bounds();
            QPointF p = r.center();
            int w = r.width() / 2;
            int h = r.height() / 2;
            static QLine lines[] = {
                QLine(-w,0, w,0),
                QLine(0,-h, 0,h),
            };

            painter->setPen(colorPen);
            painter->setBrush(brush);
            painter->drawLines(lines, sizeof(lines) / sizeof(lines[0]));
            painter->translate(1, 1);
            painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
            painter->drawLines(lines, sizeof(lines) / sizeof(lines[0]));

            break;
        }
        case RoiObject::Polygon: {
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            QPolygonF screenPolygon = pixelToScreenCoords(polygon);

            QPen thickPen(pen);
            QPen thickColorPen(colorPen);
            thickPen.setWidthF(thickPen.widthF() * 4);
            thickColorPen.setWidthF(thickColorPen.widthF() * 4);

            painter->drawPolygon(screenPolygon);
            painter->setPen(thickPen);
            painter->drawPoint(screenPolygon.first());

            painter->setPen(colorPen);
            painter->setBrush(brush);
            screenPolygon.translate(0, -shadowOffset);

            painter->drawPolygon(screenPolygon);
            painter->setPen(thickColorPen);
            painter->drawPoint(screenPolygon.first());

            break;
        }
        case RoiObject::Polyline: {
            const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon().translated(pos);
            QPolygonF screenPolygon = pixelToScreenCoords(polygon);

            QPen thickPen(pen);
            QPen thickColorPen(colorPen);
            thickPen.setWidthF(thickPen.widthF() * 4);
            thickColorPen.setWidthF(thickColorPen.widthF() * 4);

            painter->drawPolyline(screenPolygon);
            painter->setPen(thickPen);
            painter->drawPoint(screenPolygon.first());

            pen.setColor(color);
            painter->setPen(pen);
            screenPolygon.translate(0, -shadowOffset);

            painter->drawPolyline(screenPolygon);
            painter->setPen(thickColorPen);
            painter->drawPoint(screenPolygon.first());
            break;
        }
        case RoiObject::Text:
            break;  // already handled above
        }
    }

    painter->restore();
}

QPointF IsometricRenderer::pixelToRoiCoords(qreal x, qreal y) const
{
    const int roiHeight = roimap()->roiHeight();

    return QPointF(x / roiHeight, y / roiHeight);
}

QPointF IsometricRenderer::roiToPixelCoords(qreal x, qreal y) const
{
    const int roiHeight = roimap()->roiHeight();

    return QPointF(x * roiHeight, y * roiHeight);
}

QPointF IsometricRenderer::screenToRoiCoords(qreal x, qreal y) const
{
    const int roiWidth = roimap()->roiWidth();
    const int roiHeight = roimap()->roiHeight();

    x -= roimap()->height() * roiWidth / 2;
    const qreal tileY = y / roiHeight;
    const qreal tileX = x / roiWidth;

    return QPointF(tileY + tileX,
                   tileY - tileX);
}

QPointF IsometricRenderer::roiToScreenCoords(qreal x, qreal y) const
{
    const int roiWidth = roimap()->roiWidth();
    const int roiHeight = roimap()->roiHeight();
    const int originX = roimap()->height() * roiWidth / 2;

    return QPointF((x - y) * roiWidth / 2 + originX,
                   (x + y) * roiHeight / 2);
}

QPointF IsometricRenderer::screenToPixelCoords(qreal x, qreal y) const
{
    const int roiWidth = roimap()->roiWidth();
    const int roiHeight = roimap()->roiHeight();

    x -= roimap()->height() * roiWidth / 2;
    const qreal tileY = y / roiHeight;
    const qreal tileX = x / roiWidth;

    return QPointF((tileY + tileX) * roiHeight,
                   (tileY - tileX) * roiHeight);
}

QPointF IsometricRenderer::pixelToScreenCoords(qreal x, qreal y) const
{
    const int roiWidth = roimap()->roiWidth();
    const int roiHeight = roimap()->roiHeight();
    const int originX = roimap()->height() * roiWidth / 2;
    const qreal tileY = y / roiHeight;
    const qreal tileX = x / roiHeight;

    return QPointF((tileX - tileY) * roiWidth / 2 + originX,
                   (tileX + tileY) * roiHeight / 2);
}

QPolygonF IsometricRenderer::pixelRectToScreenPolygon(const QRectF &rect) const
{
    QPolygonF polygon;
    polygon << QPointF(pixelToScreenCoords(rect.topLeft()));
    polygon << QPointF(pixelToScreenCoords(rect.topRight()));
    polygon << QPointF(pixelToScreenCoords(rect.bottomRight()));
    polygon << QPointF(pixelToScreenCoords(rect.bottomLeft()));
    return polygon;
}

QPolygonF IsometricRenderer::tileRectToScreenPolygon(const QRect &rect) const
{
    const int roiWidth = roimap()->roiWidth();
    const int roiHeight = roimap()->roiHeight();

    const QPointF topRight = roiToScreenCoords(rect.topRight());
    const QPointF bottomRight = roiToScreenCoords(rect.bottomRight());
    const QPointF bottomLeft = roiToScreenCoords(rect.bottomLeft());

    QPolygonF polygon;
    polygon << QPointF(roiToScreenCoords(rect.topLeft()));
    polygon << QPointF(topRight.x() + roiWidth / 2,
                       topRight.y() + roiHeight / 2);
    polygon << QPointF(bottomRight.x(), bottomRight.y() + roiHeight);
    polygon << QPointF(bottomLeft.x() - roiWidth / 2,
                       bottomLeft.y() + roiHeight / 2);
    return polygon;
}
