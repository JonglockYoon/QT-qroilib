//
// camcapture.cpp
//
//QROILIB : QT Vision ROI Library
//Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
//
#include <roilib_export.h>

#include "camcapture.h"
#include "mainwindow.h"
#include <QImage>
#include <QPixmap>


#ifdef Q_OS_WIN
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#include <windows.h>
#endif

//
// fuser /dev/video0
// kill -9 xxxx
//

//using namespace cv;

CamCapture::CamCapture(QObject *parent, int nNv, int nCam) : QThread(parent)
{
    mParent = parent;
    _flipImage = false;
    stop = true;
    mnCam = nCam;
    seq= nNv;
    //bCamPause = false;
    bOpen = false;
}

CamCapture::~CamCapture()
{
    stop = true;
    wait();
    capture.release();
}

bool CamCapture::CloseCam()
{
    try {
    capture.release();
    } catch (cv::Exception &e) {
        qDebug() << "CamCapture::CloseCam(e) = " << e.what();
        return false;
    } catch (...) {
        qFatal("Error <unknown> CamCapture::CloseCam()");
        return false;
    }
    return true;
}

bool CamCapture::OpenCam()
{
    int n = mnCam;
    memset(buff, 128, sizeof(buff));
    img = QImage((const unsigned char*)buff,MAX_CAMERA_WIDTH,MAX_CAMERA_HEIGHT,QImage::Format_RGB888);
    emit processedImage(img, seq);

    //if (n == 0) n++; // test
    //else n = n + 2;
    //mnCam = -1;
    //qDebug() << " cam_n: " << mnCam;
    try {
        capture.open(n); // -1
    } catch (cv::Exception &e) {
        qDebug() << "CamCapture::OpenCam(e) = " << e.what();
    } catch (...) {
        qFatal("Error <unknown> CamCapture::OpenCam()");
    }

    //capture.open(-1);
    if (capture.isOpened())
    {
        capture.set(CV_CAP_PROP_FRAME_WIDTH, MAX_CAMERA_WIDTH);
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, MAX_CAMERA_HEIGHT);
        //capture.set(CV_CAP_PROP_FRAME_WIDTH, 2592);
        //capture.set(CV_CAP_PROP_FRAME_HEIGHT, 1944);
        capture.set(CV_CAP_PROP_AUTO_EXPOSURE, 1); // 1-manual, 3-auto
/* CAP_PROP_EXPOSURE	Exposure Time
-1	640 ms -2	320 ms -3	160 ms -4	80 ms -5	40 ms -6	20 ms
-7	10 ms -8	5 ms -9	2.5 ms -10	1.25 ms -11	650 μs -12	312 μs -13	150 μs
*/
        //capture.set(CV_CAP_PROP_EXPOSURE, 10000);

        frameRate = (int) capture.get(CV_CAP_PROP_FPS);
        if (frameRate == 0)
            frameRate = 10;
        bOpen = true;
        //frameRate = 5; // test
        qDebug() << "OpenCam succ" << mnCam << frameRate;
        return true;
    }
    else {
        qDebug() << "OpenCam fail" << mnCam;
        return false;
    }
}

void CamCapture::Play()
{
    if (!isRunning()) {
        if (isStopped()){
            stop = false;
        }
        //start(HighestPriority);
        start();
    }
}

QImage convertToGrayScale(const QImage &srcImage)
{
     // Convert to 32bit pixel format
     QImage dstImage = srcImage.convertToFormat(srcImage.hasAlphaChannel() ?
              QImage::Format_ARGB32 : QImage::Format_RGB32);

     unsigned int *data = (unsigned int*)dstImage.bits();
     int pixelCount = dstImage.width() * dstImage.height();

     // Convert each pixel to grayscale
     for(int i = 0; i < pixelCount; ++i) {
        int val = qGray(*data);
        *data = qRgba(val, val, val, qAlpha(*data));
        ++data;
     }

     return dstImage;
}

void CamCapture::run()
{
    QThread::msleep(1000);

    int delay = (1000/frameRate);
    while(!stop){

        mutex.lock();

        try {
            if (!capture.read(frame))
                stop = true;
            else {
                if (frame.channels()== 3){
                    cv::cvtColor(frame, RGBframe, CV_BGR2RGB);
                    img = QImage((const unsigned char*)(RGBframe.data),
                        RGBframe.cols,RGBframe.rows,QImage::Format_RGB888);
                    //img = convertToGrayScale(img);
                }
                else
                {
                    cv::cvtColor(frame1, RGBframe, CV_GRAY2RGB);
                    img = QImage((const unsigned char*)(RGBframe.data),
                        RGBframe.cols,RGBframe.rows,QImage::Format_RGB888);
                }
                emit processedImage(img, seq);
            }
        } catch (cv::Exception &e) {
            qDebug() << "e = " << e.what();
        } catch (...) {
            qFatal("Error <unknown> CamCapture::run()");
        }

        mutex.unlock();


        QThread::msleep(delay);
    }
}

void CamCapture::Stop()
{
    stop = true;
}

bool CamCapture::isStopped() const{
    return this->stop;
}
