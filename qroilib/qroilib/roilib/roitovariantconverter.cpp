/*
 * maptovariantconverter.cpp
 * Copyright 2011, Porfírio José Pereira Ribeiro <porfirioribeiro@gmail.com>
 * Copyright 2011-2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "roitovariantconverter.h"

#include "grouplayer.h"

#include "roimap.h"
#include "roiobject.h"
#include "objectgroup.h"
#include "properties.h"

using namespace Qroilib;

static QString colorToString(const QColor &color)
{
    if (color.alpha() != 255)
        return color.name(QColor::HexArgb);
    return color.name();
}

QVariant RoiToVariantConverter::toVariant(const RoiMap &roimap, const QDir &mapDir)
{
    mRoiDir = mapDir;

    QVariantMap mapVariant;

    mapVariant[QLatin1String("type")] = QLatin1String("roimap");
    mapVariant[QLatin1String("version")] = 1.0;
    mapVariant[QLatin1String("orientation")] = orientationToString(roimap.orientation());
    mapVariant[QLatin1String("renderorder")] = renderOrderToString(roimap.renderOrder());
    mapVariant[QLatin1String("width")] = roimap.width();
    mapVariant[QLatin1String("height")] = roimap.height();
    mapVariant[QLatin1String("roiwidth")] = roimap.roiWidth();
    mapVariant[QLatin1String("roiheight")] = roimap.roiHeight();
    mapVariant[QLatin1String("nextobjectid")] = roimap.nextObjectId();

    addProperties(mapVariant, roimap.properties());

    if (roimap.orientation() == RoiMap::Hexagonal) {
        mapVariant[QLatin1String("hexsidelength")] = roimap.hexSideLength();
    }

    if (roimap.orientation() == RoiMap::Hexagonal) {
        mapVariant[QLatin1String("staggeraxis")] = staggerAxisToString(roimap.staggerAxis());
        mapVariant[QLatin1String("staggerindex")] = staggerIndexToString(roimap.staggerIndex());
    }

    const QColor bgColor = roimap.backgroundColor();
    if (bgColor.isValid())
        mapVariant[QLatin1String("backgroundcolor")] = colorToString(bgColor);

    mapVariant[QLatin1String("layers")] = toVariant(roimap.layers(),
                                                    roimap.layerDataFormat());

    return mapVariant;
}

QVariant RoiToVariantConverter::toVariant(const Properties &properties) const
{
    QVariantMap variantMap;

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        QVariant value = toExportValue(it.value());

        if (it.value().userType() == filePathTypeId())
            value = mRoiDir.relativeFilePath(value.toString());

        variantMap[it.key()] = value;
    }

    return variantMap;
}

QVariant RoiToVariantConverter::propertyTypesToVariant(const Properties &properties) const
{
    QVariantMap variantMap;

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it)
        variantMap[it.key()] = typeToName(it.value().userType());

    return variantMap;
}

QVariant RoiToVariantConverter::toVariant(const QList<Layer *> &layers,
                                          RoiMap::LayerDataFormat format) const
{
    QVariantList layerVariants;

    for (const Layer *layer : layers) {
        switch (layer->layerType()) {
        case Layer::ObjectGroupType:
            layerVariants << toVariant(*static_cast<const ObjectGroup*>(layer));
            break;
        case Layer::GroupLayerType:
            layerVariants << toVariant(*static_cast<const GroupLayer*>(layer), format);
        }
    }

    return layerVariants;
}

QVariant RoiToVariantConverter::toVariant(const ObjectGroup &objectGroup) const
{
    QVariantMap objectGroupVariant;
    objectGroupVariant[QLatin1String("type")] = QLatin1String("objectgroup");

    if (objectGroup.color().isValid())
        objectGroupVariant[QLatin1String("color")] = colorToString(objectGroup.color());

    objectGroupVariant[QLatin1String("draworder")] = drawOrderToString(objectGroup.drawOrder());

    addLayerAttributes(objectGroupVariant, objectGroup);
    QVariantList objectVariants;
    for (const RoiObject *object : objectGroup.objects()) {
        QVariantMap objectVariant;
        const QString &name = object->name();
        const QString &type = object->type();

        addProperties(objectVariant, object->properties());

        objectVariant[QLatin1String("id")] = object->id();
        objectVariant[QLatin1String("name")] = name;
        objectVariant[QLatin1String("type")] = type;
        objectVariant[QLatin1String("x")] = object->x();
        objectVariant[QLatin1String("y")] = object->y();
        objectVariant[QLatin1String("width")] = object->width();
        objectVariant[QLatin1String("height")] = object->height();
        objectVariant[QLatin1String("rotation")] = object->rotation();

        objectVariant[QLatin1String("visible")] = object->isVisible();

        /* Polygons are stored in this format:
         *
         *   "polygon/polyline": [
         *       { "x": 0, "y": 0 },
         *       { "x": 1, "y": 1 },
         *       ...
         *   ]
         */
        switch (object->shape()) {
        case RoiObject::Rectangle:
            break;
        case RoiObject::Pattern: {
//            QVariantList patternRectVariant;
//            QVariantMap patternVariant;
//            patternVariant[QLatin1String("x")] = object->patternRect().x();
//            patternVariant[QLatin1String("y")] = object->patternRect().y();
//            patternVariant[QLatin1String("width")] = object->patternRect().width();
//            patternVariant[QLatin1String("height")] = object->patternRect().height();
//            patternRectVariant.append(patternVariant);
//            objectVariant[QLatin1String("pattern")] = patternRectVariant;
            break;
        }
        case RoiObject::Polygon:
        case RoiObject::Polyline: {
            QVariantList pointVariants;
            for (const QPointF &point : object->polygon()) {
                QVariantMap pointVariant;
                pointVariant[QLatin1String("x")] = point.x();
                pointVariant[QLatin1String("y")] = point.y();
                pointVariants.append(pointVariant);
            }

            if (object->shape() == RoiObject::Polygon)
                objectVariant[QLatin1String("polygon")] = pointVariants;
            else
                objectVariant[QLatin1String("polyline")] = pointVariants;
            break;
        }
        case RoiObject::Ellipse:
            objectVariant[QLatin1String("ellipse")] = true;
            break;
        case RoiObject::Text:
            objectVariant[QLatin1String("text")] = toVariant(object->textData());
            break;
        }

        objectVariants << objectVariant;
    }

    objectGroupVariant[QLatin1String("objects")] = objectVariants;
    return objectGroupVariant;
}

QVariant RoiToVariantConverter::toVariant(const TextData &textData) const
{
    QVariantMap textVariant;

    textVariant[QLatin1String("text")] = textData.text;

    if (textData.font.family() != QLatin1String("sans-serif"))
        textVariant[QLatin1String("fontfamily")] = textData.font.family();
    if (textData.font.pixelSize() >= 0 && textData.font.pixelSize() != 16)
        textVariant[QLatin1String("pixelsize")] = textData.font.pixelSize();
    if (textData.wordWrap)
        textVariant[QLatin1String("wrap")] = textData.wordWrap;
    if (textData.color != Qt::black)
        textVariant[QLatin1String("color")] = colorToString(textData.color);
    if (textData.font.bold())
        textVariant[QLatin1String("bold")] = textData.font.bold();
    if (textData.font.italic())
        textVariant[QLatin1String("italic")] = textData.font.italic();
    if (textData.font.underline())
        textVariant[QLatin1String("underline")] = textData.font.underline();
    if (textData.font.strikeOut())
        textVariant[QLatin1String("strikeout")] = textData.font.strikeOut();
    if (!textData.font.kerning())
        textVariant[QLatin1String("kerning")] = textData.font.kerning();

    if (!textData.alignment.testFlag(Qt::AlignLeft)) {
        if (textData.alignment.testFlag(Qt::AlignHCenter))
            textVariant[QLatin1String("halign")] = QLatin1String("center");
        else if (textData.alignment.testFlag(Qt::AlignRight))
            textVariant[QLatin1String("halign")] = QLatin1String("right");
    }

    if (!textData.alignment.testFlag(Qt::AlignTop)) {
        if (textData.alignment.testFlag(Qt::AlignVCenter))
            textVariant[QLatin1String("valign")] = QLatin1String("center");
        else if (textData.alignment.testFlag(Qt::AlignBottom))
            textVariant[QLatin1String("valign")] = QLatin1String("bottom");
    }

    return textVariant;
}

QVariant RoiToVariantConverter::toVariant(const GroupLayer &groupLayer,
                                          RoiMap::LayerDataFormat format) const
{
    QVariantMap groupLayerVariant;
    groupLayerVariant[QLatin1String("type")] = QLatin1String("group");

    addLayerAttributes(groupLayerVariant, groupLayer);

    groupLayerVariant[QLatin1String("layers")] = toVariant(groupLayer.layers(),
                                                           format);

    return groupLayerVariant;
}

void RoiToVariantConverter::addLayerAttributes(QVariantMap &layerVariant,
                                               const Layer &layer) const
{
    layerVariant[QLatin1String("name")] = layer.name();
    layerVariant[QLatin1String("x")] = layer.x();
    layerVariant[QLatin1String("y")] = layer.y();
    layerVariant[QLatin1String("visible")] = layer.isVisible();
    layerVariant[QLatin1String("opacity")] = layer.opacity();

    const QPointF offset = layer.offset();
    if (!offset.isNull()) {
        layerVariant[QLatin1String("offsetx")] = offset.x();
        layerVariant[QLatin1String("offsety")] = offset.y();
    }

    addProperties(layerVariant, layer.properties());
}

void RoiToVariantConverter::addProperties(QVariantMap &variantMap,
                                          const Properties &properties) const
{
    if (properties.isEmpty())
        return;

    QVariantMap propertiesMap;
    QVariantMap propertyTypesMap;

    Properties::const_iterator it = properties.constBegin();
    Properties::const_iterator it_end = properties.constEnd();
    for (; it != it_end; ++it) {
        int type = it.value().userType();
        QVariant value = toExportValue(it.value());

        if (type == filePathTypeId())
            value = mRoiDir.relativeFilePath(value.toString());

        propertiesMap[it.key()] = value;
        propertyTypesMap[it.key()] = typeToName(type);
    }

    variantMap[QLatin1String("properties")] = propertiesMap;
    variantMap[QLatin1String("propertytypes")] = propertyTypesMap;
}
