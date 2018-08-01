/*
 * mapobjectmodel.h
 * Copyright 2012, Tim Baker <treectrl@hotmail.com>
 * Copyright 2012-2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "roiobject.h"

#include <QAbstractItemModel>
#include <QIcon>

namespace Qroilib {

class GroupLayer;
class Layer;
class RoiObject;
class RoiMap;
class ObjectGroup;

class DocumentView;

/**
 * Provides a tree view on the objects present on a roimap. Also has member
 * functions to modify objects that emit the appropriate signals to allow
 * the UI to update.
 */
class RoiObjectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum UserRoles {
        OpacityRole = Qt::UserRole
    };

    enum Column {
        Name,
        Type,
        Id,
        Position,
        ColumnCount
    };

    RoiObjectModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QModelIndex index(Layer *layer) const;
    QModelIndex index(RoiObject *roiObject, int column = 0) const;

    Layer *toLayer(const QModelIndex &index) const;
    RoiObject *toRoiObject(const QModelIndex &index) const;
    ObjectGroup *toObjectGroup(const QModelIndex &index) const;
    GroupLayer *toGroupLayer(const QModelIndex &index) const;
    ObjectGroup *toObjectGroupContext(const QModelIndex &index) const;

    void setMapDocument(DocumentView *mapDocument);
    DocumentView *mapDocument() const { return mRoiDocument; }

    void insertObject(ObjectGroup *og, int index, RoiObject *o);
    int removeObject(ObjectGroup *og, RoiObject *o);
    void moveObjects(ObjectGroup *og, int from, int to, int count);

    void setObjectPolygon(RoiObject *o, const QPolygonF &polygon);
    void setObjectPosition(RoiObject *o, const QPointF &pos);
    void setObjectSize(RoiObject *o, const QSizeF &size);
    void setObjectRotation(RoiObject *o, qreal rotation);

    void setObjectProperty(RoiObject *o, RoiObject::Property property, const QVariant &value);
    void emitObjectsChanged(const QList<RoiObject *> &objects, const QList<Column> &columns = QList<Column>());
    void emitObjectsChanged(const QList<RoiObject*> &objects, Column column);

signals:
    void objectsAdded(const QList<RoiObject *> &objects);
    void objectsChanged(const QList<RoiObject *> &objects);
    void objectsTypeChanged(const QList<RoiObject *> &objects);
    void objectsRemoved(const QList<RoiObject *> &objects);

private slots:
    void layerAdded(Layer *layer);
    void layerChanged(Layer *layer);
    void layerAboutToBeRemoved(GroupLayer *groupLayer, int index);

private:
    DocumentView *mRoiDocument;
    RoiMap *mRoi;

    // cache
    mutable QMap<GroupLayer*, QList<Layer*>> mFilteredLayers;
    QList<Layer *> &filteredChildLayers(GroupLayer *parentLayer) const;

    QIcon mObjectGroupIcon;
};

} // namespace Qroilib
