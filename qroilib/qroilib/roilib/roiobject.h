/*
 * roiobject.h
 * Copyright 2008-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
 * Copyright 2018, Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
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

#pragma once

#include <vector>
#include "object.h"
#include "objectgroup.h"
#include "roid.h"
#include "qpatternroif.h"
#include <opencv2/core/core.hpp>
#include "properties.h"

#include <QColor>
#include <QString>
#include <QFont>
#include <QPolygonF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QTextOption>

namespace Qroilib {

#define _INSPECT_PATT_START 10
#define _INSPACT_ROI_START 500
#define _INSPACT_POINT_START 1000
#define _INSPACT_TYPE_END 1100

class ObjectGroup;

enum  ValueType{
    _BoolValue,
    _IntValue,
    _DoubleValue,
    _StringValue,
    _ComboValue,
};


#define RESULTTYPE_POINT 0x0001
#define RESULTTYPE_RECT  0x0002
#define RESULTTYPE_RECT4P  0x0004
#define RESULTTYPE_POLYGON  0x0008
#define RESULTTYPE_RADIUS  0x0010
#define RESULTTYPE_ANGLE  0x0020
#define RESULTTYPE_MATCHRATE  0x0040
#define RESULTTYPE_STRING  0x0080
#define RESULTTYPE_BOOL  0x0100
#define RESULTTYPE_IMAGE  0x0200
#define RESULTTYPE_LENGTH  0x0400
#define RESULTTYPE_WIDTH  0x0800
#define RESULTTYPE_HEIGHT  0x1000

typedef struct  _tagDetectResult
{
    int resultType; // point, rect, rect4p, ...

    CvPoint2D32f pt; // point
    CvPoint2D32f tl; // rect, rect4p
    CvPoint2D32f tr; // rect4p
    CvPoint2D32f bl; // rect4p
    CvPoint2D32f br; // rect, rect4p
    bool	result; // bool
    //std::vector<CvPoint2D32f> vpolygon;

    double dRadius;
    double dAngle;
    double dMatchRate;
    double dValue;
    double dWidth;
    double dHeight;
    char str[128];
    //std::string str;
    //IplImage* img;

} DetectResult;


class ROIDSHARED_EXPORT CParam
{
public:
    int		stepType;	// 마스크, 전처리, Process, 후처리,...
    QString	Name;
    enum ValueType		valueType;	// _BoolValue, ...
    QString	Value;
    QString	Detail; // Combo일경우 Combo 항목들을 콤마(Comma)로 구분해서 넣어둔다.
    std::vector<QString> m_vecDetail;

public:
    CParam(void){};
    ~CParam(void){};
    CParam(int nStep, QString sName, enum ValueType nType, QString sVal, QString sDetail = (""))
    {
        stepType = nStep;
        Name = sName.toLatin1();//.data();
        valueType = nType;
        Value = sVal;
        Detail = sDetail;
    };


};

typedef struct ROIDSHARED_EXPORT _tagParamTable {
    int nInspectType;
    CParam param;
} ParamTable;


struct ROIDSHARED_EXPORT TextData
{
    enum FontAttributes {
        FontFamily  = 0x1,
        FontSize    = 0x2,
        FontStyle   = 0x8
    };

    TextData();

    QString text;
    QFont font;
    QColor color = Qt::black;
    Qt::Alignment alignment = Qt::AlignTop | Qt::AlignLeft;
    bool wordWrap = true;

    int flags() const;
    QTextOption textOption() const;
    QSizeF textSize() const;
};

/**
 * An object on a roimap. Objects are positioned and scaled using floating point
 * values, ensuring they are not limited to the tile grid. They are suitable
 * for adding any kind of annotation to your maps, as well as free placement of
 * images.
 *
 * Common usages of objects include defining portals, monsters spawn areas,
 * ambient effects, scripted areas, etc.
 */
class ROIDSHARED_EXPORT RoiObject : public Object
{
public:
    RoiObject& operator=(const RoiObject &src );

    /**
     * Enumerates the different object shapes. Rectangle is the default shape.
     * When a polygon is set, the shape determines whether it should be
     * interpreted as a filled polygon or a line.
     *
     * Text objects contain arbitrary text, contained withih their rectangle
     * (in screen coordinates).
     */
    enum Shape {
        Rectangle,
        Polygon,
        Polyline,
        Ellipse,
        Text,
        Pattern,
        Point
    };

    /**
     * Can be used to get/set property values using QVariant.
     */
    enum Property {
        NameProperty,
        TypeProperty,
        VisibleProperty,
        TextProperty,
        TextFontProperty,
        TextAlignmentProperty,
        TextWordWrapProperty,
        TextColorProperty
    };

    RoiObject();

    RoiObject(const QString &name, const QString &type,
              const QPointF &pos,
              const QSizeF &size);

    int id() const;
    void setId(int id);
    void resetId();

    const QString &name() const;
    void setName(const QString &name);

    const QString &type() const;
    void setType(const QString &type);

    const QString &effectiveType() const;

    const QPointF &position() const;
    void setPosition(const QPointF &pos);
    const QPointF &oldPosition() const;
    void setOldPosition(const QPointF &pos);

    qreal x() const;
    void setX(qreal x);

    qreal y() const;
    void setY(qreal y);

    const QSizeF &size() const;
    void setSize(const QSizeF &size);
    void setSize(qreal width, qreal height);

    qreal width() const;
    void setWidth(qreal width);

    qreal height() const;
    void setHeight(qreal height);

    void setBounds(const QRectF &bounds);

    const TextData &textData() const;
    void setTextData(const TextData &textData);

    const QPolygonF &polygon() const;
    void setPolygon(const QPolygonF &polygon);

    const QRectF &patternRoi() const;
    void setPatternRoi(const QRectF &r);

    Shape shape() const;
    void setShape(Shape shape);

    bool isPolyShape() const;

    QRectF bounds() const;

    ObjectGroup *objectGroup() const;
    void setObjectGroup(ObjectGroup *objectGroup);
    QString groupName()
    {
        if (objectGroup())
            return objectGroup()->name();
        else
            return "";
    }

    qreal rotation() const;
    void setRotation(qreal rotation);

    Alignment alignment() const;

    bool isVisible() const;
    void setVisible(bool visible);

    QVariant roiObjectProperty(Property property) const;
    void setRoiObjectProperty(Property property, const QVariant &value);

    void flip(FlipDirection direction, const QPointF &origin);

    RoiObject *clone() const;

    void setPattern(RoiObject *pattern);
    void setParent(RoiObject *parent);
    void setInspectType(int inspectType); // InspectType
    void setIplTemplate(IplImage *templateimage);
    void setVecParams(std::vector<ParamTable> vecParams);
    void setDetectResult(std::vector<DetectResult> vecDetectResult);

public:
    RoiObject *mParent;
    RoiObject *mPattern;
    ObjectGroup *mObjectGroup;

private:
    void flipRectObject(const QTransform &flipTransform);
    void flipPolygonObject(const QTransform &flipTransform);
    void flipTileObject(const QTransform &flipTransform);

    int mId;
    QString mName;
    QString mType;
    QPointF mPos;
    QPointF mOldPos;
    QSizeF mSize;
    TextData mTextData;
    QPolygonF mPolygon;
    Shape mShape;
    qreal mRotation;
    bool mVisible;

public:
    //QPatternRoiF mPatternRoi;
    //bool bActive;

    //int m_nCh;	// ROI가 속해있는 Channel - 보통 카메라로 구분된다.
    //int m_nObjectType; // 패턴, 영역, 포인터 구분; ObjectTypeList 참조
    //QString m_sName; // ROI이름
    int mInspectType; // Inspection Type, RecipeData.h의 InspectType 참조
    //QPointF m_dCenterPos;	// Point 중심점 또는 Rect의 중심점
    //QRectF	m_RoiArea; // ROI의 전체 영역을 나타낸다.

    //QRectF ptnRoi;
    IplImage*	iplTemplate;

    std::vector<ParamTable> m_vecParams;	// stepType, Name, valueType, Value, Detail 를 담은 vector


public:
    std::vector<DetectResult>	m_vecDetectResult;
    bool	m_bVisionError;			// _Inspect_Roi_Pin_ErrorCheck를 위해 추가

    int AddReplaceParam(CParam param);
    CParam* getParam(QString sName, int t = -1);

    int getIntParam(QString sName, int &nParam);

};

inline RoiObject& RoiObject::operator=(const RoiObject &src )
{
    if( this != &src )
    {
        mId = src.mId;
        mName = src.mName;
        mType = src.mType;
        mPos = src.mPos;
        mOldPos = src.mOldPos;
        mSize = src.mSize;
        mTextData = src.mTextData;
        mPolygon = src.mPolygon;
        mShape = src.mShape;
        mObjectGroup = src.mObjectGroup;
        mRotation = src.mRotation;
        mVisible = src.mVisible;
        mInspectType = src.mInspectType;
        iplTemplate = src.iplTemplate;
        m_bVisionError = src.m_bVisionError;
        m_vecParams = src.m_vecParams;
        m_vecDetectResult = src.m_vecDetectResult;

        mParent = src.mParent;
        mPattern = src.mPattern;
    }

    return *this;
}

/**
 * Returns the id of this object. Each object gets an id assigned that is
 * unique for the roimap the object is on.
 */
inline int RoiObject::id() const
{ return mId; }

/**
 * Sets the id of this object.
 */
inline void RoiObject::setId(int id)
{ mId = id; }

/**
 * Sets the id back to 0. Mostly used when a new id should be assigned
 * after the object has been cloned.
 */
inline void RoiObject::resetId()
{ setId(0); }

/**
 * Returns the name of this object. The name is usually just used for
 * identification of the object in the editor.
 */
inline const QString &RoiObject::name() const
{ return mName; }

/**
 * Sets the name of this object.
 */
inline void RoiObject::setName(const QString &name)
{ mName = name; }

/**
 * Returns the type of this object. The type usually says something about
 * how the object is meant to be interpreted by the engine.
 */
inline const QString &RoiObject::type() const
{ return mType; }

/**
 * Sets the type of this object.
 */
inline void RoiObject::setType(const QString &type)
{ mType = type; }

/**
 * Returns the position of this object.
 */
inline const QPointF &RoiObject::position() const
{ return mPos; }

/**
 * Sets the position of this object.
 */
inline void RoiObject::setPosition(const QPointF &pos)
{ mPos = pos; }

inline const QPointF &RoiObject::oldPosition() const
{ return mOldPos; }
inline void RoiObject::setOldPosition(const QPointF &pos)
{ mOldPos = pos; }

/**
 * Returns the x position of this object.
 */
inline qreal RoiObject::x() const
{ return mPos.x(); }

/**
 * Sets the x position of this object.
 */
inline void RoiObject::setX(qreal x)
{ mPos.setX(x); }

/**
 * Returns the y position of this object.
 */
inline qreal RoiObject::y() const
{ return mPos.y(); }

/**
 * Sets the x position of this object.
 */
inline void RoiObject::setY(qreal y)
{ mPos.setY(y); }

/**
 * Returns the size of this object.
 */
inline const QSizeF &RoiObject::size() const
{ return mSize; }

/**
 * Sets the size of this object.
 */
inline void RoiObject::setSize(const QSizeF &size)
{ mSize = size; }

inline void RoiObject::setSize(qreal width, qreal height)
{ setSize(QSizeF(width, height)); }

/**
 * Returns the width of this object.
 */
inline qreal RoiObject::width() const
{ return mSize.width(); }

/**
 * Sets the width of this object.
 */
inline void RoiObject::setWidth(qreal width)
{ mSize.setWidth(width); }

/**
 * Returns the height of this object.
 */
inline qreal RoiObject::height() const
{ return mSize.height(); }

/**
 * Sets the height of this object.
 */
inline void RoiObject::setHeight(qreal height)
{ mSize.setHeight(height); }

/**
 * Sets the position and size of this object.
 */
inline void RoiObject::setBounds(const QRectF &bounds)
{
    mPos = bounds.topLeft();
    mSize = bounds.size();
}

/**
 * Returns the text associated with this object, when it is a text object.
 */
inline const TextData &RoiObject::textData() const
{ return mTextData; }

/**
 * Returns the polygon associated with this object. Returns an empty
 * polygon when no polygon is associated with this object.
 */
inline const QPolygonF &RoiObject::polygon() const
{ return mPolygon; }

/**
 * Sets the polygon associated with this object. The polygon is only used
 * when the object shape is set to either Polygon or Polyline.
 *
 * \sa setShape()
 */
inline void RoiObject::setPolygon(const QPolygonF &polygon)
{ mPolygon = polygon; }

/**
 *
 */
//inline void RoiObject::setPatternRoi(const QRectF &r)
//{ mPatternRect.setRect(r); }

//inline const QRectF &RoiObject::patternRoi() const
//{ return mPatternRect.getRect(); }


/**
 * Returns the shape of the object.
 */
inline RoiObject::Shape RoiObject::shape() const
{ return mShape; }

/**
 * Sets the shape of the object.
 */
inline void RoiObject::setShape(RoiObject::Shape shape)
{ mShape = shape; }

/**
 * Returns true if this is a Polygon or a Polyline.
 */
inline bool RoiObject::isPolyShape() const
{ return mShape == Polygon || mShape == Polyline; }

/**
 * Shortcut to getting a QRectF from position() and size().
 */
inline QRectF RoiObject::bounds() const
{ return QRectF(mPos, mSize); }

/**
 * Returns the object group this object belongs to.
 */
inline ObjectGroup *RoiObject::objectGroup() const
{ return mObjectGroup; }

/**
 * Sets the object group this object belongs to. Should only be called
 * from the ObjectGroup class.
 */
inline void RoiObject::setObjectGroup(ObjectGroup *objectGroup)
{ mObjectGroup = objectGroup; }

/**
 * Returns the rotation of the object in degrees clockwise.
 */
inline qreal RoiObject::rotation() const
{ return mRotation; }

/**
 * Sets the rotation of the object in degrees clockwise.
 */
inline void RoiObject::setRotation(qreal rotation)
{ mRotation = rotation; }

inline bool RoiObject::isVisible() const
{ return mVisible; }

inline void RoiObject::setVisible(bool visible)
{ mVisible = visible; }

inline void RoiObject::setPattern(RoiObject *pattern)
{ mPattern = pattern; }

inline void RoiObject::setParent(RoiObject *parent)
{ mParent = parent; }

inline void RoiObject::setInspectType(int inspectType) // InspectType
{ mInspectType = inspectType; }

inline void RoiObject::setIplTemplate(IplImage *templateimage)
{ iplTemplate = templateimage; }

inline void RoiObject::setVecParams(std::vector<ParamTable> vecParams)
{ m_vecParams = vecParams; }

inline void RoiObject::setDetectResult(std::vector<DetectResult> vecDetectResult)
{ m_vecDetectResult = vecDetectResult; }

} // namespace Qroilib

#if QT_VERSION < 0x050500
Q_DECLARE_METATYPE(Qt::Alignment)
#endif
