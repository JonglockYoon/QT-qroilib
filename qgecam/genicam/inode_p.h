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

#ifndef INODE_P_H
#define INODE_P_H

#include "pvalue.h"

#include <QString>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QMap>

namespace Jgv {

namespace GenICam {

namespace IPort {
class Interface;
} // namespace IPort

namespace Enumeration {
class Object;
}


namespace Inode {

class Object;

namespace Attributes {
static const char * Name = "Name";
static const char * NameSpace = "NameSpace";
}

namespace Properties {
static const char * ToolTip = "ToolTip";
static const char * Description = "Description";
static const char * DisplayName = "DisplayName";
static const char * Visibility = "Visibility";
static const char * ImposedAccessMode = "ImposedAccessMode";
static const char * EventID = "EventID";
static const char * pIsImplemented = "pIsImplemented";
static const char * pIsAvailable = "pIsAvailable";
static const char * pIsLocked = "pIsLocked";
static const char * pBlockPolling = "pBlockPolling";
static const char * pAlias = "pAlias";
static const char * pCastAlias = "pCastAlias";
static const char * Streamable = "Streamable";
static const char * pError = "pError";
}

namespace Visibility {
static const char * Beginner = "Beginner";
static const char * Expert = "Expert";
static const char * Guru = "Guru";
static const char * Invisible = "Invisible";
}

namespace AccessMode {
static const char * ReadOnly = "RO";
static const char * ReadWrite = "RW";
static const char * WriteOnly = "WO";
}

class ObjectPrivate
{
    using evaluateAsBool = bool (ObjectPrivate::*) () const;
    using evaluateAsInteger = qint64 (ObjectPrivate::*) () const;

public:
    ObjectPrivate(QSharedPointer<IPort::Interface> iport)
        : iport(iport)
    {
        isImplemented = &ObjectPrivate::trueResponse;
        isAvailable = &ObjectPrivate::trueResponse;
        isLocked = &ObjectPrivate::falseResponse;
        error = &ObjectPrivate::noError;

    }
    virtual ~ObjectPrivate() = default;

    QString sType { "UndefinedInode" };

    QString toolTip { "Empty" };
    QString description { "No description" };
    QString featureName { "Undefined" };
    QString displayName;
    QString visibility { Visibility::Beginner };

    quint64 eventID { 0 };

    pValueInteger pIsImplemented;
    pValueInteger pIsAvailable;
    pValueInteger pIsLocked;

    bool imposedCanRead = true;
    bool imposedCanWrite = true;
    bool namespaceIsCustom = false;

    QWeakPointer<Object> pBlockingPolling;
    QWeakPointer<Object> pAlias;
    QWeakPointer<Object> pCastAlias;

    bool isStreamable { false };

    pValueInteger pError;
    QMap<quint64, QString> errorsMap;

    int row = -1;
    QWeakPointer<Inode::Object> parent;
    QList<QWeakPointer<Object> > inodesToInvalidate;

    QSharedPointer<IPort::Interface> iport;

    evaluateAsBool isImplemented;// = &trueResponse;
    evaluateAsBool isAvailable;// = &ObjectPrivate::trueResponse;
    evaluateAsBool isLocked;// = &ObjectPrivate::falseResponse;
    evaluateAsInteger error;// = &ObjectPrivate::noError;

    bool trueResponse() const;
    bool falseResponse() const;
    bool isImplementedByPValue() const;
    bool isAvailableByPValue() const;
    bool isLockedByPValue() const;
    qint64 fakeInteger() const;
    qint64 noError() const;
    qint64 errorNum() const;

}; // ObjectPrivate

} // Inode

} // namespace GenICam

} // namespace Jgv

#endif // INODE_P_H
