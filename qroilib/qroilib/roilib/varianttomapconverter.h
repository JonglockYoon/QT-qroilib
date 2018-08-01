/*
 * varianttomapconverter.h
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

#pragma once

#include "roid_global.h"
//#include "gidmapper.h"
#include "roiobject.h"

#include <QCoreApplication>
#include <QDir>
#include <QVariant>

namespace Qroilib {

class GroupLayer;
class Layer;
class RoiMap;
class ObjectGroup;
class Properties;

/**
 * Converts a QVariant to a RoiMap instance. Meant to be used together with
 * JsonReader.
 */
class ROIDSHARED_EXPORT VariantToMapConverter
{
    // Using the RoiReader context since the messages are the same
    Q_DECLARE_TR_FUNCTIONS(RoiReader)

public:
    VariantToMapConverter()
        : mRoi(nullptr)
    {}

    /**
     * Tries to convert the given \a variant to a RoiMap instance. The \a mapDir
     * is necessary to resolve any relative references to external images.
     *
     * Returns 0 in case of an error. The error can be obstained using
     * errorString().
     */
    RoiMap *toMap(const QVariant &variant, const QDir &mapDir);

    /**
     * Returns the last error, if any.
     */
    QString errorString() const { return mError; }

private:
    Properties toProperties(const QVariant &propertiesVariant,
                            const QVariant &propertyTypesVariant) const;
    Layer *toLayer(const QVariant &variant);
    ObjectGroup *toObjectGroup(const QVariantMap &variantMap);
    GroupLayer *toGroupLayer(const QVariantMap &variantMap);

    QPolygonF toPolygon(const QVariant &variant) const;
    TextData toTextData(const QVariantMap &variant) const;

    Properties extractProperties(const QVariantMap &variantMap) const;

    RoiMap *mRoi;
    QDir mRoiDir;
    QString mError;
};

} // namespace Qroilib
