/*
 * roimap.cpp
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

#include "roimap.h"

#include "layer.h"
#include "objectgroup.h"
#include "roiobject.h"

#include <cmath>

using namespace Qroilib;

RoiMap::RoiMap(Orientation orientation,
         int width, int height, int roiWidth, int roiHeight):
    Object(MapType),
    mOrientation(orientation),
    mRenderOrder(RightDown),
    mWidth(width),
    mHeight(height),
    mRoiWidth(roiWidth),
    mRoiHeight(roiHeight),
    mHexSideLength(0),
    mStaggerAxis(StaggerY),
    mStaggerIndex(StaggerOdd),
    //mDrawMarginsDirty(true),
    mLayerDataFormat(Base64Zlib),
    mNextObjectId(1)
{
}

RoiMap::RoiMap(const RoiMap &roimap):
    Object(roimap),
    mOrientation(roimap.mOrientation),
    mRenderOrder(roimap.mRenderOrder),
    mWidth(roimap.mWidth),
    mHeight(roimap.mHeight),
    mRoiWidth(roimap.mRoiWidth),
    mRoiHeight(roimap.mRoiHeight),
    mHexSideLength(roimap.mHexSideLength),
    mStaggerAxis(roimap.mStaggerAxis),
    mStaggerIndex(roimap.mStaggerIndex),
    mBackgroundColor(roimap.mBackgroundColor),
    mDrawMargins(roimap.mDrawMargins),
    //mDrawMarginsDirty(roimap.mDrawMarginsDirty),
    mLayerDataFormat(roimap.mLayerDataFormat),
    mNextObjectId(1)
{
    for (const Layer *layer : roimap.mLayers) {
        Layer *clone = layer->clone();
        clone->setMap(this);
        mLayers.append(clone);
    }
}

RoiMap::~RoiMap()
{
    qDeleteAll(mLayers);
}

//QMargins RoiMap::drawMargins() const
//{
//    if (mDrawMarginsDirty)
//        recomputeDrawMargins();

//    return mDrawMargins;
//}

static QMargins maxMargins(const QMargins &a,
                           const QMargins &b)
{
    return QMargins(qMax(a.left(), b.left()),
                    qMax(a.top(), b.top()),
                    qMax(a.right(), b.right()),
                    qMax(a.bottom(), b.bottom()));
}

/**
 * Computes the extra margins due to layer offsets. These need to be taken into
 * account when determining the bounding rect of the roimap for example.
 */
QMargins RoiMap::computeLayerOffsetMargins() const
{
    QMargins offsetMargins;

    for (const Layer *layer : mLayers) {
        const QPointF offset = layer->offset();
        offsetMargins = maxMargins(QMargins(std::ceil(-offset.x()),
                                            std::ceil(-offset.y()),
                                            std::ceil(offset.x()),
                                            std::ceil(offset.y())),
                                   offsetMargins);
    }

    return offsetMargins;
}

int RoiMap::layerCount(Layer::TypeFlag type) const
{
    int count = 0;
    LayerIterator iterator(this);
    while (Layer *layer = iterator.next())
       if (layer->layerType() == type)
           count++;
    return count;
}

QList<ObjectGroup*> RoiMap::objectGroups() const
{
    QList<ObjectGroup*> layers;
    LayerIterator iterator(this);
    while (Layer *layer = iterator.next())
        if (ObjectGroup *og = layer->asObjectGroup())
            layers.append(og);
    return layers;
}

void RoiMap::addLayer(Layer *layer)
{
    adoptLayer(layer);
    mLayers.append(layer);
}

void RoiMap::adoptLayer(Layer *layer)
{
    layer->setMap(this);

    if (ObjectGroup *group = layer->asObjectGroup())
        initializeObjectIds(*group);
}

void RoiMap::initializeObjectIds(ObjectGroup &objectGroup)
{
    for (RoiObject *o : objectGroup) {
        if (o->id() == 0)
            o->setId(takeNextObjectId());
    }
}


QString Qroilib::staggerAxisToString(RoiMap::StaggerAxis staggerAxis)
{
    switch (staggerAxis) {
    default:
    case RoiMap::StaggerY:
        return QLatin1String("y");
        break;
    case RoiMap::StaggerX:
        return QLatin1String("x");
        break;
    }
}

RoiMap::StaggerAxis Qroilib::staggerAxisFromString(const QString &string)
{
    RoiMap::StaggerAxis staggerAxis = RoiMap::StaggerY;
    if (string == QLatin1String("x"))
        staggerAxis = RoiMap::StaggerX;
    return staggerAxis;
}

QString Qroilib::staggerIndexToString(RoiMap::StaggerIndex staggerIndex)
{
    switch (staggerIndex) {
    default:
    case RoiMap::StaggerOdd:
        return QLatin1String("odd");
        break;
    case RoiMap::StaggerEven:
        return QLatin1String("even");
        break;
    }
}

RoiMap::StaggerIndex Qroilib::staggerIndexFromString(const QString &string)
{
    RoiMap::StaggerIndex staggerIndex = RoiMap::StaggerOdd;
    if (string == QLatin1String("even"))
        staggerIndex = RoiMap::StaggerEven;
    return staggerIndex;
}

QString Qroilib::orientationToString(RoiMap::Orientation orientation)
{
    switch (orientation) {
    default:
    case RoiMap::Unknown:
        return QLatin1String("unknown");
        break;
    case RoiMap::Orthogonal:
        return QLatin1String("orthogonal");
        break;
    case RoiMap::Isometric:
        return QLatin1String("isometric");
        break;
    case RoiMap::Hexagonal:
        return QLatin1String("hexagonal");
        break;
    }
}

RoiMap::Orientation Qroilib::orientationFromString(const QString &string)
{
    RoiMap::Orientation orientation = RoiMap::Unknown;
    if (string == QLatin1String("orthogonal")) {
        orientation = RoiMap::Orthogonal;
    } else if (string == QLatin1String("isometric")) {
        orientation = RoiMap::Isometric;
    } else if (string == QLatin1String("hexagonal")) {
        orientation = RoiMap::Hexagonal;
    }
    return orientation;
}

QString Qroilib::renderOrderToString(RoiMap::RenderOrder renderOrder)
{
    switch (renderOrder) {
    default:
    case RoiMap::RightDown:
        return QLatin1String("right-down");
        break;
    case RoiMap::RightUp:
        return QLatin1String("right-up");
        break;
    case RoiMap::LeftDown:
        return QLatin1String("left-down");
        break;
    case RoiMap::LeftUp:
        return QLatin1String("left-up");
        break;
    }
}

RoiMap::RenderOrder Qroilib::renderOrderFromString(const QString &string)
{
    RoiMap::RenderOrder renderOrder = RoiMap::RightDown;
    if (string == QLatin1String("right-up")) {
        renderOrder = RoiMap::RightUp;
    } else if (string == QLatin1String("left-down")) {
        renderOrder = RoiMap::LeftDown;
    } else if (string == QLatin1String("left-up")) {
        renderOrder = RoiMap::LeftUp;
    }
    return renderOrder;
}
