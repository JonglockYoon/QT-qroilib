
#ifndef PROCESSINGTHREAD_H
#define PROCESSINGTHREAD_H

// Qt header files
#include <QThread>
#include <QtGui>
// OpenCV header files
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class ImageBuffer;

class ProcessingThread : public QThread
{
    Q_OBJECT

public:
    ProcessingThread(int windowNr,ImageBuffer *imageBuffer);
    ~ProcessingThread();
    void stopProcessingThread();
    int getAvgFPS();
    int getCurrentSizeOfBuffer();

private:
    void updateFPS(int);

    ImageBuffer *imageBuffer;
    volatile bool stopped;
    int currentSizeOfBuffer;
    cv::Mat currentFrame;
    cv::Mat currentFrameGrayscale;

    QImage frame;
    QTime t;
    int processingTime;
    QQueue<int> fps;
    int fpsSum;
    int sampleNo;
    int avgFPS;
    QMutex stoppedMutex;
    QMutex updateMembersMutex;
    cv::Size frameSize;
    cv::Point framePoint;

    int windowNumber;
protected:
    void run();

signals:
    void newFrame(const QImage &frame, int windowNumber);
};

#endif // PROCESSINGTHREAD_H
