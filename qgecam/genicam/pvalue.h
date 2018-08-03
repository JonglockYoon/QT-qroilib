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

#ifndef PVALUE_H
#define PVALUE_H


#include "inode.h"
#include "iinteger.h"
#include "ifloat.h"
#include "ienumeration.h"
#include "iboolean.h"
#include "enumentry.h"

#include <QWeakPointer>
#include <limits>

namespace Jgv {

namespace GenICam {

namespace Inode {

class Object;

}

template <typename T>
class pValue : public QWeakPointer<Inode::Object>
{

public:
    explicit pValue()
        : QWeakPointer<Inode::Object>()
    {
        m_get = &pValue::fakeValue;
        m_set = &pValue::setFake;
        m_evalPValue = &pValue::fakePValue;
        m_setPValue = &pValue::setPFake;

        m_min = &pValue::fakePValue;
        m_max = &pValue::fakePValue;
        m_inc = &pValue::fakePValue;
    }

    explicit pValue(T fixedValue)
        : QWeakPointer<Inode::Object>(), m_fixedValue(fixedValue)
    {
        m_get = &pValue::valueByfixedValue;
        m_set = &pValue::setFixedValue;
    }

    explicit pValue(QWeakPointer<Inode::Object> pInodeObject)
        : QWeakPointer<Inode::Object>(pInodeObject)
    {
        m_get = &pValue::valueByPValue;
        m_set = &pValue::setPValue;

        QSharedPointer<Inode::Object> object = this->toStrongRef();
        if (object) {
            if (JGV_ITYPE(object) == Jgv::GenICam::Type::IInteger) {
                m_evalPValue = &pValue::iintegerValue;
                m_setPValue = &pValue::setIInteger;
                m_min = &pValue::iintegerMin;
                m_max = &pValue::iintegerMax;
                m_inc = &pValue::iintegerInc;
            }
            else if (JGV_ITYPE(object) == Jgv::GenICam::Type::IEnumeration) {
                m_evalPValue = &pValue::ienumerationValue;
                m_setPValue = &pValue::setIEnumeration;

            }
            else if (JGV_ITYPE(object) == Jgv::GenICam::Type::IFloat) {
                m_evalPValue = &pValue::ifloatValue;
                m_setPValue = &pValue::setIFloat;
                m_min = &pValue::ifloatMin;
                m_max = &pValue::ifloatMax;
                m_inc = &pValue::ifloatInc;
            }
            else if (JGV_ITYPE(object) == Jgv::GenICam::Type::IBoolean) {
                m_evalPValue = &pValue::ibooleanValue;
                m_setPValue = &pValue::setIBoolean;
            }
        }
        else {
            qWarning("pValue: construction width invalid weak pointer");
        }
    }

    T value() const
    {
        return (this->* m_get)();
    }

    void set(T value)
    {
        (this->* m_set) (value);
    }

    T min() const
    {
        QSharedPointer<Inode::Object> object = this->toStrongRef();
        if (object) {
            return (this->*m_min)(object.data());
        }

        qWarning("pValue min(): weak pointer is not valid");
        return std::numeric_limits<T>::lowest();
    }

    T max() const
    {
        QSharedPointer<Inode::Object> object = this->toStrongRef();
        if (object) {
            return (this->*m_max)(object.data());
        }

        qWarning("pValue max(): weak pointer is not valid");
        return std::numeric_limits<T>::max();
    }

    T inc() const
    {
        QSharedPointer<Inode::Object> object = this->toStrongRef();
        if (object) {
            return (this->*m_inc)(object.data());
        }

        qWarning("pValue inc(): weak pointer is not valid");
        return -1;
    }

private:
    using getVal = T (pValue::*)() const;
    using setVal = void (pValue::*) (T value);

    using evaluate = T (pValue::*)(Inode::Object *object) const;
    using setEvaluation = void (pValue::*)(Inode::Object *object, T value);

    T m_fixedValue;

    getVal m_get;// = &pValue::fakeValue;
    setVal m_set;// = &pValue::setFake;
    evaluate m_evalPValue;// = &pValue::fakePValue;
    setEvaluation m_setPValue;// = &pValue::setPFake;

    evaluate m_min;// = &pValue::fakePValue;
    evaluate m_max;// = &pValue::fakePValue;
    evaluate m_inc;// = &pValue::fakePValue;

    T fakeValue() const
    {
        QSharedPointer<Inode::Object> object = this->toStrongRef();
        if (object) {
            return fakePValue(object.data());
        }
        qWarning("pValue value(): weak pointer is not valid");
        return 0;
    }

    T fakePValue(Inode::Object *object) const
    {
        qWarning("pValue %s: wrong interface %s", qPrintable(object->featureName()), qPrintable(object->interface()->interfaceTypeString()));
        return 0;
    }

    T valueByfixedValue() const
    {
        return m_fixedValue;
    }

    T valueByPValue() const
    {
        QSharedPointer<Inode::Object> object = this->toStrongRef();
        if (object) {
            return (this->*m_evalPValue)(object.data());
        }

        qWarning("pValue value(): weak pointer is not valid");
        return -1;
    }

    T iintegerValue(Inode::Object *object) const
    {
        return JGV_IINTEGER(object)->getValue();
    }

    T ienumerationValue(Inode::Object *object) const
    {
        return JGV_IENUMERATION(object)->getIntValue();
    }

    T ifloatValue(Inode::Object *object) const
    {
        return JGV_IFLOAT(object)->getValue();
    }

    T ibooleanValue(Inode::Object *object) const
    {
        return JGV_IBOOLEAN(object)->getValue();
    }

    T iintegerMin(Inode::Object *object) const
    {
        return JGV_IINTEGER(object)->getMin();
    }

    T ifloatMin(Inode::Object *object) const
    {
        return JGV_IFLOAT(object)->getMin();
    }

    T iintegerMax(Inode::Object *object) const
    {
        return JGV_IINTEGER(object)->getMax();
    }

    T ifloatMax(Inode::Object *object) const
    {
        return JGV_IFLOAT(object)->getMax();
    }

    T iintegerInc(Inode::Object *object) const
    {
        return JGV_IINTEGER(object)->getInc();
    }

    T ifloatInc(Inode::Object *object) const
    {
        return JGV_IFLOAT(object)->getInc();
    }

    void setFixedValue(T value)
    {
        m_fixedValue = value;
    }

    void setPValue(T value)
    {
        QSharedPointer<Inode::Object> object = this->toStrongRef();
        if (object) {
            (this->* m_setPValue)(object.data(), value);
        }
        else {
            qWarning("pValue set: weak pointer is not valid");
        }
    }

    void setFake(T value)
    {
        Q_UNUSED(value)
        qWarning("pValue setFake !");
    }

    void setPFake(Inode::Object *object, T value)
    {
        Q_UNUSED(object)
        Q_UNUSED(value)
        qWarning("pValue setFake !");
    }

    void setIInteger(Inode::Object *object, T value)
    {
        JGV_IINTEGER(object)->setValue(value);
    }

    void setIEnumeration(Inode::Object *object, T value)
    {
        JGV_IENUMERATION(object)->setIntValue(value);
    }

    void setIFloat(Inode::Object *object, T value)
    {
        JGV_IFLOAT(object)->setValue(value);
    }

    void setIBoolean(Inode::Object *object, T value)
    {
        JGV_IBOOLEAN(object)->setValue(value);
    }

}; // class pValue

using pValueInteger = pValue<qint64>;
using pValueFloat = pValue<double>;

} // namespace GenICam

} // namespace Jgv

#endif // PVALUE_H
