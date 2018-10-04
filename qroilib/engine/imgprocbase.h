#ifndef CIMGPROCBASE_H
#define CIMGPROCBASE_H

#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QVector>

#include "blob.h"
#include "blobresult.h"
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "edgessubpix.h"

using namespace std;
//using namespace cv;

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

struct FocusState
{
    double step;
    int direction;
    double minFocusStep;
    double rate;
    double rateMax;
};

namespace Qroilib
{

class ROIDSHARED_EXPORT CImgProcBase
{
public:
    CImgProcBase();
    ~CImgProcBase();

public:
    void CopyImageROI(IplImage* pImgIn, IplImage* pImgOut, CvRect rect);



protected:
    //반지름을 계산 : 원둘레 = 2 x PI x 반지름.
    double CalculRadiusFromCircumference(double dCircumValue)
    {
        return (dCircumValue / (2 * PI));
    }

    cv::Point2f getCorner(std::vector<cv::Point2f>& corners, cv::Point2f center, int CornerType);
    double getObjectAngle(IplImage *src);
    double GetDistance2D(CvPoint p1, CvPoint p2);

public:
    cv::Point2f CenterOfMoment(CvSeq* c);

    enum eShiftDirection{
        ShiftUp = 1, ShiftRight, ShiftDown, ShiftLeft
    };

    cv::Mat shiftFrame(cv::Mat frame, int pixels, eShiftDirection direction);

    bool checkCross(const cv::Point& AP1, const cv::Point& AP2, const cv::Point& BP1, const cv::Point& BP2, cv::Point* IP);
    bool getIntersectionPoint(cv::Point a1, cv::Point a2, cv::Point b1, cv::Point b2, cv::Point & intPnt);
    double Dist2LineSegment(double px, double py, double X1, double Y1, double X2, double Y2, double &nearX, double &nearY);

    void bhm_line(int x1, int y1, int x2, int y2, std::vector<cv::Point>* points);
    int GetAngleABC(cv::Point a, cv::Point b, cv::Point c);
    int find_thresholdOTSU(IplImage* image);
    double calcAngle(cv::Point pt1, cv::Point pt2, cv::Point pt0);


    int SubPixelHessianEigenEdge(IplImage *src, vector<Contour> &contours);
    int SubPixelCorner(IplImage *src, vector<cv::Point2f> &points);
    double SubPixelRampEdgeImage(IplImage* edgeImage, int nDir);
    double SubPixelRampEdge(unsigned char *fx, int pCnt);


    void AffineTransform(std::vector<cv::Point2f> vec, cv::Point2f srcTri[], cv::Point2f dstTri[]);
    void WarpPerspectiveImage(IplImage* img, CvPoint2D32f srcTri[], CvPoint2D32f dstTri[]);
    void WarpAffineImage(IplImage* img, CvPoint2D32f srcTri[], CvPoint2D32f dstTri[]);
    void RotateImage(IplImage* img, double angle, CvPoint2D32f center, double scale);
    void RotateImage(IplImage* img, double angle, CvPoint2D32f center);
    void RotateImage(IplImage* img, double angle);


    bool checkForBurryImage(cv::Mat matImage);
    double rateFrame(cv::Mat frame);
    double correctFocus(FocusState & state, double rate);

    cv::Point2f FindCenterOfBlob(IplImage * pImage);
    void FilterLargeArea(IplImage* grayImg);
    void FilterLargeDiameter(IplImage* grayImg);

    int FilterBlobBoundingBoxLength(IplImage* grayImg, int nMinLength, int nMaxLength);
    int FilterBlobBoundingBoxXLength(IplImage* grayImg, int nMinLength, int nMaxLength);
    int FilterBlobBoundingBoxYLength(IplImage* grayImg, int nMinLength, int nMaxLength);
    int FilterBlobLength(IplImage* grayImg, int nMinLength, int nMaxLength);
    int FilterIncludeLargeBlob(IplImage* grayImg);
    int EraseLargeBlob(IplImage* grayImg);

    int FilterLargeBlob(IplImage* grayImg, int nAxisLength);
    int IncludeRadiusBlob(IplImage* grayImg, int nMinCircleRadius, int nMaxCircleRadius);

    double ROIPixEdge(IplImage* croppedImage, int nDir, double dRejectLow, double dRejectHigh);
private:
    double isCross(cv::Point v1, cv::Point v2);

};

}  // namespace

#endif // CIMGPROCBASE_H
