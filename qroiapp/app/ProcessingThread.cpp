 

#include "ImageBuffer.h"
#include "ProcessingThread.h"
#include "MatToQImage.h"

// Qt header files
#include <QDebug>
// OpenCV header files
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// Configuration header file
#include "Config.h"

using namespace cv;

ProcessingThread::ProcessingThread(int windowNr,ImageBuffer *imageBuffer) : QThread(),
                                   imageBuffer(imageBuffer)
{
    windowNumber = windowNr;
    // Initialize variables
    stopped=false;
    sampleNo=0;
    fpsSum=0;
    avgFPS=0;
    fps.clear();

} // ProcessingThread constructor

ProcessingThread::~ProcessingThread()
{
} // ProcessingThread destructor

void ProcessingThread::run()
{
    while(1)
    {
        /////////////////////////////////
        // Stop thread if stopped=TRUE //
        /////////////////////////////////
        stoppedMutex.lock();
        if (stopped)
        {
            stopped=false;
            stoppedMutex.unlock();
            break;
        }
        stoppedMutex.unlock();
        /////////////////////////////////
        /////////////////////////////////

        // Save processing time
        processingTime=t.elapsed();
        // Start timer (used to calculate processing rate)
        t.start();

        // Get frame from queue, store in currentFrame, set ROI
        currentFrame=imageBuffer->getFrame();

        updateMembersMutex.lock();

        frame=MatToQImage(currentFrame);
        updateMembersMutex.unlock();

        // Update statistics
        updateFPS(processingTime);
        currentSizeOfBuffer=imageBuffer->getSizeOfImageBuffer();
        // Inform GUI thread of new frame (QImage)
        emit newFrame(frame,windowNumber);

#ifdef QT_NO_DEBUG
        msleep(20);
#else
        msleep(100);
#endif
    }
    qDebug() << "Stopping processing thread...";
}

void ProcessingThread::updateFPS(int timeElapsed)
{
    // Add instantaneous FPS value to queue
    if(timeElapsed>0)
    {
        fps.enqueue((int)1000/timeElapsed);
        // Increment sample number
        sampleNo++;
    }
    // Maximum size of queue is DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH
    if(fps.size()>DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH)
        fps.dequeue();
    // Update FPS value every DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH samples
    if((fps.size()==DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH)&&(sampleNo==DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH))
    {
        // Empty queue and store sum
        while(!fps.empty())
            fpsSum+=fps.dequeue();
        // Calculate average FPS
        avgFPS=fpsSum/DEFAULT_PROCESSING_FPS_STAT_QUEUE_LENGTH;
        // Reset sum
        fpsSum=0;
        // Reset sample number
        sampleNo=0;
    }
} // updateFPS()

void ProcessingThread::stopProcessingThread()
{
    stoppedMutex.lock();
    stopped=true;
    stoppedMutex.unlock();
} // stopProcessingThread()

int ProcessingThread::getAvgFPS()
{
    return avgFPS;
} // getAvgFPS()

int ProcessingThread::getCurrentSizeOfBuffer()
{
    return currentSizeOfBuffer;
} // getCurrentSizeOfBuffer()
