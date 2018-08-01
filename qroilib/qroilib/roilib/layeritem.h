/*
 * layeritem.h
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QGraphicsItem>

namespace Qroilib {

class Layer;
class DocumentView;

class LayerItem : public QGraphicsItem
{
public:
    LayerItem(Layer *layer, QGraphicsItem *parent = nullptr);

    Layer *layer() const { return mLayer; }
    QRectF boundingRect() const override;
    void setMapDocument(DocumentView *roimap) { mRoiDocument = roimap; }
    DocumentView *mapDocument() const { return mRoiDocument; }

private:
    //QRectF mBoundingRect;
    Layer *mLayer;
    DocumentView *mRoiDocument;
};

} // namespace Qroilib
