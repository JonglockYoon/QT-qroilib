
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

CamCapture::CamCapture(Qroilib::DocumentView *parent, int nNv, int nCam) : QThread(parent)
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
    QThread::msleep(2000);

    int delay = (1000/frameRate);
    while(!stop){


        if (theMainWindow->bCamPause[seq])
            continue;

        mutex.lock();

        try {
            if (!capture.read(frame))
                stop = true;
            else {
                if (frame.channels()== 3){
                    cv::cvtColor(frame, RGBframe, CV_BGR2RGB);
                    img = QImage((const unsigned char*)(RGBframe.data),
                        RGBframe.cols,RGBframe.rows,QImage::Format_RGB888);
                    img = convertToGrayScale(img);
                }
                else
                {
                    cv::cvtColor(frame, RGBframe, CV_GRAY2RGB);
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


//
// here we will convert OpenCV image to QImage, copying the pixels row-by-row
// OpenCV stores the pixel data in row-major order, in BGR format
// QImage expects RGB, so we need to swap pixel valules
//
// for the sake of simplicity, only BGR24 images are handled (3 channels, 8-bit each)
//
// easier way is to pass image->imageData buffer directly to QImage constructor
// and then use QImage::rgbSwapped(), but its good to know how to manipulate pixel
// values directly
//

//QImage CamCapture::convert( IplImage * image )
//{
//    static QImage result;
//    if( image ){
//        uchar * buffer = NULL;
//        QImage::Format format = QImage::Format_Invalid;
//        if( IPL_DEPTH_8U == image->depth && 3 == image->nChannels ){
//            // prepare image data buffer, we need enough space for WxH pixels, each pixel have 3 'uchar' values (r,g,b)
//            buffer = (uchar *) malloc(image->width * image->height * 3 * sizeof(uchar));
//            format = QImage::Format_RGB888;
//            if( !buffer ){
//                qWarning() << tr("Insufficient memory for image buffer!");
//                return result;
//            }
//            // copy image data row by row
//            // image data layout is like this (w=image width, b(i) - blue component of i-th pixel in row, g - green, r - red):
//            // row 0: [ b(0), g(0), r(0), b(1), g(1), r(1), ..., b(w-1), g(w-1), r(w-1) ]
//            // row 1: [ b(w), g(w), r(w), b(w+1), g(w+1), r(w+2), ... ]
//            // row 2: [ b(2*w), ... ]
//            // then each row is placed one after another (h=image height):
//            // image->imageData = [ row0, row1, row2, ..., row(h-1) ]
//            for( int y = 0; y < image->height; ++y ){
//                // each row pixels data starts at row*width position (if flipped, we count rows from h-1 to 0)
//                const int srcRow = _flipImage ?  y : (image->height-y-1);
//                const int dstRow = y;
//                for( int x = 0; x < image->width; ++x ){
//                    // multiply by 3, because each pixel occupies 3 entries in array
//                    const int srcPixel = (x*3)+(srcRow*image->widthStep);
//                    const int dstPixel = (x*3)+(dstRow*image->width*image->nChannels);
//                    // copy pixel value, swap r<->b channels
//                    buffer[ dstPixel + 0 ] = image->imageData[ srcPixel + 2 ];	// red
//                    buffer[ dstPixel + 1 ] = image->imageData[ srcPixel + 1 ];	// green
//                    buffer[ dstPixel + 2 ] = image->imageData[ srcPixel + 0 ];	// blue
//                }
//            }
//        } else if( IPL_DEPTH_8U == image->depth && 1 == image->nChannels ){
//            // prepare image data buffer, we need enough space for WxH pixels, each pixel have 3 'uchar' values (r,g,b)
//            buffer = (uchar *) malloc(image->width * image->height * sizeof(uchar));
//            format = QImage::Format_Mono;
//            if( !buffer ){
//                qWarning() << tr("Insufficient memory for image buffer!");
//                return result;
//            }
//            // copy image data row by row
//            for( int y = 0; y < image->height; ++y ){
//                // each row pixels data starts at row*width position (if flipped, we count rows from h-1 to 0)
//                const int srcRow = _flipImage ?  y : (image->height-y-1);
//                const int dstRow = y;
//                for( int x = 0; x < image->width; ++x ){
//                    const int srcPixel = x+(srcRow*image->widthStep);
//                    const int dstPixel = x+(dstRow*image->width*image->nChannels);
//                    // copy pixel value
//                    buffer[ dstPixel ] = image->imageData[ srcPixel ];
//                }
//            }
//        } else{
//            qWarning() << tr("Format not supported: depth=%1, channels=%2").arg(image->depth).arg(image->nChannels);
//            return result;
//        }

//        if( buffer ){
//            // when creating QImage from buffer data, we need to make sure that
//            // buffer remains valid until image is deleted, so we make explicit copy
//            // and free the buffer immediately
//            result= QImage(reinterpret_cast<unsigned char*>(buffer), image->width, image->height, format);
//            free(buffer);
//            //result.save("filename.bmp");
//        }
//    } else{
//        qWarning() << tr("Image pointer is NULL");
//    }
//    return result;
//}

void CamCapture::Stop()
{
    stop = true;
}

void CamCapture::msleep(int ms){
#ifdef Q_OS_WIN
    Sleep(ms);
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}
bool CamCapture::isStopped() const{
    return this->stop;
}
