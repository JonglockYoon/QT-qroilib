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

#include "leftdock.h"
#include "ui_leftdock.h"
#include "roiobject.h"
#include "utils.h"
#include <qroilib/documentview/documentview.h>
#include "mainwindow.h"
#include "recipedata.h"
#include "common.h"

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


void LeftDock::setExposureValue(int val)
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

int LeftDock::getExposureValue()
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

void LeftDock::on_buttonOpenCamera_clicked()
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

void LeftDock::on_pushButton_CreateRectROI_clicked()
{
    Qroilib::DocumentView* v = theMainWindow->currentView();
    if (!v)
        return;
    v->selectTool(v->actCreateRectangle);
}

void LeftDock::on_pushButton_CreatePatternROI_clicked()
{
    Qroilib::DocumentView* v = theMainWindow->currentView();
    if (!v)
        return;
    v->selectTool(v->actCreatePattern);
}

void LeftDock::on_pushButton_ReadROI_clicked()
{    
    Qroilib::DocumentView* v;
    ViewMainPage* pMainView = theMainWindow->viewMainPage();

    int seq = 0;
    QList<const Qroilib::RoiObject*> selectedObjects;
    while (true) {
        v = pMainView->view(seq);
        if (v == nullptr)
            break;

        for (const Layer *layer : v->mRoi->objectGroups()) {
            const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
            for (const Qroilib::RoiObject *roiObject : objectGroup) {
                selectedObjects.append(roiObject);
            }
        }
        if (selectedObjects.size() > 0) {
            v->setSelectedObjects((const QList<RoiObject *> &)selectedObjects);
            v->delete_();
        }
        selectedObjects.clear();
        seq++;
    }

    g_cRecipeData->LoadRecipeData();

    seq = 0;
    while (true) {
        v = pMainView->view(seq);
        if (v == nullptr)
            break;
        v->zoomActualSize();
        v->setZoomToFit(true);
        seq++;
    }
    pMainView->updateLayout();

    return;
}

void LeftDock::on_pushButton_WriteROI_clicked()
{
    g_cRecipeData->SaveRecipeData();

    return;
}

int LeftDock::PrepareImage()
{
    ViewMainPage* pMainView = theMainWindow->viewMainPage();
    DocumentView* pdocview = pMainView->currentView();
    const QImage *camimg = pdocview->image();
    if (camimg->isNull())
        return -1;

    cv::Mat frame;
    qimage_to_mat(camimg, frame);

    IplImage riplImg;
    IplImage *iplImg;
    riplImg = frame;
    iplImg = &riplImg;


    CvSize searchSize = cvSize(iplImg->width, iplImg->height);
    graySearchImg = cvCreateImage(searchSize, IPL_DEPTH_8U, 1);
    if (iplImg->nChannels == 3)
        cvCvtColor(iplImg, graySearchImg, CV_RGB2GRAY);
    else if (iplImg->nChannels == 4) {
        if (strncmp(iplImg->channelSeq, "BGRA", 4) == 0)
            cvCvtColor(iplImg, graySearchImg, CV_BGRA2GRAY);
        else
            cvCvtColor(iplImg, graySearchImg, CV_RGBA2GRAY);
    } else
        cvCopy(iplImg, graySearchImg);

    return 0;
}

//
// _Inspect_Roi_Align_TowPoint ROI 2개가 있어야한다.
//
void LeftDock::on_pushButton_Tow_Point_Align_clicked()
{
    if (PrepareImage() != 0)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    ViewMainPage* pMainView = theMainWindow->viewMainPage();
    DocumentView* pdocview = pMainView->currentView();
    pMainView->bPreview = false;

    theMainWindow->align.TowPointAlignImage(graySearchImg);

    // 결과이미지를 화면에 반영.
    cv::Mat mat = cv::cvarrToMat(graySearchImg);
    QImage img;
    mat_to_qimage(mat, img);

    if (pdocview->document()) {
        pdocview->document()->setImageInternal(img);
        pdocview->imageView()->updateBuffer();
    }

    cvReleaseImage(&graySearchImg);

    QApplication::restoreOverrideCursor();
}

//
// _Inspect_Roi_Align_Measure ROI 3개가 있어야한다.
//
void LeftDock::on_pushButton_Measure_Align_clicked()
{
    if (PrepareImage() != 0)
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    ViewMainPage* pMainView = theMainWindow->viewMainPage();
    DocumentView* pdocview = pMainView->currentView();
    pMainView->bPreview = false;

    theMainWindow->align.MeasureAlignImage(graySearchImg);

    // 결과이미지를 화면에 반영.
    cv::Mat mat = cv::cvarrToMat(graySearchImg);
    QImage img;
    mat_to_qimage(mat, img);

    if (pdocview->document()) {
        pdocview->document()->setImageInternal(img);
        pdocview->imageView()->updateBuffer();
    }

    cvReleaseImage(&graySearchImg);

    QApplication::restoreOverrideCursor();

}

void LeftDock::on_pushButton_Preview_clicked()
{
    ViewMainPage* pMainView = theMainWindow->viewMainPage();
    pMainView->bPreview = true;
}
