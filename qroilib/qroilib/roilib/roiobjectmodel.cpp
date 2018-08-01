/*
 * mapobjectmodel.cpp
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

#include "roiobjectmodel.h"


#include "grouplayer.h"

#include "roimap.h"
#include "document.h"
#include "objectgroup.h"
#include <qroilib/documentview/documentview.h>

#include <QApplication>
#include <QPalette>
#include <QStyle>

namespace Qroilib {

RoiObjectModel::RoiObjectModel(QObject *parent):
    QAbstractItemModel(parent),
    mRoiDocument(nullptr),
    mRoi(nullptr),
    mObjectGroupIcon(QLatin1String(":/images/16x16/layer-object.png"))
{
    mObjectGroupIcon.addFile(QLatin1String(":images/32x32/layer-object.png"));
}

QModelIndex RoiObjectModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if (ObjectGroup *objectGroup = toObjectGroup(parent)) {
        if (row < objectGroup->objectCount())
            return createIndex(row, column, objectGroup->objectAt(row));
        return QModelIndex();
    }

    GroupLayer *parentLayer = toGroupLayer(parent); // may be nullptr
    const QList<Layer *> &layers = filteredChildLayers(parentLayer);

    if (row < layers.size())
        return createIndex(row, column, layers.at(row));

    return QModelIndex();
}

QModelIndex RoiObjectModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Object *object = static_cast<Object*>(index.internalPointer());
    switch (object->typeId()) {
    case Object::LayerType:
        if (Layer *layer = static_cast<Layer*>(object)->parentLayer())
            return this->index(layer);
        break;
    case Object::RoiObjectType:
        return this->index(static_cast<RoiObject*>(object)->objectGroup());
    default:
        break;
    }

    return QModelIndex();
}

int RoiObjectModel::rowCount(const QModelIndex &parent) const
{
    if (!mRoiDocument)
        return 0;
    if (!parent.isValid())
        return filteredChildLayers(nullptr).size();

    Object *object = static_cast<Object*>(parent.internalPointer());
    if (object->typeId() == Object::LayerType) {
        Layer *layer = static_cast<Layer*>(object);
        switch (layer->layerType()) {
        case Layer::GroupLayerType:
            return filteredChildLayers(static_cast<GroupLayer*>(layer)).size();
        case Layer::ObjectGroupType:
            return static_cast<ObjectGroup*>(layer)->objectCount();
        default:
            break;
        }
    }
    return 0;
}

int RoiObjectModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant RoiObjectModel::data(const QModelIndex &index, int role) const
{
    if (RoiObject *roiObject = toRoiObject(index)) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column()) {
            case Name:
                return roiObject->name();
            case Type:
                return roiObject->effectiveType();
            case Id:
                return roiObject->id();
            case Position:
                return QLatin1Char('(')
                        + QString::number(roiObject->x())
                        + QLatin1String(", ")
                        + QString::number(roiObject->y())
                        + QLatin1Char(')');
            }
            break;
        case Qt::ForegroundRole:
            if (index.column() == 1) {
                const QPalette palette = QApplication::palette();
                const auto typeColorGroup = roiObject->type().isEmpty() ? QPalette::Disabled
                                                                        : QPalette::Active;
                return palette.brush(typeColorGroup, QPalette::WindowText);
            }
            return QVariant();
        case Qt::CheckStateRole:
            if (index.column() > 0)
                return QVariant();
            return roiObject->isVisible() ? Qt::Checked : Qt::Unchecked;
        case OpacityRole:
            return qreal(1);
        default:
            return QVariant();
        }
    }
    if (Layer *layer = toLayer(index)) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return index.column() ? QVariant() : layer->name();
        case Qt::DecorationRole:
            if (index.column() == 0) {
                if (layer->isObjectGroup())
                    return mObjectGroupIcon;
                else
                    return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
            }
            return QVariant();
        case Qt::CheckStateRole:
            if (index.column() > 0)
                return QVariant();
            return layer->isVisible() ? Qt::Checked : Qt::Unchecked;
        case OpacityRole:
            return layer->opacity();
        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool RoiObjectModel::setData(const QModelIndex &index, const QVariant &value,
                             int role)
{
    if (RoiObject *roiObject = toRoiObject(index)) {
        switch (role) {
        }
        return false;
    }
    return false;
}

Qt::ItemFlags RoiObjectModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractItemModel::flags(index);
    if (index.column() == 0)
        rc |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
    else if (toRoiObject(index)) {
        if (index.column() == Type)// allow to edit only type column
            rc |= Qt::ItemIsEditable;
    }
    return rc;
}

QVariant RoiObjectModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case Name: return tr("Name");
        case Type: return tr("Type");
        case Id: return tr("ID");
        case Position: return tr("Position");
        }
    }
    return QVariant();
}

QModelIndex RoiObjectModel::index(Layer *layer) const
{
    Q_ASSERT(layer->isObjectGroup() || layer->isGroupLayer());
    const int row = 0;//filteredChildLayers(layer->parentLayer()).indexOf(layer); // test
    return createIndex(row, 0, layer);
}

QModelIndex RoiObjectModel::index(RoiObject *roiObject, int column) const
{
    const int row = roiObject->objectGroup()->objects().indexOf(roiObject);
    return createIndex(row, column, roiObject);
}

Layer *RoiObjectModel::toLayer(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    Object *object = static_cast<Object*>(index.internalPointer());
    if (object->typeId() == Object::LayerType)
        return static_cast<Layer*>(object);

    return nullptr;
}

RoiObject *RoiObjectModel::toRoiObject(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    Object *object = static_cast<Object*>(index.internalPointer());
    if (object->typeId() == Object::RoiObjectType)
        return static_cast<RoiObject*>(object);

    return nullptr;
}

ObjectGroup *RoiObjectModel::toObjectGroup(const QModelIndex &index) const
{
    if (Layer *layer = toLayer(index))
        return layer->asObjectGroup();
    return nullptr;
}

GroupLayer *RoiObjectModel::toGroupLayer(const QModelIndex &index) const
{
    if (Layer *layer = toLayer(index))
        return layer->asGroupLayer();
    return nullptr;
}

ObjectGroup *RoiObjectModel::toObjectGroupContext(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    Object *object = static_cast<Object*>(index.internalPointer());
    if (object->typeId() == Object::LayerType) {
        if (auto objectGroup = static_cast<Layer*>(object)->asObjectGroup())
            return objectGroup;
    } else if (object->typeId() == Object::RoiObjectType) {
        return static_cast<RoiObject*>(object)->objectGroup();
    }

    return nullptr;
}

void RoiObjectModel::setMapDocument(DocumentView *mapDocument)
{
    if (mRoiDocument == mapDocument)
        return;

    if (mRoiDocument)
        mRoiDocument->disconnect(this);

    beginResetModel();
    mRoiDocument = mapDocument;
    mRoi = nullptr;

    mFilteredLayers.clear();

    if (mRoiDocument) {
        mRoi = mRoiDocument->roimap();

        connect(mRoiDocument, &DocumentView::layerAdded,
                this, &RoiObjectModel::layerAdded);
        connect(mRoiDocument, &DocumentView::layerChanged,
                this, &RoiObjectModel::layerChanged);
        connect(mRoiDocument, &DocumentView::layerAboutToBeRemoved,
                this, &RoiObjectModel::layerAboutToBeRemoved);
    }

    endResetModel();
}

void RoiObjectModel::layerAdded(Layer *layer)
{
    if (layer->isObjectGroup() || layer->isGroupLayer()) {
        const auto &siblings = layer->siblings();

        Layer *prev = nullptr;
        for (int i = siblings.indexOf(layer) - 1; i >= 0; --i) {
            auto sibling = siblings.at(i);
            if (sibling->isObjectGroup() || sibling->isGroupLayer()) {
                prev = sibling;
                break;
            }
        }

        auto &filtered = filteredChildLayers(layer->parentLayer());
        int row = prev ? filtered.indexOf(prev) + 1 : 0;

        QModelIndex parent;
        if (layer->parentLayer())
            parent = index(layer->parentLayer());

        beginInsertRows(parent, row, row);
        filtered.insert(row, layer);
        endInsertRows();
    }
}

void RoiObjectModel::layerChanged(Layer *layer)
{
    if (layer->isObjectGroup() || layer->isGroupLayer()) {
        QModelIndex index = this->index(layer);
        emit dataChanged(index, index);
    }
}

void RoiObjectModel::layerAboutToBeRemoved(GroupLayer *groupLayer, int index)
{
    const auto &layers = groupLayer ? groupLayer->layers() : mRoi->layers();
    Layer *layer = layers.at(index);

    if (layer->isObjectGroup() || layer->isGroupLayer()) {
        auto &filtered = filteredChildLayers(groupLayer);
        const int row = filtered.indexOf(layer);

        QModelIndex parent = groupLayer ? this->index(groupLayer) : QModelIndex();

        beginRemoveRows(parent, row, row);
        filtered.removeAt(row);
        endRemoveRows();
    }
}

void RoiObjectModel::emitObjectsChanged(const QList<RoiObject *> &objects, const QList<Column> &columns)
{
    emit objectsChanged(objects);
    if (columns.isEmpty())
        return;

    auto minMaxPair = std::minmax_element(columns.begin(), columns.end());
    for (auto object : objects) {
        emit dataChanged(index(object, *minMaxPair.first), index(object, *minMaxPair.second));
    }
}

void RoiObjectModel::emitObjectsChanged(const QList<RoiObject *> &objects, Column column)
{
    emitObjectsChanged(objects,
                       QList<RoiObjectModel::Column>() << column);
}

QList<Layer *> &RoiObjectModel::filteredChildLayers(GroupLayer *parentLayer) const
{
    if (!mFilteredLayers.contains(parentLayer)) {
        QList<Layer*> &filtered = mFilteredLayers[parentLayer];
        const auto &layers = parentLayer ? parentLayer->layers() : mRoi->layers();
        for (Layer *layer : layers)
            if (layer->isObjectGroup() || layer->isGroupLayer())
                filtered.append(layer);
        return filtered;
    }

    return mFilteredLayers[parentLayer];
}

void RoiObjectModel::insertObject(ObjectGroup *og, int index, RoiObject *o)
{
    const int row = (index >= 0) ? index : og->objectCount();
    beginInsertRows(this->index(og), row, row);
    og->insertObject(row, o);
    endInsertRows();
    emit objectsAdded(QList<RoiObject*>() << o);
}

int RoiObjectModel::removeObject(ObjectGroup *og, RoiObject *o)
{
    if (og == nullptr)
        return -1;
    QList<RoiObject*> objects;
    objects << o;

    const int row = og->objects().indexOf(o);
    beginRemoveRows(index(og), row, row);
    og->removeObjectAt(row);
    endRemoveRows();
    emit objectsRemoved(objects);
    return row;
}

void RoiObjectModel::moveObjects(ObjectGroup *og, int from, int to, int count)
{
    const QModelIndex parent = index(og);
    if (!beginMoveRows(parent, from, from + count - 1, parent, to)) {
        Q_ASSERT(false); // The code should never attempt this
        return;
    }

    og->moveObjects(from, to, count);
    endMoveRows();
}

void RoiObjectModel::setObjectPolygon(RoiObject *o, const QPolygonF &polygon)
{
    if (o->polygon() == polygon)
        return;

    o->setPolygon(polygon);
    emit objectsChanged(QList<RoiObject*>() << o);
}

void RoiObjectModel::setObjectPosition(RoiObject *o, const QPointF &pos)
{
    if (o->position() == pos)
        return;

    o->setPosition(pos);
    emit objectsChanged(QList<RoiObject*>() << o);
}

void RoiObjectModel::setObjectSize(RoiObject *o, const QSizeF &size)
{
    if (o->size() == size)
        return;

    o->setSize(size);
    emit objectsChanged(QList<RoiObject*>() << o);
}

void RoiObjectModel::setObjectRotation(RoiObject *o, qreal rotation)
{
    if (o->rotation() == rotation)
        return;

    o->setRotation(rotation);
    emit objectsChanged(QList<RoiObject*>() << o);
}

void RoiObjectModel::setObjectProperty(RoiObject *o,
                                       RoiObject::Property property,
                                       const QVariant &value)
{
    if (o->roiObjectProperty(property) == value)
        return;

    o->setRoiObjectProperty(property, value);

    QList<RoiObject*> objects = QList<RoiObject*>() << o;

    // Notify views about certain property changes
    switch (property) {
    case RoiObject::NameProperty:
    case RoiObject::VisibleProperty: {
        QModelIndex index = this->index(o, 0);
        emit dataChanged(index, index);
        break;
    }
    case RoiObject::TypeProperty: {
        QModelIndex index = this->index(o, 1);
        emit dataChanged(index, index);
        emit objectsTypeChanged(objects);
        break;
    }
    default:
        break;
    }

    emit objectsChanged(objects);
}
} // namespace Qroilib

