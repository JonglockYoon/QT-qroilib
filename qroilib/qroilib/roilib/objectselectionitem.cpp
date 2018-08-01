/*
 * objectselectionitem.cpp
 * Copyright 2015-2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *Qroilib : QT ROI Lib
 * Copyright 2018 Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "objectselectionitem.h"

#include "roiobject.h"
#include "roiobjectitem.h"
#include "roirenderer.h"
#include "objectgroup.h"
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>

#include <QGuiApplication>
#include <QTimerEvent>

namespace Qroilib {

static const qreal labelMargin = 3;
static const qreal labelDistance = 12;

// TODO: Unduplicate the following helper functions between this and
// ObjectSelectionTool

static QPointF alignmentOffset(QRectF &r, Alignment alignment)
{
    switch (alignment) {
    case TopLeft:       break;
    case Top:           return QPointF(r.width() / 2, 0);               break;
    case TopRight:      return QPointF(r.width(), 0);                   break;
    case Left:          return QPointF(0, r.height() / 2);              break;
    case Center:        return QPointF(r.width() / 2, r.height() / 2);  break;
    case Right:         return QPointF(r.width(), r.height() / 2);      break;
    case BottomLeft:    return QPointF(0, r.height());                  break;
    case Bottom:        return QPointF(r.width() / 2, r.height());      break;
    case BottomRight:   return QPointF(r.width(), r.height());          break;
    }
    return QPointF();
}

// TODO: Check whether this function should be moved into RoiObject::bounds
static void align(QRectF &r, Alignment alignment)
{
    r.translate(-alignmentOffset(r, alignment));
}

/* This function returns the actual bounds of the object, as opposed to the
 * bounds of its visualization that the RoiRenderer::boundingRect function
 * returns.
 */
static QRectF objectBounds(RoiObject *object,
                           const RoiRenderer *renderer)
{
    {
        switch (object->shape()) {
        case RoiObject::Ellipse:
        case RoiObject::Rectangle:
        case RoiObject::Point: {
            QRectF bounds(object->bounds());
            align(bounds, object->alignment());
            QPolygonF screenPolygon = renderer->pixelToScreenCoords(bounds);
            return screenPolygon.boundingRect();
        }
        case RoiObject::Polygon:
        case RoiObject::Polyline: {
            // Alignment is irrelevant for polygon objects since they have no size

            const QPointF pos = object->position() - object->oldPosition();

            const QPolygonF &polygon = object->polygon().translated(pos);
            object->setPolygon(polygon);
            const QPolygonF screenPolygon = renderer->pixelToScreenCoords(polygon);

            return screenPolygon.boundingRect();
        }
        case RoiObject::Text:
            return renderer->boundingRect(object);
        case RoiObject::Pattern: {
            QRectF bounds(object->bounds());
            align(bounds, object->alignment());
            QPolygonF screenPolygon = renderer->pixelToScreenCoords(bounds);
            return screenPolygon.boundingRect();
            }
            break;
        }
    }

    return QRectF();
}


//static Preferences::ObjectLabelVisiblity objectLabelVisibility()
//{
//    return Preferences::instance()->objectLabelVisibility();
//}


class RoiObjectOutline : public QGraphicsObject
{
public:
    RoiObjectOutline(RoiObject *object, QGraphicsItem *parent = nullptr)
        : QGraphicsObject(parent)
        , mObject(object)
    {
        setZValue(1); // makes sure outlines are above labels
    }

    void syncWithRoiObject(RoiRenderer *renderer);

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;
    //void setOffset(QPointF offsetIn) { offset = offsetIn; }
    //void setScale(qreal scaleIn) { scale = scaleIn; }

    void setMapDocument(DocumentView *roimap) { mRoiDocument = roimap; }
    DocumentView *mapDocument() const { return mRoiDocument; }

protected:
    void timerEvent(QTimerEvent *event) override;

private:
     DocumentView *mRoiDocument;

    QRectF mBoundingRect;
    RoiObject *mObject;
    //QPointF offset;
    //qreal scale;

    // Marching ants effect
    int mUpdateTimer = startTimer(100);
    int mOffset = 0;
};

void RoiObjectOutline::syncWithRoiObject(RoiRenderer *renderer)
{
    //const QPointF pixelPos = renderer->pixelToScreenCoords(mObject->position());
    QRectF bounds = objectBounds(mObject, renderer);

    // 회전에 대한 기능은 검증되지 않음. - 2018.5.22
    QTransform t;
    t.translate(bounds.x(), bounds.y());
    t.rotate(rotation() + mObject->rotation());
    t.translate(-bounds.x(), -bounds.y());
    setTransform(t);

    if (mBoundingRect != bounds) {
        prepareGeometryChange();
        mBoundingRect = bounds;
    }
}

QRectF RoiObjectOutline::boundingRect() const
{
    if (mObject->objectGroup() == nullptr)
        return QRectF();
    if (!mObject->objectGroup()->isVisible() || !mObject->isVisible())
        return QRectF();
    DocumentView *v = mapDocument();
    if (v->bMultiView)   // view가 두개 이상 display되어 있으면 ROI정보를 나타내지 않는다.
        return QRectF();
    if (!v->isVisible())
        return QRectF();

    RasterImageView *pView = mapDocument()->imageView();
    //QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = mapDocument()->zoom();

    QRectF rect;
    rect.setLeft(mBoundingRect.left()/scale + scroll.x()/scale);
    rect.setTop(mBoundingRect.top()/scale + scroll.y()/scale);
    rect.setRight(rect.x() + mBoundingRect.width());
    rect.setBottom(rect.y() + mBoundingRect.height());

    return rect;
}

void RoiObjectOutline::paint(QPainter *painter,
                          const QStyleOptionGraphicsItem *,
                          QWidget *)
{
    if (mObject->objectGroup() == nullptr)
        return;
    if (!mObject->objectGroup()->isVisible() || !mObject->isVisible())
        return;
    DocumentView *v = mapDocument();
    if (v->bMultiView)   // view가 두개 이상 display되어 있으면 ROI정보를 나타내지 않는다.
        return;
    if (!v->isVisible())
        return;

    RasterImageView *pView = mapDocument()->imageView();
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = mapDocument()->zoom();

    const QLineF lines[4] = {
        QLineF(mBoundingRect.topLeft()*scale + offset - scroll, mBoundingRect.topRight()*scale + offset - scroll),
        QLineF(mBoundingRect.bottomLeft()*scale + offset - scroll, mBoundingRect.bottomRight()*scale + offset - scroll),
        QLineF(mBoundingRect.topLeft()*scale + offset - scroll, mBoundingRect.bottomLeft()*scale + offset - scroll),
        QLineF(mBoundingRect.topRight()*scale + offset - scroll, mBoundingRect.bottomRight()*scale + offset - scroll)
    };


    //painter->translate(offset);

    // Draw a solid white line
    QPen pen(Qt::SolidLine);
    pen.setCosmetic(true);
    pen.setColor(Qt::white);
    painter->setPen(pen);
    painter->drawLines(lines, 4);

    // Draw a black dashed line above the white line
    pen.setColor(Qt::black);
    pen.setCapStyle(Qt::FlatCap);
    pen.setDashPattern({5, 5});
    pen.setDashOffset(mOffset);
    painter->setPen(pen);
    painter->drawLines(lines, 4);

    //painter->translate(-offset);

    //qDebug() << "RoiObjectOutline::paint()" << mOffset;

}

void RoiObjectOutline::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mUpdateTimer) {
        // Update offset used in drawing black dashed line
        mOffset++;
        update();
        //qDebug() << mBoundingRect;
    } else {
        QGraphicsObject::timerEvent(event);
    }
}


class RoiObjectLabel : public QGraphicsItem
{
public:
    RoiObjectLabel(RoiObject *object, QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
        , mObject(object)
        , mColor(RoiObjectItem::objectColor(mObject))
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations |
                 QGraphicsItem::ItemIgnoresParentOpacity);
    }
    void setMapDocument(DocumentView *roimap) { mRoiDocument = roimap; }
    DocumentView *mapDocument() const { return mRoiDocument; }

    RoiObject *roiObject() const { return mObject; }
    void syncWithRoiObject(RoiRenderer *renderer);
    void updateColor();

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;
private:
     DocumentView *mRoiDocument;

private:
    QRectF mBoundingRect;
    RoiObject *mObject;
    QColor mColor;
};

void RoiObjectLabel::syncWithRoiObject(RoiRenderer *renderer)
{
    const bool nameVisible = mObject->isVisible() && !mObject->name().isEmpty();
    setVisible(nameVisible);

    if (!nameVisible)
        return;
    DocumentView *v = mapDocument();
    if (v->bMultiView)   // view가 두개 이상 display되어 있으면 ROI정보를 나타내지 않는다.
        return;
    if (!v->isVisible())
        return;

    const QFontMetricsF metrics(QGuiApplication::font());
    QRectF boundingRect = metrics.boundingRect(mObject->name());
    boundingRect.translate(-boundingRect.width() / 2, -labelDistance);
    boundingRect.adjust(-labelMargin*2, -labelMargin, labelMargin*2, labelMargin);

    QPointF pixelPos = renderer->pixelToScreenCoords(mObject->position());
    QRectF bounds = objectBounds(mObject, renderer);

    // Adjust the bounding box for object rotation
    QTransform transform;
    transform.translate(pixelPos.x(), pixelPos.y());
    transform.rotate(mObject->rotation());
    transform.translate(-pixelPos.x(), -pixelPos.y());
    bounds = transform.mapRect(bounds);

    // Center the object name on the object bounding box
    const qreal scale = mapDocument()->zoom();
    QPointF pos((bounds.left() + bounds.right()) / 2 * scale, bounds.top() * scale);

    setPos(pos + mObject->objectGroup()->totalOffset());

    if (mBoundingRect != boundingRect) {
        prepareGeometryChange();
        mBoundingRect = boundingRect;
    }

    updateColor();
}

void RoiObjectLabel::updateColor()
{
    QColor color = RoiObjectItem::objectColor(mObject);
    if (mColor != color) {
        mColor = color;
        update();
    }
}

QRectF RoiObjectLabel::boundingRect() const
{
    if (mObject->objectGroup() == nullptr)
        return QRectF();
    if (!mObject->objectGroup()->isVisible() || !mObject->isVisible())
        return QRectF();
    DocumentView *v = mapDocument();
    if (v->bMultiView)   // view가 두개 이상 display되어 있으면 ROI정보를 나타내지 않는다.
        return QRectF();
    if (!v->isVisible())
        return QRectF();
    return mBoundingRect.adjusted(0, 0, 1, 1);
}

void RoiObjectLabel::paint(QPainter *painter,
                           const QStyleOptionGraphicsItem *,
                           QWidget *)
{
    if (mObject->objectGroup() == nullptr)
        return;
    if (!mObject->objectGroup()->isVisible() || !mObject->isVisible())
        return;
    if (mapDocument()->bMultiView)   // view가 두개 이상 display되어 있으면 ROI정보를 나타내지 않는다.
        return;

    QRectF rect = mBoundingRect;

    RasterImageView *pView = mapDocument()->imageView();
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();

    painter->translate(offset - scroll);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::black);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(rect.translated(1, 1), 4, 4);
    painter->setBrush(mColor);
    painter->drawRoundedRect(rect, 4, 4);

    QPointF textPos(-(rect.width() - labelMargin*4) / 2, -labelDistance);

    painter->drawRoundedRect(rect, 4, 4);
    painter->setPen(Qt::black);
    painter->drawText(textPos + QPointF(1,1), mObject->name());
    painter->setPen(Qt::white);
    painter->drawText(textPos, mObject->name());

    painter->translate(-(offset - scroll));
}

ObjectSelectionItem::ObjectSelectionItem(DocumentView *mapDocument)
    : mRoiDocument(mapDocument)
{
    setFlag(QGraphicsItem::ItemHasNoContents);

    connect(mapDocument, &DocumentView::selectedObjectsChanged,
            this, &ObjectSelectionItem::selectedObjectsChanged);

    connect(mapDocument, &DocumentView::mapChanged,
            this, &ObjectSelectionItem::mapChanged);

    connect(mapDocument, &DocumentView::layerAdded,
            this, &ObjectSelectionItem::layerAdded);

    connect(mapDocument, &DocumentView::objectsChanged,
            this, &ObjectSelectionItem::syncOverlayItems);

    connect(mapDocument, &DocumentView::objectsAdded,
            this, &ObjectSelectionItem::objectsAdded);

    connect(mapDocument, &DocumentView::objectsRemoved,
            this, &ObjectSelectionItem::objectsRemoved);

//    Preferences *prefs = Preferences::instance();

//    connect(prefs, &Preferences::objectLabelVisibilityChanged,
//            this, &ObjectSelectionItem::objectLabelVisibilityChanged);

//    connect(prefs, &Preferences::objectTypesChanged,
//            this, &ObjectSelectionItem::updateObjectLabelColors);

    //if (objectLabelVisibility() == Preferences::AllObjectLabels)
        addRemoveObjectLabels();
}

void ObjectSelectionItem::selectedObjectsChanged()
{
    addRemoveObjectLabels();
    addRemoveObjectOutlines();
}

void ObjectSelectionItem::mapChanged()
{
    syncOverlayItems(mRoiDocument->selectedObjects());
}

void ObjectSelectionItem::layerAdded(Layer *layer)
{
    ObjectGroup *objectGroup = layer->asObjectGroup();
    if (!objectGroup)
        return;

    // The layer may already have objects, for example when the addition is the
    // undo of a removal.
//    if (objectLabelVisibility() == Preferences::AllObjectLabels) {
        RoiRenderer *renderer = mRoiDocument->renderer();

        for (RoiObject *object : *objectGroup) {
            Q_ASSERT(!mObjectLabels.contains(object));

            RoiObjectLabel *labelItem = new RoiObjectLabel(object, this);
            labelItem->setMapDocument(mRoiDocument);
            labelItem->syncWithRoiObject(renderer);
            mObjectLabels.insert(object, labelItem);
        }
//    }
}

void ObjectSelectionItem::syncOverlayItems(const QList<RoiObject*> &objects)
{
    RoiRenderer *renderer = mRoiDocument->renderer();

    for (RoiObject *object : objects) {
        if (RoiObjectOutline *outlineItem = mObjectOutlines.value(object)) {
            outlineItem->setMapDocument(mRoiDocument);
            outlineItem->syncWithRoiObject(renderer);
        }
        if (RoiObjectLabel *labelItem = mObjectLabels.value(object))
            labelItem->syncWithRoiObject(renderer);
    }
}

void ObjectSelectionItem::updateObjectLabelColors()
{
    for (RoiObjectLabel *label : mObjectLabels)
        label->updateColor();
}

void ObjectSelectionItem::objectsAdded(const QList<RoiObject *> &objects)
{
    //if (objectLabelVisibility() == Preferences::AllObjectLabels)
    {
        RoiRenderer *renderer = mRoiDocument->renderer();

        for (RoiObject *object : objects) {
            Q_ASSERT(!mObjectLabels.contains(object));

            RoiObjectLabel *labelItem = new RoiObjectLabel(object, this);
            labelItem->setMapDocument(mRoiDocument);
            labelItem->syncWithRoiObject(renderer);
            mObjectLabels.insert(object, labelItem);
        }
    }
}

void ObjectSelectionItem::objectsRemoved(const QList<RoiObject *> &objects)
{
    //if (objectLabelVisibility() == Preferences::AllObjectLabels)
        for (RoiObject *object : objects)
            delete mObjectLabels.take(object);
}

void ObjectSelectionItem::objectLabelVisibilityChanged()
{
    addRemoveObjectLabels();
}

void ObjectSelectionItem::addRemoveObjectLabels()
{
    QHash<RoiObject*, RoiObjectLabel*> labelItems;
    RoiRenderer *renderer = mRoiDocument->renderer();

    auto ensureLabel = [this,&labelItems,renderer] (RoiObject *object) {
        if (labelItems.contains(object))
            return;

        RoiObjectLabel *labelItem = mObjectLabels.take(object);
        if (!labelItem) {
            labelItem = new RoiObjectLabel(object, this);
            labelItem->setMapDocument(mRoiDocument);
            labelItem->syncWithRoiObject(renderer);
        }

        labelItems.insert(object, labelItem);
    };

//    switch (objectLabelVisibility()) {
//    case Preferences::AllObjectLabels:

//    case Preferences::SelectedObjectLabels:
        for (RoiObject *object : mRoiDocument->selectedObjects())
            ensureLabel(object);

//    case Preferences::NoObjectLabels:
//        break;
//    }

    qDeleteAll(mObjectLabels); // delete remaining items
    mObjectLabels.swap(labelItems);
}

void ObjectSelectionItem::addObjectOutline(RoiObject *roiObject)
{
    RasterImageView *pView = mRoiDocument->imageView();
    if (pView == nullptr)
        return;
    //QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = mRoiDocument->zoom();
    RoiRenderer *renderer = mRoiDocument->renderer();
    RoiObjectOutline *outlineItem = mObjectOutlines.take(roiObject);
    if (!outlineItem) {
        outlineItem = new RoiObjectOutline(roiObject, this);
        outlineItem->setMapDocument(mRoiDocument);
        outlineItem->syncWithRoiObject(renderer);
    }
    //outlineItem->setOffset(offset-scroll);
    //outlineItem->setScale(scale);
    mObjectOutlines.insert(roiObject, outlineItem);
}

void ObjectSelectionItem::addRemoveObjectOutlines()
{
    RasterImageView *pView = mRoiDocument->imageView();
    if (pView == nullptr)
        return;
    //QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = mRoiDocument->zoom();

    QHash<RoiObject*, RoiObjectOutline*> outlineItems;
    RoiRenderer *renderer = mRoiDocument->renderer();

    for (RoiObject *roiObject : mRoiDocument->selectedObjects()) {
        RoiObjectOutline *outlineItem = mObjectOutlines.take(roiObject);
        if (!outlineItem) {
            outlineItem = new RoiObjectOutline(roiObject, this);
            outlineItem->syncWithRoiObject(renderer);
        }
        outlineItem->setMapDocument(mRoiDocument);
        //outlineItem->setOffset(offset-scroll);
        //outlineItem->setScale(scale);
        outlineItems.insert(roiObject, outlineItem);
    }

    qDeleteAll(mObjectOutlines); // delete remaining items
    mObjectOutlines.swap(outlineItems);
}

} // namespace Qroilib
