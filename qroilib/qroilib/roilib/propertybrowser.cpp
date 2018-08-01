/*
 * propertybrowser.cpp
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of Roid.
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

#if 1

#include "propertybrowser.h"

#include "changeobjectgroupproperties.h"
#include "changeproperties.h"
#include "roimap.h"
#include "roiobject.h"
#include "moveroiobject.h"
#include "objectgroup.h"
#include "resizeroiobject.h"
#include "rotateroiobject.h"
#include "utils.h"
#include "variantpropertymanager.h"
#include <qroilib/documentview/documentview.h>

#include <QtGroupPropertyManager>

#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>

namespace Qroilib {

PropertyBrowser::PropertyBrowser(QWidget *parent)
    : QtTreePropertyBrowser(parent)
    , mUpdating(false)
    , mObject(nullptr)
    , mDocument(nullptr)
    , mRoiDocument(nullptr)
    , mVariantManager(new VariantPropertyManager(this))
    , mGroupManager(new QtGroupPropertyManager(this))
    , mCustomPropertiesGroup(nullptr)
{
    //VariantEditorFactory *variantEditorFactory = new VariantEditorFactory(this);

    //setFactoryForManager(mVariantManager, variantEditorFactory);
    setResizeMode(ResizeToContents);
    setRootIsDecorated(false);
    setPropertiesWithoutValueMarked(true);

    mStaggerAxisNames.append(tr("X"));
    mStaggerAxisNames.append(tr("Y"));

    mStaggerIndexNames.append(tr("Odd"));
    mStaggerIndexNames.append(tr("Even"));

    mOrientationNames.append(QCoreApplication::translate("Tiled::Internal::NewMapDialog", "Orthogonal"));
    mOrientationNames.append(QCoreApplication::translate("Tiled::Internal::NewMapDialog", "Isometric"));
    mOrientationNames.append(QCoreApplication::translate("Tiled::Internal::NewMapDialog", "Isometric (Staggered)"));
    mOrientationNames.append(QCoreApplication::translate("Tiled::Internal::NewMapDialog", "Hexagonal (Staggered)"));

    mRoisetOrientationNames.append(mOrientationNames.at(0));
    mRoisetOrientationNames.append(mOrientationNames.at(1));

    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "XML"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "Base64 (uncompressed)"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "Base64 (gzip compressed)"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "Base64 (zlib compressed)"));
    mLayerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "CSV"));

    mRenderOrderNames.append(QCoreApplication::translate("PreferencesDialog", "Right Down"));
    mRenderOrderNames.append(QCoreApplication::translate("PreferencesDialog", "Right Up"));
    mRenderOrderNames.append(QCoreApplication::translate("PreferencesDialog", "Left Down"));
    mRenderOrderNames.append(QCoreApplication::translate("PreferencesDialog", "Left Up"));

    mFlippingFlagNames.append(tr("Horizontal"));
    mFlippingFlagNames.append(tr("Vertical"));

    mDrawOrderNames.append(tr("Top Down"));
    mDrawOrderNames.append(tr("Manual"));

    connect(mVariantManager, SIGNAL(valueChanged(QtProperty*,QVariant)),
            SLOT(valueChanged(QtProperty*,QVariant)));

//    connect(variantEditorFactory, &VariantEditorFactory::resetProperty,
//            this, &PropertyBrowser::resetProperty);

    //connect(Preferences::instance(), &Preferences::objectTypesChanged,
    //        this, &PropertyBrowser::objectTypesChanged);
}

void PropertyBrowser::setObject(Object *object)
{
    if (mObject == object)
        return;

    removeProperties();
    mObject = object;

    addProperties();
}

void PropertyBrowser::setDocument(DocumentView *document)
{
    DocumentView *mapDocument = qobject_cast<DocumentView*>(document);

    if (mDocument == document)
        return;

    if (mDocument) {
        mDocument->disconnect(this);
    }

    mDocument = document;
    mRoiDocument = mapDocument;

    if (mapDocument) {
        connect(mapDocument, SIGNAL(mapChanged()),
                SLOT(mapChanged()));
        connect(mapDocument, SIGNAL(objectsChanged(QList<RoiObject*>)),
                SLOT(objectsChanged(QList<RoiObject*>)));
        connect(mapDocument, SIGNAL(objectsTypeChanged(QList<RoiObject*>)),
                SLOT(objectsTypeChanged(QList<RoiObject*>)));
//        connect(mapDocument, &MapDocument::layerChanged,
//                this, &PropertyBrowser::layerChanged);
        connect(mapDocument, SIGNAL(objectGroupChanged(ObjectGroup*)),
                SLOT(objectGroupChanged(ObjectGroup*)));

    }
}

bool PropertyBrowser::isCustomPropertyItem(QtBrowserItem *item) const
{
    return item && mPropertyToId[item->property()] == CustomProperty;
}

void PropertyBrowser::editCustomProperty(const QString &name)
{
    QtVariantProperty *property = mNameToProperty.value(name);
    if (!property)
        return;

    const QList<QtBrowserItem*> propertyItems = items(property);
    if (!propertyItems.isEmpty())
        editItem(propertyItems.first());
}

bool PropertyBrowser::event(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();

    return QtTreePropertyBrowser::event(event);
}

void PropertyBrowser::mapChanged()
{
    if (mObject == mRoiDocument->roimap())
        updateProperties();
}

void PropertyBrowser::objectsChanged(const QList<RoiObject *> &objects)
{
    if (mObject && mObject->typeId() == Object::RoiObjectType)
        if (objects.contains(static_cast<RoiObject*>(mObject)))
            updateProperties();
}

void PropertyBrowser::objectsTypeChanged(const QList<RoiObject *> &objects)
{
    if (mObject && mObject->typeId() == Object::RoiObjectType)
        if (objects.contains(static_cast<RoiObject*>(mObject)))
            updateCustomProperties();
}

void PropertyBrowser::layerChanged(Layer *layer)
{
    if (mObject == layer)
        updateProperties();
}

void PropertyBrowser::objectGroupChanged(ObjectGroup *objectGroup)
{
    if (mObject == objectGroup)
        updateProperties();
}

static QVariant predefinedPropertyValue(Object *object, const QString &name)
{
    QString objectType;

    switch (object->typeId()) {
    case Object::RoiObjectType: {
        auto roiObject = static_cast<RoiObject*>(object);
        objectType = roiObject->type();
        break;
    }
    case Object::LayerType:
    case Object::MapType:
        break;
    }

    if (objectType.isEmpty())
        return QVariant();

    return QVariant();
}

static bool anyObjectHasProperty(const QList<Object*> &objects, const QString &name)
{
    for (Object *obj : objects) {
        if (obj->hasProperty(name))
            return true;
    }
    return false;
}

static bool propertyValueAffected(Object *currentObject,
                                  Object *changedObject,
                                  const QString &propertyName)
{
    if (currentObject == changedObject)
        return true;

    return false;
}

static bool objectPropertiesRelevant(DocumentView *document, Object *object)
{
    auto currentObject = document->currentObject();
    if (!currentObject)
        return false;

    if (currentObject == object)
        return true;

    if (document->currentObjects().contains(object))
        return true;

    return false;
}

void PropertyBrowser::selectedObjectsChanged()
{
    updateCustomProperties();
}

void PropertyBrowser::selectedTilesChanged()
{
    updateCustomProperties();
}

void PropertyBrowser::objectTypesChanged()
{
    if (mObject && mObject->typeId() == Object::RoiObjectType)
        updateCustomProperties();
}

void PropertyBrowser::valueChanged(QtProperty *property, const QVariant &val)
{
    if (mUpdating)
        return;
    if (!mObject || !mDocument)
        return;
    if (!mPropertyToId.contains(property))
        return;

    const PropertyId id = mPropertyToId.value(property);

    if (id == CustomProperty) {
        QUndoStack *undoStack = mDocument->undoStack();
        undoStack->push(new SetProperty(mDocument,
                                        mDocument->currentObjects(),
                                        property->propertyName(),
                                        val));
        return;
    }

}

void PropertyBrowser::resetProperty(QtProperty *property)
{
    switch (mVariantManager->propertyType(property)) {
    case QVariant::Color:
        // At the moment it is only possible to reset color values
        mVariantManager->setValue(property, QColor());
        break;

    default:
        qWarning() << "Resetting of property type not supported right now";
    }
}

void PropertyBrowser::addMapProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("RoiMap"));

    QtVariantProperty *orientationProperty =
            addProperty(OrientationProperty,
                        QtVariantPropertyManager::enumTypeId(),
                        tr("Orientation"),
                        groupProperty);

    orientationProperty->setAttribute(QLatin1String("enumNames"), mOrientationNames);

    addProperty(WidthProperty, QVariant::Int, tr("Width"), groupProperty)->setEnabled(false);
    addProperty(HeightProperty, QVariant::Int, tr("Height"), groupProperty)->setEnabled(false);
    addProperty(RoiWidthProperty, QVariant::Int, tr("RoiMap Width"), groupProperty);
    addProperty(RoiHeightProperty, QVariant::Int, tr("RoiMap Height"), groupProperty);

    addProperty(HexSideLengthProperty, QVariant::Int, tr("RoiMap Side Length (Hex)"), groupProperty);

    QtVariantProperty *staggerAxisProperty =
            addProperty(StaggerAxisProperty,
                        QtVariantPropertyManager::enumTypeId(),
                        tr("Stagger Axis"),
                        groupProperty);

    staggerAxisProperty->setAttribute(QLatin1String("enumNames"), mStaggerAxisNames);

    QtVariantProperty *staggerIndexProperty =
            addProperty(StaggerIndexProperty,
                        QtVariantPropertyManager::enumTypeId(),
                        tr("Stagger Index"),
                        groupProperty);

    staggerIndexProperty->setAttribute(QLatin1String("enumNames"), mStaggerIndexNames);

    QtVariantProperty *layerFormatProperty =
            addProperty(LayerFormatProperty,
                        QtVariantPropertyManager::enumTypeId(),
                        tr("Tile Layer Format"),
                        groupProperty);

    layerFormatProperty->setAttribute(QLatin1String("enumNames"), mLayerFormatNames);

    QtVariantProperty *renderOrderProperty =
            addProperty(RenderOrderProperty,
                        QtVariantPropertyManager::enumTypeId(),
                        tr("Tile Render Order"),
                        groupProperty);

    renderOrderProperty->setAttribute(QLatin1String("enumNames"), mRenderOrderNames);

    addProperty(BackgroundColorProperty, QVariant::Color, tr("Background Color"), groupProperty);
    addProperty(groupProperty);
}

static QStringList objectTypeNames()
{
    QStringList names;
//    for (const ObjectType &type : Preferences::instance()->objectTypes())
//        names.append(type.name);
    return names;
}

void PropertyBrowser::addRoiObjectProperties()
{
    // DEFAULT MAP OBJECT PROPERTIES
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Object"));

    addProperty(IdProperty, QVariant::Int, tr("ID"), groupProperty)->setEnabled(false);
    addProperty(NameProperty, QVariant::String, tr("Name"), groupProperty);

    QtVariantProperty *typeProperty =
            addProperty(TypeProperty, QVariant::String, tr("Type"), groupProperty);
    typeProperty->setAttribute(QLatin1String("suggestions"), objectTypeNames());

    addProperty(VisibleProperty, QVariant::Bool, tr("Visible"), groupProperty);
    addProperty(XProperty, QVariant::Double, tr("X"), groupProperty);
    addProperty(YProperty, QVariant::Double, tr("Y"), groupProperty);

    auto roiObject = static_cast<const RoiObject*>(mObject);

    if (!roiObject->isPolyShape()) {
        addProperty(WidthProperty, QVariant::Double, tr("Width"), groupProperty);
        addProperty(HeightProperty, QVariant::Double, tr("Height"), groupProperty);
    }

    addProperty(RotationProperty, QVariant::Double, tr("Rotation"), groupProperty);

    if (roiObject->shape() == RoiObject::Text) {
        addProperty(TextProperty, QVariant::String, tr("Text"), groupProperty)->setAttribute(QLatin1String("multiline"), true);
//        addProperty(TextAlignmentProperty, VariantPropertyManager::flagTypeId(), tr("Alignment"), groupProperty);
        addProperty(FontProperty, QVariant::Font, tr("Font"), groupProperty);
        addProperty(WordWrapProperty, QVariant::Bool, tr("Word Wrap"), groupProperty);
        addProperty(ColorProperty, QVariant::Color, tr("Color"), groupProperty);
    }

    addProperty(groupProperty);
}

void PropertyBrowser::addLayerProperties(QtProperty *parent)
{
    addProperty(NameProperty, QVariant::String, tr("Name"), parent);
    addProperty(VisibleProperty, QVariant::Bool, tr("Visible"), parent);

    QtVariantProperty *opacityProperty =
            addProperty(OpacityProperty, QVariant::Double, tr("Opacity"), parent);
    opacityProperty->setAttribute(QLatin1String("minimum"), 0.0);
    opacityProperty->setAttribute(QLatin1String("maximum"), 1.0);
    opacityProperty->setAttribute(QLatin1String("singleStep"), 0.1);

    addProperty(OffsetXProperty, QVariant::Double, tr("Horizontal Offset"), parent);
    addProperty(OffsetYProperty, QVariant::Double, tr("Vertical Offset"), parent);
}

void PropertyBrowser::addTileLayerProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Tile Layer"));
    addLayerProperties(groupProperty);
    addProperty(groupProperty);
}

void PropertyBrowser::addObjectGroupProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Object Layer"));
    addLayerProperties(groupProperty);

    addProperty(ColorProperty, QVariant::Color, tr("Color"), groupProperty);

    QtVariantProperty *drawOrderProperty =
            addProperty(DrawOrderProperty,
                        QtVariantPropertyManager::enumTypeId(),
                        tr("Drawing Order"),
                        groupProperty);

    drawOrderProperty->setAttribute(QLatin1String("enumNames"), mDrawOrderNames);

    addProperty(groupProperty);
}

void PropertyBrowser::addGroupLayerProperties()
{
    QtProperty *groupProperty = mGroupManager->addProperty(tr("Group Layer"));
    addLayerProperties(groupProperty);
    addProperty(groupProperty);
}

void PropertyBrowser::applyTileLayerValue(PropertyId id, const QVariant &val)
{
    Q_UNUSED(id)
    Q_UNUSED(val)
}

void PropertyBrowser::applyObjectGroupValue(PropertyId id, const QVariant &val)
{
    ObjectGroup *objectGroup = static_cast<ObjectGroup*>(mObject);
    QUndoCommand *command = nullptr;

    switch (id) {
    case ColorProperty: {
        const QColor color = val.value<QColor>();
        command = new ChangeObjectGroupProperties(mRoiDocument,
                                                  objectGroup,
                                                  color,
                                                  objectGroup->drawOrder());
        break;
    }
    case DrawOrderProperty: {
        ObjectGroup::DrawOrder drawOrder = static_cast<ObjectGroup::DrawOrder>(val.toInt());
        command = new ChangeObjectGroupProperties(mRoiDocument,
                                                  objectGroup,
                                                  objectGroup->color(),
                                                  drawOrder);
        break;
    }
    default:
        break;
    }

    if (command)
        mDocument->undoStack()->push(command);
}

void PropertyBrowser::applyGroupLayerValue(PropertyId id, const QVariant &val)
{
    Q_UNUSED(id)
    Q_UNUSED(val)
}

/**
 * @warning This function does not add the property to the view.
 */
QtVariantProperty *PropertyBrowser::createProperty(PropertyId id, int type,
                                                   const QString &name)
{
    QtVariantProperty *property = mVariantManager->addProperty(type, name);
    if (!property) {
        // fall back to string property for unsupported property types
        property = mVariantManager->addProperty(QVariant::String, name);
    }

    if (type == QVariant::Bool)
        property->setAttribute(QLatin1String("textVisible"), false);
    if (type == QVariant::String && id == CustomProperty)
        property->setAttribute(QLatin1String("multiline"), true);
    if (type == QVariant::Double && id == CustomProperty)
        property->setAttribute(QLatin1String("decimals"), 9);

    mPropertyToId.insert(property, id);

    if (id != CustomProperty) {
        Q_ASSERT(!mIdToProperty.contains(id));
        mIdToProperty.insert(id, property);
    } else {
        Q_ASSERT(!mNameToProperty.contains(name));
        mNameToProperty.insert(name, property);
    }

    return property;
}

QtVariantProperty *PropertyBrowser::addProperty(PropertyId id, int type,
                                                const QString &name,
                                                QtProperty *parent)
{
    QtVariantProperty *property = createProperty(id, type, name);

    parent->addSubProperty(property);

    if (id == CustomProperty) {
        // Collapse custom color properties, to save space
        if (type == QVariant::Color)
            setExpanded(items(property).first(), false);
    }

    return property;
}

void PropertyBrowser::addProperties()
{
    if (!mObject)
        return;

    mUpdating = true;

    // Add the built-in properties for each object type
    switch (mObject->typeId()) {
    case Object::MapType:               addMapProperties(); break;
    case Object::RoiObjectType:         addRoiObjectProperties(); break;
    case Object::LayerType:
        switch (static_cast<Layer*>(mObject)->layerType()) {
        case Layer::ObjectGroupType:    addObjectGroupProperties(); break;

        case Layer::GroupLayerType:     addGroupLayerProperties();  break;
        }
        break;
    }

    // Make sure the color and font properties are collapsed, to save space
    if (QtProperty *colorProperty = mIdToProperty.value(ColorProperty))
        setExpanded(items(colorProperty).first(), false);
    if (QtProperty *colorProperty = mIdToProperty.value(BackgroundColorProperty))
        setExpanded(items(colorProperty).first(), false);
    if (QtProperty *fontProperty = mIdToProperty.value(FontProperty))
        setExpanded(items(fontProperty).first(), false);

    // Add a node for the custom properties
    mCustomPropertiesGroup = mGroupManager->addProperty(tr("Custom Properties"));
    addProperty(mCustomPropertiesGroup);

    mUpdating = false;

    updateProperties();
    updateCustomProperties();
}

void PropertyBrowser::removeProperties()
{
    mVariantManager->clear();
    mGroupManager->clear();
    mPropertyToId.clear();
    mIdToProperty.clear();
    mNameToProperty.clear();
    mCustomPropertiesGroup = nullptr;
}

void PropertyBrowser::updateProperties()
{
    Q_ASSERT(mObject);

    mUpdating = true;

    switch (mObject->typeId()) {
    case Object::MapType: {
        const RoiMap *roimap = static_cast<const RoiMap*>(mObject);
        mIdToProperty[WidthProperty]->setValue(roimap->width());
        mIdToProperty[HeightProperty]->setValue(roimap->height());
        mIdToProperty[RoiWidthProperty]->setValue(roimap->roiWidth());
        mIdToProperty[RoiHeightProperty]->setValue(roimap->roiHeight());
        mIdToProperty[OrientationProperty]->setValue(roimap->orientation() - 1);
        mIdToProperty[HexSideLengthProperty]->setValue(roimap->hexSideLength());
        mIdToProperty[StaggerAxisProperty]->setValue(roimap->staggerAxis());
        mIdToProperty[StaggerIndexProperty]->setValue(roimap->staggerIndex());
        mIdToProperty[LayerFormatProperty]->setValue(roimap->layerDataFormat());
        mIdToProperty[RenderOrderProperty]->setValue(roimap->renderOrder());
        mIdToProperty[BackgroundColorProperty]->setValue(roimap->backgroundColor());
        break;
    }
    case Object::RoiObjectType: {
        const RoiObject *roiObject = static_cast<const RoiObject*>(mObject);

        const QString &type = roiObject->effectiveType();
        const auto typeColorGroup = roiObject->type().isEmpty() ? QPalette::Disabled
                                                                : QPalette::Active;

        mIdToProperty[IdProperty]->setValue(roiObject->id());
        mIdToProperty[NameProperty]->setValue(roiObject->name());
        mIdToProperty[TypeProperty]->setValue(type);
        mIdToProperty[TypeProperty]->setValueColor(palette().color(typeColorGroup, QPalette::WindowText));
        mIdToProperty[VisibleProperty]->setValue(roiObject->isVisible());
        mIdToProperty[XProperty]->setValue(roiObject->x());
        mIdToProperty[YProperty]->setValue(roiObject->y());

        if (!roiObject->isPolyShape()) {
            mIdToProperty[WidthProperty]->setValue(roiObject->width());
            mIdToProperty[HeightProperty]->setValue(roiObject->height());
        }

        mIdToProperty[RotationProperty]->setValue(roiObject->rotation());

        if (QtVariantProperty *property = mIdToProperty[FlippingProperty]) {
            int flippingFlags = 0;
            property->setValue(flippingFlags);
        }

        if (roiObject->shape() == RoiObject::Text) {
            const auto& textData = roiObject->textData();
            mIdToProperty[TextProperty]->setValue(textData.text);
            mIdToProperty[FontProperty]->setValue(textData.font);
//            mIdToProperty[TextAlignmentProperty]->setValue(QVariant::fromValue(textData.alignment));
            mIdToProperty[WordWrapProperty]->setValue(textData.wordWrap);
            mIdToProperty[ColorProperty]->setValue(textData.color);
        }
        break;
    }
    case Object::LayerType: {
        const Layer *layer = static_cast<const Layer*>(mObject);

        mIdToProperty[NameProperty]->setValue(layer->name());
        mIdToProperty[VisibleProperty]->setValue(layer->isVisible());
        mIdToProperty[OpacityProperty]->setValue(layer->opacity());
        mIdToProperty[OffsetXProperty]->setValue(layer->offset().x());
        mIdToProperty[OffsetYProperty]->setValue(layer->offset().y());

        switch (layer->layerType()) {
        case Layer::ObjectGroupType: {
            const ObjectGroup *objectGroup = static_cast<const ObjectGroup*>(layer);
            const QColor color = objectGroup->color();
            mIdToProperty[ColorProperty]->setValue(color);
            mIdToProperty[DrawOrderProperty]->setValue(objectGroup->drawOrder());
            break;
        }
        case Layer::GroupLayerType:
            break;
        }
        break;
    }
    }

    mUpdating = false;
}

void PropertyBrowser::updateCustomProperties()
{
    if (!mObject)
        return;

    bool wasUpdating = mUpdating;
    mUpdating = true;

    qDeleteAll(mNameToProperty);
    mNameToProperty.clear();

    mCombinedProperties = mObject->properties();
    // Add properties from selected objects which mObject does not contain to mCombinedProperties.
    for (Object *obj : mDocument->currentObjects()) {
        if (obj == mObject)
            continue;

        QMapIterator<QString,QVariant> it(obj->properties());

        while (it.hasNext()) {
            it.next();
            if (!mCombinedProperties.contains(it.key()))
                mCombinedProperties.insert(it.key(), QString());
        }
    }

    QString objectType;

    switch (mObject->typeId()) {
    case Object::RoiObjectType: {
        auto roiObject = static_cast<RoiObject*>(mObject);
        objectType = roiObject->type();
        break;
    }
    case Object::LayerType:
    case Object::MapType:
        break;
    }

    QMapIterator<QString,QVariant> it(mCombinedProperties);

    while (it.hasNext()) {
        it.next();
        QtVariantProperty *property = addProperty(CustomProperty,
                                                  it.value().userType(),
                                                  it.key(),
                                                  mCustomPropertiesGroup);

        property->setValue(it.value());
        updatePropertyColor(it.key());
    }

    mUpdating = wasUpdating;
}

// If there are other objects selected check if their properties are equal. If not give them a gray color.
void PropertyBrowser::updatePropertyColor(const QString &name)
{
    QtVariantProperty *property = mNameToProperty.value(name);
    if (!property)
        return;

    QString propertyName = property->propertyName();
    QString propertyValue = property->valueText();

    const auto &objects = mDocument->currentObjects();

    QColor textColor = palette().color(QPalette::Active, QPalette::WindowText);
    QColor disabledTextColor = palette().color(QPalette::Disabled, QPalette::WindowText);

    // If one of the objects doesn't have this property then gray out the name and value.
    for (Object *obj : objects) {
        if (!obj->hasProperty(propertyName)) {
            property->setNameColor(disabledTextColor);
            property->setValueColor(disabledTextColor);
            return;
        }
    }

    // If one of the objects doesn't have the same property value then gray out the value.
    for (Object *obj : objects) {
        if (obj == mObject)
            continue;
        if (obj->property(propertyName) != propertyValue) {
            property->setNameColor(textColor);
            property->setValueColor(disabledTextColor);
            return;
        }
    }

    property->setNameColor(textColor);
    property->setValueColor(textColor);
}

} // namespace Qroilib

#endif
