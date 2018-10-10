//
// common.cpp
//
//QROILIB : QT Vision ROI Library
//Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
//
#include "common.h"
#include <QSettings>
#include <QTextCodec>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

//using namespace cv;

void Tabs2Spaces(char *pBuff)
{
#define TABS 4
    char c;
    int cnt = 0;
    char newline[2048];
    int i = 0;
    while ((c = pBuff[i]) != '\0')
    {
        if (c == '\t') {
            do{
                newline[cnt] = ' ';
                ++cnt;
            } while (cnt % TABS);  //find the Tab spot by modulus. Clean
            //division (no remainder) defines a Tab spot.
        }
        else {
            newline[cnt] = c;
            ++cnt;
        }
        i++;
        if (cnt>=2048)
            break;
    }
    newline[cnt] = '\0';
    strcpy(pBuff, newline);
}

string string_format(const std::string fmt, ...)
{
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    string str="";
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    return str;
}

void GetPrivateProfileString(string sectionName, char *pTitle, char *Default, char *szData, int nLen, string pathname)
{
    QSettings* settings = new QSettings(pathname.c_str(), QSettings::IniFormat);
    settings->beginGroup(QLatin1String(sectionName.c_str()));

    const QString val = settings->value(QLatin1String(pTitle), QString(Default)).toString();
    strcpy(szData, val.toStdString().c_str());//.toLatin1());

    settings->endGroup();

}

int GetPrivateProfileInt(string sectionName, char *pTitle, int Default, string pathname)
{
    QSettings* settings = new QSettings(pathname.c_str(), QSettings::IniFormat);

    settings->beginGroup(QLatin1String(sectionName.c_str()));

    QVariant val = settings->value(QLatin1String(pTitle), QVariant(Default));

    settings->endGroup();
    return val.toInt();
}

void WritePrivateProfileString(string sectionName, char *pTitle, char *szData, string pathname)
{
    QSettings* settings = new QSettings(pathname.c_str(), QSettings::IniFormat);

    settings->beginGroup(sectionName.c_str());

    settings->setValue(QLatin1String(pTitle), QVariant(szData));

    settings->endGroup();
}

QString toUniString(char* str)
{
    QTextCodec * codec = QTextCodec::codecForName("eucKR");
    QString localeStr = codec->toUnicode(str);
    return localeStr;
}


QString rstrip(const QString& str)
{
  int n = str.size() - 1;
  for (; n >= 0; --n) {
    if (!str.at(n).isSpace()) {
      return str.left(n + 1);
    }
  }
  return "";
}

void qimage_to_mat(const QImage* image, cv::OutputArray out)
{

    switch(image->format()) {
        case QImage::Format_Invalid:
        {
            cv::Mat empty;
            empty.copyTo(out);
            break;
        }
        case QImage::Format_RGB32:
        {
            cv::Mat view(image->height(),image->width(),CV_8UC4,(void *)image->constBits(),image->bytesPerLine());
            view.copyTo(out);
            break;
        }
        case QImage::Format_RGB888:
        {
            cv::Mat view(image->height(),image->width(),CV_8UC3,(void *)image->constBits(),image->bytesPerLine());
            //cv::cvtColor(view, out, cv::COLOR_RGB2BGR);
            view.copyTo(out);
            break;
        }
        default:
        {
            //QImage conv = image->convertToFormat(QImage::Format_ARGB32);
            cv::Mat view(image->height(),image->width(),CV_8UC4,(void *)image->constBits(),image->bytesPerLine());
            view.copyTo(out);
            break;
        }
    }
}



void mat_to_qimage(cv::InputArray image, QImage& out)
{
    switch(image.type())
    {
        case CV_8UC4:
        {
            cv::Mat view(image.getMat());
            QImage view2(view.data, view.cols, view.rows, view.step[0], QImage::Format_ARGB32);
            out = view2.copy();
            break;
        }
        case CV_8UC3:
        {
            cv::Mat mat;
            cvtColor(image, mat, cv::COLOR_BGR2BGRA); //COLOR_BGR2RGB doesn't behave so use RGBA
            QImage view(mat.data, mat.cols, mat.rows, mat.step[0], QImage::Format_ARGB32);
            out = view.copy();
            break;
        }
        case CV_8UC1:
        {
            cv::Mat mat;
            cvtColor(image, mat, cv::COLOR_GRAY2BGRA);
            QImage view(mat.data, mat.cols, mat.rows, mat.step[0], QImage::Format_ARGB32);
            out = view.copy();
            break;
        }
        default:
        {
            //throw invalid_argument("Image format not supported");
            break;
        }
    }
}
