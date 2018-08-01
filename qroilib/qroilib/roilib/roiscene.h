/*
 * roiscene.h
 * Copyright 2008-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#pragma once

#include <roilib_export.h>
#include <QColor>
#include <QGraphicsScene>
#include <QMap>
#include <QSet>

namespace Qroilib {

class Layer;
class RoiObject;
class ObjectGroup;

class AbstractTool;
class LayerItem;
class DocumentView;
class RoiObjectItem;
class RoiScene;
class ObjectGroupItem;
class ObjectSelectionItem;

/**
 * A graphics scene that represents the contents of a roimap.
 */
class ROIDSHARED_EXPORT RoiScene : public QGraphicsScene
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    RoiScene(QObject *parent);

    /**
     * Destructor.
     */
    ~RoiScene();

    /**
     * Returns the roimap document this scene is displaying.
     */
    DocumentView *mapDocument() const { return mRoiDocument; }

    /**
     * Sets the roimap this scene displays.
     */
    void setMapDocument(DocumentView *roimap);

    /**
     * Returns whether the tile grid is visible.
     */
    bool isGridVisible() const { return mGridVisible; }

    /**
     * Returns the set of selected roimap object items.
     */
    const QSet<RoiObjectItem*> &selectedObjectItems() const
    { return mSelectedObjectItems; }

    /**
     * Sets the set of selected roimap object items. This translates to a call to
     * MapDocument::setSelectedObjects.
     */
    void setSelectedObjectItems(const QSet<RoiObjectItem*> &items);

    /**
     * Returns the RoiObjectItem associated with the given \a roiObject.
     */
    RoiObjectItem *itemForObject(RoiObject *object) const
    { return mObjectItems.value(object); }

    /**
     * Enables the selected tool at this roimap scene.
     * Therefore it tells that tool, that this is the active roimap scene.
     */
    void enableSelectedTool();
    void disableSelectedTool();

    /**
     * Sets the currently selected tool.
     */
    void setSelectedTool(AbstractTool *tool);


    QPointF getLastMousePos() const
    {
        return mLastDragPos;
    }

    void resizeAllObjectItems();

signals:
    void selectedObjectItemsChanged();

protected:
    /**
     * QGraphicsScene::drawForeground override that draws the tile grid.
     */
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    /**
     * Override for handling enter and leave events.
     */
    bool event(QEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;

private slots:
    void setGridVisible(bool visible);
    void setObjectLineWidth(qreal lineWidth);
    void setShowTileObjectOutlines(bool enabled);

    /**
     * Sets whether the current layer should be highlighted.
     */
    void setHighlightCurrentLayer(bool highlightCurrentLayer);

    /**
     * Refreshes the roimap scene.
     */
    void refreshScene();

    void currentLayerChanged();

    void mapChanged();

    void layerAdded(Layer *layer);
    void layerRemoved(Layer *layer);
    void layerChanged(Layer *layer);

    void objectGroupChanged(ObjectGroup *objectGroup);

    void objectsInserted(ObjectGroup *objectGroup, int first, int last);
    void objectsRemoved(const QList<RoiObject*> &objects);
    void objectsChanged(const QList<RoiObject*> &objects);
    void objectsIndexChanged(ObjectGroup *objectGroup, int first, int last);

    void updateSelectedObjectItems();
    void syncAllObjectItems();

public:
    void updateSceneRect();

private:
    void createLayerItems(const QList<Layer *> &layers);
    LayerItem *createLayerItem(Layer *layer);

    void updateDefaultBackgroundColor();
    void updateCurrentLayerHighlight();

    bool eventFilter(QObject *object, QEvent *event) override;

    DocumentView *mRoiDocument;
    AbstractTool *mSelectedTool;
    AbstractTool *mActiveTool;
    bool mGridVisible;
    qreal mObjectLineWidth;
    bool mShowTileObjectOutlines;
    bool mHighlightCurrentLayer;
    bool mUnderMouse;
    Qt::KeyboardModifiers mCurrentModifiers;
    QPointF mLastMousePos;
    QMap<Layer*, LayerItem*> mLayerItems;
    QGraphicsRectItem *mDarkRectangle;
    QColor mDefaultBackgroundColor;
    ObjectSelectionItem *mObjectSelectionItem;

    QMap<RoiObject*, RoiObjectItem*> mObjectItems;
    QSet<RoiObjectItem*> mSelectedObjectItems;

    QPointF mLastDragPos;

};

} // namespace Qroilib
