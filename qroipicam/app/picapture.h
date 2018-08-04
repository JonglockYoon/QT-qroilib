#ifndef CAMCAPTURE_H
#define CAMCAPTURE_H

//#define PICAM 1

#include <QtCore>
#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#ifdef PICAM
#include <raspicam/raspicam_cv.h>
#endif
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <qroilib/documentview/documentview.h>

#define MAX_CAMERA_WIDTH 640 //1280//640 // 1600 //640
#define MAX_CAMERA_HEIGHT 480 //1024//480 //1200 //480
//#define MAX_CAMERA_WIDTH 2592
//#define MAX_CAMERA_HEIGHT 1944

class CamCapture : public QThread
{
    Q_OBJECT
public:
    cv::Mat RGBframe;
    QObject *mParent;
#ifdef PICAM
    raspicam::RaspiCam_Cv capture;
#else
    cv::VideoCapture capture;
#endif

private:
    QImage img;
    //char buff[MAX_CAMERA_WIDTH*MAX_CAMERA_HEIGHT*4];

    bool stop;
    QMutex mutex;
    QWaitCondition condition;
    cv::Mat frame;
    cv::Mat frame1;
    float frameRate;
    bool _flipImage;
    int seq;    // ch - DocumentView[]
    int mnCam;  // camera number
signals:
    //Signal to output frame to be displayed
    void processedImage(const QImage &image, int seq);
protected:
    void run();
public:
    //Constructor
    CamCapture(QObject *parent, int nNv, int nCam);
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
