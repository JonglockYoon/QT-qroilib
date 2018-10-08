//
//Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail>
//
//

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

#include <QMessageBox>
#include <QDebug>
#include <QCameraInfo>

#include "dialogcamera.h"
#include "ui_dialogcamera.h"
#include "mainwindow.h"

DialogCamera::DialogCamera(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCamera)
{
    ui->setupUi(this);

    model2 = new QStringListModel(this);

    m_nCamExposure = getExposureValue();
    m_sCamera1 = "";

#ifdef Q_OS_WIN
    ui->sliderExposure->setRange(0, 9);
#else
    ui->sliderExposure->setRange(0, 15000);
#endif
    ui->sliderExposure->setSingleStep(1);
    ui->sliderExposure->setValue(m_nCamExposure);
    setExposureValue(m_nCamExposure);
    ui->sliderExposure->setValue(m_nCamExposure);

    connect(ui->sliderExposure, SIGNAL(valueChanged(int)), this, SLOT(setExposureValue(int)));


    QStringList List2;
    List2 << "None";
    int i = 0;
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
        i++;
        QString str;
        str.sprintf("%02d:%s", i, cameraInfo.description().toLatin1().data());
        List2 << str;
    }
    //count = List2.count();
#ifndef Q_OS_WIN
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
    model2->setStringList(List2);
    ui->comboBoxCam1Port->setModel(model2);
    ui->comboBoxCam1Port->setCurrentIndex(-1);
}

DialogCamera::~DialogCamera()
{
    delete ui;
}

void DialogCamera::setExposureValue(int val)
{
    ViewMainPage* pView = theMainWindow->viewMainPage();
    if (!pView->myCamCapture)
        return;

#ifdef Q_OS_WIN
    m_nCamExposure = val;
    if (pView->myCamCapture)
        pView->myCamCapture->capture.set(CV_CAP_PROP_EXPOSURE, val-10);
#else
    int ncam1 = ui->comboBoxCam1Port->currentIndex();
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
        ctrl.value = val;
        if (-1 == v4l2_ioctl(fd,VIDIOC_S_CTRL,&ctrl)) {
            perror("setting exposure absolute");
            ::v4l2_close(fd);
            return;
        }
        qDebug() << "setExposureValue ok" << val;
    } catch (cv::Exception &e) {
        qDebug() << "e = " << e.what();
    } catch (...) {
        qFatal("Error <unknown> CamCapture::run()");
    }

    ::v4l2_close(fd);

    m_nCamExposure = val;

#endif

}

int DialogCamera::getExposureValue()
{
    int val = 0;
    ViewMainPage* pView = theMainWindow->viewMainPage();
    if (!pView->myCamCapture)
        return -1;

#ifdef Q_OS_WIN
    val = pView->myCamCapture->capture.get(CV_CAP_PROP_EXPOSURE) + 13;
#else
    int ncam1 = ui->comboBoxCam1Port->currentIndex();
    char video[64];
    struct v4l2_queryctrl qctrl;
    sprintf(video, "/dev/video%d", ncam1-1);
    int fd = ::v4l2_open(video,O_RDWR);
    if (fd == -1) {
        perror("opening video device");
        return -1;
    }

    qctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    qctrl.type = V4L2_EXPOSURE_MANUAL;
    if (-1 == v4l2_ioctl(fd,VIDIOC_QUERYCTRL,&qctrl)) {
        perror("getting exposure absolute");
        ::v4l2_close(fd);
        return -1;
    }

    ::v4l2_close(fd);
    val = qctrl.default_value;
#endif
    return val;
}

void DialogCamera::on_buttonOpenCamera_clicked()
{
    int ncam1 = ui->comboBoxCam1Port->currentIndex();

    ViewMainPage* pView = theMainWindow->viewMainPage();
    QString str;
    pView->CloseCam(0);
    str = ui->comboBoxCam1Port->currentText();
    if (str == "None" || str == "")
        m_sCamera1 = "";
    else {
        m_sCamera1 = str;
        pView->OpenCam(0, ncam1-1);
    }

    m_nCamExposure = getExposureValue();
    ui->sliderExposure->setValue(m_nCamExposure);
}

void DialogCamera::on_pushButtonClose_clicked()
{
    hide();
}

void DialogCamera::on_buttonCloseCamera_clicked()
{
    ViewMainPage* pView = theMainWindow->viewMainPage();
    pView->CloseCam(0);
}
