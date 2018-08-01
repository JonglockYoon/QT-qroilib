/*
 * varianttomapconverter.cpp
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

#include "varianttomapconverter.h"

#include "grouplayer.h"

#include "roimap.h"
#include "objectgroup.h"
#include "properties.h"

#include <QScopedPointer>

namespace Qroilib {

static QString resolvePath(const QDir &dir, const QVariant &variant)
{
    QString fileName = variant.toString();
    if (!fileName.isEmpty() && QDir::isRelativePath(fileName))
        return QDir::cleanPath(dir.absoluteFilePath(fileName));
    return fileName;
}

RoiMap *VariantToMapConverter::toMap(const QVariant &variant,
                                  const QDir &mapDir)
{
    mRoiDir = mapDir;

    const QVariantMap variantMap = variant.toMap();
    const QString orientationString = variantMap[QLatin1String("orientation")].toString();

    RoiMap::Orientation orientation = orientationFromString(orientationString);

    if (orientation == RoiMap::Unknown) {
        mError = tr("Unsupported roimap orientation: \"%1\"")
                .arg(orientationString);
        return nullptr;
    }

    const QString staggerAxisString = variantMap[QLatin1String("staggeraxis")].toString();
    RoiMap::StaggerAxis staggerAxis = staggerAxisFromString(staggerAxisString);

    const QString staggerIndexString = variantMap[QLatin1String("staggerindex")].toString();
    RoiMap::StaggerIndex staggerIndex = staggerIndexFromString(staggerIndexString);

    const QString renderOrderString = variantMap[QLatin1String("renderorder")].toString();
    RoiMap::RenderOrder renderOrder = renderOrderFromString(renderOrderString);

    const int nextObjectId = variantMap[QLatin1String("nextobjectid")].toInt();

    QScopedPointer<RoiMap> roimap(new RoiMap(orientation,
                            variantMap[QLatin1String("width")].toInt(),
                            variantMap[QLatin1String("height")].toInt(),
                            variantMap[QLatin1String("roiwidth")].toInt(),
                            variantMap[QLatin1String("roiheight")].toInt()));
    roimap->setHexSideLength(variantMap[QLatin1String("hexsidelength")].toInt());
    roimap->setStaggerAxis(staggerAxis);
    roimap->setStaggerIndex(staggerIndex);
    roimap->setRenderOrder(renderOrder);
    if (nextObjectId)
        roimap->setNextObjectId(nextObjectId);

    mRoi = roimap.data();
    roimap->setProperties(extractProperties(variantMap));

    const QString bgColor = variantMap[QLatin1String("backgroundcolor")].toString();
    if (!bgColor.isEmpty() && QColor::isValidColor(bgColor))
        roimap->setBackgroundColor(QColor(bgColor));

    const auto layerVariants = variantMap[QLatin1String("layers")].toList();
    for (const QVariant &layerVariant : layerVariants) {
        Layer *layer = toLayer(layerVariant);
        if (!layer)
            return nullptr;

        roimap->addLayer(layer);
    }

    return roimap.take();
}

Properties VariantToMapConverter::toProperties(const QVariant &propertiesVariant,
                                               const QVariant &propertyTypesVariant) const
{
    const QVariantMap propertiesMap = propertiesVariant.toMap();
    const QVariantMap propertyTypesMap = propertyTypesVariant.toMap();

    Properties properties;

    QVariantMap::const_iterator it = propertiesMap.constBegin();
    QVariantMap::const_iterator it_end = propertiesMap.constEnd();
    for (; it != it_end; ++it) {
        int type = nameToType(propertyTypesMap.value(it.key()).toString());
        if (type == QVariant::Invalid)
            type = QVariant::String;

        QVariant value = it.value();

        if (type == filePathTypeId())
            value = resolvePath(mRoiDir, value);

        value = fromExportValue(value, type);

        properties[it.key()] = value;
    }

    return properties;
}

Layer *VariantToMapConverter::toLayer(const QVariant &variant)
{
    const QVariantMap variantMap = variant.toMap();
    Layer *layer = nullptr;

    if (variantMap[QLatin1String("type")] == QLatin1String("objectgroup"))
        layer = toObjectGroup(variantMap);
    else if (variantMap[QLatin1String("type")] == QLatin1String("group"))
        layer = toGroupLayer(variantMap);

    if (layer) {
        layer->setProperties(extractProperties(variantMap));

        const QPointF offset(variantMap[QLatin1String("offsetx")].toDouble(),
                             variantMap[QLatin1String("offsety")].toDouble());
        layer->setOffset(offset);
    }

    return layer;
}

ObjectGroup *VariantToMapConverter::toObjectGroup(const QVariantMap &variantMap)
{
    typedef QScopedPointer<ObjectGroup> ObjectGroupPtr;
    ObjectGroupPtr objectGroup(new ObjectGroup(variantMap[QLatin1String("name")].toString(),
                                               variantMap[QLatin1String("x")].toInt(),
                                               variantMap[QLatin1String("y")].toInt()));

    const qreal opacity = variantMap[QLatin1String("opacity")].toReal();
    const bool visible = variantMap[QLatin1String("visible")].toBool();

    objectGroup->setOpacity(opacity);
    objectGroup->setVisible(visible);

    objectGroup->setColor(variantMap.value(QLatin1String("color")).value<QColor>());

    const QString drawOrderString = variantMap.value(QLatin1String("draworder")).toString();
    if (!drawOrderString.isEmpty()) {
        objectGroup->setDrawOrder(drawOrderFromString(drawOrderString));
        if (objectGroup->drawOrder() == ObjectGroup::UnknownOrder) {
            mError = tr("Invalid draw order: %1").arg(drawOrderString);
            return nullptr;
        }
    }

    const auto objectVariants = variantMap[QLatin1String("objects")].toList();
    for (const QVariant &objectVariant : objectVariants) {
        const QVariantMap objectVariantMap = objectVariant.toMap();

        const QString name = objectVariantMap[QLatin1String("name")].toString();
        const QString type = objectVariantMap[QLatin1String("type")].toString();
        const int id = objectVariantMap[QLatin1String("id")].toInt();
        const int gid = objectVariantMap[QLatin1String("gid")].toInt();
        const qreal x = objectVariantMap[QLatin1String("x")].toReal();
        const qreal y = objectVariantMap[QLatin1String("y")].toReal();
        const qreal width = objectVariantMap[QLatin1String("width")].toReal();
        const qreal height = objectVariantMap[QLatin1String("height")].toReal();
        const qreal rotation = objectVariantMap[QLatin1String("rotation")].toReal();

        const QPointF pos(x, y);
        const QSizeF size(width, height);

        RoiObject *object = new RoiObject(name, type, pos, size);
        object->setId(id);
        object->setRotation(rotation);

        if (objectVariantMap.contains(QLatin1String("visible")))
            object->setVisible(objectVariantMap[QLatin1String("visible")].toBool());

        object->setProperties(extractProperties(objectVariantMap));
        objectGroup->addObject(object);

        const QVariant polylineVariant = objectVariantMap[QLatin1String("polyline")];
        const QVariant polygonVariant = objectVariantMap[QLatin1String("polygon")];
        const QVariant textVariant = objectVariantMap[QLatin1String("text")];

        if (polygonVariant.isValid()) {
            object->setShape(RoiObject::Polygon);
            object->setPolygon(toPolygon(polygonVariant));
        }
        if (polylineVariant.isValid()) {
            object->setShape(RoiObject::Polyline);
            object->setPolygon(toPolygon(polylineVariant));
        }
        if (objectVariantMap.contains(QLatin1String("ellipse")))
            object->setShape(RoiObject::Ellipse);
        if (textVariant.isValid()) {
            object->setTextData(toTextData(textVariant.toMap()));
            object->setShape(RoiObject::Text);
        }
    }

    return objectGroup.take();
}

GroupLayer *VariantToMapConverter::toGroupLayer(const QVariantMap &variantMap)
{
    const QString name = variantMap[QLatin1String("name")].toString();
    const int x = variantMap[QLatin1String("x")].toInt();
    const int y = variantMap[QLatin1String("y")].toInt();
    const qreal opacity = variantMap[QLatin1String("opacity")].toReal();
    const bool visible = variantMap[QLatin1String("visible")].toBool();

    QScopedPointer<GroupLayer> groupLayer(new GroupLayer(name, x, y));

    groupLayer->setOpacity(opacity);
    groupLayer->setVisible(visible);

    const auto layerVariants = variantMap[QLatin1String("layers")].toList();
    for (const QVariant &layerVariant : layerVariants) {
        Layer *layer = toLayer(layerVariant);
        if (!layer)
            return nullptr;

        groupLayer->addLayer(layer);
    }

    return groupLayer.take();
}

QPolygonF VariantToMapConverter::toPolygon(const QVariant &variant) const
{
    QPolygonF polygon;
    const auto pointVariants = variant.toList();
    for (const QVariant &pointVariant : pointVariants) {
        const QVariantMap pointVariantMap = pointVariant.toMap();
        const qreal pointX = pointVariantMap[QLatin1String("x")].toReal();
        const qreal pointY = pointVariantMap[QLatin1String("y")].toReal();
        polygon.append(QPointF(pointX, pointY));
    }
    return polygon;
}

TextData VariantToMapConverter::toTextData(const QVariantMap &variant) const
{
    TextData textData;

    const QString family = variant[QLatin1String("fontfamily")].toString();
    const int pixelSize = variant[QLatin1String("pixelsize")].toInt();

    if (!family.isEmpty())
        textData.font = QFont(family);
    if (pixelSize > 0)
        textData.font.setPixelSize(pixelSize);

    textData.wordWrap = variant[QLatin1String("wrap")].toInt() == 1;
    textData.font.setBold(variant[QLatin1String("bold")].toInt() == 1);
    textData.font.setItalic(variant[QLatin1String("italic")].toInt() == 1);
    textData.font.setUnderline(variant[QLatin1String("underline")].toInt() == 1);
    textData.font.setStrikeOut(variant[QLatin1String("strikeout")].toInt() == 1);
    if (variant.contains(QLatin1String("kerning")))
        textData.font.setKerning(variant[QLatin1String("kerning")].toInt() == 1);

    QString colorString = variant[QLatin1String("color")].toString();
    if (!colorString.isEmpty())
        textData.color = QColor(colorString);

    Qt::Alignment alignment = 0;

    QString hAlignString = variant[QLatin1String("halign")].toString();
    if (hAlignString == QLatin1String("center"))
        alignment |= Qt::AlignHCenter;
    else if (hAlignString == QLatin1String("right"))
        alignment |= Qt::AlignRight;
    else
        alignment |= Qt::AlignLeft;

    QString vAlignString = variant[QLatin1String("valign")].toString();
    if (vAlignString == QLatin1String("center"))
        alignment |= Qt::AlignVCenter;
    else if (vAlignString == QLatin1String("bottom"))
        alignment |= Qt::AlignBottom;
    else
        alignment |= Qt::AlignTop;

    textData.alignment = alignment;

    textData.text = variant[QLatin1String("text")].toString();

    return textData;
}

Properties VariantToMapConverter::extractProperties(const QVariantMap &variantMap) const
{
    return toProperties(variantMap[QLatin1String("properties")],
                        variantMap[QLatin1String("propertytypes")]);
}

} // namespace Qroilib
