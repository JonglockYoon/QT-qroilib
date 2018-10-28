

#ifndef MATTOQIMAGE_H
#define MATTOQIMAGE_H

// Qt header files
#include <QtGui>
#include <QDebug>
// OpenCV header files
#include <opencv/cv.h>
#include <opencv/highgui.h>

QImage MatToQImage(const cv::Mat&);

#endif // MATTOQIMAGE_H
