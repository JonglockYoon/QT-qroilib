/*
 * roireader.cpp
 * Copyright 2008-2014, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Dennis Honeyman <arcticuno@gmail.com>
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

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QVector>
#include <QXmlStreamReader>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>

#include "roireader.h"
//#include "common.h"
#include "compression.h"
#include "grouplayer.h"
#include "objectgroup.h"
#include "roimap.h"
#include "roiobject.h"
//#include "config.h"

using namespace std;
//using namespace cv;
using namespace Qroilib;

namespace Qroilib {

class RoiReaderPrivate
{
    Q_DECLARE_TR_FUNCTIONS(RoiReader)

    friend class Qroilib::RoiReader;

public:
    RoiReaderPrivate(RoiReader *mapReader):
        q(mapReader),
        mReadingExternalTileset(false)
    {}

    RoiMap *readRoi(QIODevice *device, const QString &path);

    bool openFile(QFile *file);

    QString errorString() const;

private:
    void readUnknownElement();

    RoiMap *readRoi();

    //ImageReference readImage();
    ParamTable *pParamTable;

    Layer *tryReadLayer();

    ObjectGroup *readObjectGroup();
    RoiObject *readObject();
    QPolygonF readPolygon();
    TextData readObjectText();

    GroupLayer *readGroupLayer();

    Properties readProperties();
    void readProperty(Properties *properties);

    RoiReader *q;

    QString mError;
    QString mPath;
    QScopedPointer<RoiMap> mRoi;
    bool mReadingExternalTileset;

    QXmlStreamReader xml;

    ObjectGroup *objectGroup;
};

RoiMap *RoiReaderPrivate::readRoi(QIODevice *device, const QString &path)
{
    mError.clear();
    mPath = path;
    RoiMap *roimap = nullptr;

    xml.setDevice(device);

    if (xml.readNextStartElement() && xml.name() == QLatin1String("roimap")) {
        roimap = readRoi();
    } else {
        xml.raiseError(tr("Not a roimap file."));
    }

    //qDebug() << "readRoi";

    return roimap;
}

QString RoiReaderPrivate::errorString() const
{
    if (!mError.isEmpty()) {
        return mError;
    } else {
        return tr("%3\n\nLine %1, column %2")
                .arg(xml.lineNumber())
                .arg(xml.columnNumber())
                .arg(xml.errorString());
    }
}

bool RoiReaderPrivate::openFile(QFile *file)
{
    if (!file->exists()) {
        mError = tr("File not found: %1").arg(file->fileName());
        return false;
    } else if (!file->open(QFile::ReadOnly | QFile::Text)) {
        mError = tr("Unable to read file: %1").arg(file->fileName());
        return false;
    }

    return true;
}

void RoiReaderPrivate::readUnknownElement()
{
    qDebug().nospace() << "Unknown element (fixme): " << xml.name()
                       << " at line " << xml.lineNumber()
                       << ", column " << xml.columnNumber();
    xml.skipCurrentElement();
}

RoiMap *RoiReaderPrivate::readRoi()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("roimap"));

    const QXmlStreamAttributes atts = xml.attributes();
    const int mapWidth = atts.value(QLatin1String("width")).toInt();
    const int mapHeight = atts.value(QLatin1String("height")).toInt();
    const int roiWidth = atts.value(QLatin1String("roiwidth")).toInt();
    const int roiHeight = atts.value(QLatin1String("roiheight")).toInt();
    const int hexSideLength = atts.value(QLatin1String("hexsidelength")).toInt();

    const QString orientationString =
            atts.value(QLatin1String("orientation")).toString();
    const RoiMap::Orientation orientation =
            orientationFromString(orientationString);

    if (orientation == RoiMap::Unknown) {
        xml.raiseError(tr("Unsupported roimap orientation: \"%1\"")
                       .arg(orientationString));
    }

    const QString staggerAxisString =
            atts.value(QLatin1String("staggeraxis")).toString();
    const RoiMap::StaggerAxis staggerAxis =
            staggerAxisFromString(staggerAxisString);

    const QString staggerIndexString =
            atts.value(QLatin1String("staggerindex")).toString();
    const RoiMap::StaggerIndex staggerIndex =
            staggerIndexFromString(staggerIndexString);

    const QString renderOrderString =
            atts.value(QLatin1String("renderorder")).toString();
    const RoiMap::RenderOrder renderOrder =
            renderOrderFromString(renderOrderString);

    const int nextObjectId =
            atts.value(QLatin1String("nextobjectid")).toInt();

    mRoi.reset(new RoiMap(orientation, mapWidth, mapHeight, roiWidth, roiHeight));
    mRoi->setHexSideLength(hexSideLength);
    mRoi->setStaggerAxis(staggerAxis);
    mRoi->setStaggerIndex(staggerIndex);
    mRoi->setRenderOrder(renderOrder);
    if (nextObjectId)
        mRoi->setNextObjectId(nextObjectId);

    QStringRef bgColorString = atts.value(QLatin1String("backgroundcolor"));
    if (!bgColorString.isEmpty())
        mRoi->setBackgroundColor(QColor(bgColorString.toString()));

    while (xml.readNextStartElement()) {
        if (Layer *layer = tryReadLayer())
            mRoi->addLayer(layer);
        else if (xml.name() == QLatin1String("properties"))
            mRoi->mergeProperties(readProperties());
        else
            readUnknownElement();
    }

    // Clean up in case of error
    if (xml.hasError()) {
        mRoi.reset();
    }

    return mRoi.take();
}

Layer *RoiReaderPrivate::tryReadLayer()
{
    Q_ASSERT(xml.isStartElement());

    if (xml.name() == QLatin1String("objectgroup"))
        return readObjectGroup();
    else if (xml.name() == QLatin1String("group"))
        return readGroupLayer();
    else
        return nullptr;
}

static void readLayerAttributes(Layer &layer,
                                const QXmlStreamAttributes &atts)
{
    const QStringRef opacityRef = atts.value(QLatin1String("opacity"));
    const QStringRef visibleRef = atts.value(QLatin1String("visible"));

    bool ok;
    const float opacity = opacityRef.toFloat(&ok);
    if (ok)
        layer.setOpacity(opacity);

    const int visible = visibleRef.toInt(&ok);
    if (ok)
        layer.setVisible(visible);

    const QPointF offset(atts.value(QLatin1String("offsetx")).toDouble(),
                         atts.value(QLatin1String("offsety")).toDouble());

    layer.setOffset(offset);
}

ObjectGroup *RoiReaderPrivate::readObjectGroup()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("objectgroup"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toInt();
    const int y = atts.value(QLatin1String("y")).toInt();


    objectGroup = new ObjectGroup(name, x, y);
    readLayerAttributes(*objectGroup, atts);

    const QString color = atts.value(QLatin1String("color")).toString();
    if (!color.isEmpty())
        objectGroup->setColor(color);

    if (atts.hasAttribute(QLatin1String("draworder"))) {
        QString value = atts.value(QLatin1String("draworder")).toString();
        ObjectGroup::DrawOrder drawOrder = drawOrderFromString(value);
        if (drawOrder == ObjectGroup::UnknownOrder) {
            delete objectGroup;
            xml.raiseError(tr("Invalid draw order: %1").arg(value));
            return nullptr;
        }
        objectGroup->setDrawOrder(drawOrder);
    }

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("object"))
            objectGroup->addObject(readObject());
        else if (xml.name() == QLatin1String("properties"))
            objectGroup->mergeProperties(readProperties());
        else
            readUnknownElement();
    }

    for (RoiObject* obj : objectGroup->objects()) {
        if (obj->shape() == RoiObject::Pattern) {
            const void *childid = static_cast<const void *>(obj->mPattern);
            const RoiObject *child = objectGroup->find((long)childid);
            obj->mPattern = (RoiObject *)child;
            //obj->mObjectGroup = objectGroup;

            if (obj->mPattern){
                if (obj->iplTemplate)
                {
                    cvReleaseImage(&obj->iplTemplate);
                    obj->iplTemplate = nullptr;
                }
                QString name = obj->name();
                if (name.isEmpty())
                    name = "noname";
                QString sub;
                sub = QString("%1/%2.bmp").arg(mPath).arg(name);
                //cv::Mat m = cv::imread(sub.toStdString(), 0);
                //obj->iplTemplate = new IplImage(m);
                obj->iplTemplate = cvLoadImage((const char*)sub.toStdString().c_str(), CV_LOAD_IMAGE_COLOR);
            }
        }
    }


    return objectGroup;
}

RoiObject *RoiReaderPrivate::readObject()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("object"));

    const QXmlStreamAttributes atts = xml.attributes();
    const int id = atts.value(QLatin1String("id")).toInt();
    const QString name = atts.value(QLatin1String("name")).toString();
    //const unsigned gid = atts.value(QLatin1String("gid")).toUInt();
    const qreal x = atts.value(QLatin1String("x")).toDouble();
    const qreal y = atts.value(QLatin1String("y")).toDouble();
    const qreal width = atts.value(QLatin1String("width")).toDouble();
    const qreal height = atts.value(QLatin1String("height")).toDouble();
    const QString type = atts.value(QLatin1String("type")).toString();
    const QStringRef visibleRef = atts.value(QLatin1String("visible"));

    const QPointF pos(x, y);
    const QSizeF size(width, height);

    RoiObject *object = new RoiObject(name, type, pos, size);
    object->setId(id);

    bool ok;
    const qreal rotation = atts.value(QLatin1String("rotation")).toDouble(&ok);
    if (ok)
        object->setRotation(rotation);

    const int visible = visibleRef.toInt(&ok);
    if (ok)
        object->setVisible(visible);

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("properties")) {
            object->mergeProperties(readProperties());
        } else if (xml.name() == QLatin1String("pattern")) {
            const QXmlStreamAttributes atts = xml.attributes();
            const int childid = atts.value(QLatin1String("child")).toInt();
            object->mPattern = (RoiObject *)childid; // readObjectGroup()에서 실address로 변환된다.
            //object->mObjectGroup = objectGroup;

            xml.skipCurrentElement();
            object->setShape(RoiObject::Pattern);
        } else if (xml.name() == QLatin1String("polygon")) {
            object->setPolygon(readPolygon());
            object->setShape(RoiObject::Polygon);
        } else if (xml.name() == QLatin1String("polyline")) {
            object->setPolygon(readPolygon());
            object->setShape(RoiObject::Polyline);
        } else if (xml.name() == QLatin1String("ellipse")) {
            xml.skipCurrentElement();
            object->setShape(RoiObject::Ellipse);
        } else if (xml.name() == QLatin1String("rectangle")) {
            const QXmlStreamAttributes atts = xml.attributes();
            const int parentid = atts.value(QLatin1String("parent")).toInt();
            const RoiObject *parent = objectGroup->find(parentid);
            if (parent != nullptr) {
                object->mParent = (RoiObject *)parent;
            }

            xml.skipCurrentElement();
            object->setShape(RoiObject::Rectangle);
        } else if (xml.name() == QLatin1String("text")) {
            object->setTextData(readObjectText());
            object->setShape(RoiObject::Text);
        } else if (xml.name() == QLatin1String("param")) {
            const QXmlStreamAttributes atts = xml.attributes();
            const int inspType = (int)atts.value(QLatin1String("InspectType")).toInt(); // InspectType
            CParam param;
            object->m_vecParams.clear();

            object->mInspectType = inspType;

            for (int n = 0;; n++)
            {
                if (pParamTable[n].nInspectType == _INSPACT_TYPE_END)
                    break;

                if (pParamTable[n].nInspectType == object->mInspectType)
                    object->m_vecParams.push_back(pParamTable[n]);
            }

            int seq = 0;
            QString sub;
            while (xml.readNextStartElement()) {
                sub.sprintf("param%02d", seq);
                if (xml.name() == sub) {
                    const QXmlStreamAttributes atts = xml.attributes();
                    param.stepType = (int)atts.value(QLatin1String("steptype")).toInt();
                    param.Name = atts.value(QLatin1String("name")).toString();
                    param.valueType = (enum ValueType)atts.value(QLatin1String("valuetype")).toInt();
                    param.Value = atts.value(QLatin1String("value")).toString();
                    param.Detail = atts.value(QLatin1String("detail")).toString();
                    object->AddReplaceParam(param);
                }
                xml.skipCurrentElement();
                seq++;
            }
        } else {
            readUnknownElement();
        }

    }

    return object;
}

QPolygonF RoiReaderPrivate::readPolygon()
{
    Q_ASSERT(xml.isStartElement() && (xml.name() == QLatin1String("polygon") ||
                                      xml.name() == QLatin1String("polyline")));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString points = atts.value(QLatin1String("points")).toString();
    const QStringList pointsList = points.split(QLatin1Char(' '),
                                                QString::SkipEmptyParts);

    QPolygonF polygon;
    bool ok = true;

    for (const QString &point : pointsList) {
        const int commaPos = point.indexOf(QLatin1Char(','));
        if (commaPos == -1) {
            ok = false;
            break;
        }

        const qreal x = point.left(commaPos).toDouble(&ok);
        if (!ok)
            break;
        const qreal y = point.mid(commaPos + 1).toDouble(&ok);
        if (!ok)
            break;

        polygon.append(QPointF(x, y));
    }

    if (!ok)
        xml.raiseError(tr("Invalid points data for polygon"));

    xml.skipCurrentElement();
    return polygon;
}

static int intAttribute(const QXmlStreamAttributes& atts, const char *name, int def)
{
    bool ok = false;
    int value = atts.value(QLatin1String(name)).toInt(&ok);
    return ok ? value : def;
}

TextData RoiReaderPrivate::readObjectText()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("text"));

    const QXmlStreamAttributes atts = xml.attributes();

    TextData textData;

    if (atts.hasAttribute(QLatin1String("fontfamily")))
        textData.font = QFont(atts.value(QLatin1String("fontfamily")).toString());

    if (atts.hasAttribute(QLatin1String("pixelsize")))
        textData.font.setPixelSize(atts.value(QLatin1String("pixelsize")).toInt());

    textData.wordWrap = intAttribute(atts, "wrap", 0) == 1;
    textData.font.setBold(intAttribute(atts, "bold", 0) == 1);
    textData.font.setItalic(intAttribute(atts, "italic", 0) == 1);
    textData.font.setUnderline(intAttribute(atts, "underline", 0) == 1);
    textData.font.setStrikeOut(intAttribute(atts, "strikeout", 0) == 1);
    textData.font.setKerning(intAttribute(atts, "kerning", 1) == 1);

    QStringRef colorString = atts.value(QLatin1String("color"));
    if (!colorString.isEmpty())
        textData.color = QColor(colorString.toString());

    Qt::Alignment alignment = 0;

    QStringRef hAlignString = atts.value(QLatin1String("halign"));
    if (hAlignString == QLatin1String("center"))
        alignment |= Qt::AlignHCenter;
    else if (hAlignString == QLatin1String("right"))
        alignment |= Qt::AlignRight;
    else
        alignment |= Qt::AlignLeft;

    QStringRef vAlignString = atts.value(QLatin1String("valign"));
    if (vAlignString == QLatin1String("center"))
        alignment |= Qt::AlignVCenter;
    else if (vAlignString == QLatin1String("bottom"))
        alignment |= Qt::AlignBottom;
    else
        alignment |= Qt::AlignTop;

    textData.alignment = alignment;

    textData.text = xml.readElementText();

    return textData;
}

GroupLayer *RoiReaderPrivate::readGroupLayer()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("group"));

    const QXmlStreamAttributes atts = xml.attributes();
    const QString name = atts.value(QLatin1String("name")).toString();
    const int x = atts.value(QLatin1String("x")).toInt();
    const int y = atts.value(QLatin1String("y")).toInt();

    GroupLayer *groupLayer = new GroupLayer(name, x, y);
    readLayerAttributes(*groupLayer, atts);

    while (xml.readNextStartElement()) {
        if (Layer *layer = tryReadLayer())
            groupLayer->addLayer(layer);
        else if (xml.name() == QLatin1String("properties"))
            groupLayer->mergeProperties(readProperties());
        else
            readUnknownElement();
    }

    return groupLayer;
}

Properties RoiReaderPrivate::readProperties()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("properties"));

    Properties properties;

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("property"))
            readProperty(&properties);
        else
            readUnknownElement();
    }

    return properties;
}

void RoiReaderPrivate::readProperty(Properties *properties)
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("property"));

    const QXmlStreamAttributes atts = xml.attributes();
    QString propertyName = atts.value(QLatin1String("name")).toString();
    QString propertyValue = atts.value(QLatin1String("value")).toString();
    QString propertyType = atts.value(QLatin1String("type")).toString();

    while (xml.readNext() != QXmlStreamReader::Invalid) {
        if (xml.isEndElement()) {
            break;
        } else if (xml.isCharacters() && !xml.isWhitespace()) {
            if (propertyValue.isEmpty())
                propertyValue = xml.text().toString();
        } else if (xml.isStartElement()) {
            readUnknownElement();
        }
    }

    QVariant variant(propertyValue);

    if (!propertyType.isEmpty()) {
        int type = nameToType(propertyType);

        if (type == filePathTypeId())
            variant = q->resolveReference(variant.toString(), mPath);

        variant = fromExportValue(variant, type);
    }

    properties->insert(propertyName, variant);
}


RoiReader::RoiReader(ParamTable *pParamTab)
    : d(new RoiReaderPrivate(this))
{
    d->pParamTable = pParamTab;
}

RoiReader::~RoiReader()
{
    delete d;
}

RoiMap *RoiReader::readRoi(QIODevice *device, const QString &path)
{
    return d->readRoi(device, path);
}

RoiMap *RoiReader::readRoi(const QString &fileName)
{
    QFile file(fileName);
    if (!d->openFile(&file))
        return nullptr;

    return readRoi(&file, QFileInfo(fileName).absolutePath());
}

QString RoiReader::errorString() const
{
    return d->errorString();
}

QString RoiReader::resolveReference(const QString &reference,
                                    const QString &mapPath)
{
    if (!reference.isEmpty() && QDir::isRelativePath(reference))
        return QDir::cleanPath(mapPath + QLatin1Char('/') + reference);
    return reference;
}

void RoiReader::setPath(QString _path)
{
    dirpath = _path;
}

} // namespace Qroilib
