/*
 * teachdock.h
 */

#pragma once

#include <QDockWidget>
#include <QVariant>
#include <QStringListModel>
#include <qroilib/documentview/documentview.h>

#include "common.h"

using namespace std;
//class Qroilib::DocumentView;

namespace Ui
{
class LeftDock;
}


class LeftDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit LeftDock(QString name, QWidget *parent = nullptr);

public:
    int getExposureValue();
    void updateValue();

private slots:
    void setExposureValue(int val);
    void setThresholdLowValue(int val);
    void setThresholdHighValue(int val);
    void on_buttonOpenCamera_clicked();

private:
    QString m_sCamera1;
    Ui::LeftDock *ui;
    QStringListModel *model2;

public:
    int thresholdLow;
    int thresholdHigh;
};

