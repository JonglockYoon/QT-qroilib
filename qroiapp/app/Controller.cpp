

#include "Controller.h"
#include "ImageBuffer.h"

// Qt header files
#include <QtGui>

using namespace cv;

Controller::Controller(int windowNr)
{
    windowNumber = windowNr;
    processingThread = nullptr;
    captureThread = nullptr;
    imageBuffer = nullptr;
} // Controller constructor

Controller::~Controller()
{
} // Controller destructor

bool Controller::connectToCamera(int deviceNr, int imageBufferSize, bool dropFrame, int capThreadPrio, int procThreadPrio)
{
    deviceNumber = deviceNr;
    // Local variables
    bool isOpened=false;
    // Store imageBufferSize in private member
    this->imageBufferSize=imageBufferSize;
    // Create image buffer with user-defined settings
    imageBuffer = new ImageBuffer(imageBufferSize,dropFrame);
    // Create capture thread
    captureThread = new CaptureThread(imageBuffer);
    // Attempt to connect to camera
    if((isOpened=captureThread->connectToCamera(deviceNumber)))
    {
        // Create processing thread
        //captureThread->getInputSourceWidth(),captureThread->getInputSourceHeight()
        processingThread = new ProcessingThread(windowNumber,imageBuffer);
        // Start capturing frames from camera
        captureThread->start((QThread::Priority)capThreadPrio);
        // Start processing captured frames
        processingThread->start((QThread::Priority)procThreadPrio);
    }
    else
    {
        // Delete capture thread
        deleteCaptureThread();
        // Delete image buffer
        deleteImageBuffer();
    }
    // Return camera open result
    return isOpened;
} // connectToCamera()

void Controller::disconnectCamera()
{
    if (processingThread == nullptr)
        return;
    // Stop processing thread
    if(processingThread->isRunning())
        stopProcessingThread();
    // Stop capture thread
    if(captureThread->isRunning())
        stopCaptureThread();
    // Clear image buffer
    clearImageBuffer();
    // Disconnect camera
    captureThread->disconnectCamera();
    // Delete threads
    deleteCaptureThread();
    deleteProcessingThread();
    // Delete image buffer
    deleteImageBuffer();
} // disconnectCamera()

void Controller::stopCaptureThread()
{
    qDebug() << "About to stop capture thread...";
    captureThread->stopCaptureThread();
    // Take one frame off a FULL queue to allow the capture thread to finish
    if(imageBuffer->getSizeOfImageBuffer()==imageBufferSize)
        Mat temp=imageBuffer->getFrame();
    captureThread->wait();
    qDebug() << "Capture thread successfully stopped.";
} // stopCaptureThread()

void Controller::stopProcessingThread()
{
    qDebug() << "About to stop processing thread...";
    processingThread->stopProcessingThread();
    processingThread->wait();
    qDebug() << "Processing thread successfully stopped.";
} // stopProcessingThread()

void Controller::deleteCaptureThread()
{
    // Delete thread
    delete captureThread;
} // deleteCaptureThread()

void Controller::deleteProcessingThread()
{
    // Delete thread
    delete processingThread;
} // deleteProcessingThread()

void Controller::clearImageBuffer()
{
    imageBuffer->clearBuffer();
} // clearImageBuffer()

void Controller::deleteImageBuffer()
{
    // Delete image buffer
    delete imageBuffer;
} // deleteImageBuffer()
