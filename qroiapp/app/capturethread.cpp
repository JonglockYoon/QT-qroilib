

#include "capturethread.h"
#include "imagebuffer.h"

// Qt header files
#include <QDebug>
// Configuration header file
#include "config.h"

CaptureThread::CaptureThread(ImageBuffer *imageBuffer) : QThread(),
                             imageBuffer(imageBuffer)
{
    // Initialize variables
    stopped=false;
    sampleNo=0;
    fpsSum=0;
    avgFPS=0;
    fps.clear();
} // CaptureThread constructor

void CaptureThread::run()
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

        if (bCamPause) {
            msleep(1);
            continue;
        }


        /////////////////////////////////
        // Save capture time
        captureTime=t.elapsed();
        // Start timer (used to calculate capture rate)
        t.start();
        // Capture and add frame to buffer
        cap>>grabbedFrame;
        imageBuffer->addFrame(grabbedFrame);
        // Update statistics
        updateFPS(captureTime);
    }
    qDebug() << "Stopping capture thread...";
} // run()

bool CaptureThread::connectToCamera(int deviceNumber)
{
    // Open camera and return result
    return cap.open(deviceNumber);
} // connectToCamera()

void CaptureThread::disconnectCamera()
{
    // Check if camera is connected
    if(cap.isOpened())
    {
        // Disconnect camera
        cap.release();
        qDebug() << "Camera successfully disconnected.";
    }
} // disconnectCamera()

void CaptureThread::updateFPS(int timeElapsed)
{
    // Add instantaneous FPS value to queue
    if(timeElapsed>0)
    {
        fps.enqueue((int)1000/timeElapsed);
        // Increment sample number
        sampleNo++;
    }
    // Maximum size of queue is DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH
    if(fps.size()>DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH)
        fps.dequeue();
    // Update FPS value every DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH samples
    if((fps.size()==DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH)&&(sampleNo==DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH))
    {
        // Empty queue and store sum
        while(!fps.empty())
            fpsSum+=fps.dequeue();
        // Calculate average FPS
        avgFPS=fpsSum/DEFAULT_CAPTURE_FPS_STAT_QUEUE_LENGTH;
        // Reset sum
        fpsSum=0;
        // Reset sample number
        sampleNo=0;
    }
} // updateFPS()

void CaptureThread::stopCaptureThread()
{
    stoppedMutex.lock();
    stopped=true;
    stoppedMutex.unlock();
} // stopCaptureThread()

int CaptureThread::getAvgFPS()
{
    return avgFPS;
} // getAvgFPS()

bool CaptureThread::isCameraConnected()
{
    if(cap.isOpened())
        return true;
    else
        return false;
} // isCameraConnected()

int CaptureThread::getInputSourceWidth()
{
    return cap.get(CV_CAP_PROP_FRAME_WIDTH);
} // getInputSourceWidth()

int CaptureThread::getInputSourceHeight()
{
    return cap.get(CV_CAP_PROP_FRAME_HEIGHT);
} // getInputSourceHeight()
