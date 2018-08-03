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

#ifndef INODEVALUE_H
#define INODEVALUE_H

#include "inode.h"
#include "iinteger.h"
#include "inodevalue_p.h"
#include "xmlhelper.h"
#include "genicamobjectbuilder.h"

namespace Jgv {

namespace GenICam {

namespace InodeValue {

template<typename T>
class Object : public Inode::Object
{

protected:
    Object(ObjectPrivate<T> &dd)
        : Inode::Object(dd)
    {}

public:
    virtual ~Object() = default;



    virtual void prepare(InterfaceBuilder &builder, const XMLHelper &helper) override
    {
        InodeValue::ObjectPrivate<T> * d = reinterpret_cast<ObjectPrivate<T> *>(d_ptr.data());

        Inode::Object::prepare(builder, helper);

        QString prop = helper.getProperty(Properties::Value);
        if (!prop.isEmpty()) {
            bool ok = false;
            T value = stringCast(prop, &ok);
            if (ok) {
                d->pVal = pValue<T>(value);
                d->evalValue = &ObjectPrivate<T>::valueByPValue;
                d->setValue = &ObjectPrivate<T>::setByPValue;
            }
            else {
                qWarning("InodeValue: %s failed to parse Value %s", qPrintable(featureName()), qPrintable(prop));
            }
        }
        else {
            prop = helper.getProperty(Properties::pValue);
            if (!prop.isEmpty()) {
                d->pVal = pValue<T>(builder.buildInode(prop));
                d->evalValue = &ObjectPrivate<T>::valueByPValue;
                d->setValue = &ObjectPrivate<T>::setByPValue;
                d->evalMin = &ObjectPrivate<T>::minByPValue;
                d->evalMax = &ObjectPrivate<T>::maxByPValue;
                d->evalInc = &ObjectPrivate<T>::incByPValue;

                // si on a une pValue, on peut avoir des pValueCopy
                QStringList props = helper.getProperties(Properties::pValueCopy);
                auto it = props.constBegin();
                for (; it != props.constEnd(); ++it) {
                    d->pValueCopies.append(pValue<T>(builder.buildInode(*it)));
                }
            }
            else {
                prop = helper.getProperty(Properties::pIndex);
                if (!prop.isEmpty()) {
                    d->pIndex = pValueInteger(builder.buildInode(prop));
                    d->evalValue = &ObjectPrivate<T>::valueByIndex;
                    d->setValue = &ObjectPrivate<T>::setByIndex;

                    bool ok = false;
                    // création de la map des index
                    QMap<QString, QString> map = helper.getAttributeValueMap(Properties::ValueIndexed, Attributes::Index);
                    auto it = map.constBegin();
                    for (; it != map.constEnd(); ++it) {
                        qint64 key = it.key().toLongLong(&ok, 0);
                        if (ok) {
                            T value = stringCast(it.value(), &ok);
                            if (ok) {
                                d->indexes.insert(key, value);
                            }
                        }
                    }

                    // création de la map des pIndex
                    map = helper.getAttributeValueMap(Properties::pValueIndexed, Attributes::Index);
                    for (it = map.constBegin(); it != map.constEnd(); ++it) {
                        d->pIndexes.insert(it.key().toLongLong(&ok, 0), pValue<T>(builder.buildInode(it.value())));
                    }

                    // on fixe les valeurs par défaut
                    prop = helper.getProperty(Properties::ValueDefault);
                    if (!prop.isEmpty()) {
                        bool ok = false;
                        T defaultValue = stringCast(prop, &ok);
                        if (ok) {
                            d->pValueDefault = pValue<T>(defaultValue);
                        }
                        else {
                            qWarning("InodeValue: %s failed to parse default value %s", qPrintable(featureName()), qPrintable(prop));
                        }
                    }
                    else {
                        prop = helper.getProperty(Properties::pValueDefault);
                        if (!prop.isEmpty()) {
                            d->pValueDefault = pValue<T>(builder.buildInode(prop));
                        }
                    }
                }
                else {
                    qWarning("InodeValue: %s failed to find Value, pValue or pIndex", qPrintable(featureName()));
                }
            }
        }

        prop = helper.getProperty(Properties::Min);
        if (!prop.isEmpty()) {
            bool ok = false;
            T min = stringCast(prop, &ok);
            if (ok) {
                d->pMin = pValue<T>(min);
                d->evalMin = &ObjectPrivate<T>::minByPMin;
            }
            else {
                qWarning("InodeValue: %s failed to parse Min %s", qPrintable(featureName()), qPrintable(prop));
            }
        }
        else {
            prop = helper.getProperty(Properties::pMin);
            if (!prop.isEmpty()) {
                d->pMin = pValue<T>(builder.buildInode(prop));
                d->evalMin = &ObjectPrivate<T>::minByPMin;
            }
        }

        prop = helper.getProperty(Properties::Max);
        if (!prop.isEmpty()) {
            bool ok = false;
            T max = stringCast(prop, &ok);
            if (ok) {
                d->pMax = pValue<T>(max);
                d->evalMax = &ObjectPrivate<T>::maxByPMax;
            }
            else {
                qWarning("InodeValue: %s failed to parse Max %s", qPrintable(featureName()), qPrintable(prop));
            }
        }
        else {
            prop = helper.getProperty(Properties::pMax);
            if (!prop.isEmpty()) {
                d->pMax = pValue<T>(builder.buildInode(prop));
                d->evalMax = &ObjectPrivate<T>::maxByPMax;
            }
        }

        prop = helper.getProperty(Properties::Inc);
        if (!prop.isEmpty()) {
            bool ok = false;
            T inc = stringCast(prop, &ok);
            if (ok) {
                d->pInc = pValue<T>(inc);
                d->evalInc = &ObjectPrivate<T>::incByPInc;
            }
            else {
                qWarning("InodeValue: %s failed to parse Inc %s", qPrintable(featureName()), qPrintable(prop));
            }
        }
        else {
            prop = helper.getProperty(Properties::pInc);
            if (!prop.isEmpty()) {
                d->pInc = pValue<T>(builder.buildInode(prop));
                d->evalInc = &ObjectPrivate<T>::incByPInc;
            }
        }

        d->representation = helper.getProperty(Properties::Representation);
        d->unit = helper.getProperty(Properties::Unit);
    }

private:
    inline T stringCast(const QString &, bool *) const;

}; // class Object

template<> inline
qint64 Object<qint64>::stringCast(const QString &number, bool *ok) const
{
    return number.toLongLong(ok, 0);
}

template<> inline
double Object<double>::stringCast(const QString &number, bool *ok) const
{
    return number.toDouble(ok);
}

} // namespace InodeValue

} // namespace GenICam

} // namespace Jgv

#endif // INODEVALUE_H
