#pragma once

//#include <roilib_export.h>

#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QVector>

#include "imgprocbase.h"
#include "recipedata.h"
#include "voronoithinner.h"


typedef struct {
    cv::Point2d center;
    double radius;
    double cPerc;
} RANSACIRCLE;


using namespace std;
using namespace cv;
using namespace Qroilib;

class CImgProcEngine : public CImgProcBase
{
public:
    CImgProcEngine();
    ~CImgProcEngine();


public:
    int InspectOneItem(IplImage* img, Qroilib::RoiObject *pData);

    bool SetROIAreaForCriteriaPosition(Qroilib::RoiObject *pData, QString strCriteriaROI);

    double SingleROIEdgeWithThreshold(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    double SingleROISubPixelEdge(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICorner(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);

    int EdgeCorner(Qroilib::RoiObject *pData, IplImage* graySearchImgIn, int CornerType, CvPoint2D32f &outCorner);
    int EdgeCornerByLine(Qroilib::RoiObject *pData, IplImage* graySearchImgIn, int CornerType, CvPoint2D32f &corner);

    Qroilib::RoiObject *GetCriteriaROI(Qroilib::RoiObject *pData, QString title);

    void SaveOutImage(IplImage* pImgOut, Qroilib::RoiObject *pData, QString strMsg, bool bClear = false);

    void Smooth(Qroilib::RoiObject *pData, IplImage* ImgIn, int iImgID);

    int Threshold(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);
    int ThresholdRange(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);
    double ThresholdOTSU(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);

    int AdaptiveThreshold(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg);
    int NoiseOut(Qroilib::RoiObject *pData, IplImage* grayImg, int t = -1, int nDbg = 100, int h = -1);
    int PostNoiseOut(Qroilib::RoiObject *pData, IplImage* grayImg, int t = -1, int nDbg = 100, int h = -1);
    int Expansion(Qroilib::RoiObject *pData, IplImage* grayImg, int t = -1, int nDbg = 150, int h = -1);
    int PostExpansion(Qroilib::RoiObject *pData, IplImage* grayImg, int t = -1, int nDbg = 150, int h = -1);


    void DrawResultCrossMark(IplImage *iplImage, Qroilib::RoiObject *pData);

private:
    IplImage* curImg;
    bool m_bSaveEngineImg;
    VoronoiThinner thinner;

public:
    QString m_sDebugPath;
    Qroilib::DetectResult m_DetectResult;
    QVector<CvPoint2D32f>	alignpt;
    QVector<CvPoint2D32f>	insppt;

    int AdaptiveThreshold(IplImage* grayImg, IplImage* outImg);
    int MakeMask(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg);

    int ColorCheck(IplImage *mask, IplImage *colorImg, QString &str);
    int MeasureBySubpixelEdge(Qroilib::RoiObject *pData, IplImage* grayImg, CvPoint2D32f &pt1, CvPoint2D32f &pt2);
    int MakeMaskUsingCannyEdge(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg);
    int MeasureLCDPixelSize(Qroilib::RoiObject *pData, IplImage* iplImg, IplImage *colorImg);
};

