//
// Copyright (C) 2018
// All rights reserved by jerry1455@gmail.com
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

#include "dialogconfig.h"
#include "ui_dialogconfig.h"
#include "qextserialenumerator.h"
#include "mainwindow.h"
#include "config.h"
#include "controller.h"
#include "capturethread.h"

DialogConfig::DialogConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfig)
{
    ui->setupUi(this);

    model2 = new QStringListModel(this);

#ifdef Q_OS_WIN
    int val = gCfg.m_nCamExposure;//getExposureValue(0);
    ui->sliderExposure->setRange(0, 9);
    ui->sliderExposure->setSingleStep(1);
    setExposureValue(val);
    ui->sliderExposure->setValue(val);
#else
    ui->sliderExposure->setRange(10, 30000);
    ui->sliderExposure->setSingleStep(100);
    int val = gCfg.m_nCamExposure;//getExposureValue(0);
    setExposureValue(val);
    ui->sliderExposure->setValue(val);
#endif

    enableAutoExposure(false);

    if (gCfg.m_nCamNumber < 2)
    {
        ui->label_4->hide();
        ui->comboBoxCam2Port->hide();
    }

    SetData();

    connect(ui->sliderExposure, SIGNAL(valueChanged(int)), this, SLOT(setExposureValue(int)));

    this->setWindowTitle("Config");

}

DialogConfig::~DialogConfig()
{
    gCfg.ReadConfig();
    delete ui;
}

void DialogConfig::enableAutoExposure(bool on)
{
    ViewMainPage* pView = theMainWindow->viewMainPage();
    int seq = 0;
    while (true) {
        if (seq >= gCfg.m_nCamNumber)
            break;

        if (!pView->myCamController[seq]) {
            seq++;
            continue;
        }

        theMainWindow->SetCameraPause(seq, true);

#ifdef Q_OS_WIN
        Controller* pController = pView->myCamController[seq];
        if (pController->captureThread == nullptr)
            return;
        if (on)
            pController->captureThread->cap.set(CV_CAP_PROP_AUTO_EXPOSURE, 3); // 1-manual, 3-auto
        else
            pController->captureThread->cap.set(CV_CAP_PROP_AUTO_EXPOSURE, 1); // 1-manual, 3-auto
#else
		char video[64];
        struct v4l2_control ctrl;
        sprintf(video, "/dev/video%d", seq);
		int fd = ::v4l2_open(video,O_RDWR);
		if (fd == -1) {
			perror("opening video device");
            theMainWindow->SetCameraPause(seq, false);
		    seq++;
			continue;
		}

		ctrl.id = V4L2_CID_EXPOSURE_AUTO;
        if (on)
            ctrl.value = V4L2_EXPOSURE_AUTO;
        else
            ctrl.value = V4L2_EXPOSURE_MANUAL;

        try {
            if (-1 == v4l2_ioctl(fd,VIDIOC_S_CTRL,&ctrl)) {
                perror("setting exposure auto");
                ::v4l2_close(fd);
                theMainWindow->SetCameraPause(seq, false);
                seq++;
                continue;
            }
        } catch (cv::Exception &e) {
            qDebug() << "e = " << e.what();
        } catch (...) {
            qFatal("Error <unknown> CamCapture::run()");
        }

		::v4l2_close(fd);
#endif
        theMainWindow->SetCameraPause(seq, false);
        seq++;
    }
}

void DialogConfig::setExposureValue(int val)
{
    ViewMainPage* pView = theMainWindow->viewMainPage();
    int seq = 0;
    qDebug() << "setExposureValue" << val;
    while (true) {
        if (seq >= gCfg.m_nCamNumber)
            break;

        if (!pView->myCamController[seq]) {
            seq++;
            continue;
        }


        theMainWindow->SetCameraPause(seq, true);
#ifdef Q_OS_WIN
        gCfg.m_nCamExposure = val;
        Controller* pController = pView->myCamController[seq];
        if (pController->captureThread)
            pController->captureThread->cap.set(CV_CAP_PROP_EXPOSURE, val-10);
#else
        char video[64];
		struct v4l2_control ctrl;
		sprintf(video, "/dev/video%d", seq);
		int fd = ::v4l2_open(video,O_RDWR);
		if (fd == -1) {
			perror("opening video device");
            theMainWindow->SetCameraPause(seq, false);
		    seq++;
			continue;
		}

        try {
            ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
            ctrl.value = val;
            if (-1 == v4l2_ioctl(fd,VIDIOC_S_CTRL,&ctrl)) {
                perror("setting exposure absolute");
                ::v4l2_close(fd);
                theMainWindow->SetCameraPause(seq, false);
                seq++;
                continue;
            }
    		qDebug() << "setExposureValue ok" << val;
        } catch (cv::Exception &e) {
            qDebug() << "e = " << e.what();
        } catch (...) {
            qFatal("Error <unknown> CamCapture::run()");
        }

		::v4l2_close(fd);

        gCfg.m_nCamExposure = val;

#endif
        theMainWindow->SetCameraPause(seq, false);
        seq++;
    }
}

int DialogConfig::getExposureValue(int seq)
{
    int val = 0;
    ViewMainPage* pView = theMainWindow->viewMainPage();

    if (!pView->myCamController[seq])
        return -1;

    theMainWindow->SetCameraPause(seq, true);

#ifdef Q_OS_WIN
    Controller* pController = pView->myCamController[seq];
    val = pController->captureThread->cap.get(CV_CAP_PROP_EXPOSURE) + 13;
#else
    char video[64];
    struct v4l2_queryctrl qctrl;
    sprintf(video, "/dev/video%d", seq);
    int fd = ::v4l2_open(video,O_RDWR);
    if (fd == -1) {
        perror("opening video device");
        theMainWindow->SetCameraPause(seq, false);
        return -1;
    }

    qctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
	qctrl.type = V4L2_EXPOSURE_MANUAL;
    if (-1 == v4l2_ioctl(fd,VIDIOC_QUERYCTRL,&qctrl)) {
        perror("getting exposure absolute");
        ::v4l2_close(fd);
        theMainWindow->SetCameraPause(seq, false);
        return -1;
    }

    ::v4l2_close(fd);

    //qDebug() << qctrl.id;
    //qDebug() << qctrl.type;
    //printf("%s\n", qctrl.name);
    //qDebug() << qctrl.minimum;
    //qDebug() << qctrl.maximum;
    //qDebug() << qctrl.step;
    //qDebug() << qctrl.default_value;
    val = qctrl.default_value;
	
#endif

    theMainWindow->SetCameraPause(seq, false);
    return val;
}


void DialogConfig::SetData()
{
    ///////////////////////// RS232c port setting //////////////////////////////////
    QStringList List1;
    List1 << "None";

#ifndef Q_OS_WIN // test
    {
        char tty0tty[64];
        for (int i=0; i<12; i++) {
            sprintf(tty0tty, "/dev/tnt%d", i);
            int fd = ::open(tty0tty,O_RDWR);
            if (fd != -1) {
                List1 << tty0tty;
                ::close(fd);
            }
        }
    }
#endif


    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        List1 << info.portName;

    ///////////////////////// Camera setting //////////////////////////////////
    QStringList List2;
    List2 << "None";
    int i = 0;
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
        i++;
        QString str;
        str.sprintf("%02d:%s", i, cameraInfo.description().toLatin1().data());
        List2 << str;
    }
    int count = List2.count();
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
    ui->comboBoxCam2Port->setModel(model2);

    ui->comboBoxCam1Port->setCurrentIndex(-1);
    ui->comboBoxCam2Port->setCurrentIndex(-1);

    qDebug() << gCfg.m_sCamera1;
    qDebug() << gCfg.m_sCamera2;

    count = List2.count();
    for (int i=0; i<count; i++) {
        if (List2[i] == gCfg.m_sCamera1)
            ui->comboBoxCam1Port->setCurrentIndex(i);
        else if (List2[i] == gCfg.m_sCamera2)
            ui->comboBoxCam2Port->setCurrentIndex(i);
    }

    QString str;
    str.sprintf("%.3f", gCfg.m_pCamInfo[0].dResX);
    ui->lineEditResX->setText(str);
    str.sprintf("%.3f", gCfg.m_pCamInfo[0].dResY);
    ui->lineEditResY->setText(str);

}

void DialogConfig::on_pushButtonSave_clicked()
{
    if (QMessageBox::Yes != QMessageBox::question(this, tr("   Save   "), tr("Save setting data?"), QMessageBox::Yes | QMessageBox::No))
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    ViewMainPage* pView = theMainWindow->viewMainPage();
    int ncam1 = ui->comboBoxCam1Port->currentIndex();
    int ncam2 = ui->comboBoxCam2Port->currentIndex();

    QString str;
    pView->disconnectCamera(0);
    str = ui->comboBoxCam1Port->currentText();
    if (str == "None" || str == "")
        gCfg.m_sCamera1 = "";
    else {
        gCfg.m_sCamera1 = str;
        pView->connectToCamera(0, ncam1-1);
    }

    if (gCfg.m_nCamNumber > 1)
    {
        pView->disconnectCamera(1);
        str = ui->comboBoxCam2Port->currentText();
        if (str == "None" || str == "")
            gCfg.m_sCamera2 = "";
        else {
            gCfg.m_sCamera2 = str;
            pView->connectToCamera(1, ncam2-1);
        }
    }

    gCfg.m_pCamInfo[0].dResX = ui->lineEditResX->text().toDouble();
    gCfg.m_pCamInfo[0].dResY = ui->lineEditResY->text().toDouble();

    gCfg.WriteConfig();

    QApplication::restoreOverrideCursor();

    hide();
}

void DialogConfig::on_pushButtonClose_clicked()
{
    hide();
}

