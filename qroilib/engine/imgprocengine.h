#pragma once

#include <QRectF>
#include "blob.h"
#include "blobresult.h"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "recipedata.h"
#include "edgessubpix.h"

using namespace std;
//using namespace cv;
using namespace Qroilib;

#define CVX_RED		CV_RGB(255,0,0)
#define CVX_ORANGE	CV_RGB(255,165,0)
#define CVX_YELLOW	CV_RGB(255,255,0)
#define CVX_GREEN	CV_RGB(0,255,0)
#define CVX_BLUE	CV_RGB(0,0,255)
#define CVX_PINK	CV_RGB(255,0,255)
#define CVX_BLICK	CV_RGB(0,0,0)
#define CVX_WHITE	CV_RGB(255,255,255)

#define PI 3.14159

enum EDGEDIR { BOTH, VERTICAL, HORIZONTAL, NOTHING };
const double FOCUS_STEP = 1.0; // 1mm

typedef struct {
    cv::Point2d center;
    double radius;
    double cPerc;
} RANSACIRCLE;

struct FocusState
{
    double step;
    int direction;
    double minFocusStep;
    //int lastDirectionChange;
    double rate;
    double rateMax;
};

class CImgProcEngine
{
public:
    //CWnd* m_pMainWnd;

    QString m_sDebugPath;

    Qroilib::DetectResult m_DetectResult;

public:
    void CopyImageROI(IplImage* pImgIn, IplImage* pImgOut, CvRect rect)
    {
        cvSetImageROI(pImgIn, rect);
        cvCopy(pImgIn, pImgOut);
        cvResetImageROI(pImgIn);
    }

protected:
    //전체 원 둘레의 길이를 통해 반지름을 계산 : 원둘레 = 2 x PI x 반지름.
    // 외곽선 근사화를 통해 원 둘레를 알 수 있다.
    double CalculRadiusFromCircumference(double dCircumValue)
    {
        return (dCircumValue / (2 * PI));
    }

    cv::Point2f getCorner(std::vector<cv::Point2f>& corners, cv::Point2f center, int CornerType);
    double getObjectAngle(IplImage *src);
    double GetDistance2D(CvPoint p1, CvPoint p2);
    void GetMidpoint(CvPoint p1, CvPoint p2, CvPoint *p3);
    int CenterOfGravity(Qroilib::RoiObject *pData, IplImage* croppedImageIn, CvPoint2D32f &cog, int nRetry = 0);
    void Smooth(Qroilib::RoiObject *pData, IplImage* ImgIn, int iImgID);
    double TemplateMatch(Qroilib::RoiObject *pData, IplImage* SearchImg, IplImage* TemplateImg, CvPoint &left_top, double &dMatchShapes);
    cv::Point getValueX(std::vector<cv::Point> points, int pos); // x point 평균처리
    cv::Point getValueY(std::vector<cv::Point> points, int pos); // y point 평균처리
    //외곽선 추적 : 검색된 외곽선 개수를 반환

    vector<RANSACIRCLE> vecRansicCircle;
    float verifyCircle(cv::Mat dt, cv::Point2f center, float radius, std::vector<cv::Point2f> & inlierSet);
    inline void getCircle(cv::Point2f& p1, cv::Point2f& p2, cv::Point2f& p3, cv::Point2f& center, float& radius);
    std::vector<cv::Point2f> getPointPositions(cv::Mat binaryImage);

public:
    Point2f CenterOfMoment(CvSeq* c);

    CImgProcEngine();
    ~CImgProcEngine();

    enum eShiftDirection{
        ShiftUp = 1, ShiftRight, ShiftDown, ShiftLeft
    };

    cv::Mat shiftFrame(cv::Mat frame, int pixels, eShiftDirection direction);
    int AlignImage(Qroilib::RoiObject* pData, IplImage* img);
    int InspectOneItem(IplImage* img, Qroilib::RoiObject *pData);

    bool checkCross(const cv::Point& AP1, const cv::Point& AP2, const cv::Point& BP1, const cv::Point& BP2, cv::Point* IP);
    double isCross(Point v1, Point v2);
    bool getIntersectionPoint(Point a1, Point a2, Point b1, Point b2, Point & intPnt);
    double Dist2LineSegment(double px, double py, double X1, double Y1, double X2, double Y2, double &nearX, double &nearY);
    unsigned int GetLineEq_LineToPoint(double eqline[], double point[], double nleq[], double intspoint[]);
    unsigned int GetLineEq_PointToPoint(double p1[], double p2[], double leq[]);

    int Threshold(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);
    int ThresholdRange(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);
    double ThresholdOTSU(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);

    int AdaptiveThreshold(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg);
    int NoiseOut(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);
    int PostNoiseOut(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 100);
    int Expansion(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 150);
    int PostExpansion(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 150);
    int Find_RANSAC_Circle(IplImage* grayImg, Qroilib::RoiObject *pData, int retry, int nMax = 10);

    void bhm_line(int x1, int y1, int x2, int y2, std::vector<cv::Point>* points);
    int FilterBlobBoundingBoxLength(IplImage* grayImg, Qroilib::RoiObject *pData, int nMinLength, int nMaxLength);
    int FilterBlobBoundingBoxXLength(IplImage* grayImg, Qroilib::RoiObject *pData, int nMinLength, int nMaxLength);
    int FilterBlobLength(IplImage* grayImg, Qroilib::RoiObject *pData, int nMinLength, int nMaxLength);
    int FilterIncludeLargeBlob(IplImage* grayImg, Qroilib::RoiObject *pData);
    int FilterExcludeLargeBlob(IplImage* grayImg, Qroilib::RoiObject *pData);

    int SubPixelHessianEigenEdge(IplImage *src, vector<Contour> &contours);
    int SubPixelCorner(IplImage *src, vector<Point2f> &points);
    double SubPixelRampEdgeImage(IplImage* edgeImage, Qroilib::RoiObject *pData, int nDir);
    double SubPixelRampEdge(unsigned char *fx, int pCnt, int nStep=1);
    double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0);
    Point2f CrossPointOfThinner(Qroilib::RoiObject *pData, IplImage* img, CvSeq* ptseq);
    int CenterOfPlusmarkVerify(Qroilib::RoiObject *pData, IplImage* imageIn, cv::Point2f cpt);

    bool SetROIAreaForCriteriaPosition(Qroilib::RoiObject *pData, QString strCriteriaROI);

    int BondingHoleRANSAC(IplImage* grayImg, Qroilib::RoiObject *pData, QRectF rect, int retry);
    int BondingHoleGravity(IplImage* grayImg, Qroilib::RoiObject *pData, QRectF rect, int retry);

    int SingleROICenterOfPlusMark(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rectIn);
    int SingleROIBondingCircleCheck(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rectIn);
    int SingleROIBondingRectCheck(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rectIn);
    double SingleROISubPixEdgeWithThreshold(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICircleWithThreshold(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICircleWithEdge(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SinglePattIdentify(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIBondingHole(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIScrew(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIBondingLine(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIBondingPosition(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIPinErrorCheck(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROISide3LaserPoint(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect);

    Qroilib::RoiObject *GetCriteriaROI(Qroilib::RoiObject *pData, QString title);

    //int find_threshold(IplImage* image);
    int find_thresholdOTSU(IplImage* image);
    void Thinner(Qroilib::RoiObject *pData, IplImage* grayImg, int nDbg = 220);

    void SaveOutImage(IplImage* pImgOut, Qroilib::RoiObject *pData, QString strMsg, bool bClear = false);

    void AffineTransform(std::vector<Point2f> vec, cv::Point2f srcTri[], cv::Point2f dstTri[]);
    void WarpPerspectiveImage(IplImage* img, CvPoint2D32f srcTri[], CvPoint2D32f dstTri[]);
    void WarpAffineImage(IplImage* img, CvPoint2D32f srcTri[], CvPoint2D32f dstTri[]);
    void RotateImage(IplImage* img, double angle, CvPoint2D32f center, double scale);
    void RotateImage(IplImage* img, double angle, CvPoint2D32f center);
    void RotateImage(IplImage* img, double angle);

    void DrawResultCrossMark(IplImage *iplImage, Qroilib::RoiObject *pData);

    bool checkForBurryImage(cv::Mat matImage);
    double rateFrame(Mat frame);
    double correctFocus(FocusState & state, double rate);

    int ExcludeLargeBlob(IplImage* grayImg, Qroilib::RoiObject *pData, int nAxisLength, int nDbg = 132);
    int IncludeRangeBlob(IplImage* grayImg, Qroilib::RoiObject *pData, int nMinCircleRadius, int nMaxCircleRadius);
    Point2f FindCenterOfBlob(IplImage * pImage);
    void FilterLargeArea(IplImage* grayImg);
    void FilterLargeDiameter(IplImage* grayImg);

private:
    IplImage* curImg;
    bool m_bSaveEngineImg;
};

