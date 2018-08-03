//
// dialogcamera.cpp
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
#include "qextserialenumerator.h"
#include "mainwindow.h"

DialogCamera::DialogCamera(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCamera)
{
    ui->setupUi(this);

    model2 = new QStringListModel(this);

#ifdef Q_OS_WIN
    int val = m_nCamExposure;//getExposureValue(0);
    ui->sliderExposure->setRange(0, 9);
    ui->sliderExposure->setSingleStep(1);
    setExposureValue(val);
    ui->sliderExposure->setValue(val);
#else
    ui->sliderExposure->setRange(10, 30000);
    ui->sliderExposure->setSingleStep(100);
    int val = m_nCamExposure;//getExposureValue(0);
    setExposureValue(val);
    ui->sliderExposure->setValue(val);
#endif

    enableAutoExposure(false);

    SetData();

    connect(ui->sliderExposure, SIGNAL(valueChanged(int)), this, SLOT(setExposureValue(int)));

    this->setWindowTitle("Camera");

}

DialogCamera::~DialogCamera()
{
    delete ui;
}

void DialogCamera::enableAutoExposure(bool on)
{
    ViewMainPage* pView = theMainWindow->viewMainPage();
    int seq = 0;
    while (true) {
        if (seq > 1)
            break;
//        Qroilib::DocumentView* v = pView->view(seq);
//        if (v == nullptr)
//            break;
        if (!pView->myCamCapture[seq]) {
            seq++;
            continue;
        }
//        if (pView->myCamCapture[seq]->bOpen == false) {
//            seq++;
//            continue;
//        }

#ifdef Q_OS_WIN
        if (on)
            pView->myCamCapture[seq]->capture.set(CV_CAP_PROP_AUTO_EXPOSURE, 3); // 1-manual, 3-auto
        else
            pView->myCamCapture[seq]->capture.set(CV_CAP_PROP_AUTO_EXPOSURE, 1); // 1-manual, 3-auto
#else
		char video[64];
        struct v4l2_control ctrl;
        sprintf(video, "/dev/video%d", seq);
		int fd = ::v4l2_open(video,O_RDWR);
		if (fd == -1) {
			perror("opening video device");
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
        seq++;
    }
}

void DialogCamera::setExposureValue(int val)
{
    ViewMainPage* pView = theMainWindow->viewMainPage();
    int seq = 0;
    qDebug() << "setExposureValue" << val;
    while (true) {
        if (seq > 1)
            break;
//        Qroilib::DocumentView* v = pView->view(seq);
//        if (v == nullptr)
//            break;
        if (!pView->myCamCapture[seq]) {
            seq++;
            continue;
        }
//        if (pView->myCamCapture[seq]->bOpen == false) {
//            seq++;
//            continue;
//        }

#ifdef Q_OS_WIN
        m_nCamExposure = val;
        if (pView->myCamCapture[seq])
            pView->myCamCapture[seq]->capture.set(CV_CAP_PROP_EXPOSURE, val-10);
#else
        //char cmd[512];
        //sprintf(cmd, "uvcdynctrl -s 'Exposure (Absolute) %d", val);
	//system(cmd);

        char video[64];
		struct v4l2_control ctrl;
		sprintf(video, "/dev/video%d", seq);
		int fd = ::v4l2_open(video,O_RDWR);
		if (fd == -1) {
			perror("opening video device");
		    seq++;
			continue;
		}

        try {
            ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
            ctrl.value = val;
            if (-1 == v4l2_ioctl(fd,VIDIOC_S_CTRL,&ctrl)) {
                perror("setting exposure absolute");
                ::v4l2_close(fd);
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

        m_nCamExposure = val;

#endif
        seq++;
    }
}

int DialogCamera::getExposureValue(int seq)
{
    int val = 0;
    ViewMainPage* pView = theMainWindow->viewMainPage();
//    Qroilib::DocumentView* v = pView->view(seq);
//    if (v == nullptr)
//        return -1;
    if (!pView->myCamCapture[seq])
        return -1;
    if (pView->myCamCapture[seq]->bOpen == false)
        return -1;


#ifdef Q_OS_WIN
    val = pView->myCamCapture[seq]->capture.get(CV_CAP_PROP_EXPOSURE) + 13;
#else
    char video[64];
    struct v4l2_queryctrl qctrl;
    sprintf(video, "/dev/video%d", seq);
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

    //qDebug() << qctrl.id;
    //qDebug() << qctrl.type;
    //printf("%s\n", qctrl.name);
    //qDebug() << qctrl.minimum;
    //qDebug() << qctrl.maximum;
    //qDebug() << qctrl.step;
    //qDebug() << qctrl.default_value;
    val = qctrl.default_value;
	
#endif
    return val;
}


void DialogCamera::SetData()
{
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

    qDebug() << m_sCamera1;
    qDebug() << m_sCamera2;

    count = List2.count();
    for (int i=0; i<count; i++) {
        if (List2[i] == m_sCamera1)
            ui->comboBoxCam1Port->setCurrentIndex(i);
        else if (List2[i] == m_sCamera2)
            ui->comboBoxCam2Port->setCurrentIndex(i);
    }

}

void DialogCamera::on_pushButtonOpen_clicked()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    ViewMainPage* pView = theMainWindow->viewMainPage();
    int ncam1 = ui->comboBoxCam1Port->currentIndex();
    int ncam2 = ui->comboBoxCam2Port->currentIndex();

    QString str;
    pView->CloseCam(0);
    str = ui->comboBoxCam1Port->currentText();
    if (str == "None" || str == "")
        m_sCamera1 = "";
    else {
        m_sCamera1 = str;
        pView->OpenCam(0, ncam1-1);
    }

    pView->CloseCam(1);
    str = ui->comboBoxCam2Port->currentText();
    if (str == "None" || str == "")
        m_sCamera2 = "";
    else {
        m_sCamera2 = str;
        pView->OpenCam(1, ncam2-1);
    }

    QApplication::restoreOverrideCursor();

    hide();
}



void DialogCamera::on_pushButtonClose_clicked()
{
    hide();
}
