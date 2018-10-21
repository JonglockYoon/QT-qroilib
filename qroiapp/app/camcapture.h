#ifndef CAMCAPTURE_H
#define CAMCAPTURE_H

#include "roid_global.h"

#include <QtCore>
#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <qroilib/documentview/documentview.h>

using namespace Qroilib;

class Qroilib::DocumentView;

class CamCapture : public QThread
{
    Q_OBJECT
public:
    cv::Mat RGBframe;
    Qroilib::DocumentView *mParent;
    cv::VideoCapture capture;

private:
    QImage img;
    char buff[640*480*4];

    bool stop;
    QMutex mutex;
    QWaitCondition condition;
    cv::Mat frame;
    float frameRate;
    bool _flipImage;
    int seq;    // ch - DocumentView[]
    int mnCam;  // camera number
signals:
    //Signal to output frame to be displayed
    void processedImage(const QImage &image, int seq);
protected:
    void run();
    void msleep(int ms);
public:
    //Constructor
    CamCapture(Qroilib::DocumentView *parent, int nNv, int nCam);
    //Destructor
    ~CamCapture();
    bool CloseCam();
    bool OpenCam();
    void Play();

    QImage convert( IplImage * image );

    //Stop the video
    void Stop();
    //check if the player has been stopped
    bool isStopped() const;

    //bool bCamPause;
    bool bOpen;
};

#endif // CAMCAPTURE_H
