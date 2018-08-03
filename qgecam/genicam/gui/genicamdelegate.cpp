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

#include "genicamdelegate.h"
#include "inode.h"
#include "iinterface.h"
#include "iinteger.h"
#include "ifloat.h"
#include "ienumeration.h"
#include "iboolean.h"
#include "istring.h"
#include "enumentry.h"
#include "genicameditor.h"

#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QList>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QPainter>

using namespace Jgv::GenICam;

GenicamDelegate::GenicamDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QWidget *GenicamDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    Inode::Object *inode = index.model()->data(index, Qt::UserRole).value<Inode::Ptr>().data;
    if (inode == nullptr) {
        return new QWidget;
    }

    switch (JGV_ITYPE(inode)) {

    case Type::ICommand: {
        CommandEditor *editor = EditorFactory::createCommandEditor(inode->featureName(), inode->displayName(), parent);
        editor->setMinimumHeight(editor->sizeHint().height());
        editor->setFocusPolicy(Qt::StrongFocus);
        connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
        return editor;
    }

    case Type::IInteger: {
        IntegerEditor *editor = EditorFactory::createIntegerEditor(inode->featureName(), inode->displayName(), parent);
        editor->setRange(JGV_IINTEGER(inode)->getMin(), JGV_IINTEGER(inode)->getMax(), JGV_IINTEGER(inode)->getInc());
        editor->setMinimumHeight(editor->sizeHint().height());
        editor->setFocusPolicy(Qt::StrongFocus);
        connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
        return editor;
    }

    case Type::IFloat: {
        FloatEditor *editor = EditorFactory::createDoubleEditor(inode->featureName(), inode->displayName(), parent);
        editor->setRange(JGV_IFLOAT(inode)->getMin(), JGV_IFLOAT(inode)->getMax());
        editor->setMinimumHeight(editor->sizeHint().height());
        editor->setFocusPolicy(Qt::StrongFocus);
        connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
        return editor;
    }

    case Type::IEnumeration: {
        EnumerationEditor *editor = EditorFactory::createEnumerationEditor(inode->featureName(), inode->displayName(), parent);
        editor->setMinimumHeight(editor->sizeHint().height());
        editor->setFocusPolicy(Qt::StrongFocus);
        connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
        return editor;
    }

    case Type::IBoolean: {
        BooleanEditor *editor = EditorFactory::createBooleanEditor(inode->featureName(), inode->displayName(), parent);
        editor->setMinimumHeight(editor->sizeHint().height());
        editor->setFocusPolicy(Qt::StrongFocus);
        connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
        return editor;
    }
    case Type::IString: {
        StringEditor *editor = EditorFactory::createStringEditor(inode->featureName(), inode->displayName(), parent);
        editor->setMinimumHeight(editor->sizeHint().height());
        editor->setFocusPolicy(Qt::StrongFocus);
        connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
        return editor;
    }

    default:
        // qWarning("%s", qPrintable(trUtf8("GenicamItemDelegate::createEditor %0 %1").arg(inode->featureName()).arg(inode->interface()->interfaceType())));
        break;
    }

    return new QWidget;
}

void GenicamDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Inode::Object *inode = index.model()->data(index, Qt::UserRole).value<Inode::Ptr>().data;
    if (inode == nullptr) {
        return;
    }

    switch (JGV_ITYPE(inode)) {

    case Type::IInteger: {
        IntegerEditor *widget = qobject_cast<IntegerEditor *>(editor);
        widget->setValue(JGV_IINTEGER(inode)->getValue());
        break;
    }

    case Type::IFloat: {
        FloatEditor *widget = qobject_cast<FloatEditor *>(editor);
        widget->setValue(JGV_IFLOAT(inode)->getValue());
        break;
    }

    case Type::IEnumeration: {
        EnumerationEditor *widget = qobject_cast<EnumerationEditor *>(editor);
        QList<Enumentry::Object *> entries = JGV_IENUMERATION(inode)->getEntries();
        qint64 current = JGV_IENUMERATION(inode)->getIntValue();
        for (int index=0; index<entries.count(); ++index) {
            if (entries.at(index)->isImplemented()) {
                widget->addItem(entries.at(index)->featureName(), entries.at(index)->getIntValue());
                if (entries.at(index)->getIntValue() == current) {
                    widget->setCurrentIndex(index);
                }
            }
        }
        break;
    }

    case Type::IBoolean: {
        BooleanEditor *widget = qobject_cast<BooleanEditor *>(editor);
        widget->setValue(JGV_IBOOLEAN(inode)->getValue());
        return;
    }

    case Type::IString: {
        StringEditor *widget = qobject_cast<StringEditor *>(editor);
        widget->setValue(JGV_ISTRING(inode)->getValue());
        break;
    }

    default:
        break;

    }
}

void GenicamDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    Inode::Object *inode = index.model()->data(index, Qt::UserRole).value<Inode::Ptr>().data;
    if (inode == nullptr) {
        return;
    }

    switch (JGV_ITYPE(inode)) {

    case Type::ICommand: {
        CommandEditor *widget = qobject_cast<CommandEditor *>(editor);
        if (widget->isClicked()) {
            model->setData(index, 0, Qt::EditRole);
        }
        break;
    }

    case Type::IInteger: {
        IntegerEditor *widget = qobject_cast<IntegerEditor *>(editor);
        model->setData(index, widget->value(), Qt::EditRole);
        break;
    }

    case Type::IFloat: {
        FloatEditor *widget = qobject_cast<FloatEditor *>(editor);
        model->setData(index, widget->value(), Qt::EditRole);
        break;
    }

    case Type::IEnumeration: {
        EnumerationEditor *widget = qobject_cast<EnumerationEditor *>(editor);
        model->setData(index, widget->value(), Qt::EditRole);
        break;
    }

    case Type::IBoolean: {
        BooleanEditor *widget = qobject_cast<BooleanEditor *>(editor);
        model->setData(index, widget->value(), Qt::EditRole);
        break;
    }

    case Type::IString: {
        StringEditor *widget = qobject_cast<StringEditor *>(editor);
        model->setData(index, widget->value(), Qt::EditRole);
        break;
    }

    default:
        break;
    }

}

void GenicamDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    QRect rect = editor->rect();
    rect.setWidth(option.rect.width());
    rect.moveCenter(option.rect.center());
    editor->setGeometry(rect);
}

void GenicamDelegate::commitAndCloseEditor()
{
    GenICam::Editor *editor = qobject_cast<GenICam::Editor *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}



//QSize GenicamItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//    QSize size = QStyledItemDelegate::sizeHint(option, index);
//    size.rheight() *= 1.25;
//    return size;
//}

