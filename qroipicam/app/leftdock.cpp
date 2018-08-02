//
// leftdock.cpp
//
//QROILIB : QT Vision ROI Library
//Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
//
#include <QLabel>
#include <QApplication>
#include <QHeaderView>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QTextCodec>
#include <QCameraInfo>

#ifndef Q_OS_WIN
#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <libv4l1.h>
#include <libv4l2.h>
#endif

#include "picapture.h"
#include "leftdock.h"
#include "ui_leftdock.h"
#include "roiobject.h"
#include "utils.h"
#include <qroilib/documentview/documentview.h>
#include "mainwindow.h"

using namespace Qroilib;
using namespace std;

LeftDock::LeftDock(QString name, QWidget *parent)
    : QDockWidget(name, parent)
     ,ui(new Ui::LeftDock)
{
    ui->setupUi(this);

    setMinimumWidth(230);
    //setMaximumWidth(400);
    //setMinimumHeight(300);
    //setMaximumHeight(340);


    ui->lineEditWidth->setText("640");
    ui->lineEditHeight->setText("480");
    ui->lineEditFPS->setText("10");

    model2 = new QStringListModel(this);

    //theMainWindow->nCamExposure = getExposureValue();
    m_sCamera1 = "";
    sLow = 0;
    vLow = 0;
    setS(sLow);
    setV(vLow);

    ui->sliderExposure->setRange(0, 15);
    ui->sliderExposure->setSingleStep(1);
    ui->sliderExposure->setValue(theMainWindow->nCamExposure);
    setExposureValue(theMainWindow->nCamExposure);

    connect(ui->sliderExposure, SIGNAL(valueChanged(int)), this, SLOT(setExposureValue(int)));

    ui->sliderS->setRange(0, 255);
    ui->sliderV->setRange(0, 255);
    connect(ui->sliderS, SIGNAL(valueChanged(int)), this, SLOT(setS(int)));
    connect(ui->sliderV, SIGNAL(valueChanged(int)), this, SLOT(setV(int)));

    QStringList List2;
#ifdef PICAM
    List2 << "PiCam";
#else
    List2 << "None";
    int i = 0;
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
        i++;
        QString str;
        str.sprintf("%02d:%s", i, cameraInfo.description().toLatin1().data());
        List2 << str;
    }
#endif

#ifndef Q_OS_WIN
#ifndef PICAM
    {
        int seq = 0;
        char video[64];
        for (int i=0; i<12; i++) {
            sprintf(video, "/dev/video%d", i);
            int fd = ::v4l2_open(video,O_RDWR);
            if (fd != -1) {
                sprintf(video, "%02d:/dev/video%d", seq+1, i);
                List2 << video;
                ::v4l2_close(fd);
                seq++;
            }
        }
    }
#endif
#endif

    model2->setStringList(List2);
    ui->comboBoxCam1Port->setModel(model2);
    ui->comboBoxCam1Port->setCurrentIndex(0);

}


void LeftDock::setExposureValue(int val)
{
    ViewMainPage* pView = theMainWindow->viewMainPage();
    if (!pView->myCamCapture)
        return;


#if defined(Q_OS_WIN) || defined(PICAM)
    theMainWindow->nCamExposure = val;
    if (pView->myCamCapture) {
        pView->myCamCapture->capture.set(CV_CAP_PROP_EXPOSURE, val-5);
        qDebug() << "setExposureValue ok" << val-5;
    }
#else
    //int ncam1 = ui->comboBoxCam1Port->currentIndex();
    int ncam1 = m_sCamera1.mid(0,2).toInt();
    char video[64];
    struct v4l2_control ctrl;
    sprintf(video, "/dev/video%d", ncam1-1);
    int fd = ::v4l2_open(video,O_RDWR);
    if (fd == -1) {
        perror("opening video device");
        return;
    }

    try {
        ctrl.id = V4L2_CID_EXPOSURE_AUTO;
        ctrl.value = V4L2_EXPOSURE_MANUAL;

        if (-1 == v4l2_ioctl(fd,VIDIOC_S_CTRL,&ctrl)) {
            perror("setting exposure manual");
            ::v4l2_close(fd);
            return;
        }


        ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
        ctrl.value = val-5;
        if (-1 == v4l2_ioctl(fd,VIDIOC_S_CTRL,&ctrl)) {
            perror("setting exposure absolute");
            ::v4l2_close(fd);
            return;
        }
        qDebug() << "setExposureValue ok" << val-5;
    } catch (cv::Exception &e) {
        qDebug() << "e = " << e.what();
    } catch (...) {
        qFatal("Error <unknown> CamCapture::run()");
    }

    ::v4l2_close(fd);

    theMainWindow->nCamExposure = val;

#endif
}

//int LeftDock::getExposureValue()
//{
//    int val = 0;
//    ViewMainPage* pView = theMainWindow->viewMainPage();
//    if (!pView->myCamCapture)
//        return -1;

//    val = pView->myCamCapture->capture.get(CV_CAP_PROP_EXPOSURE);
//    return val - 5;
//}

void LeftDock::on_buttonOpenCamera_clicked()
{
    //int ncam1 = ui->comboBoxCam1Port->currentIndex();

    theMainWindow->camWidth = ui->lineEditWidth->text().toUInt();
    theMainWindow->camHeight =ui->lineEditHeight->text().toUInt();
    theMainWindow->fps =ui->lineEditFPS->text().toUInt();

    ViewMainPage* pView = theMainWindow->viewMainPage();
    QString str;
    pView->CloseCam(0);
    str = ui->comboBoxCam1Port->currentText();
    if (str == "None" || str == "")
        m_sCamera1 = "";
    else {
        m_sCamera1 = str;
        int ncam1 = m_sCamera1.mid(0,2).toInt();
        //theMainWindow->nCamExposure = getExposureValue();
        pView->OpenCam(0, ncam1-1);
    }

    //ui->sliderExposure->setValue(theMainWindow->nCamExposure);
}

void LeftDock::setS(int val)
{
    QString str;
    str = QString("%1").arg(val);
    ui->lineEditS->setText(str);
    sLow = val;
}
void LeftDock::setV(int val)
{
    QString str;
    str = QString("%1").arg(val);
    ui->lineEditV->setText(str);
    vLow = val;
}
