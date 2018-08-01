/*
 * maprenderer.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "roirenderer.h"

#include <QPaintEngine>
#include <QPainter>
#include <QVector2D>

#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>

using namespace Qroilib;

void RoiRenderer::setFlag(RenderFlag flag, bool enabled)
{
    if (enabled)
        mFlags |= flag;
    else
        mFlags &= ~flag;
}

/**
 * Converts a line running from \a start to \a end to a polygon which
 * extends 5 pixels from the line in all directions.
 */
QPolygonF RoiRenderer::lineToPolygon(const QPointF &start, const QPointF &end)
{
    QPointF direction = QVector2D(end - start).normalized().toPointF();
    QPointF perpendicular(-direction.y(), direction.x());

    const qreal thickness = 5.0f; // 5 pixels on each side
    direction *= thickness;
    perpendicular *= thickness;

    QPolygonF polygon(4);
    polygon[0] = start + perpendicular - direction;
    polygon[1] = start - perpendicular - direction;
    polygon[2] = end - perpendicular + direction;
    polygon[3] = end + perpendicular + direction;
    return polygon;
}



void RoiRenderer::setMapDocument(DocumentView *mapDocument)
{
    mRoiDocument = mapDocument;
}

static void renderMissingImageMarker(QPainter &painter, const QRectF &rect)
{
    QRectF r { rect.adjusted(0.5, 0.5, -0.5, -0.5) };
    QPen pen { Qt::red, 1 };
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);

    painter.save();
    painter.fillRect(r, QColor(0, 0, 0, 128));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);
    painter.drawRect(r);
    painter.drawLine(r.topLeft(), r.bottomRight());
    painter.drawLine(r.topRight(), r.bottomLeft());
    painter.restore();
}

static bool hasOpenGLEngine(const QPainter *painter)
{
    const QPaintEngine::Type type = painter->paintEngine()->type();
    return (type == QPaintEngine::OpenGL ||
            type == QPaintEngine::OpenGL2);
}
