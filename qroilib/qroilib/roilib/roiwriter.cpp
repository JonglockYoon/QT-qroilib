/*
 * roiwriter.cpp
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

#include "roiwriter.h"

#include "compression.h"
#include "grouplayer.h"
#include "roimap.h"
#include "roiobject.h"
#include "objectgroup.h"
#include "savefile.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QXmlStreamWriter>

using namespace Qroilib;

static QString colorToString(const QColor &color)
{
    if (color.alpha() != 255)
        return color.name(QColor::HexArgb);
    return color.name();
}

namespace Qroilib {

class RoiWriterPrivate
{
    Q_DECLARE_TR_FUNCTIONS(RoiReader)

public:
    RoiWriterPrivate();

    void writeRoi(const RoiMap *roimap, QIODevice *device,
                  const QString &path);


    bool openFile(SaveFile *file);

    QString mError;
    bool mDtdEnabled;

    void writeRoiGroupStart(const RoiMap *roimap, QIODevice *device, const QString &path);
    void writeRoiGroup(const RoiMap *roimap);
    void writeRoiGroupEnd();

private:
    void writeRoi(QXmlStreamWriter &w, const RoiMap &roimap, bool isGroup);
    void writeLayers(QXmlStreamWriter &w, const QList<Layer *> &layers);
    void writeLayerAttributes(QXmlStreamWriter &w, const Layer &layer);
    void writeObjectGroup(QXmlStreamWriter &w, const ObjectGroup &objectGroup);
    void writeObject(QXmlStreamWriter &w, const RoiObject &roiObject);
    void writeObjectText(QXmlStreamWriter &w, const TextData &textData);
    void writeGroupLayer(QXmlStreamWriter &w, const GroupLayer &groupLayer);
    void writeProperties(QXmlStreamWriter &w,
                         const Properties &properties);

    QDir mRoiDir;     // The directory in which the roimap is being saved
    bool mUseAbsolutePaths;

     QXmlStreamWriter *writer;
};

} // namespace Qroilib


RoiWriterPrivate::RoiWriterPrivate()
    : mDtdEnabled(false)
    , mUseAbsolutePaths(false)
{
}

bool RoiWriterPrivate::openFile(SaveFile *file)
{
    if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        mError = tr("Could not open file for writing.");
        return false;
    }

    return true;
}

static QXmlStreamWriter *createWriter(QIODevice *device)
{
    QXmlStreamWriter *writer = new QXmlStreamWriter(device);
    writer->setAutoFormatting(true);
    writer->setAutoFormattingIndent(1);
    return writer;
}

void RoiWriterPrivate::writeRoi(const RoiMap *roimap, QIODevice *device,
                                const QString &path)
{
    mRoiDir = QDir(path);
    mUseAbsolutePaths = path.isEmpty();

    writer = createWriter(device);
    writer->writeStartDocument();

    if (mDtdEnabled) {
        writer->writeDTD(QLatin1String("<!DOCTYPE roimap SYSTEM \""
                                       "http://mapeditor.org/dtd/1.0/"
                                       "roimap.dtd\">"));
    }

    writeRoi(*writer, *roimap, false);
    writer->writeEndDocument();
    delete writer;
}

void RoiWriterPrivate::writeRoiGroupStart(const RoiMap *roimap, QIODevice *device, const QString &path)
{
    mRoiDir = QDir(path);
    mUseAbsolutePaths = path.isEmpty();

    writer = createWriter(device);
    QXmlStreamWriter &w = *writer;

    w.writeStartDocument();

    if (mDtdEnabled) {
        w.writeDTD(QLatin1String("<!DOCTYPE roimap SYSTEM \""
                                       "http://mapeditor.org/dtd/1.0/"
                                       "roimap.dtd\">"));
    }


    w.writeStartElement(QLatin1String("roimap"));

    const QString orientation = orientationToString(roimap->orientation());
    const QString renderOrder = renderOrderToString(roimap->renderOrder());

    w.writeAttribute(QLatin1String("version"), QLatin1String("1.0"));
    w.writeAttribute(QLatin1String("orientation"), orientation);
    w.writeAttribute(QLatin1String("renderorder"), renderOrder);
    w.writeAttribute(QLatin1String("width"), QString::number(roimap->width()));
    w.writeAttribute(QLatin1String("height"), QString::number(roimap->height()));
    w.writeAttribute(QLatin1String("roiwidth"),
                     QString::number(roimap->roiWidth()));
    w.writeAttribute(QLatin1String("roiheight"),
                     QString::number(roimap->roiHeight()));

    if (roimap->orientation() == RoiMap::Hexagonal) {
        w.writeAttribute(QLatin1String("hexsidelength"),
                         QString::number(roimap->hexSideLength()));
    }

    if (roimap->orientation() == RoiMap::Hexagonal) {
        w.writeAttribute(QLatin1String("staggeraxis"),
                         staggerAxisToString(roimap->staggerAxis()));
        w.writeAttribute(QLatin1String("staggerindex"),
                         staggerIndexToString(roimap->staggerIndex()));
    }

    if (roimap->backgroundColor().isValid()) {
        w.writeAttribute(QLatin1String("backgroundcolor"),
                         colorToString(roimap->backgroundColor()));
    }

    w.writeAttribute(QLatin1String("nextobjectid"),
                     QString::number(roimap->nextObjectId()));

    writeProperties(w, roimap->properties());
}
void RoiWriterPrivate::writeRoiGroup(const RoiMap *roimap)
{
    writeRoi(*writer, *roimap, true);
}
void RoiWriterPrivate::writeRoiGroupEnd()
{
    writer->writeEndElement();
    writer->writeEndDocument();
    delete writer;
}

void RoiWriterPrivate::writeRoi(QXmlStreamWriter &w, const RoiMap &roimap, bool isGroup)
{
    if (!isGroup)
    {
        w.writeStartElement(QLatin1String("roimap"));

        const QString orientation = orientationToString(roimap.orientation());
        const QString renderOrder = renderOrderToString(roimap.renderOrder());

        w.writeAttribute(QLatin1String("version"), QLatin1String("1.0"));
        w.writeAttribute(QLatin1String("orientation"), orientation);
        w.writeAttribute(QLatin1String("renderorder"), renderOrder);
        w.writeAttribute(QLatin1String("width"), QString::number(roimap.width()));
        w.writeAttribute(QLatin1String("height"), QString::number(roimap.height()));
        w.writeAttribute(QLatin1String("roiwidth"),
                         QString::number(roimap.roiWidth()));
        w.writeAttribute(QLatin1String("roiheight"),
                         QString::number(roimap.roiHeight()));

        if (roimap.orientation() == RoiMap::Hexagonal) {
            w.writeAttribute(QLatin1String("hexsidelength"),
                             QString::number(roimap.hexSideLength()));
        }

        if (roimap.orientation() == RoiMap::Hexagonal) {
            w.writeAttribute(QLatin1String("staggeraxis"),
                             staggerAxisToString(roimap.staggerAxis()));
            w.writeAttribute(QLatin1String("staggerindex"),
                             staggerIndexToString(roimap.staggerIndex()));
        }

        if (roimap.backgroundColor().isValid()) {
            w.writeAttribute(QLatin1String("backgroundcolor"),
                             colorToString(roimap.backgroundColor()));
        }

        w.writeAttribute(QLatin1String("nextobjectid"),
                         QString::number(roimap.nextObjectId()));

        writeProperties(w, roimap.properties());
    }

    writeLayers(w, roimap.layers());

    if (!isGroup) {
        w.writeEndElement();
    }
}

void RoiWriterPrivate::writeLayers(QXmlStreamWriter &w, const QList<Layer*> &layers)
{
    for (const Layer *layer : layers) {
        switch (layer->layerType()) {
        case Layer::ObjectGroupType:
            writeObjectGroup(w, *static_cast<const ObjectGroup*>(layer));
            break;
        case Layer::GroupLayerType:
            writeGroupLayer(w, *static_cast<const GroupLayer*>(layer));
            break;
        }
    }
}

void RoiWriterPrivate::writeLayerAttributes(QXmlStreamWriter &w,
                                            const Layer &layer)
{
    if (!layer.name().isEmpty())
        w.writeAttribute(QLatin1String("name"), layer.name());

    const int x = layer.x();
    const int y = layer.y();
    const qreal opacity = layer.opacity();
    if (x != 0)
        w.writeAttribute(QLatin1String("x"), QString::number(x));
    if (y != 0)
        w.writeAttribute(QLatin1String("y"), QString::number(y));

    if (!layer.isVisible())
        w.writeAttribute(QLatin1String("visible"), QLatin1String("0"));
    if (opacity != qreal(1))
        w.writeAttribute(QLatin1String("opacity"), QString::number(opacity));

    const QPointF offset = layer.offset();
    if (!offset.isNull()) {
        w.writeAttribute(QLatin1String("offsetx"), QString::number(offset.x()));
        w.writeAttribute(QLatin1String("offsety"), QString::number(offset.y()));
    }
}

void RoiWriterPrivate::writeObjectGroup(QXmlStreamWriter &w,
                                        const ObjectGroup &objectGroup)
{
    w.writeStartElement(QLatin1String("objectgroup"));

    if (objectGroup.color().isValid())
        w.writeAttribute(QLatin1String("color"),
                         colorToString(objectGroup.color()));

    if (objectGroup.drawOrder() != ObjectGroup::TopDownOrder) {
        w.writeAttribute(QLatin1String("draworder"),
                         drawOrderToString(objectGroup.drawOrder()));
    }

    writeLayerAttributes(w, objectGroup);
    writeProperties(w, objectGroup.properties());

    for (const RoiObject *roiObject : objectGroup.objects()) {
        if (roiObject->shape() == RoiObject::Pattern)
            writeObject(w, *roiObject);
    }
    for (const RoiObject *roiObject : objectGroup.objects()) {
        if (roiObject->shape() != RoiObject::Pattern)
            writeObject(w, *roiObject);
    }

    w.writeEndElement();
}

void RoiWriterPrivate::writeObject(QXmlStreamWriter &w,
                                   const RoiObject &roiObject)
{
    w.writeStartElement(QLatin1String("object"));
    w.writeAttribute(QLatin1String("id"), QString::number(roiObject.id()));
    const QString &name = roiObject.name();
    const QString &type = roiObject.type();
    if (!name.isEmpty())
        w.writeAttribute(QLatin1String("name"), name);
    if (!type.isEmpty())
        w.writeAttribute(QLatin1String("type"), type);

    const QPointF pos = roiObject.position();
    const QSizeF size = roiObject.size();

    w.writeAttribute(QLatin1String("x"), QString::number(pos.x()));
    w.writeAttribute(QLatin1String("y"), QString::number(pos.y()));

    if (size.width() != 0)
        w.writeAttribute(QLatin1String("width"), QString::number(size.width()));
    if (size.height() != 0)
        w.writeAttribute(QLatin1String("height"), QString::number(size.height()));

    const qreal rotation = roiObject.rotation();
    if (rotation != 0.0)
        w.writeAttribute(QLatin1String("rotation"), QString::number(rotation));

    if (!roiObject.isVisible())
        w.writeAttribute(QLatin1String("visible"), QLatin1String("0"));

    writeProperties(w, roiObject.properties());

    switch (roiObject.shape()) {
    case RoiObject::Pattern: {
        w.writeStartElement(QLatin1String("pattern"));
        if (roiObject.mPattern != nullptr)
           w.writeAttribute(QLatin1String("child"), QString::number(roiObject.mPattern->id()));
        w.writeEndElement();
        break;
    }
    case RoiObject::Polygon:
    case RoiObject::Polyline: {
        if (roiObject.shape() == RoiObject::Polygon)
            w.writeStartElement(QLatin1String("polygon"));
        else
            w.writeStartElement(QLatin1String("polyline"));

        QString points;
        for (const QPointF &point : roiObject.polygon()) {
            points.append(QString::number(point.x()));
            points.append(QLatin1Char(','));
            points.append(QString::number(point.y()));
            points.append(QLatin1Char(' '));
        }
        points.chop(1);
        w.writeAttribute(QLatin1String("points"), points);
        w.writeEndElement();
        break;
    }
    case RoiObject::Rectangle:
        w.writeStartElement(QLatin1String("rectangle"));
        if (roiObject.mParent != nullptr)
           w.writeAttribute(QLatin1String("parent"), QString::number(roiObject.mParent->id()));
        w.writeEndElement();
        break;
    case RoiObject::Ellipse:
        w.writeEmptyElement(QLatin1String("ellipse"));
        break;
    case RoiObject::Text: {
        writeObjectText(w, roiObject.textData());
        break;
    }
    }

    QString sub;
    w.writeStartElement(QLatin1String("param"));
    w.writeAttribute(QLatin1String("InspectType"), QString::number(roiObject.m_vecParams[0].nInspectType));
    int nParamSize = roiObject.m_vecParams.size();
    for (int j = 0; j < nParamSize; j++) {
        sub.sprintf("param%02d", j);
        w.writeStartElement(sub);
        const CParam *pParam = &roiObject.m_vecParams[j].param;
        w.writeAttribute(QLatin1String("steptype"), QString::number(pParam->stepType));
        w.writeAttribute(QLatin1String("name"), pParam->Name);
        w.writeAttribute(QLatin1String("valuetype"), QString::number(pParam->valueType));
        w.writeAttribute(QLatin1String("value"), pParam->Value);
        w.writeAttribute(QLatin1String("detail"), pParam->Detail);
        w.writeEndElement();
    }
    w.writeEndElement();


    w.writeEndElement();
}

void RoiWriterPrivate::writeObjectText(QXmlStreamWriter &w, const TextData &textData)
{
    w.writeStartElement(QLatin1String("text"));

    if (textData.font.family() != QLatin1String("sans-serif"))
        w.writeAttribute(QLatin1String("fontfamily"), textData.font.family());
    if (textData.font.pixelSize() >= 0 && textData.font.pixelSize() != 16)
        w.writeAttribute(QLatin1String("pixelsize"), QString::number(textData.font.pixelSize()));
    if (textData.wordWrap)
        w.writeAttribute(QLatin1String("wrap"), QLatin1String("1"));
    if (textData.color != Qt::black)
        w.writeAttribute(QLatin1String("color"), colorToString(textData.color));
    if (textData.font.bold())
        w.writeAttribute(QLatin1String("bold"), QLatin1String("1"));
    if (textData.font.italic())
        w.writeAttribute(QLatin1String("italic"), QLatin1String("1"));
    if (textData.font.underline())
        w.writeAttribute(QLatin1String("underline"), QLatin1String("1"));
    if (textData.font.strikeOut())
        w.writeAttribute(QLatin1String("strikeout"), QLatin1String("1"));
    if (!textData.font.kerning())
        w.writeAttribute(QLatin1String("kerning"), QLatin1String("0"));

    if (!textData.alignment.testFlag(Qt::AlignLeft)) {
        if (textData.alignment.testFlag(Qt::AlignHCenter))
            w.writeAttribute(QLatin1String("halign"), QLatin1String("center"));
        else if (textData.alignment.testFlag(Qt::AlignRight))
            w.writeAttribute(QLatin1String("halign"), QLatin1String("right"));
    }

    if (!textData.alignment.testFlag(Qt::AlignTop)) {
        if (textData.alignment.testFlag(Qt::AlignVCenter))
            w.writeAttribute(QLatin1String("valign"), QLatin1String("center"));
        else if (textData.alignment.testFlag(Qt::AlignBottom))
            w.writeAttribute(QLatin1String("valign"), QLatin1String("bottom"));
    }

    w.writeCharacters(textData.text);
    w.writeEndElement();
}

void RoiWriterPrivate::writeGroupLayer(QXmlStreamWriter &w,
                                       const GroupLayer &groupLayer)
{
    w.writeStartElement(QLatin1String("group"));
    writeLayerAttributes(w, groupLayer);

    writeProperties(w, groupLayer.properties());
    writeLayers(w, groupLayer.layers());

    w.writeEndElement();
}

void RoiWriterPrivate::writeProperties(QXmlStreamWriter &w,
                                       const Properties &properties)
{
    if (properties.isEmpty())
        return;

    w.writeStartElement(QLatin1String("properties"));

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        w.writeStartElement(QLatin1String("property"));
        w.writeAttribute(QLatin1String("name"), it.key());

        int type = it.value().userType();
        QString typeName = typeToName(type);
        if (typeName != QLatin1String("string"))
            w.writeAttribute(QLatin1String("type"), typeName);

        QString value = toExportValue(it.value()).toString();

        if (type == filePathTypeId() && !mUseAbsolutePaths)
            value = mRoiDir.relativeFilePath(value);

        if (value.contains(QLatin1Char('\n')))
            w.writeCharacters(value);
        else
            w.writeAttribute(QLatin1String("value"), value);

        w.writeEndElement();
    }

    w.writeEndElement();
}


RoiWriter::RoiWriter()
    : d(new RoiWriterPrivate)
    ,file(nullptr)
{
}

RoiWriter::~RoiWriter()
{
    delete d;
}

bool RoiWriter::writeRoiGroupStart(const RoiMap *roimap, const QString &fileName)
{
    file = new SaveFile(fileName);
    if (file == nullptr)
        return false;
    if (!d->openFile(file))
        return false;
    d->writeRoiGroupStart(roimap, file->device(), QFileInfo(fileName).absolutePath());
    return true;
}
bool RoiWriter::writeRoiGroup(const RoiMap *roimap)
{
    d->writeRoiGroup(roimap);
    return true;
}
bool RoiWriter::writeRoiGroupEnd()
{
    d->writeRoiGroupEnd();
    if (file == nullptr)
        return false;
    if (file->error() != QFileDevice::NoError) {
        d->mError = file->errorString();
        return false;
    }

    if (!file->commit()) {
        d->mError = file->errorString();
        return false;
    }
    return true;
}

bool RoiWriter::writeRoi(const RoiMap *roimap, const QString &fileName)
{
    SaveFile file(fileName);
    if (!d->openFile(&file))
        return false;

    d->writeRoi(roimap, file.device(), QFileInfo(fileName).absolutePath());

    if (file.error() != QFileDevice::NoError) {
        d->mError = file.errorString();
        return false;
    }

    if (!file.commit()) {
        d->mError = file.errorString();
        return false;
    }

    return true;
}

QString RoiWriter::errorString() const
{
    return d->mError;
}

void RoiWriter::setDtdEnabled(bool enabled)
{
    d->mDtdEnabled = enabled;
}

bool RoiWriter::isDtdEnabled() const
{
    return d->mDtdEnabled;
}

void RoiWriter::setPath(QString _path)
{
    dirpath = _path;
}
