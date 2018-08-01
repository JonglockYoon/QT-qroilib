/*
 * roiobject.cpp
 * Copyright 2008-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2017, Klimov Viktor <vitek.fomino@bk.ru>
 *Qroilib : QT ROI Lib
 * Copyright 2018 Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
 *
 * This file is part of qroilib.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QFontMetricsF>
#include <qmath.h>
#include <QTime>
#include <QDebug>

#include "roiobject.h"
#include "roimap.h"
#include "objectgroup.h"
//#include "recipedata.h"

namespace Qroilib {


QString GetTimeString(void)
{
    QTime NowTime = QTime::currentTime();
    static QString sTime;

    sTime.sprintf(("%02d%02d%02d"),NowTime.hour(),NowTime.minute(),NowTime.second());

    return sTime;
}

//날짜문자열을 세글자의 Alphabet으로 변환한다.
QString GetDateStringAlpha(void)
{
    QDate NowTime = QDate::currentDate();
    QString sDate;

    sDate.sprintf(("%c%c%c"), NowTime.year() % 26 + 'a', (NowTime.month()-1) % 26 + 'a', NowTime.day() % 26 + 'a');

    return sDate;
}


TextData::TextData()
    : font(QStringLiteral("sans-serif"))
{
    font.setPixelSize(16);
}

int TextData::flags() const
{
    return wordWrap ? (alignment | Qt::TextWordWrap) : alignment;
}

QTextOption TextData::textOption() const
{
    QTextOption option(alignment);

    if (wordWrap)
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    else
        option.setWrapMode(QTextOption::ManualWrap);

    return option;
}

/**
 * Returns the size of the text when drawn without wrapping.
 */
QSizeF TextData::textSize() const
{
    QFontMetricsF fontMetrics(font);
    return fontMetrics.size(0, text);
}


RoiObject::RoiObject():
    Object(RoiObjectType),
    mId(0),
    mSize(0, 0),
    mShape(Rectangle),
    mParent(nullptr),
    mPattern(nullptr),
    mObjectGroup(nullptr),
    mRotation(0.0f),
    mVisible(true)
{
    mInspectType = 0; // InspectType
    iplTemplate = nullptr;
}

RoiObject::RoiObject(const QString &name, const QString &type,
                     const QPointF &pos,
                     const QSizeF &size):
    Object(RoiObjectType),
    mId(0),
    mName(name),
    mType(type),
    mPos(pos),
    mSize(size),
    mShape(Rectangle),
    mObjectGroup(nullptr),
    mParent(nullptr),
    mPattern(nullptr),
    mRotation(0.0f),
    mVisible(true)
{
    iplTemplate = nullptr;
}

/**
 * Returns the affective type of this object. This may be the type of its tile,
 * if the object does not have a type set explicitly.
 */
const QString &RoiObject::effectiveType() const
{
    return mType;
}

/**
 * Sets the text data associated with this object.
 */
void RoiObject::setTextData(const TextData &textData)
{
    mTextData = textData;
}

Alignment RoiObject::alignment() const
{
    return TopLeft;
}

QVariant RoiObject::roiObjectProperty(Property property) const
{
    switch (property) {
    case NameProperty:          return mName;
    case TypeProperty:          return mType;
    case VisibleProperty:       return mVisible;
    case TextProperty:          return mTextData.text;
    case TextFontProperty:      return mTextData.font;
    case TextAlignmentProperty: return QVariant::fromValue(mTextData.alignment);
    case TextWordWrapProperty:  return mTextData.wordWrap;
    case TextColorProperty:     return mTextData.color;
    }
    return QVariant();
}

void RoiObject::setRoiObjectProperty(Property property, const QVariant &value)
{
    switch (property) {
    case NameProperty:          mName = value.toString(); break;
    case TypeProperty:          mType = value.toString(); break;
    case VisibleProperty:       mVisible = value.toBool(); break;
    case TextProperty:          mTextData.text = value.toString(); break;
    case TextFontProperty:
        mTextData.font = value.value<QFont>();
        break;
    case TextAlignmentProperty: mTextData.alignment = value.value<Qt::Alignment>(); break;
    case TextWordWrapProperty:  mTextData.wordWrap = value.toBool(); break;
    case TextColorProperty:     mTextData.color = value.value<QColor>(); break;
    }
}

/**
 * Flip this object in the given \a direction. This doesn't change the size
 * of the object.
 */
void RoiObject::flip(FlipDirection direction, const QPointF &origin)
{
    //computing new rotation and flip transform
    QTransform flipTransform;
    flipTransform.translate(origin.x(), origin.y());
    qreal newRotation = 0;
    if (direction == FlipHorizontally) {
        newRotation = 180.0 - rotation();
        flipTransform.scale(-1, 1);
    } else { //direction == FlipVertically
        flipTransform.scale(1, -1);
        newRotation = -rotation();
    }
    flipTransform.translate(-origin.x(), -origin.y());


    if (!mPolygon.isEmpty())
        flipPolygonObject(flipTransform);
    else
        flipRectObject(flipTransform);

    //installing new rotation after computing new position
    setRotation(newRotation);
}

/**
 * Returns a duplicate of this object. The caller is responsible for the
 * ownership of this newly created object.
 */
RoiObject *RoiObject::clone() const
{
    RoiObject *o = new RoiObject(mName, mType, mPos, mSize);
    o->setId(mId);
    o->setProperties(properties());
    o->setTextData(mTextData);
    o->setPolygon(mPolygon);
    o->setShape(mShape);
    o->setRotation(mRotation);
    o->setVisible(mVisible);
    o->setPattern(mPattern);
    o->setParent(mParent);
    o->setInspectType(mInspectType);
    o->setIplTemplate(iplTemplate);
    o->setVecParams(m_vecParams);
    o->setDetectResult(m_vecDetectResult);

    return o;
}

void RoiObject::flipRectObject(const QTransform &flipTransform)
{
    QPointF oldBottomLeftPoint = QPointF(cos(qDegreesToRadians(rotation() + 90)) * height() + x(),
                                         sin(qDegreesToRadians(rotation() + 90)) * height() + y());
    QPointF newPos = flipTransform.map(oldBottomLeftPoint);

    setPosition(newPos);
}

void RoiObject::flipPolygonObject(const QTransform &flipTransform)
{
    QTransform polygonToMapTransform;
    polygonToMapTransform.translate(x(), y());
    polygonToMapTransform.rotate(rotation());

    QPointF localPolygonCenter = mPolygon.boundingRect().center();
    QTransform polygonFlip;
    polygonFlip.translate(localPolygonCenter.x(), localPolygonCenter.y());
    polygonFlip.scale(1, -1);
    polygonFlip.translate(-localPolygonCenter.x(), -localPolygonCenter.y());

    QPointF oldBottomLeftPoint = polygonToMapTransform.map(polygonFlip.map(QPointF(0, 0)));
    QPointF newPos = flipTransform.map(oldBottomLeftPoint);

    mPolygon = polygonFlip.map(mPolygon);
    setPosition(newPos);
}

void RoiObject::flipTileObject(const QTransform &flipTransform)
{
    //old tile position is bottomLeftPoint
    QPointF topLeftTilePoint = QPointF(cos(qDegreesToRadians(rotation() - 90)) * height() + x(),
                                       sin(qDegreesToRadians(rotation() - 90)) * height() + y());
    QPointF newPos = flipTransform.map(topLeftTilePoint);

    setPosition(newPos);
}



int RoiObject::AddReplaceParam(CParam param)
{
    QString str;

    int size = m_vecParams.size();
    for (int i = 0; i < size; i++)
    {
        if (param.stepType == m_vecParams[i].param.stepType && param.Name == m_vecParams[i].param.Name)
        {
            //m_vecParams[i] = param;
            m_vecParams[i].param.Value = param.Value;
            //qDebug() << "Param" << m_vecParams[i].param.Value << param.Value;
            return 0;
        }
    }

    return 0;
}

CParam* RoiObject::getParam(QString sName, int t)
{
    int size = m_vecParams.size();
    for (int i = 0; i < size; i++)
    {
        if (sName == m_vecParams[i].param.Name)
        {
            if (t >= 0) {
                if (m_vecParams[i].param.stepType == t)
                    return &m_vecParams[i].param;
            }
            else return &m_vecParams[i].param;
        }
    }
    return NULL;
}

int RoiObject::getIntParam(QString sName, int &nParam)
{
    CParam *pParam = NULL;
    int size = m_vecParams.size();
    for (int i = 0; i < size; i++)
    {
        if (sName == m_vecParams[i].param.Name)
        {
            pParam = &m_vecParams[i].param;
            nParam = pParam->Value.toDouble();
            return 0;
        }
    }

    return -1;
}

} // namespace Qroilib
