#pragma once


#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QVector>

#include "imgprocbase.h"
#include "recipedata.h"


using namespace std;
//using namespace cv;
using namespace Qroilib;

class CImgAlign : public CImgProcBase
{
public:
    CImgAlign();
    ~CImgAlign();


public:
    int GetCornerAlignPoint(IplImage* graySearchImg);
    int TowPointAlignImage(IplImage* src);
    int MeasureAlignImage(IplImage* img);
    double SingleROISubPixEdgeWithThreshold(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICorner(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int EdgeCorner(Qroilib::RoiObject *pData, IplImage* graySearchImgIn, int CornerType, CvPoint2D32f &outCorner);
    int EdgeCornerByLine(Qroilib::RoiObject *pData, IplImage* graySearchImgIn, int CornerType, CvPoint2D32f &corner);

    int ThresholdRange(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);
    int NoiseOut(Qroilib::RoiObject *pData, IplImage* grayImg, int t = -1, int nDbg = 100, int h = -1);
    void SaveOutImage(IplImage* pImgOut, Qroilib::RoiObject *pData, QString strMsg, bool bClear = false);

public:
    DetectResult m_DetectResult;
    //QVector<CvPoint2D32f>	alignpt;
    QVector<CvPoint2D32f>	insppt;
};

