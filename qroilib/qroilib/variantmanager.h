#pragma once

#include <roilib_export.h>
// Qt
#include <QObject>
#include <QString>
#include <QVariant>
#include <QtVariantProperty>

namespace Qroilib
{


class ROIDSHARED_EXPORT VariantManager : public QtVariantPropertyManager
{
    Q_OBJECT
public:
    VariantManager(QObject *parent = 0);
    ~VariantManager();

    virtual QVariant value(const QtProperty *property) const;
    virtual int valueType(int propertyType) const;
    virtual bool isPropertyTypeSupported(int propertyType) const;

    QString valueText(const QtProperty *property) const;

public slots:
    virtual void setValue(QtProperty *property, const QVariant &val);
protected:
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);
private slots:
    void slotValueChanged(QtProperty *property, const QVariant &value);
    void slotPropertyDestroyed(QtProperty *property);
private:
    struct Data {
        QVariant value;
        QtVariantProperty *x;
        QtVariantProperty *y;
    };
    QMap<const QtProperty *, Data> propertyToData;
    QMap<const QtProperty *, QtProperty *> xToProperty;
    QMap<const QtProperty *, QtProperty *> yToProperty;
};

}
