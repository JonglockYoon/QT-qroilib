/*
 * objectgroup.cpp
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2008-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
 *Qroilib : QT ROI Lib
 * Copyright 2018 Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
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

#include "objectgroup.h"

#include "layer.h"
#include "roimap.h"
#include "roiobject.h"

#include <cmath>

using namespace Qroilib;

ObjectGroup::ObjectGroup()
    : Layer(ObjectGroupType, QString(), 0, 0)
    , mDrawOrder(TopDownOrder)
{
}

ObjectGroup::ObjectGroup(const QString &name, int x, int y)
    : Layer(ObjectGroupType, name, x, y)
    , mDrawOrder(TopDownOrder)
{
}

ObjectGroup::~ObjectGroup()
{
    qDeleteAll(mObjects);
}

void ObjectGroup::addObject(RoiObject *object)
{
    mObjects.append(object);
    object->setObjectGroup(this);
    if (mRoi && object->id() == 0)
        object->setId(mRoi->takeNextObjectId());
}

void ObjectGroup::insertObject(int index, RoiObject *object)
{
    mObjects.insert(index, object);
    object->setObjectGroup(this);
    if (mRoi && object->id() == 0)
        object->setId(mRoi->takeNextObjectId());
}

int ObjectGroup::removeObject(RoiObject *object)
{
    const int index = mObjects.indexOf(object);
    Q_ASSERT(index != -1);

    mObjects.removeAt(index);
    object->setObjectGroup(nullptr);
    return index;
}

void ObjectGroup::removeObjectAt(int index)
{
    RoiObject *object = mObjects.takeAt(index);
    object->setObjectGroup(nullptr);
}

void ObjectGroup::moveObjects(int from, int to, int count)
{
    // It's an error when 'to' lies within the moving range of objects
    Q_ASSERT(count >= 0);
    Q_ASSERT(to <= from || to >= from + count);

    // Nothing to be done when 'to' is the start or the end of the range, or
    // when the number of objects to be moved is 0.
    if (to == from || to == from + count || count == 0)
        return;

    const QList<RoiObject*> movingObjects = mObjects.mid(from, count);
    mObjects.erase(mObjects.begin() + from,
                   mObjects.begin() + from + count);

    if (to > from)
        to -= count;

    for (int i = 0; i < count; ++i)
        mObjects.insert(to + i, movingObjects.at(i));
}

QRectF ObjectGroup::objectsBoundingRect() const
{
    QRectF boundingRect;
    for (const RoiObject *object : mObjects)
        boundingRect = boundingRect.united(object->bounds());
    return boundingRect;
}

bool ObjectGroup::isEmpty() const
{
    return mObjects.isEmpty();
}

void ObjectGroup::offsetObjects(const QPointF &offset,
                                const QRectF &bounds,
                                bool wrapX, bool wrapY)
{
    for (RoiObject *object : mObjects) {
        const QPointF objectCenter = object->bounds().center();
        if (!bounds.contains(objectCenter))
            continue;

        QPointF newCenter(objectCenter + offset);

        if (wrapX && bounds.width() > 0) {
            qreal nx = std::fmod(newCenter.x() - bounds.left(), bounds.width());
            newCenter.setX(bounds.left() + (nx < 0 ? bounds.width() + nx : nx));
        }

        if (wrapY && bounds.height() > 0) {
            qreal ny = std::fmod(newCenter.y() - bounds.top(), bounds.height());
            newCenter.setY(bounds.top() + (ny < 0 ? bounds.height() + ny : ny));
        }

        object->setPosition(object->position() + (newCenter - objectCenter));
    }
}

/**
 * Returns a duplicate of this ObjectGroup.
 *
 * \sa Layer::clone()
 */
ObjectGroup *ObjectGroup::clone() const
{
    return initializeClone(new ObjectGroup(mName, mX, mY));
}

/**
 * Resets the ids of all objects to 0. Mostly used when new ids should be
 * assigned after the object group has been cloned.
 */
void ObjectGroup::resetObjectIds()
{
    const QList<RoiObject*> &objects = mObjects;
    for (RoiObject *object : objects)
        object->resetId();
}

/**
 * Returns the highest object id in use by this object group, or 0 if no object
 * with assigned id exists.
 */
int ObjectGroup::highestObjectId() const
{
    int id = 0;
    for (const RoiObject *object : mObjects)
        id = std::max(id, object->id());
    return id;
}

ObjectGroup *ObjectGroup::initializeClone(ObjectGroup *clone) const
{
    Layer::initializeClone(clone);
    for (const RoiObject *object : mObjects)
        clone->addObject(object->clone());
    clone->setColor(mColor);
    clone->setDrawOrder(mDrawOrder);
    return clone;
}


QString Qroilib::drawOrderToString(ObjectGroup::DrawOrder drawOrder)
{
    switch (drawOrder) {
    default:
    case ObjectGroup::UnknownOrder:
        return QLatin1String("unknown");
        break;
    case ObjectGroup::TopDownOrder:
        return QLatin1String("topdown");
        break;
    case ObjectGroup::IndexOrder:
        return QLatin1String("index");
        break;
    }
}

ObjectGroup::DrawOrder Qroilib::drawOrderFromString(const QString &string)
{
    ObjectGroup::DrawOrder drawOrder = ObjectGroup::UnknownOrder;

    if (string == QLatin1String("topdown"))
        drawOrder = ObjectGroup::TopDownOrder;
    else if (string == QLatin1String("index"))
        drawOrder = ObjectGroup::IndexOrder;

    return drawOrder;
}

const RoiObject *ObjectGroup::find(int id)
{
    for (const RoiObject *object : mObjects) {
        if (object->id() == id)
            return object;
    }
    return nullptr;
}
