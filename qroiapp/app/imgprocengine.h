#pragma once

//#include <roilib_export.h>

#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QVector>

#include "imgprocbase.h"
#include "recipedata.h"
#include "voronoithinner.h"
#include <opencv2/xfeatures2d/nonfree.hpp>

namespace tesseract {
class TessBaseAPI;
}

typedef struct {
    cv::Point2d center;
    double radius;
    double cPerc;
} RANSACIRCLE;


struct SURFDetector
{
    cv::Ptr<cv::Feature2D> surf;
    SURFDetector(double hessian = 800.0)
    {
        surf = cv::xfeatures2d::SURF::create(hessian);
    }
    template<class T>
    void operator()(const T& in, const T& mask, std::vector<cv::KeyPoint>& pts, T& descriptors, bool useProvided = false)
    {
        surf->detectAndCompute(in, mask, pts, descriptors, useProvided);
    }
};
struct SIFTDetector
{
    cv::Ptr<cv::Feature2D> sift;
    SIFTDetector(int nFeatures = 0)
    {
        sift = cv::xfeatures2d::SIFT::create(nFeatures);
    }
    template<class T>
    void operator()(const T& in, const T& mask, std::vector<cv::KeyPoint>& pts, T& descriptors, bool useProvided = false)
    {
        sift->detectAndCompute(in, mask, pts, descriptors, useProvided);
    }
};
struct ORBDetector
{
    cv::Ptr<cv::Feature2D> orb;
    ORBDetector(int nFeatures = 500)
    {
        orb = cv::ORB::create(nFeatures);
    }
    template<class T>
    void operator()(const T& in, const T& mask, std::vector<cv::KeyPoint>& pts, T& descriptors, bool useProvided = false)
    {
        orb->detectAndCompute(in, mask, pts, descriptors, useProvided);
    }
};

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
    int TowPointAlignImage(IplImage* gray);
    int MeasureAlignImage(IplImage* src);
    int InspectOneItem(IplImage* img, Qroilib::RoiObject *pData);

    int OneMatchShapes(vector<vector<Point> >& contours, vector<vector<Point> >& templateseq, Qroilib::RoiObject *pData, int seq);
    typedef struct {
        cv::Point2f first;
        cv::Point2f center;
        cv::Point2f second;

        cv::Point2f p1;
        cv::Point2f p2;
        int len;
    } ElemLineIt;
    void MakeOneElemLine(Point cen, double dAngle, ElemLineIt &elem);
    int AppendOneLine(cv::Mat& mat, vector<ElemLineIt> &vecLineIt, ElemLineIt e, int interval, double dAngle);
    int OneLineMeasurement(cv::Mat& mat, vector<Point>& cone, Qroilib::RoiObject *pData, vector<ElemLineIt> &vecLineIt);

    bool SetROIAreaForCriteriaPosition(Qroilib::RoiObject *pData, QString strCriteriaROI);

    int SingleROICenterOfPlusMark(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rectIn);
    double SingleROISubPixEdgeWithThreshold(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICircleWithThreshold(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICircleWithEdge(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SinglePattIdentify(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SinglePattMatchShapes(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SinglePattFeatureMatch(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIFindShape(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROISubPixEdge(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICorner(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIOCR(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIBarCode(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROILineMeasurement(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIColorMatching(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);

    double HistEMD(cv::Mat& hist, cv::Mat& target, int dims);

    cv::Mat drawGoodMatches(
        const cv::Mat& img1,
        const cv::Mat& img2,
        const std::vector<cv::KeyPoint>& keypoints1,
        const std::vector<cv::KeyPoint>& keypoints2,
        std::vector<cv::DMatch>& matches,
        std::vector<cv::Point2f>& scene_corners_
    );

    int EdgeCorner(Qroilib::RoiObject *pData, IplImage* graySearchImgIn, int CornerType, CvPoint2D32f &outCorner);
    int EdgeCornerByLine(Qroilib::RoiObject *pData, IplImage* graySearchImgIn, int CornerType, CvPoint2D32f &corner);

    Qroilib::RoiObject *GetCriteriaROI(Qroilib::RoiObject *pData, QString title);

    //int find_threshold(IplImage* image);
    void Thinner(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 220);

    void SaveOutImage(IplImage* pImgOut, Qroilib::RoiObject *pData, QString strMsg, bool bClear = false);
    void SaveOutImage(cv::Mat imgOut, Qroilib::RoiObject *pData, QString strMsg);

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
    int NoiseOut(Qroilib::RoiObject *pData, Mat grayImg, int t = -1, int nDbg = 100, int h = -1);
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

