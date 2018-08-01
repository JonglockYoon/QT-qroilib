/*
 * createtextobjecttool.cpp
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

#include "createtextobjecttool.h"

//#include "mapdocument.h"
#include "roiobject.h"
#include "roiobjectitem.h"
#include "roirenderer.h"
#include "snaphelper.h"
#include "utils.h"
#include <qroilib/documentview/documentview.h>

namespace Qroilib {

CreateTextObjectTool::CreateTextObjectTool(QObject *parent)
    : CreateObjectTool(parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-text.png")));
    setThemeIcon(this, "insert-text");
    languageChanged();
}

void CreateTextObjectTool::mouseMovedWhileCreatingObject(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    const RoiRenderer *renderer = mapDocument()->renderer();

    const RoiObject *roiObject = mNewRoiObjectItem->roiObject();
    const QPointF diff(-roiObject->width() / 2, -roiObject->height() / 2);
    QPointF pixelCoords = renderer->screenToPixelCoords(pos + diff);

    SnapHelper(renderer, modifiers).snap(pixelCoords);

    mNewRoiObjectItem->roiObject()->setPosition(pixelCoords);
    mNewRoiObjectItem->syncWithRoiObject();
    mNewRoiObjectItem->setZValue(10000); // sync may change it
}

void CreateTextObjectTool::mousePressedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        cancelNewRoiObject();
}

void CreateTextObjectTool::mouseReleasedWhileCreatingObject(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        finishNewRoiObject();
}

void CreateTextObjectTool::languageChanged()
{
    setName(tr("Insert Text"));
    setShortcut(QKeySequence(tr("X")));
}

RoiObject *CreateTextObjectTool::createNewRoiObject()
{
    TextData textData;
    textData.text = tr("Hello World");

    RoiObject *newRoiObject = new RoiObject;
    newRoiObject->setShape(RoiObject::Text);
    newRoiObject->setTextData(textData);
    newRoiObject->setSize(textData.textSize());
    return newRoiObject;
}

} // namespace Qroilib
