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
    // construit l'arborescence complète des inodes
    builder.buildInode("Root");

    // gère la durée de vie des inodes
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
    if (JGV_ITYPE(inodePtr.data()) == Type::IString) {
        return JGV_ISTRING(inodePtr)->getValue();
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

    // teste l'index (row >= 0, column >= 0, row < rowCount(parent), column < columnCount(parent))
    if (Q_UNLIKELY(!hasIndex(row, column, parent))) {
        return QModelIndex();
    }

    // si pas de parent, on est sur la racine
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
        // on renvoie le pointeur
        return qVariantFromValue(Inode::Ptr(inode));

    case Qt::DisplayRole:
        // colonne displayName
        if (index.column() == 0) {
            return inode->displayName();
        }
        else if (index.column() == 1) {
            if (!inode->isImplemented()) return trUtf8("{Non implémenté}");
            if (!inode->isAvailable()) return trUtf8("{Indisponible}");
            if (inode->typeString() == "Category") return " ";
            return inode->getVariant();
        }
        else if (index.column() == 2) {
            return inode->visibility();
        }
        break;
    case Qt::FontRole:
        // première colonne avec enfant, en gras
        if (index.column() == 0 && inode->haveChild()) {
            QFont font;
            font.setBold(true);
            return font;
        }
        break;
    case Qt::ForegroundRole:
        // sans enfant (seule ICategory a des enfants)
        if (!inode->haveChild()) {
            // première colonne bleu foncé
            if (index.column() == 0) {
                return QVariant(QColor(Qt::darkBlue));
            }
            // deuxième colonne en gris si pas modifiable, en rouge si verrouillé
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
            // première colonne category en noir
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
    // si l'index est invalide, ou concerne une colonne non visible
    if (!index.isValid() || index.column() > 1) {
        return Qt::NoItemFlags;
    }

    Inode::Object *inode = static_cast<Inode::Object *>(index.internalPointer());
    if (!inode) {
        return Qt::NoItemFlags;
    }

    // ne sont activé que les items de la 2eme colonne sans enfants
    if ((index.column() != 1) || inode->haveChild()) {
        return Qt::NoItemFlags;
    }

    // si l'inode est writable et pas verrouillé, il devient éditable
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
        const auto invalidateNames = inode->invalidateNames();
        for (auto const &name: invalidateNames) {
            const auto invalideInode = d->inodes.value(name);
            if (invalideInode) {
                const auto invalideInodeIndex = createIndex(invalideInode->row(), 1, invalideInode.data());
                dataChanged(invalideInodeIndex, invalideInodeIndex);
            }
        }
    }
    return ret;
}

QString Model::name() const
{
    Q_D(const Model);

    // on passe par les inodes
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

