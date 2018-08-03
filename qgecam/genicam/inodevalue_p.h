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

#ifndef INODEVALUE_P_H
#define INODEVALUE_P_H

#include "inode_p.h"
#include "pvalue.h"

#include <QList>
#include <QMap>

namespace Jgv {

namespace GenICam {

namespace InodeValue {

namespace Properties {
static const char * Value = "Value";
static const char * Min = "Min";
static const char * Max = "Max";
static const char * Inc = "Inc";
static const char * ValueIndexed = "ValueIndexed";
static const char * pValue = "pValue";
static const char * pMin = "pMin";
static const char * pMax = "pMax";
static const char * pInc = "pInc";
static const char * pIndex = "pIndex";
static const char * pValueIndexed = "pValueIndexed";
static const char * Representation = "Representation";
static const char * ValueDefault = "ValueDefault";
static const char * pValueDefault = "pValueDefault";
static const char * pValueCopy = "pValueCopy";
static const char * Unit = "Unit";
}

namespace Attributes {
static const char * Index = "Index";
}


template <typename T>
class ObjectPrivate : public Inode::ObjectPrivate
{
    using get = T (ObjectPrivate::*) () const;
    using set = void (ObjectPrivate::*) (T);

public:
    ObjectPrivate(QSharedPointer<IPort::Interface> iport)
        : Inode::ObjectPrivate(iport)
    {
        evalValue = &ObjectPrivate<T>::fakeValue;
        evalMin = &ObjectPrivate<T>::fakeValue;
        evalMax = &ObjectPrivate<T>::fakeValue;
        evalInc = &ObjectPrivate<T>::fakeValue;

        setValue = &ObjectPrivate::setFake;

    }

    pValue<T> pVal { 0 };
    pValue<T> pMin { std::numeric_limits<T>::lowest() };
    pValue<T> pMax { std::numeric_limits<T>::max() };
    pValue<T> pInc { 0 };
    pValue<T> pValueDefault { 0 };
    pValueInteger pIndex;


    QMap<qint64, T> indexes;
    QMap<qint64, pValue<T> > pIndexes;
    QList<pValue<T> > pValueCopies;

    QString representation;
    QString unit;

    get evalValue;// = &ObjectPrivate<T>::fakeValue;
    get evalMin;// = &ObjectPrivate<T>::fakeValue;
    get evalMax;// = &ObjectPrivate<T>::fakeValue;
    get evalInc;// = &ObjectPrivate<T>::fakeValue;

    set setValue;// = &ObjectPrivate::setFake;

    T fakeValue() const
    {
        qWarning("InodeValue %s: Fake Value", qPrintable(featureName));
        return -1;
    }

    T valueByPValue() const
    {
        return pVal.value();
    }

    T valueByIndex() const
    {
        const qint64 index = pIndex.value();
        if (indexes.contains(index)) {
            return indexes[index];
        }
        else if (pIndexes.contains(index)) {
            return pIndexes[index].value();
        }
        else {
            // on retourne la valeur par defaut
            return pValueDefault.value();
        }
    }

    T minByPMin() const
    {
        return pMin.value();
    }

    T minByPValue() const
    {
        return pVal.min();
    }

    T maxByPMax() const
    {
        return pMax.value();
    }

    T maxByPValue() const
    {
        return pVal.max();
    }

    T incByPInc() const
    {
        return pInc.value();
    }

    T incByPValue() const
    {
        return pVal.inc();
    }

    void setFake(T)
    {
        qWarning("InodeValue %s: set Fake Value", qPrintable(featureName));
    }


    void setByPValue(T value)
    {

        pVal.set(value);

        // met à jour les copies
        if (!pValueCopies.isEmpty()) {
            typename QList< pValue<T> >::iterator it = pValueCopies.begin();
            for (; it != pValueCopies.end(); ++it) {
                (*it).set(value);
            }
        }
    }

    void setByIndex(T value)
    {
        {
            auto it = qFind(indexes.constBegin(), indexes.constEnd(), value);
            if (it != indexes.constEnd()) {
                pIndex.set(it.key());
                return;
            }
        }
        {
            auto it = pIndexes.constBegin();
            for (; it!=pIndexes.constEnd(); ++it) {
                if (it.value().value() == value) {
                    pIndex.set(it.key());
                    return;
                }
            }
        }
        qWarning("InodeValue: %s failed to set value by Index (not found) %lld", qPrintable(featureName), value);
    }

}; // class ObjectPrivate

} // namespace InodeValue

} // namespace GenICam

} // namespace Jgv



#endif // INODEVALUE_P_H
