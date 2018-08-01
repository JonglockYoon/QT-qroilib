/*
 * staggeredrenderer.h
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

#pragma once

#include "orthogonalrenderer.h"

namespace Qroilib {

/**
 * A hexagonal renderer.
 *
 * Only pointy-topped hexagons are supported at the moment, shifting either the
 * odd or the even rows to the right.
 *
 * The same problems as present when using the StaggeredRenderer happen with
 * this renderer.
 */
class ROIDSHARED_EXPORT HexagonalRenderer : public OrthogonalRenderer
{
protected:
    struct RenderParams
    {
        RenderParams(const RoiMap *roimap);

        bool doStaggerX(int x) const
        { return staggerX && (x & 1) ^ staggerEven; }

        bool doStaggerY(int y) const
        { return !staggerX && (y & 1) ^ staggerEven; }

        const int roiWidth;
        const int roiHeight;
        int sideLengthX;
        int sideOffsetX;
        int sideLengthY;
        int sideOffsetY;
        int rowHeight;
        int columnWidth;
        const bool staggerX;
        const bool staggerEven;
    };

public:
    HexagonalRenderer(const RoiMap *roimap) : OrthogonalRenderer(roimap) {}

    QSize mapSize() const override;

    QRect boundingRect(const QRect &rect) const override;

    using OrthogonalRenderer::pixelToRoiCoords;
    QPointF pixelToRoiCoords(qreal x, qreal y) const override;

    using OrthogonalRenderer::roiToPixelCoords;
    QPointF roiToPixelCoords(qreal x, qreal y) const override;

    using OrthogonalRenderer::screenToRoiCoords;
    QPointF screenToRoiCoords(qreal x, qreal y) const override;

    using OrthogonalRenderer::roiToScreenCoords;
    QPointF roiToScreenCoords(qreal x, qreal y) const override;

    // Functions specific to this type of renderer
    QPoint topLeft(int x, int y) const;
    QPoint topRight(int x, int y) const;
    QPoint bottomLeft(int x, int y) const;
    QPoint bottomRight(int x, int y) const;

    QPolygonF tileToScreenPolygon(int x, int y) const;
};

} // namespace Qroilib
