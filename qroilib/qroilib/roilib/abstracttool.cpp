/*
 * abstracttool.cpp
 * Copyright 2009-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "abstracttool.h"

//#include "document.h"
#include <qroilib/documentview/documentview.h>

#include <QKeyEvent>

namespace Qroilib {

AbstractTool::AbstractTool(const QString &name, const QIcon &icon,
                           const QKeySequence &shortcut, QObject *parent)
    : QObject(parent)
    , mName(name)
    , mIcon(icon)
    , mShortcut(shortcut)
    , mEnabled(false)
    , mRoiDocument(nullptr)
{
}

/**
 * Sets the current status information for this tool. This information will be
 * displayed in the status bar.
 */
void AbstractTool::setStatusInfo(const QString &statusInfo)
{
    if (mStatusInfo != statusInfo) {
        mStatusInfo = statusInfo;
        emit statusInfoChanged(mStatusInfo);
    }
}

/**
 * Sets the cursor used by this tool. This will be the cursor set on the
 * viewport of the MapView while the tool is active.
 */
void AbstractTool::setCursor(const QCursor &cursor)
{
    mCursor = cursor;
    emit cursorChanged(cursor);
}

void AbstractTool::setEnabled(bool enabled)
{
    if (mEnabled == enabled)
        return;

    mEnabled = enabled;
    emit enabledChanged(enabled);
}

void AbstractTool::keyPressed(QKeyEvent *event)
{
    event->ignore();
}

void AbstractTool::setMapDocument(DocumentView *mapDocument)
{
    if (mRoiDocument == mapDocument)
        return;

//    if (mRoiDocument) {
//        disconnect(mRoiDocument, &MapDocument::layerChanged,
//                   this, &AbstractTool::updateEnabledState);
//        disconnect(mRoiDocument, &MapDocument::currentLayerChanged,
//                   this, &AbstractTool::updateEnabledState);
//    }

    DocumentView *oldDocument = mRoiDocument;
    mRoiDocument = mapDocument;
    mapDocumentChanged(oldDocument, mRoiDocument);

//    if (mRoiDocument) {
//        connect(mRoiDocument, &MapDocument::layerChanged,
//                this, &AbstractTool::updateEnabledState);
//        connect(mRoiDocument, &MapDocument::currentLayerChanged,
//                this, &AbstractTool::updateEnabledState);
//    }
    updateEnabledState();
}

void AbstractTool::updateEnabledState()
{
    setEnabled(mRoiDocument != nullptr);
}

Layer *AbstractTool::currentLayer() const
{
    return mRoiDocument ? mRoiDocument->currentLayer() : nullptr;
}


void AbstractTool::deleteLater()
{

}

} // namespace Qroilib
