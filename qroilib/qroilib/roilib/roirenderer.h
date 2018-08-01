/*
 * roirenderer.h
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

#pragma once

#include "roid_global.h"

#include <QPainter>

namespace Qroilib {

class Layer;
class RoiMap;
class RoiObject;
class RoiObjectItem;

class DocumentView;

enum RenderFlag {
    ShowTileObjectOutlines = 0x1
};

Q_DECLARE_FLAGS(RenderFlags, RenderFlag)

/**
 * This interface is used for rendering tile layers and retrieving associated
 * metrics. The different implementations deal with different roimap
 * orientations.
 */
class ROIDSHARED_EXPORT RoiRenderer
{
public:
    RoiRenderer(const RoiMap *roimap)
        : mRoi(roimap)
        , mFlags(nullptr)
        , mObjectLineWidth(2)
        , mPainterScale(1)
    {}

    virtual ~RoiRenderer() {}

    /**
     * Returns the roimap this renderer is associated with.
     */
    const RoiMap *roimap() const;

    /**
     * Returns the size in pixels of the roimap associated with this renderer.
     */
    virtual QSize mapSize() const = 0;

    /**
     * Returns the bounding rectangle in pixels of the given \a rect given in
     * tile coordinates.
     *
     * This is useful for calculating the bounding rect of a tile layer or of
     * a region of tiles that was changed.
     */
    virtual QRect boundingRect(const QRect &rect) const = 0;

    /**
     * Returns the bounding rectangle in pixels of the given \a object, as it
     * would be drawn by drawRoiObject().
     */
    virtual QRectF boundingRect(const RoiObject *object) const = 0;

    /**
     * Draws the \a object in the given \a color using the \a painter.
     */
    virtual void drawRoiObject(QPainter *painter,
                               const RoiObject *object,
                               const QColor &color) const = 0;

    /**
     * Returns the tile coordinates matching the given pixel position.
     */
    virtual QPointF pixelToRoiCoords(qreal x, qreal y) const = 0;

    inline QPointF pixelToRoiCoords(const QPointF &point) const
    { return pixelToRoiCoords(point.x(), point.y()); }

    QPolygonF pixelToScreenCoords(const QPolygonF &polygon) const
    {
        QPolygonF screenPolygon(polygon.size());
        for (int i = polygon.size() - 1; i >= 0; --i)
            screenPolygon[i] = pixelToScreenCoords(polygon[i]);
        return screenPolygon;
    }

    /**
     * Returns the pixel coordinates matching the given tile coordinates.
     */
    virtual QPointF roiToPixelCoords(qreal x, qreal y) const = 0;

    inline QPointF roiToPixelCoords(const QPointF &point) const
    { return roiToPixelCoords(point.x(), point.y()); }

    inline QRectF roiToPixelCoords(const QRectF &area) const
    {
        return QRectF(roiToPixelCoords(area.topLeft()),
                      roiToPixelCoords(area.bottomRight()));
    }

    /**
     * Returns the tile coordinates matching the given screen position.
     */
    virtual QPointF screenToRoiCoords(qreal x, qreal y) const = 0;
    inline QPointF screenToRoiCoords(const QPointF &point) const;

    /**
     * Returns the screen position matching the given tile coordinates.
     */
    virtual QPointF roiToScreenCoords(qreal x, qreal y) const = 0;
    inline QPointF roiToScreenCoords(const QPointF &point) const;

    /**
     * Returns the pixel position matching the given screen position.
     */
    virtual QPointF screenToPixelCoords(qreal x, qreal y) const = 0;
    inline QPointF screenToPixelCoords(const QPointF &point) const;

    /**
     * Returns the screen position matching the given pixel position.
     */
    virtual QPointF pixelToScreenCoords(qreal x, qreal y) const = 0;
    inline QPointF pixelToScreenCoords(const QPointF &point) const;

    qreal objectLineWidth() const { return mObjectLineWidth; }
    void setObjectLineWidth(qreal lineWidth) { mObjectLineWidth = lineWidth; }

    void setFlag(RenderFlag flag, bool enabled = true);
    bool testFlag(RenderFlag flag) const
    { return mFlags.testFlag(flag); }

    qreal painterScale() const { return mPainterScale; }
    void setPainterScale(qreal painterScale) { mPainterScale = painterScale; }

    RenderFlags flags() const { return mFlags; }
    void setFlags(RenderFlags flags) { mFlags = flags; }

    static QPolygonF lineToPolygon(const QPointF &start, const QPointF &end);

    void setMapDocument(DocumentView *mapDocument);
    DocumentView *mapDocument() const { return mRoiDocument; }

private:
    const RoiMap *mRoi;
    DocumentView *mRoiDocument;

    RenderFlags mFlags;
    qreal mObjectLineWidth;
    qreal mPainterScale;
};

inline const RoiMap *RoiRenderer::roimap() const
{
    return mRoi;
}

inline QPointF RoiRenderer::screenToRoiCoords(const QPointF &point) const
{
    return screenToRoiCoords(point.x(), point.y());
}

inline QPointF RoiRenderer::roiToScreenCoords(const QPointF &point) const
{
    return roiToScreenCoords(point.x(), point.y());
}

inline QPointF RoiRenderer::screenToPixelCoords(const QPointF &point) const
{
    return screenToPixelCoords(point.x(), point.y());
}

inline QPointF RoiRenderer::pixelToScreenCoords(const QPointF &point) const
{
    return pixelToScreenCoords(point.x(), point.y());
}


} // namespace Qroilib

Q_DECLARE_OPERATORS_FOR_FLAGS(Qroilib::RenderFlags)
