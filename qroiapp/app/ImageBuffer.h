

#ifndef IMAGEBUFFER_H
#define IMAGEBUFFER_H


// FPS statistics queue lengths
#define DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH 32
#define DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH 32
#define DEFAULT_CAP_THREAD_PRIO QThread::NormalPriority
#define DEFAULT_PROC_THREAD_PRIO QThread::NormalPriority //QThread::HighPriority

// Qt header files
#include <QMutex>
#include <QQueue>
#include <QSemaphore>
// OpenCV header files
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class ImageBuffer
{

public:
    ImageBuffer(int size, bool dropFrame);
    void addFrame(const cv::Mat& frame);
    cv::Mat getFrame();
    void clearBuffer();
    int getSizeOfImageBuffer();
private:
    QMutex imageQueueProtect;
    QQueue<cv::Mat> imageQueue;
    QSemaphore *freeSlots;
    QSemaphore *usedSlots;
    QSemaphore *clearBuffer1;
    QSemaphore *clearBuffer2;
    int bufferSize;
    bool dropFrame;
};

#endif // IMAGEBUFFER_H
