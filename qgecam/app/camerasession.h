/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD                                  *
 *   cyril.baletaud@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef CAMERASESSION_H
#define CAMERASESSION_H

#include <QObject>

class QAction;
class GvcpUtils;
class QStatusBar;

namespace Qroilib {

class CameraSessionPrivate;
class CameraSession : public QObject
{

public:
    CameraSession(QObject *parent = nullptr);

protected:
    CameraSession(CameraSessionPrivate &dd, QObject *parent);

public:
    virtual ~CameraSession();

    QWidget *mainWidget() const;
    QWidget *leftDockedWidget() const;
    QStatusBar *statusBar() const;
    QWidget *bootstrapWidget() const;

    QList<QAction *> menuActions() const;
    QList<QAction *> toolbarActions() const;

    bool startSession();

protected:
    const QScopedPointer<CameraSessionPrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(CameraSession)

};

} // namespace Qroilib

#endif // CAMERASESSION_H
