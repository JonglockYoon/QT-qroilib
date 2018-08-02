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
    int PrepareImage();

private slots:
    void setExposureValue(int val);
    void on_buttonOpenCamera_clicked();

    void on_pushButton_CreateRectROI_clicked();

    void on_pushButton_CreatePatternROI_clicked();

    void on_pushButton_ReadROI_clicked();

    void on_pushButton_WriteROI_clicked();

    void on_pushButton_Tow_Point_Align_clicked();

    void on_pushButton_Measure_Align_clicked();

    void on_pushButton_Preview_clicked();

private:
    int m_nCamExposure;
    QString m_sCamera1;
    Ui::LeftDock *ui;
    QStringListModel *model2;
    IplImage* graySearchImg;
};

