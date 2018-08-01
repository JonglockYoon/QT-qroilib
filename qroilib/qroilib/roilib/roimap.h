/*
 * roimap.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2010, Andrew G. Crowell <overkill9999@gmail.com>
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

#include "layer.h"
#include "object.h"

#include <QColor>
#include <QList>
#include <QMargins>
#include <QSize>

namespace Qroilib {

class ObjectGroup;

/**
 * A roimap roimap. Consists of a stack of layers, each can be either a RoiLayer
 * or an ObjectGroup.
 */
class ROIDSHARED_EXPORT RoiMap : public Object
{
public:
    /**
     * The orientation of the roimap determines how it should be rendered. An
     * Orthogonal roimap is using rectangular tiles that are aligned on a
     * straight grid. An Isometric roimap uses diamond shaped tiles that are
     * aligned on an isometric projected grid. A Hexagonal roimap uses hexagon
     * shaped tiles that fit into each other by shifting every other row.
     */
    enum Orientation {
        Unknown,
        Orthogonal,
        Isometric,
        Hexagonal
    };

    /**
     * The different formats in which the tile layer data can be stored.
     */
    enum LayerDataFormat {
        XML        = 0,
        Base64     = 1,
        Base64Gzip = 2,
        Base64Zlib = 3,
        CSV        = 4
    };

    /**
     * The order in which tiles are rendered on screen.
     */
    enum RenderOrder {
        RightDown  = 0,
        RightUp    = 1,
        LeftDown   = 2,
        LeftUp     = 3
    };

    /**
     * Which axis is staggered. Only used by the isometric staggered and
     * hexagonal roimap renderers.
     */
    enum StaggerAxis {
        StaggerX,
        StaggerY
    };

    /**
     * When staggering, specifies whether the odd or the even rows/columns are
     * shifted half a tile right/down. Only used by the isometric staggered and
     * hexagonal roimap renderers.
     */
    enum StaggerIndex {
        StaggerOdd  = 0,
        StaggerEven = 1
    };

    /**
     * Constructor, taking roimap orientation, size and tile size as parameters.
     */
    RoiMap(Orientation orientation,
        int width, int height,
        int roiWidth, int roiHeight);

    /**
     * Copy constructor. Makes sure that a deep-copy of the layers is created.
     */
    RoiMap(const RoiMap &roimap);

    /**
     * Destructor.
     */
    ~RoiMap();

    /**
     * Returns the orientation of the roimap.
     */
    Orientation orientation() const { return mOrientation; }

    /**
     * Sets the orientation of the roimap.
     */
    void setOrientation(Orientation orientation)
    { mOrientation = orientation; }

    /**
     * Returns the render order of the roimap.
     */
    RenderOrder renderOrder() const { return mRenderOrder; }

    /**
     * Sets the render order of the roimap.
     */
    void setRenderOrder(RenderOrder renderOrder)
    { mRenderOrder = renderOrder; }

    /**
     * Returns the width of this roimap in tiles.
     */
    int width() const { return mWidth; }

    /**
     * Sets the width of this roimap in tiles.
     */
    void setWidth(int width) { mWidth = width; }

    /**
     * Returns the height of this roimap in tiles.
     */
    int height() const { return mHeight; }

    /**
     * Sets the height of this roimap in tiles.
     */
    void setHeight(int height) { mHeight = height; }

    /**
     * Returns the size of this roimap. Provided for convenience.
     */
    QSize size() const { return QSize(mWidth, mHeight); }

    /**
     * Returns the tile width of this roimap.
     */
    int roiWidth() const { return mRoiWidth; }

    /**
     * Sets the width of one tile.
     */
    void setRoiWidth(int width) { mRoiWidth = width; }

    /**
     * Returns the tile height used by this roimap.
     */
    int roiHeight() const { return mRoiHeight; }

    /**
     * Sets the height of one tile.
     */
    void setRoiHeight(int height) { mRoiHeight = height; }

    /**
     * Returns the size of one tile. Provided for convenience.
     */
    QSize roiSize() const { return QSize(mRoiWidth, mRoiHeight); }

    int hexSideLength() const;
    void setHexSideLength(int hexSideLength);

    StaggerAxis staggerAxis() const;
    void setStaggerAxis(StaggerAxis staggerAxis);

    StaggerIndex staggerIndex() const;
    void setStaggerIndex(StaggerIndex staggerIndex);
    void invertStaggerIndex();

    /**
     * Returns the margins that have to be taken into account when figuring
     * out which part of the roimap to repaint after changing some tiles.
     */
    //QMargins drawMargins() const;
    //void invalidateDrawMargins();

    QMargins computeLayerOffsetMargins() const;

    /**
     * Returns the number of layers of this roimap.
     */
    int layerCount() const
    { return mLayers.size(); }

    /**
     * Convenience function that returns the number of layers of this roimap that
     * match the given \a type.
     */
    int layerCount(Layer::TypeFlag type) const;

    int objectGroupCount() const
    { return layerCount(Layer::ObjectGroupType); }



    int groupLayerCount() const
    { return layerCount(Layer::GroupLayerType); }

    /**
     * Returns the layer at the specified index.
     */
    Layer *layerAt(int index) const
    { return mLayers.at(index); }

    /**
     * Returns the list of layers of this roimap. This is useful when you want to
     * use foreach.
     */
    const QList<Layer*> &layers() const { return mLayers; }

    QList<ObjectGroup*> objectGroups() const;

    /**
     * Adds a layer to this roimap.
     */
    void addLayer(Layer *layer);


    /**
     * Returns the background color of this roimap.
     */
    const QColor &backgroundColor() const { return mBackgroundColor; }

    /**
     * Sets the background color of this roimap.
     */
    void setBackgroundColor(QColor color) { mBackgroundColor = color; }

    LayerDataFormat layerDataFormat() const
    { return mLayerDataFormat; }
    void setLayerDataFormat(LayerDataFormat format)
    { mLayerDataFormat = format; }

    void setNextObjectId(int nextId);
    int nextObjectId() const;
    int takeNextObjectId();
    void initializeObjectIds(ObjectGroup &objectGroup);

private:
    friend class GroupLayer;    // so it can call adoptLayer

    void adoptLayer(Layer *layer);

    //void recomputeDrawMargins() const;

    Orientation mOrientation;
    RenderOrder mRenderOrder;
    int mWidth;
    int mHeight;
    int mRoiWidth;
    int mRoiHeight;
    int mHexSideLength;
    StaggerAxis mStaggerAxis;
    StaggerIndex mStaggerIndex;
    QColor mBackgroundColor;
    mutable QMargins mDrawMargins;
    //mutable bool mDrawMarginsDirty;
    QList<Layer*> mLayers;
    LayerDataFormat mLayerDataFormat;
    int mNextObjectId;
};


inline int RoiMap::hexSideLength() const
{
    return mHexSideLength;
}

inline void RoiMap::setHexSideLength(int hexSideLength)
{
    mHexSideLength = hexSideLength;
}

inline RoiMap::StaggerAxis RoiMap::staggerAxis() const
{
    return mStaggerAxis;
}

inline void RoiMap::setStaggerAxis(StaggerAxis staggerAxis)
{
    mStaggerAxis = staggerAxis;
}

inline RoiMap::StaggerIndex RoiMap::staggerIndex() const
{
    return mStaggerIndex;
}

inline void RoiMap::setStaggerIndex(StaggerIndex staggerIndex)
{
    mStaggerIndex = staggerIndex;
}

inline void RoiMap::invertStaggerIndex()
{
    mStaggerIndex = static_cast<StaggerIndex>(!mStaggerIndex);
}

//inline void RoiMap::invalidateDrawMargins()
//{
//    mDrawMarginsDirty = true;
//}

/**
 * Sets the next id to be used for objects on this roimap.
 */
inline void RoiMap::setNextObjectId(int nextId)
{
    Q_ASSERT(nextId > 0);
    mNextObjectId = nextId;
}

/**
 * Returns the next object id for this roimap.
 */
inline int RoiMap::nextObjectId() const
{
    return mNextObjectId;
}

/**
 * Returns the next object id for this roimap and allocates a new one.
 */
inline int RoiMap::takeNextObjectId()
{
    return mNextObjectId++;
}


ROIDSHARED_EXPORT QString staggerAxisToString(RoiMap::StaggerAxis);
ROIDSHARED_EXPORT RoiMap::StaggerAxis staggerAxisFromString(const QString &);

ROIDSHARED_EXPORT QString staggerIndexToString(RoiMap::StaggerIndex staggerIndex);
ROIDSHARED_EXPORT RoiMap::StaggerIndex staggerIndexFromString(const QString &);

/**
 * Helper function that converts the roimap orientation to a string value. Useful
 * for roimap writers.
 *
 * @return The roimap orientation as a lowercase string.
 */
ROIDSHARED_EXPORT QString orientationToString(RoiMap::Orientation);

/**
 * Helper function that converts a string to a roimap orientation enumerator.
 * Useful for roimap readers.
 *
 * @return The roimap orientation matching the given string, or RoiMap::Unknown if
 *         the string is unrecognized.
 */
ROIDSHARED_EXPORT RoiMap::Orientation orientationFromString(const QString &);

ROIDSHARED_EXPORT QString renderOrderToString(RoiMap::RenderOrder renderOrder);
ROIDSHARED_EXPORT RoiMap::RenderOrder renderOrderFromString(const QString &);

} // namespace Qroilib

Q_DECLARE_METATYPE(Qroilib::RoiMap::Orientation)
Q_DECLARE_METATYPE(Qroilib::RoiMap::LayerDataFormat)
Q_DECLARE_METATYPE(Qroilib::RoiMap::RenderOrder)
