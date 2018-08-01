/*
 * isometricrenderer.h
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#pragma once

#include "roirenderer.h"

namespace Qroilib {

/**
 * An isometric roimap renderer.
 *
 * Isometric maps have diamond shaped tiles. This roimap renderer renders them in
 * such a way that the roimap will also be diamond shaped. The X axis points to
 * the bottom right while the Y axis points to the bottom left.
 */
class ROIDSHARED_EXPORT IsometricRenderer : public RoiRenderer
{
public:
    IsometricRenderer(const RoiMap *roimap) : RoiRenderer(roimap) {}

    QSize mapSize() const override;

    QRect boundingRect(const QRect &rect) const override;

    QRectF boundingRect(const RoiObject *object) const override;

    void drawRoiObject(QPainter *painter,
                       const RoiObject *object,
                       const QColor &color) const override;

    using RoiRenderer::pixelToRoiCoords;
    QPointF pixelToRoiCoords(qreal x, qreal y) const override;

    using RoiRenderer::roiToPixelCoords;
    QPointF roiToPixelCoords(qreal x, qreal y) const override;

    using RoiRenderer::screenToRoiCoords;
    QPointF screenToRoiCoords(qreal x, qreal y) const override;

    using RoiRenderer::roiToScreenCoords;
    QPointF roiToScreenCoords(qreal x, qreal y) const override;

    using RoiRenderer::screenToPixelCoords;
    QPointF screenToPixelCoords(qreal x, qreal y) const override;

    using RoiRenderer::pixelToScreenCoords;
    QPointF pixelToScreenCoords(qreal x, qreal y) const override;

private:
    QPolygonF pixelRectToScreenPolygon(const QRectF &rect) const;
    QPolygonF pixelOriginToScreenPolygon(const QRectF &rect) const;
    QPolygonF tileRectToScreenPolygon(const QRect &rect) const;
};

} // namespace Qroilib
