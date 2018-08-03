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
#ifndef GENICAMMODEL_H
#define GENICAMMODEL_H

#include <QAbstractItemModel>
#include <QSharedPointer>


namespace Jgv {

namespace GenICam {

namespace IPort {
class Interface;
}

namespace Inode {
class Object;
}

class ModelPrivate;
class Model final : public QAbstractItemModel
{
public:
    Model(QSharedPointer<IPort::Interface> iport, QObject *parent = nullptr);
    virtual ~Model();

    void setGenICamXml(const QString &fileName, const QByteArray &file);
    QModelIndex searchFeature(int column, const QString &featureName) const;

    QVariant getValue(const QString &featureName) const;
    void setValue(const QString &featureName, const QVariant &value);

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;


    Qt::ItemFlags flags (const QModelIndex & index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role= Qt::EditRole) override;

    QString name() const;

protected:
    void timerEvent(QTimerEvent *event);

private:
    const QScopedPointer<ModelPrivate> d_ptr;
    Q_DISABLE_COPY(Model)
    Q_DECLARE_PRIVATE(Model)
};

} // namespace GenICam

} // namespace Jgv

#endif // GENICAMMODEL_H
