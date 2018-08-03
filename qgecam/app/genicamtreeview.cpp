/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD *
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

#include "genicamtreeview.h"

#include "inode.h"
#include "iinteger.h"
#include "ifloat.h"
#include "gui/genicammodel.h"
#include "gui/genicamdelegate.h"

#include <QAction>
#include <QTreeView>
#include <QTextEdit>
#include <QComboBox>
#include <QSortFilterProxyModel>
#include <limits>


using namespace Qroilib;

namespace  {
static const char * BEGINNER = "Beginner";
static const char * EXPERT = "Expert";
static const char * GURU = "Guru";
static const char * INVISIBLE = "Invisible";

inline void updateProxyModel(const QString &value, QSortFilterProxyModel *proxyModel)
{
    if (value.startsWith(BEGINNER)) {
        proxyModel->setFilterRegExp(BEGINNER);
    } else if (value.startsWith(EXPERT)) {
        proxyModel->setFilterRegExp(QString("%0|%1").arg(BEGINNER).arg(EXPERT));
    } else if (value.startsWith(GURU)) {
        proxyModel->setFilterRegExp(
                    QString("%0|%1|%2").arg(BEGINNER).arg(EXPERT).arg(GURU));
    } else {
        proxyModel->setFilterRegExp("");
    }
    proxyModel->invalidate();
}



inline void updateInfos(Jgv::GenICam::Inode::Object *inode, QTextEdit *infos)
{
    infos->clear();

    // No description on nodes category
    if (JGV_ITYPE(inode) == Jgv::GenICam::Type::ICategory) {
        return;
    }

    infos->append(QString("<b>%1</b>").arg(inode->displayName()));
    QString description = inode->description();
    if (description.isEmpty()) {
        description = QObject::trUtf8("no description");
    }
    infos->append(description);
    infos->append("");
    infos->append(QString("FEATURE NAME: <b> %0</b>").arg(inode->featureName()));
    infos->append(QString("TYPE: <b>%0</b>").arg(inode->typeString()));

    switch (JGV_ITYPE(inode)) {
    case Jgv::GenICam::Type::IInteger: {
        const Jgv::GenICam::Integer::Interface *iint = JGV_IINTEGER(inode);
        if (iint != NULL) {
            const qint64 min = iint->getMin();
            const qint64 max = iint->getMax();
            const qint64 inc = iint->getInc();
            if (min > std::numeric_limits<qint64>::lowest()) {
                infos->append(QString("MIN: <b>%0</b>").arg(min));
            }
            if (max < std::numeric_limits<qint64>::max()) {
                infos->append(QString("MAX: <b>%0</b>").arg(max));
            }
            if (inc > 0) {
                infos->append(QString("INCREMENT: <b>%0</b>").arg(inc));
            }
        }
        break;
    }

    case Jgv::GenICam::Type::IFloat: {
        const Jgv::GenICam::Float::Interface *ifloat = JGV_IFLOAT(inode);
        if (ifloat != NULL) {
            const double min = ifloat->getMin();
            const double max = ifloat->getMax();
            const double inc = ifloat->getInc();

            if (min > std::numeric_limits<double>::lowest()) {
                infos->append(QString("MIN: <b>%0</b>").arg(min, 0, 'f'));
            }
            if (max < std::numeric_limits<double>::max()) {
                infos->append(QString("MAX: <b>%0</b>").arg(max, 0, 'f'));
            }
            if (inc > 0.) {
                infos->append(QString("INCREMENT: <b>%0</b>").arg(inc, 0, 'f'));
            }
        }
        break;
    }
    default:
        break;
    }

    const QStringList invalidators { inode->getInvalidatorFeatureNames() };
    if (!invalidators.isEmpty()) {
        auto it = invalidators.constBegin();
        QString invalidator;
        for (; it != invalidators.constEnd(); ++it) {
            invalidator.append(QString("%0 ").arg(*it));
        }
        infos->append(QObject::trUtf8("IS INVALID BY: <b>%0</b>").arg(invalidator));
    }

    const QStringList invalidate = inode->invalidateNames();
    if (!invalidate.isEmpty()) {
        auto it = invalidate.constBegin();
        QString invalidator;
        for (; it != invalidate.constEnd(); ++it) {
            invalidator.append(QString("%0 ").arg(*it));
        }
        infos->append(QObject::trUtf8("INVALIDE: <b>%0</b>").arg(invalidator));
    }
}

} // Anonymous namespace


GenICamTreeView::GenICamTreeView(QWidget *parent)
    : QSplitter(Qt::Vertical, parent)
{}

void GenICamTreeView::setModel(Jgv::GenICam::Model *model, bool visibilitySelector)
{
    if (count() != 0) {
        qWarning("GenICamTreeView setModel failed, model allready set");
        return;
    }

    QTreeView *tree = new QTreeView;

    tree->setUniformRowHeights(true);
    tree->rootIsDecorated();
    tree->setItemDelegate(new Jgv::GenICam::GenicamDelegate(this));


    if (visibilitySelector) {
        // A proxy is used to filter according to visibility
        QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
        proxyModel->setSourceModel(model);
        // We filter on the column visibility
        proxyModel->setFilterKeyColumn(2);
        proxyModel->setFilterRegExp(BEGINNER);

        // Construction de l'him
        QComboBox *visibility = new QComboBox;
        addWidget(visibility);
        visibility->addItems({BEGINNER,EXPERT,GURU,INVISIBLE});
        visibility->setCurrentIndex(0);
        connect(visibility, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), [proxyModel](const QString &value) {
            updateProxyModel(value, proxyModel);
        });

        tree->setModel(proxyModel);
    }
    else {
        tree->setModel(model);
    }

    addWidget(tree);
    tree->resizeColumnToContents(0);
    tree->setColumnHidden(2, true);

    QTextEdit *infos = new QTextEdit;
    infos->setReadOnly(true);
    addWidget(infos);
    setStretchFactor(indexOf(tree), 4);

    connect(tree, &QTreeView::clicked, [infos](const QModelIndex &index) {
        Jgv::GenICam::Inode::Object *inode = index.model()->data(index, Qt::UserRole).value<Jgv::GenICam::Inode::Ptr>().data;
        if (inode != nullptr) {
            updateInfos(inode, infos);
        }
    });
}

