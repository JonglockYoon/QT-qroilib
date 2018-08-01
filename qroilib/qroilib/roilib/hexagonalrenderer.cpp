/*
 * hexagonalrenderer.cpp
 * Copyright 2011-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "hexagonalrenderer.h"

#include "roimap.h"
#include "roiobject.h"

#include <QVector2D>
#include <QtCore/qmath.h>

#include <limits>

using namespace Qroilib;

HexagonalRenderer::RenderParams::RenderParams(const RoiMap *roimap)
    : roiWidth(roimap->roiWidth() & ~1)
    , roiHeight(roimap->roiHeight() & ~1)
    , sideLengthX(0)
    , sideLengthY(0)
    , staggerX(roimap->staggerAxis() == RoiMap::StaggerX)
    , staggerEven(roimap->staggerIndex() == RoiMap::StaggerEven)
{
    if (roimap->orientation() == RoiMap::Hexagonal) {
        if (staggerX)
            sideLengthX = roimap->hexSideLength();
        else
            sideLengthY = roimap->hexSideLength();
    }

    sideOffsetX = (roiWidth - sideLengthX) / 2;
    sideOffsetY = (roiHeight - sideLengthY) / 2;

    columnWidth = sideOffsetX + sideLengthX;
    rowHeight = sideOffsetY + sideLengthY;
}


QSize HexagonalRenderer::mapSize() const
{
    const RenderParams p(roimap());

    // The roimap size is the same regardless of which indexes are shifted.
    if (p.staggerX) {
        QSize size(roimap()->width() * p.columnWidth + p.sideOffsetX,
                   roimap()->height() * (p.roiHeight + p.sideLengthY));

        if (roimap()->width() > 1)
            size.rheight() += p.rowHeight;

        return size;
    } else {
        QSize size(roimap()->width() * (p.roiWidth + p.sideLengthX),
                   roimap()->height() * p.rowHeight + p.sideOffsetY);

        if (roimap()->height() > 1)
            size.rwidth() += p.columnWidth;

        return size;
    }
}

QRect HexagonalRenderer::boundingRect(const QRect &rect) const
{
    const RenderParams p(roimap());

    QPoint topLeft = roiToScreenCoords(rect.topLeft()).toPoint();
    int width, height;

    if (p.staggerX) {
        width = rect.width() * p.columnWidth + p.sideOffsetX;
        height = rect.height() * (p.roiHeight + p.sideLengthY);

        if (rect.width() > 1) {
            height += p.rowHeight;
            if (p.doStaggerX(rect.x()))
                topLeft.ry() -= p.rowHeight;
        }
    } else {
        width = rect.width() * (p.roiWidth + p.sideLengthX);
        height = rect.height() * p.rowHeight + p.sideOffsetY;

        if (rect.height() > 1) {
            width += p.columnWidth;
            if (p.doStaggerY(rect.y()))
                topLeft.rx() -= p.columnWidth;
        }
    }

    return QRect(topLeft.x(), topLeft.y(), width, height);
}

QPointF HexagonalRenderer::roiToPixelCoords(qreal x, qreal y) const
{
    return HexagonalRenderer::roiToScreenCoords(x, y);
}

QPointF HexagonalRenderer::pixelToRoiCoords(qreal x, qreal y) const
{
    return HexagonalRenderer::screenToRoiCoords(x, y);
}

/**
 * Converts screen to tile coordinates. Sub-tile return values are not
 * supported by this renderer.
 */
QPointF HexagonalRenderer::screenToRoiCoords(qreal x, qreal y) const
{
    const RenderParams p(roimap());

    if (p.staggerX)
        x -= p.staggerEven ? p.roiWidth : p.sideOffsetX;
    else
        y -= p.staggerEven ? p.roiHeight : p.sideOffsetY;

    // Start with the coordinates of a grid-aligned tile
    QPoint referencePoint = QPoint(qFloor(x / (p.columnWidth * 2)),
                                   qFloor(y / (p.rowHeight * 2)));

    // Relative x and y position on the base square of the grid-aligned tile
    const QVector2D rel(x - referencePoint.x() * (p.columnWidth * 2),
                        y - referencePoint.y() * (p.rowHeight * 2));

    // Adjust the reference point to the correct tile coordinates
    int &staggerAxisIndex = p.staggerX ? referencePoint.rx() : referencePoint.ry();
    staggerAxisIndex *= 2;
    if (p.staggerEven)
        ++staggerAxisIndex;

    // Determine the nearest hexagon tile by the distance to the center
    QVector2D centers[4];

    if (p.staggerX) {
        const int left = p.sideLengthX / 2;
        const int centerX = left + p.columnWidth;
        const int centerY = p.roiHeight / 2;

        centers[0] = QVector2D(left,                    centerY);
        centers[1] = QVector2D(centerX,                 centerY - p.rowHeight);
        centers[2] = QVector2D(centerX,                 centerY + p.rowHeight);
        centers[3] = QVector2D(centerX + p.columnWidth, centerY);
    } else {
        const int top = p.sideLengthY / 2;
        const int centerX = p.roiWidth / 2;
        const int centerY = top + p.rowHeight;

        centers[0] = QVector2D(centerX,                 top);
        centers[1] = QVector2D(centerX - p.columnWidth, centerY);
        centers[2] = QVector2D(centerX + p.columnWidth, centerY);
        centers[3] = QVector2D(centerX,                 centerY + p.rowHeight);
    }

    int nearest = 0;
    qreal minDist = std::numeric_limits<qreal>::max();

    for (int i = 0; i < 4; ++i) {
        const QVector2D &center = centers[i];
        const qreal dc = (center - rel).lengthSquared();
        if (dc < minDist) {
            minDist = dc;
            nearest = i;
        }
    }

    static const QPoint offsetsStaggerX[4] = {
        QPoint( 0,  0),
        QPoint(+1, -1),
        QPoint(+1,  0),
        QPoint(+2,  0),
    };
    static const QPoint offsetsStaggerY[4] = {
        QPoint( 0,  0),
        QPoint(-1, +1),
        QPoint( 0, +1),
        QPoint( 0, +2),
    };

    const QPoint *offsets = p.staggerX ? offsetsStaggerX : offsetsStaggerY;
    return referencePoint + offsets[nearest];
}

/**
 * Converts tile to screen coordinates. Sub-tile return values are not
 * supported by this renderer.
 */
QPointF HexagonalRenderer::roiToScreenCoords(qreal x, qreal y) const
{
    const RenderParams p(roimap());
    const int tileX = qFloor(x);
    const int tileY = qFloor(y);
    int pixelX, pixelY;

    if (p.staggerX) {
        pixelY = tileY * (p.roiHeight + p.sideLengthY);
        if (p.doStaggerX(tileX))
            pixelY += p.rowHeight;

        pixelX = tileX * p.columnWidth;
    } else {
        pixelX = tileX * (p.roiWidth + p.sideLengthX);
        if (p.doStaggerY(tileY))
            pixelX += p.columnWidth;

        pixelY = tileY * p.rowHeight;
    }

    return QPointF(pixelX, pixelY);
}

QPoint HexagonalRenderer::topLeft(int x, int y) const
{
    if (roimap()->staggerAxis() == RoiMap::StaggerY) {
        if ((y & 1) ^ roimap()->staggerIndex())
            return QPoint(x, y - 1);
        else
            return QPoint(x - 1, y - 1);
    } else {
        if ((x & 1) ^ roimap()->staggerIndex())
            return QPoint(x - 1, y);
        else
            return QPoint(x - 1, y - 1);
    }
}

QPoint HexagonalRenderer::topRight(int x, int y) const
{
    if (roimap()->staggerAxis() == RoiMap::StaggerY) {
        if ((y & 1) ^ roimap()->staggerIndex())
            return QPoint(x + 1, y - 1);
        else
            return QPoint(x, y - 1);
    } else {
        if ((x & 1) ^ roimap()->staggerIndex())
            return QPoint(x + 1, y);
        else
            return QPoint(x + 1, y - 1);
    }
}

QPoint HexagonalRenderer::bottomLeft(int x, int y) const
{
    if (roimap()->staggerAxis() == RoiMap::StaggerY) {
        if ((y & 1) ^ roimap()->staggerIndex())
            return QPoint(x, y + 1);
        else
            return QPoint(x - 1, y + 1);
    } else {
        if ((x & 1) ^ roimap()->staggerIndex())
            return QPoint(x - 1, y + 1);
        else
            return QPoint(x - 1, y);
    }
}

QPoint HexagonalRenderer::bottomRight(int x, int y) const
{
    if (roimap()->staggerAxis() == RoiMap::StaggerY) {
        if ((y & 1) ^ roimap()->staggerIndex())
            return QPoint(x + 1, y + 1);
        else
            return QPoint(x, y + 1);
    } else {
        if ((x & 1) ^ roimap()->staggerIndex())
            return QPoint(x + 1, y + 1);
        else
            return QPoint(x + 1, y);
    }
}

QPolygonF HexagonalRenderer::tileToScreenPolygon(int x, int y) const
{
    const RenderParams p(roimap());
    const QPointF topRight = roiToScreenCoords(x, y);

    QPolygonF polygon(8);
    polygon[0] = topRight + QPoint(0,                           p.roiHeight - p.sideOffsetY);
    polygon[1] = topRight + QPoint(0,                           p.sideOffsetY);
    polygon[2] = topRight + QPoint(p.sideOffsetX,               0);
    polygon[3] = topRight + QPoint(p.roiWidth - p.sideOffsetX, 0);
    polygon[4] = topRight + QPoint(p.roiWidth,                 p.sideOffsetY);
    polygon[5] = topRight + QPoint(p.roiWidth,                 p.roiHeight - p.sideOffsetY);
    polygon[6] = topRight + QPoint(p.roiWidth - p.sideOffsetX, p.roiHeight);
    polygon[7] = topRight + QPoint(p.sideOffsetX,               p.roiHeight);
    return polygon;
}
