/*
 * layer.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
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

#include "object.h"

#include <QPixmap>
#include <QRect>
#include <QSet>
#include <QString>
#include <QVector>

namespace Qroilib {

class GroupLayer;
class RoiMap;
class ObjectGroup;

/**
 * A roimap layer.
 */
class ROIDSHARED_EXPORT Layer : public Object
{
public:
    enum TypeFlag {
        ObjectGroupType = 0x02,

        GroupLayerType  = 0x08
    };

    enum { AnyLayerType = 0xFF };

    /**
     * Constructor.
     */
    Layer(TypeFlag type, const QString &name, int x, int y);

    /**
     * Returns the type of this layer.
     */
    TypeFlag layerType() const { return mLayerType; }

    /**
     * Returns the name of this layer.
     */
    const QString &name() const { return mName; }

    /**
     * Sets the name of this layer.
     */
    void setName(const QString &name) { mName = name; }

    /**
     * Returns the opacity of this layer.
     */
    float opacity() const { return mOpacity; }

    /**
     * Sets the opacity of this layer.
     */
    void setOpacity(float opacity) { mOpacity = opacity; }

    float effectiveOpacity() const;

    /**
     * Returns the visibility of this layer.
     */
    bool isVisible() const { return mVisible; }

    bool isHidden() const;

    /**
     * Sets the visibility of this layer.
     */
    void setVisible(bool visible) { mVisible = visible; }

    /**
     * Returns the roimap this layer is part of.
     */
    RoiMap *roimap() const { return mRoi; }

    /**
     * Returns the parent layer, if any.
     */
    GroupLayer *parentLayer() const { return mParentLayer; }

    bool isParentOrSelf(const Layer *candidate) const;
    int depth() const;
    int siblingIndex() const;
    QList<Layer*> siblings() const;

    /**
     * Returns the x position of this layer (in tiles).
     */
    int x() const { return mX; }

    /**
     * Sets the x position of this layer (in tiles).
     */
    void setX(int x) { mX = x; }

    /**
     * Returns the y position of this layer (in tiles).
     */
    int y() const { return mY; }

    /**
     * Sets the y position of this layer (in tiles).
     */
    void setY(int y) { mY = y; }

    /**
     * Returns the position of this layer (in tiles).
     */
    QPoint position() const { return QPoint(mX, mY); }

    /**
     * Sets the position of this layer (in tiles).
     */
    void setPosition(QPoint pos) { setPosition(pos.x(), pos.y()); }
    void setPosition(int x, int y) { mX = x; mY = y; }

    void setOffset(const QPointF &offset);
    QPointF offset() const;

    QPointF totalOffset() const;

    virtual bool isEmpty() const = 0;
    /**
     * Returns a duplicate of this layer. The caller is responsible for the
     * ownership of this newly created layer.
     */
    virtual Layer *clone() const = 0;

    // These functions allow checking whether this Layer is an instance of the
    // given subclass without relying on a dynamic_cast.
    bool isObjectGroup() const { return mLayerType == ObjectGroupType; }

    bool isGroupLayer() const { return mLayerType == GroupLayerType; }

    // These actually return this layer cast to one of its subclasses.
    ObjectGroup *asObjectGroup();

    GroupLayer *asGroupLayer();

protected:
    /**
     * Sets the roimap this layer is part of. Should only be called from the
     * RoiMap class.
     */
    virtual void setMap(RoiMap *roimap) { mRoi = roimap; }
    void setParentLayer(GroupLayer *groupLayer) { mParentLayer = groupLayer; }

    Layer *initializeClone(Layer *clone) const;

    QString mName;
    TypeFlag mLayerType;
    int mX;
    int mY;
    QPointF mOffset;
    float mOpacity;
    bool mVisible;
    RoiMap *mRoi;
    GroupLayer *mParentLayer;

    friend class RoiMap;
    friend class GroupLayer;
};


/**
 * Sets the drawing offset in pixels of this layer.
 */
inline void Layer::setOffset(const QPointF &offset)
{
    mOffset = offset;
}

/**
 * Returns the drawing offset in pixels of this layer.
 */
inline QPointF Layer::offset() const
{
    return mOffset;
}


/**
 * An iterator for iterating over the layers of a roimap. When iterating forward,
 * group layers are traversed after their children.
 *
 * Modifying the layer hierarchy while an iterator is active will lead to
 * undefined results!
 */
class ROIDSHARED_EXPORT LayerIterator
{
public:
    LayerIterator(const RoiMap *roimap);
    LayerIterator(Layer *start);

    Layer *currentLayer() const;
    int currentSiblingIndex() const;

    bool hasNextSibling() const;
    bool hasPreviousSibling() const;
    bool hasParent() const;

    Layer *next();
    Layer *previous();

    void toFront();
    void toBack();

private:
    const RoiMap *mRoi;
    Layer *mCurrentLayer;
    int mSiblingIndex;
};


/**
 * Iterate the given roimap, starting from the first layer.
 */
inline LayerIterator::LayerIterator(const RoiMap *roimap)
    : mRoi(roimap)
    , mCurrentLayer(nullptr)
    , mSiblingIndex(-1)
{}

/**
 * Iterate the layer's roimap, starting at the given \a layer.
 */
inline LayerIterator::LayerIterator(Layer *start)
    : mRoi(start ? start->roimap() : nullptr)
    , mCurrentLayer(start)
    , mSiblingIndex(start ? start->siblingIndex() : -1)
{}

inline Layer *LayerIterator::currentLayer() const
{
    return mCurrentLayer;
}

inline int LayerIterator::currentSiblingIndex() const
{
    return mSiblingIndex;
}

inline bool LayerIterator::hasNextSibling() const
{
    if (!mCurrentLayer)
        return false;

    return mSiblingIndex + 1 < mCurrentLayer->siblings().size();
}

inline bool LayerIterator::hasPreviousSibling() const
{
    return mSiblingIndex > 0;
}

inline bool LayerIterator::hasParent() const
{
    return mCurrentLayer && mCurrentLayer->parentLayer();
}


ROIDSHARED_EXPORT int globalIndex(Layer *layer);
ROIDSHARED_EXPORT Layer *layerAtGlobalIndex(const RoiMap *roimap, int index);

} // namespace Qroilib
