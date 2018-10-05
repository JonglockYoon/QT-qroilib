//
//QROILIB : QT Vision ROI Library
//Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
//

#include "imgprocbase.h"
using namespace cv;


namespace Qroilib
{


CImgProcBase::CImgProcBase()
{
}

CImgProcBase::~CImgProcBase()
{
}

cv::Mat CImgProcBase::shiftFrame(cv::Mat frame, int pixels, eShiftDirection direction)
{
    //create a same sized temporary Mat with all the pixels flagged as invalid (-1)
    cv::Mat temp = cv::Mat::zeros(frame.size(), frame.type());

    switch (direction)
    {
    case(ShiftUp) :
        frame(cv::Rect(0, pixels, frame.cols, frame.rows - pixels)).copyTo(temp(cv::Rect(0, 0, temp.cols, temp.rows - pixels)));
        break;
    case(ShiftRight) :
        frame(cv::Rect(0, 0, frame.cols - pixels, frame.rows)).copyTo(temp(cv::Rect(pixels, 0, frame.cols - pixels, frame.rows)));
        break;
    case(ShiftDown) :
        frame(cv::Rect(0, 0, frame.cols, frame.rows - pixels)).copyTo(temp(cv::Rect(0, pixels, frame.cols, frame.rows - pixels)));
        break;
    case(ShiftLeft) :
        frame(cv::Rect(pixels, 0, frame.cols - pixels, frame.rows)).copyTo(temp(cv::Rect(0, 0, frame.cols - pixels, frame.rows)));
        break;
    default:
        std::cout << "Shift direction is not set properly" << std::endl;
    }

    return temp;
}

void CImgProcBase::CopyImageROI(IplImage* pImgIn, IplImage* pImgOut, CvRect rect)
{
    cvSetImageROI(pImgIn, rect);
    cvCopy(pImgIn, pImgOut);
    cvResetImageROI(pImgIn);
}


int CImgProcBase::FilterBlobLength(IplImage* grayImg, int nMinLength, int nMaxLength)
{
    IplImage *pMask = cvCloneImage(grayImg);
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, nullptr);

    //double_stl_vector elong = blobs.GetSTLResult(CBlobGetLength());
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetLength(), B_LESS, nMinLength); // 작은 Length제거
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetLength(), B_GREATER, nMaxLength); // 큰 Length 제거
    cvZero(grayImg);
    int blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
        }
    }
    cvAnd(grayImg, pMask, grayImg);
    cvReleaseImage(&pMask);
    return 0;
}

int CImgProcBase::FilterBlobBoundingBoxLength(IplImage* grayImg, int nMinLength, int nMaxLength)
{
    IplImage *pMask = cvCloneImage(grayImg);
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, nullptr);
    int blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            CvRect rect = currentBlob->GetBoundingBox();

            if (rect.width > nMaxLength || rect.height > nMaxLength) {
                currentBlob->ClearContours();
            }
            else if (rect.width < nMinLength || rect.height < nMinLength) {
                currentBlob->ClearContours();
            }
        }
    }
    cvZero(grayImg);
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
        }
    }
    cvAnd(grayImg, pMask, grayImg);
    cvReleaseImage(&pMask);

    return 0;
}

//
// blob의 폭이 nMinLength nMaxLength 사이인것만 남긴다.
//
int CImgProcBase::FilterBlobBoundingBoxXLength(IplImage* grayImg, int nMinLength, int nMaxLength)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, nullptr);
    int blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            CvRect rect = currentBlob->GetBoundingBox();

            if (rect.width > nMaxLength) {
                currentBlob->ClearContours();
            }
            else if (rect.width < nMinLength) {
                currentBlob->ClearContours();
            }
        }
    }

    cvZero(grayImg);
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
        }
    }

    return 0;
}
//
// blob의 높이가 nMinLength nMaxLength 사이인것만 남긴다.
//
int CImgProcBase::FilterBlobBoundingBoxYLength(IplImage* grayImg, int nMinLength, int nMaxLength)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, nullptr);
    int blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            CvRect rect = currentBlob->GetBoundingBox();

            if (rect.height > nMaxLength) {
                currentBlob->ClearContours();
            }
            else if (rect.height < nMinLength) {
                currentBlob->ClearContours();
            }
        }
    }

    cvZero(grayImg);
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
        }
    }

    return 0;
}

int CImgProcBase::FilterIncludeLargeBlob(IplImage* grayImg)
{
    //QString str;
    //IplImage *pMask = cvCloneImage(grayImg);
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, nullptr);
    double dLargeArea = 0;

    int n = blobs.GetNumBlobs();
    if (n > 1)
    {
        double_stl_vector area = blobs.GetSTLResult(CBlobGetArea());
        //std::sort(area.begin(), area.end(), sort_using_greater_than);
        std::stable_sort(area.begin(), area.end(), [](const double lhs, const double rhs)->bool {
            return lhs > rhs;
        });
        dLargeArea = area[0];
        blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, dLargeArea);
    }

    cvZero(grayImg);
    for (int i = 0; i < blobs.GetNumBlobs(); i++) {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(grayImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
    }
    return 0;
}

int CImgProcBase::EraseLargeBlob(IplImage* grayImg)
{
    IplImage *pMask = cvCloneImage(grayImg);
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, nullptr);
    double dLargeArea = 0;

    int n = blobs.GetNumBlobs();
    if (n > 1)
    {
        double_stl_vector area = blobs.GetSTLResult(CBlobGetArea());
        //std::sort(area.begin(), area.end(), sort_using_greater_than);
        std::stable_sort(area.begin(), area.end(), [](const double lhs, const double rhs)->bool {
            return lhs > rhs;
        });
        dLargeArea = area[0];
        blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_GREATER_OR_EQUAL, dLargeArea);
    }

    cvZero(grayImg);
    for (int i = 0; i < blobs.GetNumBlobs(); i++) {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(grayImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
    }
    cvAnd(grayImg, pMask, grayImg);
    cvReleaseImage(&pMask);
    return 0;
}


//세 점이 주어질 때 사이 각도 구하기
//http://stackoverflow.com/a/3487062
int CImgProcBase::GetAngleABC(Point a, Point b, Point c)
{
    Point ab = { b.x - a.x, b.y - a.y };
    Point cb = { b.x - c.x, b.y - c.y };

    float dot = (ab.x * cb.x + ab.y * cb.y); // dot product
    float cross = (ab.x * cb.y - ab.y * cb.x); // cross product

    float alpha = atan2(cross, dot);

    return (int)floor(alpha * 180.0 / CV_PI + 0.5);
}


double CImgProcBase::SubPixelRampEdgeImage(IplImage* edgeImage, int nDir)
{
    vector<cv::Point2f> vecEdges;

    int widthStep = edgeImage->widthStep;
    int cx = edgeImage->width;
    int cy = edgeImage->height;

    if ((nDir % 2) == 1) {
        cvFlip(edgeImage, edgeImage, -1); // 상하, 좌우반전
    }


    uchar *data = (uchar*)edgeImage->imageData;
    unsigned char *fxData = NULL;
    //int start = 0;
    //int end = 0;
    //int count = 0;
    double dEdge = 0;
    //double dEdgeOtherAxis = 0;
    switch (nDir) // ("Left2Right,Right2Left,Top2Bottom,Bottom2Top")),
    {
    case 0: //Left2Right,Right2Left
    case 1:
        fxData = (unsigned char *)malloc(cx);
        for (int y = 0; y < cy; y++) //vecEdges에서 상하 30%를 버린 중간값으로 또는 Peak edge값의 이미지로 Ramp edge를 구한다.
        {
            int cnt = 0;
            for (int x = 0; x < cx; x++)
            {
                int index = x + y*widthStep;
                fxData[cnt++] = data[index];
            }
            dEdge = SubPixelRampEdge(fxData, cnt);
            vecEdges.push_back(cv::Point2f(dEdge, y));
        }

        break;
    case 2: //Top2Bottom,Bottom2Top
    case 3:
        fxData = (unsigned char *)malloc(cy);
        for (int x = 0; x < cx; x++)
        {
            int cnt = 0;
            for (int y = 0; y < cy; y++)
            {
                int index = x + y*widthStep;
                fxData[cnt++] = data[index];
            }
            dEdge = SubPixelRampEdge(fxData, cnt);
            vecEdges.push_back(cv::Point2f(x, dEdge));
        }
        break;
    }
    if (fxData)
        free(fxData);

    switch (nDir) {
    case 0: // 세로선
    case 1:
        std::stable_sort(vecEdges.begin(), vecEdges.end(), [](const cv::Point2f lhs, const cv::Point2f rhs)->bool {
            if (lhs.x < rhs.x) // assending
                return true;
            return false;
        });
        break;
    case 2: // 가로선
    case 3:
        std::stable_sort(vecEdges.begin(), vecEdges.end(), [](const cv::Point2f lhs, const cv::Point2f rhs)->bool {
            if (lhs.y < rhs.y) // assending
                return true;
            return false;
        });
        break;
    }

    double dVal = 0;
    float sz = vecEdges.size();
    int first = sz * 0.4;
    int last = sz - (sz * 0.4);
    if (first < 0)
        first = 0;
    if (last < 0)
        last = 0;

    // 소팅을 한 결과 테이블에서 상하 40%를 버리고 중간 20%의 중간값을 구한다.
    for (int i = first; i < last; i++)
    {
        if (nDir == 0 || nDir == 1)
            dVal += vecEdges[i].x;
        else
            dVal += vecEdges[i].y;
    }

    sz = last - first;
    if (sz > 0) {
        dVal /= sz;
    }

    dEdge = dVal;
    switch (nDir)
    {
    case 0: // Left2Right 세로선
    case 1: // Right2Left
        if ((nDir % 2) == 1)
            dEdge = cx - dEdge;
        break;
    case 2: // 가로선
    case 3:
        if ((nDir % 2) == 1)
            dEdge = cy - dEdge;
        break;
    }

    return dEdge;
}

cv::Point2f CImgProcBase::getCorner(std::vector<cv::Point2f>& corners, cv::Point2f center, int CornerType)
{
    cv::Point2f edge[4] = { { 0, 0 }, };
    std::vector<cv::Point2f> top, bot;

    for (int i = 0; i < (int)corners.size(); i++)
    {
        if (corners[i].y < center.y)
            top.push_back(corners[i]);
        else
            bot.push_back(corners[i]);
    }

    if (top.size() >= 2)
    {
        edge[0] = top[0].x > top[1].x ? top[1] : top[0]; // top left
        edge[1] = top[0].x > top[1].x ? top[0] : top[1]; // top right
    }
    if (bot.size() >= 2)
    {
        edge[2] = bot[0].x > bot[1].x ? bot[1] : bot[0]; // bottom left
        edge[3] = bot[0].x > bot[1].x ? bot[0] : bot[1]; // bottom right
    }
    return edge[CornerType];
}


//
// Bresenham 알고리즘 .. 라인을 따라 점으로 처리한다.
//
void CImgProcBase::bhm_line(int x1, int y1, int x2, int y2, std::vector<cv::Point>* points)
{
    cv::Point pt1;
    int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
    dx = x2 - x1;
    dy = y2 - y1;
    dx1 = abs(dx);
    dy1 = abs(dy);
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;

    if (dy1 <= dx1)
    {
        if (dx >= 0)
        {
            x = x1;
            y = y1;
            xe = x2;
        }
        else
        {
            x = x2;
            y = y2;
            xe = x1;
        }
        pt1.x = x;
        pt1.y = y;
        points->push_back(pt1);
        for (i = 0; x<xe; i++)
        {
            x = x + 1;
            if (px<0)
            {
                px = px + 2 * dy1;
            }
            else
            {
                if ((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    y = y + 1;
                }
                else
                {
                    y = y - 1;
                }
                px = px + 2 * (dy1 - dx1);
            }
            pt1.x = x;
            pt1.y = y;
            points->push_back(pt1);
        }
    }
    else
    {
        if (dy >= 0)
        {
            x = x1;
            y = y1;
            ye = y2;
        }
        else
        {
            x = x2;
            y = y2;
            ye = y1;
        }
        pt1.x = x;
        pt1.y = y;
        points->push_back(pt1);
        for (i = 0; y<ye; i++)
        {
            y = y + 1;
            if (py <= 0)
            {
                py = py + 2 * dx1;
            }
            else
            {
                if ((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    x = x + 1;
                }
                else
                {
                    x = x - 1;
                }
                py = py + 2 * (dx1 - dy1);
            }
            pt1.x = x;
            pt1.y = y;
            points->push_back(pt1);
        }
    }
}

double CImgProcBase::getObjectAngle(IplImage *src)
{
    CvMoments cm;
    cvMoments(src, &cm, true);
    double th = 0.5 * atan((2 * cm.m11) / (cm.m20 - cm.m02));
    return th * (180 / 3.14);
}
double CImgProcBase::GetDistance2D(CvPoint p1, CvPoint p2)
{
    return sqrt(pow((float)p1.x - p2.x, 2) + pow((float)p1.y - p2.y, 2));
}

// nAxisLength : pixel count
int CImgProcBase::FilterLargeBlob(IplImage* grayImg, int nAxisLength)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, nullptr);
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetMajorAxisLength(), B_GREATER, nAxisLength); // 블럽 큰것 제거.
    IplImage *pMask = cvCloneImage(grayImg);
    cvZero(grayImg);
    int blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
        }
    }
    cvAnd(grayImg, pMask, grayImg);

    cvReleaseImage(&pMask);
    return 0;
}

int CImgProcBase::IncludeRadiusBlob(IplImage* grayImg, int nMinCircleRadius, int nMaxCircleRadius)
{

    CBlobResult blobs = CBlobResult(grayImg, NULL);

    // filter blobs.
    cvZero(grayImg);
    int   blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);

            CvSize2D32f f = currentBlob->GetEllipse().size;
            int _max = MAX(f.width, f.height);
            int _min = MIN(f.width, f.height);
            if (_max > nMaxCircleRadius)
                continue;
            if (_min < nMinCircleRadius)
                continue;

            currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
        }
    }

    return 0;
}

void CImgProcBase::FilterLargeArea(IplImage* grayImg)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, NULL);

    double dLargeArea = 0;
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++) {
        CBlob *p = blobs.GetBlob(i);
        double dArea = p->Area();
        if (dLargeArea < dArea) {
            dLargeArea = dArea;
        }
    }
    if (nBlobs > 0)
      blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, dLargeArea);

    // filter blobs.
    int blobCount = blobs.GetNumBlobs();
    if (blobCount == 0) {
        return;
    }
    IplImage *pMask = cvCloneImage(grayImg);
    cvZero(grayImg);
    for (int i = 0; i < blobCount; i++)
    {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
    }
    cvAnd(grayImg, pMask, grayImg);
    cvReleaseImage(&pMask);
}


void CImgProcBase::FilterLargeDiameter(IplImage* grayImg)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, NULL);

    CvRect LargeRect(0, 0);
    int index = -1;
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++) {
        CBlob *p = blobs.GetBlob(i);
        CvRect r = p->GetBoundingBox();
        if (max(r.height,r.width) > max(LargeRect.height,LargeRect.width))
        {
            LargeRect = r;
            index = i;
        }
    }
    if (index >= 0)
    {
#if 1
        IplImage *pMask = cvCloneImage(grayImg);
        cvZero(grayImg);
        CBlob *currentBlob = blobs.GetBlob(index);
        currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
#else
        cvSet(grayImg, cvScalar(255));
        for (int i = 0; i < nBlobs; i++) {
            if (i != index) {
                CBlob *p = blobs.GetBlob(i);
                p->FillBlob(grayImg, CVX_BLICK);
            }
        }
#endif
        cvAnd(grayImg, pMask, grayImg);
        cvReleaseImage(&pMask);
    }
}


//Moment 로 중심점 계산 : 공간 모멘트를 통해 중심점 구하기
Point2f CImgProcBase::CenterOfMoment(CvSeq* c)
{
    double M;
    int x_order, y_order;
    double cX, cY, m00 = 1.0;
    Point2f point;

    // moment 변수 선언
    CvMoments moments;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // (4) cvMoment 로 중심점 계산
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    cvMoments(c, &moments);

    for (y_order = 0; y_order <= 3; y_order++)
    {
        for (x_order = 0; x_order <= 3; x_order++)
        {
            if (x_order + y_order > 3)
                continue;
            M = cvGetSpatialMoment(&moments, x_order, y_order);
            if (x_order == 0 && y_order == 0)
                m00 = M;
            else if (x_order == 1 && y_order == 0)
                cX = M;
            else if (x_order == 0 && y_order == 1)
                cY = M;
        }
    }

    cX /= m00;
    cY /= m00;

    point.x = (float)cX;
    point.y = (float)cY;
    return point;
}

//
// 라인이 직접 만나야지 true를 리턴(확장라인이 만날때는 false)
//  ref : http://www.gisdeveloper.co.kr/?p=89
//
bool CImgProcBase::checkCross(const cv::Point& AP1, const cv::Point& AP2, const cv::Point& BP1, const cv::Point& BP2, cv::Point* IP)
{
    double t;
    double s;
    double under = (BP2.y - BP1.y)*(AP2.x - AP1.x) - (BP2.x - BP1.x)*(AP2.y - AP1.y);
    if (under == 0) return false;

    double _t = (BP2.x - BP1.x)*(AP1.y - BP1.y) - (BP2.y - BP1.y)*(AP1.x - BP1.x);
    double _s = (AP2.x - AP1.x)*(AP1.y - BP1.y) - (AP2.y - AP1.y)*(AP1.x - BP1.x);

    t = _t / under;
    s = _s / under;

    if (t<0.0 || t>1.0 || s<0.0 || s>1.0) return false;
    if (_t == 0 && _s == 0) return false;

    IP->x = (long)(AP1.x + t * (double)(AP2.x - AP1.x));
    IP->y = (long)(AP1.y + t * (double)(AP2.y - AP1.y));
    return true;

}


//
// 확장 라인까지 계산해서 만나는 지점이 있는지 계산해줌
//
// intersection() 와 getIntersectionPoint()는 동일한 결과를 가짐.
//

// Finds the intersection of two lines, or returns false.
// The lines are defined by (o1, p1) and (o2, p2).
//bool CImgProcBase::intersection(Point2f o1, Point2f p1, Point2f o2, Point2f p2, Point2f &r)
//{
//	Point2f x = o2 - o1;
//	Point2f d1 = p1 - o1;
//	Point2f d2 = p2 - o2;
//
//	float cross = d1.x*d2.y - d1.y*d2.x;
//	if (abs(cross) < /*EPS*/1e-8)
//		return false;
//
//	double t1 = (x.x * d2.y - x.y * d2.x) / cross;
//	r = o1 + d1 * t1;
//	return true;
//}

double CImgProcBase::isCross(Point v1, Point v2)
{
    return v1.x*v2.y - v1.y*v2.x;
}
bool CImgProcBase::getIntersectionPoint(Point a1, Point a2, Point b1, Point b2, Point & intPnt)
{
    Point p = a1;
    Point q = b1;
    Point r(a2 - a1);
    Point s(b2 - b1);

    if (isCross(r, s) == 0) { return false; }

    double t = isCross(q - p, s) / isCross(r, s);

    intPnt = p + t*r;
    return true;
}

//
// 한점과 직선이 만나는 가장 가까운점
// 리턴값 : 길이
// nearX, nearY : 만나는점
// 수선의 조건이 충족되면 라인상의 가장 가까운점을 찾게되고, 아니면 시작점 또는 끝점이 된다.
//
double CImgProcBase::Dist2LineSegment(double px, double py, double X1, double Y1, double X2, double Y2, double &nearX, double &nearY)
{
    double dx = X2 - X1;
    double dy = Y2 - Y1;
    //	, t;
    if (dx == 0 && dy == 0)
    {
        dx = px - X1;
        dy = py - Y1;

        nearX = X1;
        nearY = Y1;

        return sqrt((double)(dx * dx + dy * dy));
    }

    double t = (double)((px - X1) * dx + (py - Y1) * dy) / (double)(dx * dx + dy * dy);

    if (t < 0)
    {
        dx = px - X1;
        dy = py - Y1;
        nearX = X1;
        nearY = Y1;
    }
    else if (t > 1)
    {
        dx = px - X2;
        dy = py - Y2;
        nearX = X2;
        nearY = Y2;
    }
    else
    {
        nearX = X1 + t * dx;
        nearY = Y1 + t * dy;
        dx = px - nearX;
        dy = py - nearY;
    }

    return sqrt((double)(dx * dx + dy * dy));
}


//
// Hessian matrix 알고리즘을 이용한 Subpixel Edge를 구함
// 여러 edge contour를 구한다.
//
double CImgProcBase::SubPixelHessianEdge(IplImage *src, int nDir)
{
    vector<Contour> contours;

    double alpha = 1.0;
    int low = 10;
    int high = 10;
    int mode = RETR_LIST; //retrieves all of the contours without establishing any hierarchical relationships.

    Mat msrc = cvarrToMat(src);// imread(imagePath, IMREAD_GRAYSCALE);
    //vector<Contour> contours;
    vector<Vec4i> hierarchy;
    //int64 t0 = getCPUTickCount();

    // alpha - GaussianBlur = sigma이며 흐려지는 정도를 조절할 수 있다.
    // hierarchy, mode  - have the same meanings as in cv::findContours
    EdgesSubPix(msrc, alpha, low, high, contours, hierarchy, mode);



    // 윤곽선이 1개 이상 형성될것이므로 ROI Direction에 따라 첫번째 contour를 선택한다
    typedef struct {
        cv::Point2f sum;
        cv::Point2f avg;
        int count;
        int seq;
    } EDGE;
    vector<EDGE> points;

    //points.resize(contours.size());
    for (size_t i = 0; i < contours.size(); ++i)
    {
        EDGE edge;
        edge.seq = i;
        for (size_t j = 0; j < contours[i].points.size(); ++j)
        {
            cv::Point2f pt = contours[i].points[j];
            edge.sum.x += pt.x;
            edge.sum.y += pt.y;
        }
        edge.count = contours[i].points.size();
        points.push_back(edge);
    }
    for (size_t i = 0; i < points.size(); ++i)
    {
        points[i].avg = points[i].sum / points[i].count;
    }

    //int nDir = 0;
    int nDetectMethod = 0; // Average
//    CParam *pParam = pData->getParam(_T("Detect method")); // _T("Average,First")
//    if (pParam)
//        nDetectMethod = (int)_ttoi(pParam->Value.c_str());
//    pParam = pData->getParam(_T("Direction"));
//    if (pParam)
//        nDir = (int)_ttoi(pParam->Value.c_str());

    switch (nDir) // _T("Left2Right,Right2Left,Top2Bottom,Bottom2Top")),
    {
    case 0:
        std::stable_sort(points.begin(), points.end(), [](const EDGE lhs, const EDGE rhs)->bool {
            if (lhs.avg.x < rhs.avg.x) // assending
                return true;
            return false;
        });
        break;
    case 1:
        std::stable_sort(points.begin(), points.end(), [](const EDGE lhs, const EDGE rhs)->bool {
            if (lhs.avg.x > rhs.avg.x) // descending
                return true;
            return false;
        });
        break;
    case 2:
        std::stable_sort(points.begin(), points.end(), [](const EDGE lhs, const EDGE rhs)->bool {
            if (lhs.avg.y < rhs.avg.y) // assending
                return true;
            return false;
        });
        break;
    case 3:
        std::stable_sort(points.begin(), points.end(), [](const EDGE lhs, const EDGE rhs)->bool {
            if (lhs.avg.y > rhs.avg.y) // descending
                return true;
            return false;
        });
        break;

    }

    int select = -1;
    for (size_t i = 0; i < points.size(); ++i)
    {
        int seq = points[i].seq;
        if (contours[seq].points.size() > (src->width / 4)) // ROI영역의 폭의 1/4만큼의 edge라인이 형성된것중 x or y가 적은(소팅결과) edge
        {
            select = seq;
            break;
        }
    }
    //  첫번째 contour 선택 완료

    if (select < 0)
        return -1;

    // 선택된 Contour의 각 point를 가져와서 튀는 값은 버리고 중간값들중 첫번째 또는 평균 위치를 구한다.
    // Sorting해서 상위 40%, 하위 40%를 버리고 중간지점의 point만 취한다.
    Contour &contour = contours[select];
    switch (nDir)
    {
    case 0: // 세로선
    case 1:
        std::stable_sort(contour.points.begin(), contour.points.end(), [](const Point2f lhs, const Point2f rhs)->bool {
            if (lhs.x < rhs.x) // assending
                return true;
            return false;
        });
        break;
    case 2: // 가로선
    case 3:
        std::stable_sort(contour.points.begin(), contour.points.end(), [](const Point2f lhs, const Point2f rhs)->bool {
            if (lhs.y < rhs.y) // assending
                return true;
            return false;
        });
        break;
    }

    double dVal = 0;
    CvPoint2D32f pt = { 0, 0 };
    if (nDetectMethod == 0) // Average
    {
        // 소팅을 한 결과 테이블에서 상하 40%를 버리고 중간 20%의 중간값을 구한다.
        float sz = contour.points.size();
        int first = sz * 0.4;
        int last = sz - (sz * 0.4);
        for (int i = first; i < last; i++)
        {
            pt.x += contour.points[i].x;
            pt.y += contour.points[i].y;
        }

        sz = last - first;
        if (sz > 0) {
            pt.x /= sz;
            pt.y /= sz;
        }
    }
    else {	// First
        //제일처음 만나는 edge를 구한다
        float sz = contour.points.size();
        if (sz > 0) {
            if (nDir == 0)  { // Left2Right 세로선
                pt.x += contour.points[0].x;
                pt.y += contour.points[0].y;
            }
            else if (nDir == 1)  { // Right2Left 세로선
                pt.x += contour.points[sz - 1].x;
                pt.y += contour.points[sz - 1].y;
            }
            else if (nDir == 2)  { // Top2Bottom 가로선
                pt.x += contour.points[0].x;
                pt.y += contour.points[0].y;
            }
            else {				//Bottom2Top
                pt.x += contour.points[sz - 1].x;
                pt.y += contour.points[sz - 1].y;
            }
        }
    }

    if (nDir == 0 || nDir == 1) // x 위치
        dVal = pt.x;
    else
        dVal = pt.y; // y 위치

    return dVal;
}


//
// 경사진면의 edge를 구한다
// nCnt - threshold경계면부터 검색할 범위(0 ~ 512)
//
// nDir = 0 : Left2Right, Top2Bottom
//        1 : Right2Left, Bottom2Top
double CImgProcBase::SubPixelRampEdge(unsigned char *pixelData, int pCnt)
{
    double dLoc = 0.0;
    int i;
    int ffx[512+1];
    int iSumfx;

    if (pCnt > 512)
        return 0;
    memset(ffx, 0, sizeof(ffx));
    iSumfx = 0;
    for (i = 0; i < pCnt - 1; i++) {
        ffx[i + 1] = abs(pixelData[i + 1] - pixelData[i]);
        iSumfx += ffx[i + 1];
    }

    for (i = 0; i < pCnt - 1; i++) {
        if (iSumfx > 0)
            dLoc += (double(i + 1.0) * double(ffx[i + 1]) / double(iSumfx));
    }

    return dLoc;
}

//
// Subpixel을 이용하여 Corner Edge를 구한다.
//
int CImgProcBase::SubPixelCorner(IplImage *src, vector<Point2f> &points)
{
    int i, corner_count = 150;
    IplImage *src_img_gray;
    IplImage *eig_img, *temp_img;
    CvPoint2D32f *corners;

    src_img_gray = src;
    eig_img = cvCreateImage(cvGetSize(src_img_gray), IPL_DEPTH_32F, 1);
    temp_img = cvCreateImage(cvGetSize(src_img_gray), IPL_DEPTH_32F, 1);
    corners = (CvPoint2D32f *)cvAlloc(corner_count * sizeof(CvPoint2D32f));

    double quality_level = 0.2;// threshold for the eigenvalues
    double min_distance = 50;// minimum distance between two corners
    int eig_block_size = 6;// window size
    int use_harris = false;// use 'harris method' or not

    int half_win_size = 3;// the window size will be 3 +1 +3 = 7
    int iteration = 20;
    double epislon = 0.1;

    /*
    cvGoodFeaturesToTrack : 코너 점들의 좌표가 배열 형태로 반환.
    (src_img_gray, : 입력 영상, 8비트 or 32비트, 단일 채널 영상
    eig_img, : 32비트 단일 채널 영상, 영사의 각 원소에는 해당 픽셀에서 구해진 최소 고유값 저장
    temp_img,
    corners, : 코너 알고리즘이 실행된 후 결과 포인트가 저장(CvPoint2D32f)
    &corner_count, : 이 함수가 검출할 수 있는 코너점의 최대 개수
    0.1, :코너의 풒ㅁ질을 결정. 1을 넘어서는 안됨. 보통 0.10또는 0.01 정도 사용
    15, : 반환되는 코너 점들 사이의 최소(유클리디안) 거리
    NULL, :관심영역 지정
    3, : 미분 계수의 자기상관행렬을 계산할 때 사용되는 평균 블록 크기
    1, : 해리스코너 사용 : use_harris로 0이 아니면 해리스 코너 사용. 0: Shi-Tomasi방법 사용
    0.01 : 위 use_harris를 0이 아닌값으로 설정하였다면 이값은 코너 응답 함수 식에서 대각합에 대한 가중치로 작용
    );
    */

    use_harris = true;

    // Corner detection using cvCornerHarris
    corner_count = 150;
    cvGoodFeaturesToTrack(src_img_gray, eig_img, temp_img, corners, &corner_count, quality_level, min_distance, NULL, eig_block_size, use_harris, 0.01);
    cvFindCornerSubPix(src_img_gray, corners, corner_count,
        cvSize(half_win_size, half_win_size),
        cvSize(-1, -1),
        cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, iteration, epislon));

    for (i = 0; i < corner_count; i++)
        points.push_back(corners[i]);

    //for (i = 0; i < corner_count; i++)
    //	cvCircle(dst_img2, cvPointFrom32f(corners[i]), 3, CV_RGB(0, 0, 255), 2);

    //cvNamedWindow("Harris", CV_WINDOW_AUTOSIZE);
    //cvShowImage("Harris", dst_img2);

    //cvWaitKey(0);
    //cvDestroyWindow("EigenVal");
    //cvDestroyWindow("Harris");

    cvReleaseImage(&eig_img);
    cvReleaseImage(&temp_img);

    cvFree(&corners);

    return 0;
}


/**
* Helper function to find a cosine of angle between vectors
* from pt0->pt1 and pt0->pt2
*/
double CImgProcBase::calcAngle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

#if 0
int CImgProcBase::find_threshold(IplImage* image)
{
    int threshold_final;
    int i,j;
    int k = 256;
    int t_initial = k/2;

    int height = image->height;
    int width =  image->width;
    float new_t;

    CvScalar s1,s2;

    int intensity_temp1 = 0, intensity_temp2 = 0;

    IplImage* temp1 = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,1);
    IplImage* temp2 = cvCreateImage(cvGetSize(image),IPL_DEPTH_8U,1);

    int size = (temp1->height)*(temp1->width);

    uchar *data_input = (uchar*)image->imageData;
    //int data_temp1[size],data_temp2[size];
    int *data_temp1 = (int *)malloc(size * sizeof(int));
    int *data_temp2 = (int *)malloc(size * sizeof(int));
    int p=0,q=0;

    while (1)
    {

        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j++)
            {
                if (data_input[i*(image->widthStep) + j*(image->nChannels)] == 255)
                    continue;

                if(data_input[i*(image->widthStep) + j*(image->nChannels)] < t_initial )
                {
                    data_temp1[p] = data_input[i*(image->widthStep) + j*(image->nChannels)];
                    p++;
                }
                else
                {
                    data_temp2[q] = data_input[i*(image->widthStep) + j*(image->nChannels)];
                    q++;
                }
            }
        }
        for (i = 0;i<p;i++)
        {
            intensity_temp1 = intensity_temp1 + data_temp1[i];
        }

        for (i = 0;i<q;i++)
        {
            intensity_temp2 = intensity_temp2 + data_temp2[i];
        }

        float average1 = intensity_temp1/p;
        float average2 = intensity_temp2/q;

        intensity_temp1 = 0;
        intensity_temp2 = 0;
        p=0;
        q=0;

        //calculate new value of threhold as the average of these 2 means
        // T = (mu1 + mu2)/2
        new_t = (average1 + average2)/2;
        threshold_final = (int)new_t;

        if (threshold_final == t_initial)
            break;
        else
            t_initial = threshold_final;

    }
    free(data_temp1);
    free(data_temp2);

    return(threshold_final);
}
#endif

//
// OTSU Threshold를 이용하여 히스토그램 Threshold Value를 구한다.
//
int CImgProcBase::find_thresholdOTSU(IplImage* image)
{
    //compute histogram first
    cv::Mat imageh; //image edited to grayscale for histogram purpose
    //imageh=image; //to delete and uncomment below;
    //cv::cvtColor(imagecolor, imageh, CV_BGR2GRAY);
    imageh = cvarrToMat(image);

    int histSize[1] = {256}; // number of bins
    float hranges[2] = {0.0, 256.0}; // min andax pixel value
    const float* ranges[1] = {hranges};
    int channels[1] = {0}; // only 1 channel used

    cv::MatND hist;
    // Compute histogram
    calcHist(&imageh, 1, channels, cv::Mat(), hist, 1, histSize, ranges);

    IplImage* im = new IplImage(imageh);//assign the image to an IplImage pointer
    IplImage* finalIm = cvCreateImage(cvSize(im->width, im->height), IPL_DEPTH_8U, 1);
    //double otsuThreshold= cvThreshold(im, finalIm, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU );

    //cout<<"opencv otsu gives "<<otsuThreshold<<endl;

    int totalNumberOfPixels= imageh.total();
    //cout<<"total number of Pixels is " <<totalNumberOfPixels<< endl;


    float sum = 0;
    for (int t=0 ; t<256 ; t++)
    {
        sum += t * hist.at<float>(t);
    }
    //cout<<"sum is "<<sum<<endl;

    float sumB = 0; //sum of background
    int wB = 0; // weight of background
    int wF = 0; //weight of foreground

    float varMax = 0;
    int threshold = 0;

    //run an iteration to find the maximum value of the between class variance(as between class variance shld be maximise)
    for (int t=0 ; t<256 ; t++)
    {
        wB += hist.at<float>(t);               // Weight Background
        if (wB == 0) continue;

        wF = totalNumberOfPixels - wB;                 // Weight Foreground
        if (wF == 0) break;

        sumB += (float) (t * hist.at<float>(t));

        float mB = sumB / wB;            // Mean Background
        float mF = (sum - sumB) / wF;    // Mean Foreground

        // Calculate Between Class Variance
        float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

        // Check if new maximum found
        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = t;
        }
    }

    if (im)
        delete im;
    if (finalIm)
        cvReleaseImage(&finalIm);

    return threshold; //threshold value is
}


//CvPoint2D32f srcTri[4];
//CvPoint2D32f dstTri[4];
//srcTri[0] = Point2f(0, 0);
//srcTri[1] = Point2f(100, 0);
//srcTri[2] = Point2f(100, 100);
//srcTri[3] = Point2f(0, 100);
//dstTri[0] = Point2f(0, 5);
//dstTri[1] = Point2f(100, 4);
//dstTri[2] = Point2f(100, 100);
//dstTri[3] = Point2f(0, 100);
// //http://lueseypid.tistory.com/111

void CImgProcBase::AffineTransform(std::vector<Point2f> &vec, cv::Point2f srcTri[], cv::Point2f dstTri[])
{
    Mat m(2, 3, CV_32F);
    m = getAffineTransform(srcTri, dstTri);
    transform(vec, vec, m);
}

void CImgProcBase::WarpPerspectiveImage(IplImage* img, CvPoint2D32f srcTri[], CvPoint2D32f dstTri[])
{
    CvMat* warp_mat = cvCreateMat(3, 3, CV_32FC1);

    cvGetPerspectiveTransform(srcTri, dstTri, warp_mat);
    cvWarpPerspective(img, img, warp_mat);

    cvReleaseMat(&warp_mat);
}

void CImgProcBase::WarpAffineImage(IplImage* img, CvPoint2D32f srcTri[], CvPoint2D32f dstTri[])
{
    CvMat* warp_mat = cvCreateMat(2, 3, CV_32FC1);

    cvGetAffineTransform(srcTri, dstTri, warp_mat); // 트랜스폼 매트릭스 생성 from SrcTri,dstTri
    cvWarpAffine(img, img, warp_mat);

    cvReleaseMat(&warp_mat);
}

void CImgProcBase::RotateImage(IplImage* img, double angle, CvPoint2D32f center, double scale)
{
    CvMat* warp_mat = cvCreateMat(2, 3, CV_32FC1);
    //회전행렬 계산
    cv2DRotationMatrix(center, angle, scale, warp_mat); //중심점, 회전각도, 스케일을 직접 입력;
    cvWarpAffine(img, img, warp_mat);
    cvReleaseMat(&warp_mat);
}

// 중심점과 회전각도를 이용하여 이미지 변환 : 패턴매칭에서 Align에 이용
void CImgProcBase::RotateImage(IplImage* img, double angle, CvPoint2D32f center)
{
    CvMat M;
    float m[6];
    // 회전을위한 행렬 (아핀 행렬) 요소를 설정하고 CvMat 행렬 M을 초기화
    m[0] = (float)(cos(angle * CV_PI / 180));
    m[1] = (float)(-sin(angle * CV_PI / 180));
    m[2] = center.x;
    m[3] = -m[1];
    m[4] = m[0];
    m[5] = center.y;
    cvInitMatHeader(&M, 2, 3, CV_32FC1, m, CV_AUTOSTEP);// 지정된 회전 행렬은 GetQuadrangleSubPix을 이용해 이미지 전체를 회전
    cvGetQuadrangleSubPix(img, img, &M);
}
void CImgProcBase::RotateImage(IplImage* img, double angle)
{
    CvMat M;
    float m[6];
    // 회전을위한 행렬 (아핀 행렬) 요소를 설정하고 CvMat 행렬 M을 초기화
    m[0] = (float)(cos(angle * CV_PI / 180));
    m[1] = (float)(-sin(angle * CV_PI / 180));
    m[2] = img->width * 0.5;
    m[3] = -m[1];
    m[4] = m[0];
    m[5] = img->height * 0.5;
    cvInitMatHeader(&M, 2, 3, CV_32FC1, m, CV_AUTOSTEP);// 지정된 회전 행렬은 GetQuadrangleSubPix을 이용해 이미지 전체를 회전
    cvGetQuadrangleSubPix(img, img, &M);
}

#if 1
//
// AutoFocus를 위한 함수들
//

bool CImgProcBase::checkForBurryImage(cv::Mat matImage)
{
    int kBlurThreshhold = -6118750;
    cv::Mat finalImage;

    cv::Mat matImageGrey;
    cv::cvtColor(matImage, matImageGrey, CV_BGRA2GRAY);
    matImage.release();

    cv::Mat newEX;
    const int MEDIAN_BLUR_FILTER_SIZE = 15; // odd number
    cv::medianBlur(matImageGrey, newEX, MEDIAN_BLUR_FILTER_SIZE);
    matImageGrey.release();

    cv::Mat laplacianImage;
    cv::Laplacian(newEX, laplacianImage, CV_8U); // CV_8U
    newEX.release();

    cv::Mat laplacianImage8bit;
    laplacianImage.convertTo(laplacianImage8bit, CV_8UC1);
    laplacianImage.release();
    cv::cvtColor(laplacianImage8bit, finalImage, CV_GRAY2BGRA);
    laplacianImage8bit.release();

    int rows = finalImage.rows;
    int cols = finalImage.cols;
    char *pixels = reinterpret_cast<char *>(finalImage.data);
    int maxLap = -16777216;
    for (int i = 0; i < (rows*cols); i++) {
        if (pixels[i] > maxLap)
            maxLap = pixels[i];
    }

    //int soglia = -6118750;

    pixels = NULL;
    finalImage.release();

    bool isBlur = (maxLap < kBlurThreshhold) ? true : false;
    return isBlur;
}

double CImgProcBase::rateFrame(Mat frame)
{
    unsigned long int sum = 0;
    unsigned long int size = frame.cols * frame.rows;
    Mat edges;
    if (frame.channels() > 1)
        cvtColor(frame, edges, CV_BGR2GRAY);
    else edges = frame;
    GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
    Canny(edges, edges, 0, 30, 3);

    MatIterator_<uchar> it, end;
    for (it = edges.begin<uchar>(), end = edges.end<uchar>(); it != end; ++it)
    {
        sum += *it != 0;
    }

    return (double)sum / (double)size;
}

double CImgProcBase::correctFocus(FocusState & state, double rate)
{
    const double epsylon = 0.001; // compression, noice, etc. // 0.0005

    //state.lastDirectionChange++;
    double rateDelta = rate - state.rate;

    if (rate >= state.rateMax + epsylon) // Update Max
    {
        state.rateMax = rate;
        //state.lastDirectionChange = 0; // My local minimum is now on the other direction, that's why:
    }

    if (rate < epsylon)
    { // It's hard to say anything
        state.step = 1;// state.step * 0.75;
    }
    else if (rateDelta < -epsylon)
    { // Wrong direction ?
        state.direction *= -1;
        state.step = state.step * 0.75;
        //state.lastDirectionChange = 0;
    }
    else if (rate + epsylon < state.rateMax)
    {
        state.step = state.step * 0.75;
        //state.lastDirectionChange = 0; // Like reset.
    }

    // Update state.
    state.rate = rate;
    return state.step;
}
#endif


// Blob의 중심 구하기
Point2f CImgProcBase::FindCenterOfBlob(IplImage * pImage)
{
    Point2f point;
    CvSeq* m_seq = 0, *ptseq = 0;
    //int mode = CV_RETR_LIST;
    //int method = CV_CHAIN_APPROX_SIMPLE;

    CvMemStorage* storage = cvCreateMemStorage(0);
    cvFindContours(pImage, storage, &m_seq, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    if (m_seq != 0)
    {
        CvSeq* m_approxDP_seq = cvApproxPoly(m_seq, sizeof(CvContour), storage, CV_POLY_APPROX_DP, 0, 1);
        if (m_approxDP_seq == NULL)
        {
            if (storage) cvReleaseMemStorage(&storage);
            return Point2f();
        }

        for (CvSeq* c = m_approxDP_seq; c != NULL; c = c->h_next)   // 엣지의 링크드리스트를 순회하면서 각 엣지들에대해서 출력한다.
        {
            ptseq = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), storage);
            if (ptseq == NULL) continue;
            for (int i = 0; i < c->total; ++i)
            {
                CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, c, i);    // CvSeq로부터좌표를얻어낸다.
                CvPoint temp;
                temp.x = p->x;
                temp.y = p->y;
                cvSeqPush(ptseq, &temp);
            }
            point = CenterOfMoment(ptseq);
            cvClearSeq(ptseq);
        }
        if (m_approxDP_seq) cvClearSeq(m_approxDP_seq);
    }

    if (m_seq) cvClearSeq(m_seq);
    cvReleaseMemStorage(&storage);
    return point;
}


//
// Threshold 이미지에서
// Edge의 경계면을 추출하고 경계면으로부터 Subpixel edge를 추출한다.
// * Hessian Matrix 를 이용한것보다 좀더 경사진면에서의 edge를 구할수 있다.
//
// nDir = Left2Right,Right2Left,Top2Bottom,Bottom2Top
double CImgProcBase::ROIPixEdge(IplImage* croppedImage, int nDir, double dRejectLow, double dRejectHigh)
{
    QString str;
    IplImage* grayImg;

    CvRect r = CvRect(0,0,croppedImage->width, croppedImage->height);
    int x=0, y=0, width=r.width, height=r.height;

    grayImg = cvCreateImage(cvSize(width, height), croppedImage->depth, croppedImage->nChannels);
    CopyImageROI(croppedImage, grayImg, cvRect(x, y, width, height));

    double dEdge = 0;
    vector<cv::Point2f> vecEdges;

    int imgStep = grayImg->widthStep;
    uchar* imageData = (uchar*)grayImg->imageData;
    uint sum = 0;
    for (int col = 0; col < grayImg->width; col++) {
        for (int row = 0; row < grayImg->height; row++) {
            sum += imageData[row * imgStep + col];
        }
    }
    if (sum == 0) // 이미지가 모두 0이면 fail 처리
        return -1;

    int nColor;
    if (nDir == 0 || nDir == 2)
        nColor = imageData[0];
    else
        nColor = imageData[width-1];

    uchar* data = (uchar*)grayImg->imageData;
    int widthStep = grayImg->widthStep;
    int cx = grayImg->width;
    int cy = grayImg->height;

    // Threshold한 결과 이미지의 경계면을 구한다. Left2Right,Right2Left,Top2Bottom,Bottom2Top에 따라서
    switch (nDir)
    {
    case 0: //Left2Right
        for (int y = 0; y < cy; y++)
        {
            for (int x = 0; x < cx; x++)
            {
                int index = x + y*widthStep;
                if (data[index] != nColor) {
                    vecEdges.push_back(cv::Point2f(x, y));
                    break;
                }
            }
        }
        break;
    case 1: //Right2Left
        for (int y = 0; y < cy; y++)
        {
            for (int x = cx - 1; x >= 0; x--)
            {
                int index = x + y*widthStep;
                if (data[index] != nColor) {
                    vecEdges.push_back(cv::Point2f(x, y));
                    break;
                }
            }
        }
        break;
    case 2: //Top2Bottom
        for (int x = 0; x < cx; x++)
        {
            for (int y = 0; y < cy; y++)
            {
                int index = x + y*widthStep;
                if (data[index] != nColor) {
                    vecEdges.push_back(cv::Point2f(x, y));
                    break;
                }
            }
        }
        break;
    case 3: //Bottom2Top
        for (int x = 0; x < cx; x++)
        {
            for (int y = cy - 1; y >= 0; y--)
            {
                int index = x + y*widthStep;
                if (data[index] != nColor) {
                    vecEdges.push_back(cv::Point2f(x, y));
                    break;
                }
            }
        }
        break;
    }

    switch (nDir) {
    case 0: // 세로선
    case 1:
        std::stable_sort(vecEdges.begin(), vecEdges.end(), [](const cv::Point2f lhs, const cv::Point2f rhs)->bool {
            if (lhs.x < rhs.x) // assending
                return true;
            return false;
        });
        break;
    case 2: // 가로선
    case 3:
        std::stable_sort(vecEdges.begin(), vecEdges.end(), [](const cv::Point2f lhs, const cv::Point2f rhs)->bool {
            if (lhs.y < rhs.y) // assending
                return true;
            return false;
        });
        break;
    }

    double dVal = 0;
    float sz = vecEdges.size();
    int first = sz * dRejectLow;
    int last = sz - (sz * dRejectHigh);
    if (first < 0)
        first = 0;
    if (last < 0)
        last = 0;

    // 소팅을 한 결과 테이블에서 상하 45%를 버리고 중간 10%의 중간값을 구한다.
    for (int i = first; i < last; i++)
    {
        if (nDir == 0 || nDir == 1)
            dVal += vecEdges[i].x;
        else
            dVal += vecEdges[i].y;
    }

    sz = last - first;
    if (sz > 0) {
        dVal /= sz;
    }

    cvReleaseImage(&grayImg);
    // 여기까지 Threshold를 이용한 edge를 구하였다.

    switch (nDir) {
        case 0: // Left2Right
            dEdge = dVal + r.x;
            break;
        case 1: // Right2Left
            dEdge = dVal + r.x;
            break;
        case 2: // Top2Bottom
            dEdge = dVal + r.y;
            break;
        case 3: // Bottom2Top
            dEdge = dVal + r.y;
            break;
    }
    return dEdge;
}

}  // namespace

