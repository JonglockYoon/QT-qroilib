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

#ifndef GENICAMEDITOR_H
#define GENICAMEDITOR_H

#include <QFrame>

namespace Jgv {

namespace GenICam {

class Editor : public QFrame
{
    Q_OBJECT

signals:
    void editingFinished();

protected:
    Editor(const QString &name, QWidget * parent = nullptr);
    virtual ~Editor() = default;

};

class IntegerEditor : public Editor
{
    Q_OBJECT
public:
    IntegerEditor(const QString &name, QWidget * parent = nullptr) : Editor(name, parent) {}
    virtual ~IntegerEditor() = default;

    virtual void setRange(qint64 minimum, qint64 maximum, qint64 inc) = 0;
    virtual void setValue(qint64 value) = 0;
    virtual qint64 value() const = 0;

};

class FloatEditor : public Editor
{
    Q_OBJECT
public:
    FloatEditor(const QString &name, QWidget * parent = nullptr) : Editor(name, parent) {}
    virtual ~FloatEditor() = default;

    virtual void setRange(double minimum, double maximum) = 0;
    virtual void setValue(double value) = 0;
    virtual double value() const = 0;
};

class EnumerationEditor : public Editor
{
    Q_OBJECT
public:
    EnumerationEditor(const QString &name, QWidget * parent = nullptr) : Editor(name, parent) {}
    virtual ~EnumerationEditor() = default;

    virtual void addItem(const QString& text, const qint64 value) = 0;
    virtual void setCurrentIndex(int index) = 0;
    virtual qint64 value() const = 0;

};

class CommandEditor : public Editor
{
    Q_OBJECT
public:
    CommandEditor(const QString &name, QWidget * parent = nullptr) : Editor(name, parent) {}
    virtual ~CommandEditor() = default;

    virtual bool isClicked() const = 0;
};

class StringEditor : public Editor
{
    Q_OBJECT
public:
    StringEditor(const QString &name, QWidget * parent = nullptr) : Editor(name, parent) {}
    virtual ~StringEditor() = default;

    virtual QString value() const = 0;
    virtual void setValue(const QString &value) = 0;

};


class BooleanEditor : public Editor
{
    Q_OBJECT
public:
    BooleanEditor(const QString &name, QWidget * parent = nullptr) : Editor(name, parent) {}
    virtual ~BooleanEditor() = default;

    virtual bool value() const = 0;
    virtual void setValue(bool value) = 0;

};

class EditorFactory {
public:
    static IntegerEditor *createIntegerEditor(const QString &featureName, const QString &displayName, QWidget *parent);
    static FloatEditor *createDoubleEditor(const QString &featureName, const QString &displayName, QWidget *parent);
    static CommandEditor *createCommandEditor(const QString &featureName, const QString &displayName, QWidget *parent);
    static EnumerationEditor *createEnumerationEditor(const QString &featureName, const QString &displayName, QWidget *parent);
    static BooleanEditor *createBooleanEditor(const QString &featureName, const QString &displayName, QWidget *parent);
    static StringEditor *createStringEditor(const QString &featureName, const QString &displayName, QWidget *parent);
};

} // namespace GenICam

} // namespace Jgv

#endif // GENICAMEDITOR_H
