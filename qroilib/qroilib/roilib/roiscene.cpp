/*
 * roiscene.cpp
 * Copyright 2008-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "roiscene.h"

#include "abstracttool.h"
#include "grouplayer.h"
#include "roimap.h"
#include "roiobject.h"
#include "roiobjectitem.h"
#include "roirenderer.h"
#include "objectgroup.h"
#include "objectgroupitem.h"
#include "objectselectionitem.h"
#include "toolmanager.h"
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>
#include <qroilib/documentview/birdeyeview.h>

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QPalette>
#include <QDebug>

#include <cmath>

using namespace Qroilib;

static const qreal darkeningFactor = 0.6;
static const qreal opacityFactor = 0.4;

RoiScene::RoiScene(QObject *parent):
    QGraphicsScene(parent),
    mRoiDocument(nullptr),
    mSelectedTool(nullptr),
    mActiveTool(nullptr),
    mUnderMouse(false),
    mCurrentModifiers(Qt::NoModifier),
    mDarkRectangle(new QGraphicsRectItem),
    mObjectSelectionItem(nullptr)
{
    updateDefaultBackgroundColor();

    mDarkRectangle->setPen(Qt::NoPen);
    mDarkRectangle->setBrush(Qt::black);
    mDarkRectangle->setOpacity(darkeningFactor);
    addItem(mDarkRectangle);

    mGridVisible = true;//prefs->showGrid();
    mObjectLineWidth = 2;//prefs->objectLineWidth();
    mShowTileObjectOutlines = false;//prefs->showTileObjectOutlines();
    mHighlightCurrentLayer = false;//prefs->highlightCurrentLayer();

    // Install an event filter so that we can get key events on behalf of the
    // active tool without having to have the current focus.
    qApp->installEventFilter(this);
}

RoiScene::~RoiScene()
{
    qApp->removeEventFilter(this);
}

void RoiScene::setMapDocument(DocumentView *mapDocument)
{
    if (mRoiDocument) {
        mRoiDocument->disconnect(this);

        if (!mSelectedObjectItems.isEmpty()) {
            mSelectedObjectItems.clear();
            emit selectedObjectItemsChanged();
        }
    }

    mRoiDocument = mapDocument;

    if (mRoiDocument) {
        RoiRenderer *renderer = mRoiDocument->renderer();
        renderer->setObjectLineWidth(mObjectLineWidth);
        renderer->setFlag(ShowTileObjectOutlines, mShowTileObjectOutlines);

        connect(mRoiDocument, &DocumentView::mapChanged,
                this, &RoiScene::mapChanged);
        //connect(mRoiDocument, &DocumentView::regionChanged,
        //        this, &RoiScene::repaintRegion);
        //connect(mRoiDocument, &DocumentView::tileLayerDrawMarginsChanged,
        //        this, &RoiScene::tileLayerDrawMarginsChanged);
        connect(mRoiDocument, &DocumentView::layerAdded,
                this, &RoiScene::layerAdded);
        connect(mRoiDocument, &DocumentView::layerRemoved,
                this, &RoiScene::layerRemoved);
        connect(mRoiDocument, &DocumentView::layerChanged,
                this, &RoiScene::layerChanged);
        connect(mRoiDocument, &DocumentView::objectGroupChanged,
                this, &RoiScene::objectGroupChanged);
        connect(mRoiDocument, &DocumentView::objectsInserted,
                this, &RoiScene::objectsInserted);
        connect(mRoiDocument, &DocumentView::objectsRemoved,
                this, &RoiScene::objectsRemoved);
        connect(mRoiDocument, &DocumentView::objectsChanged,
                this, &RoiScene::objectsChanged);
        connect(mRoiDocument, &DocumentView::objectsIndexChanged,
                this, &RoiScene::objectsIndexChanged);
        connect(mRoiDocument, &DocumentView::selectedObjectsChanged,
                this, &RoiScene::updateSelectedObjectItems);
    }

    refreshScene();
}

void RoiScene::setSelectedObjectItems(const QSet<RoiObjectItem *> &items)
{
    if (mRoiDocument == nullptr)
        return;
    // Inform the roimap document about the newly selected objects
    QList<RoiObject*> selectedObjects;
    selectedObjects.reserve(items.size());

    for (const RoiObjectItem *item : items) {
        RoiObject *mapObj = item->roiObject();
        selectedObjects.append(mapObj);
    }

    mRoiDocument->setSelectedObjects(selectedObjects);

    if (selectedObjects.size() == 1)
    {
        RoiObject *object = selectedObjects[0];
        if (object->mParent != nullptr)
            mObjectSelectionItem->addObjectOutline(object->mParent);
        mObjectSelectionItem->addObjectOutline(object);
		if (object->shape() == RoiObject::Pattern)
            mObjectSelectionItem->addObjectOutline(object->mPattern);
    }
}

void RoiScene::setSelectedTool(AbstractTool *tool)
{
    if (tool == nullptr)
        return;
    //DocumentView *v = tool->mapDocument() ;
    mSelectedTool = tool;
}

void RoiScene::refreshScene()
{
    mLayerItems.clear();
    mObjectItems.clear();

    removeItem(mDarkRectangle);
//    clear();
    addItem(mDarkRectangle);

    if (!mRoiDocument) {
        setSceneRect(QRectF());
        return;
    }

    updateSceneRect();

    const RoiMap *roimap = mRoiDocument->roimap();

    if (roimap->backgroundColor().isValid())
        setBackgroundBrush(roimap->backgroundColor());
    else
        setBackgroundBrush(mDefaultBackgroundColor);

    createLayerItems(roimap->layers());

    mObjectSelectionItem = new ObjectSelectionItem(mRoiDocument);
    mObjectSelectionItem->setZValue(10000 - 1);
    addItem(mObjectSelectionItem);

    updateCurrentLayerHighlight();
}

void RoiScene::createLayerItems(const QList<Layer *> &layers)
{
    int layerIndex = 0;

    for (Layer *layer : layers) {
        LayerItem *layerItem = createLayerItem(layer);
        layerItem->setZValue(layerIndex);
        ++layerIndex;
    }
}

LayerItem *RoiScene::createLayerItem(Layer *layer)
{
    LayerItem *layerItem = nullptr;

    switch (layer->layerType()) {
    case Layer::ObjectGroupType: {
        auto og = static_cast<ObjectGroup*>(layer);
        const ObjectGroup::DrawOrder drawOrder = og->drawOrder();
        ObjectGroupItem *ogItem = new ObjectGroupItem(og);
        int objectIndex = 0;
        for (RoiObject *object : og->objects()) {
            RoiObjectItem *item = new RoiObjectItem(object, mRoiDocument,
                                                    ogItem);
            if (drawOrder == ObjectGroup::TopDownOrder)
                item->setZValue(item->y());
            else
                item->setZValue(objectIndex);

            mObjectItems.insert(object, item);
            ++objectIndex;
        }
        ogItem->setMapDocument(mapDocument());
        layerItem = ogItem;
        break;
    }

    }

    Q_ASSERT(layerItem);

    layerItem->setMapDocument(mapDocument());
    layerItem->setVisible(layer->isVisible());

    if (layer->parentLayer())
        layerItem->setParentItem(mLayerItems.value(layer->parentLayer()));
    else
        addItem(layerItem);

    mLayerItems.insert(layer, layerItem);

    if (GroupLayer *groupLayer = layer->asGroupLayer())
        createLayerItems(groupLayer->layers());

    return layerItem;
}

void RoiScene::updateDefaultBackgroundColor()
{
    mDefaultBackgroundColor = QGuiApplication::palette().dark().color();

    if (!mRoiDocument || !mRoiDocument->roimap()->backgroundColor().isValid())
        setBackgroundBrush(mDefaultBackgroundColor);
}

void RoiScene::updateSceneRect()
{
    if (mRoiDocument == nullptr)
        return;
    const QSize mapSize = mRoiDocument->renderer()->mapSize();
    QRectF sceneRect(0, 0, mapSize.width(), mapSize.height());

    QMargins margins = mRoiDocument->roimap()->computeLayerOffsetMargins();
    sceneRect.adjust(-margins.left(),
                     -margins.top(),
                     margins.right(),
                     margins.bottom());

    //sceneRect.adjust(-600,-600,600,600);

    setSceneRect(sceneRect);
    mDarkRectangle->setRect(sceneRect);
}

void RoiScene::updateCurrentLayerHighlight()
{
    if (!mRoiDocument)
        return;

    const auto currentLayer = mRoiDocument->currentLayer();

    if (!mHighlightCurrentLayer || !currentLayer) {
        if (mDarkRectangle->isVisible()) {
            mDarkRectangle->setVisible(false);

            // Restore opacity for all layers
            const auto layerItems = mLayerItems;
            for (auto layerItem : layerItems)
                layerItem->setOpacity(layerItem->layer()->opacity());
        }

        return;
    }

    // Darken layers below the current layer
    const int siblingIndex = currentLayer->siblingIndex();
    const auto parentLayer = currentLayer->parentLayer();
    const auto parentItem = mLayerItems.value(parentLayer);

    mDarkRectangle->setParentItem(parentItem);
    mDarkRectangle->setZValue(siblingIndex - 0.5);
    mDarkRectangle->setVisible(true);

    // Set layers above the current layer to reduced opacity
    LayerIterator iterator(mRoiDocument->roimap());
    qreal multiplier = 1;

    while (Layer *layer = iterator.next()) {
        GroupLayer *groupLayer = layer->asGroupLayer();
        if (!groupLayer)
            mLayerItems.value(layer)->setOpacity(layer->opacity() * multiplier);

        if (layer == currentLayer)
            multiplier = opacityFactor;
    }
}

//void RoiScene::repaintRegion(const QRegion &region, Layer *layer)
//{
//    const RoiRenderer *renderer = mRoiDocument->renderer();
//    const QMargins margins = mRoiDocument->roimap()->drawMargins();

//    for (const QRect &r : region.rects()) {
//        QRectF boundingRect = renderer->boundingRect(r);

//        boundingRect.adjust(-margins.left(),
//                            -margins.top(),
//                            margins.right(),
//                            margins.bottom());

//        //boundingRect.translate(layer->totalOffset());

//        update(boundingRect);
//    }
//}

void RoiScene::enableSelectedTool()
{
    if (!mSelectedTool || !mRoiDocument)
        return;

    mActiveTool = mSelectedTool;
    DocumentView *v = mActiveTool->mapDocument() ;
    mActiveTool->activate(this);

    mCurrentModifiers = QApplication::keyboardModifiers();
    if (mCurrentModifiers != Qt::NoModifier)
        mActiveTool->modifiersChanged(mCurrentModifiers);

    if (mUnderMouse) {
        mActiveTool->mouseEntered();
        mActiveTool->mouseMoved(mLastMousePos, Qt::KeyboardModifiers());
    }
}

void RoiScene::disableSelectedTool()
{
    if (!mActiveTool)
        return;

    if (mUnderMouse)
        mActiveTool->mouseLeft();
    mActiveTool->deactivate(this);
    mActiveTool = nullptr;
}

void RoiScene::currentLayerChanged()
{
    updateCurrentLayerHighlight();

    // New layer may have a different offset, affecting the grid
    if (mGridVisible)
        update();
}

/**
 * Adapts the scene, layers and objects to new roimap size, orientation or
 * background color.
 */
void RoiScene::mapChanged()
{
    //updateSceneRect();

    for (RoiObjectItem *item : mObjectItems)
        item->syncWithRoiObject();

    const RoiMap *roimap = mRoiDocument->roimap();
    if (roimap->backgroundColor().isValid())
        setBackgroundBrush(roimap->backgroundColor());
    else
        setBackgroundBrush(mDefaultBackgroundColor);
}

void RoiScene::layerAdded(Layer *layer)
{
    createLayerItem(layer);

    int z = 0;
    for (auto sibling : layer->siblings())
        mLayerItems.value(sibling)->setZValue(z++);
}

void RoiScene::layerRemoved(Layer *layer)
{
    delete mLayerItems.take(layer);
}

// Returns whether layerB is drawn above layerA
static bool isAbove(Layer *layerA, Layer *layerB)
{
    int depthA = layerA->depth();
    int depthB = layerB->depth();

    // Make sure to start comparing at a common depth
    while (depthA > 0 && depthA > depthB) {
        layerA = layerA->parentLayer();
        --depthA;
    }
    while (depthB > 0 && depthB > depthA) {
        layerB = layerB->parentLayer();
        --depthB;
    }

    // One of the layers is a child of the other
    if (layerA == layerB)
        return false;

    // Move upwards until the layers have the same parent
    while (true) {
        GroupLayer *parentA = layerA->parentLayer();
        GroupLayer *parentB = layerB->parentLayer();

        if (parentA == parentB) {
            const auto &layers = layerA->siblings();
            const int indexA = layers.indexOf(layerA);
            const int indexB = layers.indexOf(layerB);
            return indexB > indexA;
        }

        layerA = parentA;
        layerB = parentB;
    }
}

/**
 * A layer has changed. This can mean that the layer visibility, opacity or
 * offset changed.
 */
void RoiScene::layerChanged(Layer *layer)
{
    QGraphicsItem *layerItem = mLayerItems.value(layer);
    Q_ASSERT(layerItem);

    layerItem->setVisible(layer->isVisible());

    qreal multiplier = 1;
    if (mHighlightCurrentLayer && isAbove(mRoiDocument->currentLayer(), layer))
        multiplier = opacityFactor;

    layerItem->setOpacity(layer->opacity() * multiplier);
    layerItem->setPos(layer->offset());

    // Layer offset may have changed, affecting the scene rect and grid
    updateSceneRect();
    if (mGridVisible)
        update();
}

/**
 * When an object group has changed it may mean its color or drawing order
 * changed, which affects all its objects.
 */
void RoiScene::objectGroupChanged(ObjectGroup *objectGroup)
{
    objectsChanged(objectGroup->objects());
    objectsIndexChanged(objectGroup, 0, objectGroup->objectCount() - 1);
}

/**
 * Inserts roimap object items for the given objects.
 */
void RoiScene::objectsInserted(ObjectGroup *objectGroup, int first, int last)
{
    ObjectGroupItem *ogItem = nullptr;

    // Find the object group item for the object group
    for (QGraphicsItem *item : mLayerItems) {
        if (ObjectGroupItem *ogi = dynamic_cast<ObjectGroupItem*>(item)) {
            if (ogi->objectGroup() == objectGroup) {
                ogItem = ogi;
                break;
            }
        }
    }

    Q_ASSERT(ogItem);

    const ObjectGroup::DrawOrder drawOrder = objectGroup->drawOrder();

    for (int i = first; i <= last; ++i) {
        RoiObject *object = objectGroup->objectAt(i);

        RoiObjectItem *item = new RoiObjectItem(object, mRoiDocument, ogItem);
        if (drawOrder == ObjectGroup::TopDownOrder)
            item->setZValue(item->y());
        else
            item->setZValue(i);

        mObjectItems.insert(object, item);
    }
}

/**
 * Removes the roimap object items related to the given objects.
 */
void RoiScene::objectsRemoved(const QList<RoiObject*> &objects)
{
    for (RoiObject *o : objects) {
        auto i = mObjectItems.find(o);
        Q_ASSERT(i != mObjectItems.end());

        mSelectedObjectItems.remove(i.value());
        delete i.value();
        mObjectItems.erase(i);
    }
}

/**
 * Updates the roimap object items related to the given objects.
 */
void RoiScene::objectsChanged(const QList<RoiObject*> &objects)
{
    for (RoiObject *object : objects) {
        RoiObjectItem *item = itemForObject(object);
        Q_ASSERT(item);

        item->syncWithRoiObject();
    }
}

/**
 * Updates the Z value of the objects when appropriate.
 */
void RoiScene::objectsIndexChanged(ObjectGroup *objectGroup,
                                   int first, int last)
{
    if (objectGroup->drawOrder() != ObjectGroup::IndexOrder)
        return;

    for (int i = first; i <= last; ++i) {
        RoiObjectItem *item = itemForObject(objectGroup->objectAt(i));
        Q_ASSERT(item);

        item->setZValue(i);
    }
}

void RoiScene::updateSelectedObjectItems()
{
    const QList<RoiObject *> &objects = mRoiDocument->selectedObjects();

    QSet<RoiObjectItem*> items;
    for (RoiObject *object : objects) {
        RoiObjectItem *item = itemForObject(object);
        Q_ASSERT(item);

        items.insert(item);
    }

    mSelectedObjectItems = items;
    emit selectedObjectItemsChanged();
}

void RoiScene::syncAllObjectItems()
{
    for (RoiObjectItem *item : mObjectItems)
        item->syncWithRoiObject();
}


void RoiScene::resizeAllObjectItems()
{
    updateSceneRect();

    for (RoiObjectItem *item : mObjectItems)
        item->syncWithRoiObject();
}

/**
 * Sets whether the tile grid is visible.
 */
void RoiScene::setGridVisible(bool visible)
{
    if (mGridVisible == visible)
        return;

    mGridVisible = visible;
    update();
}

void RoiScene::setObjectLineWidth(qreal lineWidth)
{
    if (mObjectLineWidth == lineWidth)
        return;

    mObjectLineWidth = lineWidth;

    if (mRoiDocument) {
        mRoiDocument->renderer()->setObjectLineWidth(lineWidth);

        // Changing the line width can change the size of the object items
        if (!mObjectItems.isEmpty()) {
            for (RoiObjectItem *item : mObjectItems)
                item->syncWithRoiObject();

            update();
        }
    }
}

void RoiScene::setShowTileObjectOutlines(bool enabled)
{
    if (mShowTileObjectOutlines == enabled)
        return;

    mShowTileObjectOutlines = enabled;

    if (mRoiDocument) {
        mRoiDocument->renderer()->setFlag(ShowTileObjectOutlines, enabled);
        if (!mObjectItems.isEmpty())
            update();
    }
}

void RoiScene::setHighlightCurrentLayer(bool highlightCurrentLayer)
{
    if (mHighlightCurrentLayer == highlightCurrentLayer)
        return;

    mHighlightCurrentLayer = highlightCurrentLayer;
    updateCurrentLayerHighlight();
}

void RoiScene::drawForeground(QPainter *painter, const QRectF &rectIn)
{
    if (!mRoiDocument || !mGridVisible)
        return;

    QPointF offset;
    QRectF rect = rectIn;

    // Take into account the offset of the current layer
    //if (Layer *layer = mRoiDocument->currentLayer()) {
        //offset = layer->totalOffset();
        //offset = mRoiDocument->imageView()->imageOffset();

        //painter->translate(offset);
    //}

    //Preferences *prefs = Preferences::instance();
//    mRoiDocument->renderer()->drawGrid(painter,
//                                       rect.translated(-offset),
//                                       Qt::black);

}

bool RoiScene::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Enter:
        mUnderMouse = true;
        if (mActiveTool)
            mActiveTool->mouseEntered();
        break;
    case QEvent::Leave:
        mUnderMouse = false;
        if (mActiveTool)
            mActiveTool->mouseLeft();
        break;
    default:
        break;
    }

    return QGraphicsScene::event(event);
}

void RoiScene::keyPressEvent(QKeyEvent *event)
{
    if (mActiveTool)
        mActiveTool->keyPressed(event);

    if (!(mActiveTool && event->isAccepted()))
        QGraphicsScene::keyPressEvent(event);

     Qt::Key key = static_cast<Qt::Key>(event->key());
     if (key == Qt::Key_Delete) {
        mRoiDocument->delete_();
     }

}

void RoiScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    //qDebug() << "RoiScene::mouseMoveEvent";

    mLastMousePos = mouseEvent->scenePos();

    if (!mRoiDocument)
        return;

    QGraphicsScene::mouseMoveEvent(mouseEvent);
    //if (mouseEvent->isAccepted())
    //    return;
    if (mActiveTool) {
        const QPointF pos = mouseEvent->scenePos();
        mActiveTool->mouseMoved(pos, mouseEvent->modifiers());
        mouseEvent->accept();
    }

}

void RoiScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (!mRoiDocument)
        return;
    QGraphicsScene::mouseDoubleClickEvent(mouseEvent);

    BirdEyeView* mBirdEyeView = mRoiDocument->mBirdEyeView;
    if (!mBirdEyeView->mStartDragMousePos.isNull()) {
        return;
    }

    if (mActiveTool) {
        mouseEvent->accept();

        QList<RoiObject*> p = mRoiDocument->selectedObjects();
        RoiObject *roiObject = nullptr;
        if (!p.empty() && p.size() == 1) {
            roiObject = p.first();
            //RoiObject *roiObject = mRoiDocument->selectedObjects().first();
            mRoiDocument->setCurrentObject(roiObject);
            emit mRoiDocument->editCurrentObject();
        }

    }
}

void RoiScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mousePressEvent(mouseEvent);
    //if (mouseEvent->isAccepted())
    //    return;

    if (!mRoiDocument)
        return;

    BirdEyeView* mBirdEyeView = mRoiDocument->mBirdEyeView;
    if (!mBirdEyeView->mStartDragMousePos.isNull()) {
        return;
    }

    if (mActiveTool) {

        DocumentView *v = mActiveTool->mapDocument() ;

        mouseEvent->accept();
        mActiveTool->mousePressed(mouseEvent);
    }

    mLastDragPos = mouseEvent->pos();

}

void RoiScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (!mRoiDocument)
        return;

    QGraphicsScene::mouseReleaseEvent(mouseEvent);
    //if (mouseEvent->isAccepted())
    //    return;

    if (mActiveTool) {

        //DocumentView *v = mActiveTool->mapDocument() ;

        mouseEvent->accept();
        mActiveTool->mouseReleased(mouseEvent);
    }

    //mLastDragPos = mouseEvent->pos();
}

/**
 * Override to ignore drag enter events.
 */
void RoiScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->ignore();
}

bool RoiScene::eventFilter(QObject *, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            Qt::KeyboardModifiers newModifiers = keyEvent->modifiers();

            if (mActiveTool && newModifiers != mCurrentModifiers) {
                mActiveTool->modifiersChanged(newModifiers);
                mCurrentModifiers = newModifiers;
            }
        }
        break;
    default:
        break;
    }

    return false;
}
