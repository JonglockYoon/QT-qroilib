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
#include "genicammodel.h"
#include "genicammodel_p.h"
#include "genicamobjectbuilder.h"
#include "inode.h"
#include "inode_p.h"
#include "icommand.h"
#include "iinteger.h"
#include "ifloat.h"
#include "ienumeration.h"
#include "istring.h"
#include "iboolean.h"
#include "xmlhelper.h"

#include <QFont>
#include <QColor>
#include <QTimerEvent>
#include <QSize>
#include <QDomNode>

using namespace Jgv::GenICam;


Model::Model(QSharedPointer<IPort::Interface> iport, QObject *parent)
    : QAbstractItemModel(parent), d_ptr(new ModelPrivate(iport))
{}

Model::~Model()
{}

void Model::setGenICamXml(const QString &fileName, const QByteArray &file)
{
    Q_D(Model);

    InterfaceBuilder builder(fileName, file, d->iport);
    // Constructs the complete tree of inodes
    builder.buildInode("Root");

    // Manages the lifetime of inodes
    d->inodes = builder.allReferencedInodes();
}

QModelIndex Model::searchFeature(int column, const QString &featureName) const
{
    Q_D(const Model);

    QSharedPointer<Inode::Object> inodePtr = d->inodes.value(featureName);
    if (!inodePtr) {
        return QModelIndex();
    }
    return createIndex(inodePtr->row(), column, inodePtr.data());
}

QVariant Model::getValue(const QString &featureName) const
{
    Q_D(const Model);

    QSharedPointer<Inode::Object> inodePtr = d->inodes.value(featureName);
    if (!inodePtr) {
        return QVariant();
    }
    if (JGV_ITYPE(inodePtr.data()) == Type::IInteger) {
        return JGV_IINTEGER(inodePtr)->getValue();
    }
    if (JGV_ITYPE(inodePtr.data()) == Type::IEnumeration) {
        return JGV_IENUMERATION(inodePtr)->getIntValue();
    }

    return QVariant();
}

void Model::setValue(const QString &featureName, const QVariant &value)
{
    Q_D(Model);

    QSharedPointer<Inode::Object> inodePtr = d->inodes.value(featureName);
    if (inodePtr) {
        if (JGV_ITYPE(inodePtr) == Type::IInteger) {
            JGV_IINTEGER(inodePtr)->setValue(value.toLongLong());
        }
        else if (JGV_ITYPE(inodePtr) == Type::ICommand) {
            JGV_ICOMMAND(inodePtr)->execute();
        }
    }
}

QModelIndex Model::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const Model);

    // Test the index (row >= 0, column >= 0, row < rowCount(parent), column < columnCount(parent))
    if (Q_UNLIKELY(!hasIndex(row, column, parent))) {
        return QModelIndex();
    }

    // If no parent, we are on the root
    Inode::Object *inode = (parent.isValid())
            ? static_cast<Inode::Object *>(parent.internalPointer())
            : d->inodes.value("Root").data();

    if (inode == nullptr) {
        return QModelIndex();
    }



    QSharedPointer<Inode::Object> childPtr = inode->getChild(row).toStrongRef();
    if (childPtr) {
        return createIndex(row, column, childPtr.data());
    }


    return QModelIndex();
}

QModelIndex Model::parent(const QModelIndex &child) const
{
    if (Q_UNLIKELY(!child.isValid())) {
        return QModelIndex();
    }

    Inode::Object *inode = static_cast<Inode::Object *>(child.internalPointer());
    if (inode == nullptr) {
        return QModelIndex();
    }

    return createIndex(inode->row(), 0, inode->parent().data());
}

int Model::rowCount(const QModelIndex &parent) const
{
    Q_D(const Model);

    Inode::Object *inode = (parent.isValid())
            ? static_cast<Inode::Object *>(parent.internalPointer())
            : d->inodes.value("Root").data();
    if (inode == nullptr) {
        return 0;
    }

    return inode->childCount();
}

int Model::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    // DisplayName, Value, Visibility
    return 3;
}



QVariant Model::data(const QModelIndex &index, int role) const
{

    if (!index.isValid()) {
        return QVariant();
    }



    Inode::Object *inode = static_cast<Inode::Object *>(index.internalPointer());
    if (!inode) {
        return QVariant();
    }


    switch (role) {
    case Qt::UserRole:
        // The pointer
        return qVariantFromValue(Inode::Ptr(inode));

    case Qt::DisplayRole:
        // Column displayName
        if (index.column() == 0) {
            return inode->displayName();
        }
        else if (index.column() == 1) {
            if (!inode->isImplemented()) return trUtf8("{Not implemented}");
            if (!inode->isAvailable()) return trUtf8("{Unavailable}");
            if (inode->typeString() == "Category") return " ";
            return inode->getVariant();
        }
        else if (index.column() == 2) {
            return inode->visibility();
        }
        break;
    case Qt::FontRole:
        // First column with child, in bold
        if (index.column() == 0 && inode->haveChild()) {
            QFont font;
            font.setBold(true);
            return font;
        }
        break;
    case Qt::ForegroundRole:
        //Without children (only ICategory has children)
        if (!inode->haveChild()) {
            // First column dark blue
            if (index.column() == 0) {
                return QVariant(QColor(Qt::darkBlue));
            }
            // Second column in gray if not modifiable, in red if locked
            if (index.column() == 1) {
                if (!inode->isWritable()) {
                    return QColor(Qt::darkGray);
                }
                if (inode->isLocked())
                {
                    return QColor(Qt::darkRed);
                }
            }
        }
        else if (index.column() == 0) {
            // First category column in black
            return QColor(Qt::black);
        }

        break;
    }
    return QVariant();
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return "Feature";
        }
        if (section == 1) {
            return "Value";
        }
        if (section == 2) {
            return "Visibility";
        }
    }
    return QVariant();
}

Qt::ItemFlags Model::flags(const QModelIndex &index) const
{
    // If the index is invalid, or concerns a non-visible column
    if (!index.isValid() || index.column() > 1) {
        return Qt::NoItemFlags;
    }

    Inode::Object *inode = static_cast<Inode::Object *>(index.internalPointer());
    if (!inode) {
        return Qt::NoItemFlags;
    }

    //Are activated only items in the second column without children
    if ((index.column() != 1) || inode->haveChild()) {
        return Qt::NoItemFlags;
    }

    // If the inode is writable and not locked, it becomes editable
    return (inode->isWritable() && (!inode->isLocked())) ? Qt::ItemIsEnabled|Qt::ItemIsEditable : Qt::ItemIsEnabled;
}

bool Model::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_D(Model);

    if (role != Qt::EditRole) {
        return false;
    }


    Inode::Object *inode = static_cast<Inode::Object *>(index.internalPointer());
    if (!inode) {
        return false;
    }

    bool ret = false;
    switch (JGV_ITYPE(inode)) {

    case Type::ICommand:
        JGV_ICOMMAND(inode)->execute();
        ret = true;
        break;

    case Type::IInteger:
        JGV_IINTEGER(inode)->setValue(value.toLongLong());
        ret = true;
        break;

    case Type::IFloat:
        JGV_IFLOAT(inode)->setValue(value.toDouble());
        ret = true;
        break;

    case Type::IEnumeration:
        JGV_IENUMERATION(inode)->setIntValue(value.toLongLong());
        ret = true;
        break;

    case Type::IString:
        JGV_ISTRING(inode)->setValue(value.toByteArray());
        ret = true;
        break;

    case Type::IBoolean:
        JGV_IBOOLEAN(inode)->setValue(value.toBool());
        ret = true;
        break;


    default:
        qWarning("%s", qPrintable(QString("GenicamModel setData error %0").arg(inode->typeString())));
    }


    if (ret) {
        emit dataChanged(index, index);
        const QStringList invalidateNames = inode->invalidateNames();
        auto it = invalidateNames.constBegin();
        for (; it != invalidateNames.constEnd(); ++it) {
            QSharedPointer<Inode::Object> invalideInode = d->inodes.value(*it);
            if (invalideInode) {
                QModelIndex invalideInodeIndex = createIndex(invalideInode->row(), 1, invalideInode.data());
                dataChanged(invalideInodeIndex, invalideInodeIndex);
            }
        }
    }
    return ret;
}

QString Model::name() const
{
    Q_D(const Model);

    // We go through the inodes
    QSharedPointer<Inode::Object> inode = d->inodes.value("DeviceModelName");
    if (inode) {
        if (inode->interface()->interfaceType() == Type::IString) {
            return JGV_ISTRING(inode)->getValue();
        }
    }
    return trUtf8("GiGE Vision caméra");
}

void Model::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    //    QList<QSharedPointer<JgvInode::Object> > inodes = m_pollingsTimer.values(event->timerId());
    //    foreach(QSharedPointer<JgvInode::Object> inode, inodes) {
    //        QModelIndex index = createIndex(inode->row(), 1, inode.data());
    //        dataChanged(index, index);
    //    }
}

