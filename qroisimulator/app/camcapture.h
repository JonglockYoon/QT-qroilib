#ifndef CAMCAPTURE_H
#define CAMCAPTURE_H

#include <QtCore>
#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <qroilib/documentview/documentview.h>

class CamCapture : public QThread
{
    Q_OBJECT
public:
    cv::Mat RGBframe;
    QObject *mParent;
    cv::VideoCapture capture;

private:
    QImage img;
    char buff[MAX_CAMERA_WIDTH*MAX_CAMERA_HEIGHT*4];

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
