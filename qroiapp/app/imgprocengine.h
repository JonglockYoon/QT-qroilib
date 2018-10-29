#pragma once

//#include <roilib_export.h>

#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QVector>

#include "imgprocbase.h"
#include "recipedata.h"
#include "voronoithinner.h"

namespace tesseract {
class TessBaseAPI;
}

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
    int GetAlignPtWithMask(Qroilib::RoiObject* pData, IplImage* graySearchImg);
    int TowPointAlignImage(IplImage* src);
    int MeasureAlignImage(IplImage* img);
    int InspectOneItem(IplImage* img, Qroilib::RoiObject *pData);

    bool SetROIAreaForCriteriaPosition(Qroilib::RoiObject *pData, QString strCriteriaROI);

    int SingleROICenterOfPlusMark(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rectIn);
    double SingleROISubPixEdgeWithThreshold(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICircleWithThreshold(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICircleWithEdge(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SinglePattIdentify(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SinglePattMatchShapes(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIFindShape(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROISubPixEdge(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICorner(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIOCR(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIBarCode(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);

    int EdgeCorner(Qroilib::RoiObject *pData, IplImage* graySearchImgIn, int CornerType, CvPoint2D32f &outCorner);
    int EdgeCornerByLine(Qroilib::RoiObject *pData, IplImage* graySearchImgIn, int CornerType, CvPoint2D32f &corner);

    Qroilib::RoiObject *GetCriteriaROI(Qroilib::RoiObject *pData, QString title);

    //int find_threshold(IplImage* image);
    void Thinner(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 220);

    void SaveOutImage(IplImage* pImgOut, Qroilib::RoiObject *pData, QString strMsg, bool bClear = false);

    Point2f CrossPointOfThinner(Qroilib::RoiObject *pData, IplImage* img, CvSeq* ptseq);
    int CenterOfPlusmarkVerify(Qroilib::RoiObject *pData, IplImage* imageIn, cv::Point2f cpt);

    int CenterOfGravity(Qroilib::RoiObject *pData, IplImage* croppedImageIn, CvPoint2D32f &cog, int nRetry = 0);
    void Smooth(Qroilib::RoiObject *pData, IplImage* ImgIn, int iImgID);
    double TemplateMatch(Qroilib::RoiObject *pData, IplImage* SearchImg, IplImage* TemplateImg, CvPoint &left_top, double &dMatchShapes);

    int Threshold(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);
    int ThresholdRange(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);
    double ThresholdOTSU(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);

    int AdaptiveThreshold(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg);
    int NoiseOut(Qroilib::RoiObject *pData, IplImage* grayImg, int t = -1, int nDbg = 100, int h = -1);
    int Expansion(Qroilib::RoiObject *pData, IplImage* grayImg, int t = -1, int nDbg = 150, int h = -1);

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

public:
    int init_tess_failed = 0;
    tesseract::TessBaseAPI *tessApi;
};

