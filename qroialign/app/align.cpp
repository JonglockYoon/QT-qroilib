//
// align.cpp
//
//QROILIB : QT Vision ROI Library
//Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
//
#include <stdio.h>
#include <QDir>
#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>

#include "align.h"
#include "objectgroup.h"
#include "recipedata.h"
#include "mainwindow.h"

//using namespace cv;

CImgAlign::CImgAlign()
{
}

CImgAlign::~CImgAlign(void)
{
}

const double dResX = 0.038; // 카메라 분해능(Pixel Resolution)
const double dResY = 0.038;

int CImgAlign::TowPointAlignImage(IplImage* src)
{
    QString str;

    //if (m_bSaveEngineImg)
    {
        QString str;
        str.sprintf(("%d_TowPointAlignImageSrc.jpg"), 100);
        SaveOutImage(src, nullptr, str);
    }

    QVector<CvPoint2D32f> orginsppt;
    insppt.clear();
    GetCornerAlignPoint(src);
    //두점 사이의 각도 구하기.
#define CalculDegree(from, to)  -((double)atan2(to.y - from.y, to.x - from.x) * 180.0f / PI)
    //#ifndef PI
    //#define PI 3.141592653589793f
    //#endif
#define RADIAN(angle) angle *  PI /180

    std::stable_sort(insppt.begin(), insppt.end(), [](const CvPoint2D32f lhs, const CvPoint2D32f rhs)->bool {
        if (lhs.y < rhs.y) // assending
            return true;
        return false;
    });

    for (int i = 0; i<insppt.size(); i++) {
        insppt[i].x = insppt[i].x * dResX;
        insppt[i].y = insppt[i].y * dResY;
    }

    if (insppt.size() == 2)
    {
        double dTheta1 = 90.0; // 0 or 90 : 입력 이미지를 이 각도록 align한다.
        double dTheta2 = CalculDegree(insppt[1], insppt[0]);
        double dTheta = (dTheta1 - dTheta2);
        qDebug() << "Compensation theta value : " << dTheta;

        // center of image
        CvPoint2D32f dPointCenter;
        dPointCenter.x = src->width / 2.0  * dResX;
        dPointCenter.y = src->height / 2.0 * dResY;


        //image rotate.
        RotateImage(src, dTheta);

        double a = RADIAN(dTheta);
        CvPoint2D32f tmp;
        CvPoint2D32f dTranslateP;

        orginsppt = insppt;
        tmp.x = insppt[0].x;
        tmp.y = insppt[0].y;
        dTranslateP.x = (cos(a) * (tmp.x - dPointCenter.x)) - (sin(a) * (tmp.y - dPointCenter.y)) + dPointCenter.x;
        dTranslateP.y = (sin(a) * (tmp.x - dPointCenter.x)) + (cos(a) * (tmp.y - dPointCenter.y)) + dPointCenter.y;
        insppt[0].x = dTranslateP.x;
        insppt[0].y = dTranslateP.y;

        tmp.x = insppt[1].x;
        tmp.y = insppt[1].y;
        dTranslateP.x = (cos(a) * (tmp.x - dPointCenter.x)) - (sin(a) * (tmp.y - dPointCenter.y)) + dPointCenter.x;
        dTranslateP.y = (sin(a) * (tmp.x - dPointCenter.x)) + (cos(a) * (tmp.y - dPointCenter.y)) + dPointCenter.y;
        insppt[1].x = dTranslateP.x;
        insppt[1].y = dTranslateP.y;

        // 회전시킨후 두개지점의 X,Y이동거리를 산출한다.
        double tx1 = (orginsppt[0].x - insppt[0].x);
        double ty1 = (orginsppt[0].y - insppt[0].y);
        double tx2 = (orginsppt[1].x - insppt[1].x);
        double ty2 = (orginsppt[1].y - insppt[1].y);

        double dsx = (double)(tx1 + tx2) / 2.0; // 티칭위치를 회전이동 시킨후 안착위치와 X,Y가 움직인 거리
        double dsy = (double)(ty1 + ty2) / 2.0;
        int sx = (int)(dsx / dResX);
        int sy = (int)(dsy / dResY);
        qDebug() << ("Compensation xy value : ") << sx << sy;

        cv::Mat matIn = cv::cvarrToMat(src);
#if 1
        cv::Mat mat;

        int tx = 0;
        int ty = 0;
        int cx = matIn.cols;
        int cy = matIn.rows;

        if (sx < 0) { // shift left
            cx = cx + sx;
            tx = 0;
            sx = abs(sx);
        }
        else if (sx > 0) { // shift right
            cx = cx - sx;
            tx = sx;
            sx = 0;
        }
        if (sy < 0) { // shift up
            cy = cy + sy;
            ty = 0;
            sy = abs(sy);
        }
        else if (sy > 0) { // shift down
            cy = cy - sy;
            ty = sy;
            sy = 0;
        }
        mat = cv::Mat::zeros(matIn.size(), matIn.type());
        matIn(cv::Rect(sx, sy, cx, cy)).copyTo(mat(cv::Rect(tx, ty, cx, cy)));
#endif
        IplImage *trans = new IplImage(mat);
        cvCopy(trans, src);

        //if (m_bSaveEngineImg)
        {
            str.sprintf(("900_grapimg.jpg"));
            SaveOutImage(src, nullptr, str);
        }
    }

    return 0;
}

int CImgAlign::MeasureAlignImage(IplImage* src)
{
    int nErrorType = 0;

    vector<Qroilib::RoiObject *> vecAlign;
    int nMesAlingNum = 0;

    DocumentView* v = theMainWindow->currentView();
    for (const Layer *layer : v->mRoi->objectGroups()) {
        const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
        for (const Qroilib::RoiObject *roiObject : objectGroup) {
            Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;

            if (!mObject->objectGroup()->isVisible() || !mObject->isVisible())
                continue;

            if (mObject->mInspectType == _Inspect_Roi_Align_Measure){
                vecAlign.push_back(mObject);
                nMesAlingNum++;
                if (nMesAlingNum >= 3)
                    break;
            }
        }
    }
    if (nMesAlingNum == 0)
        return -1;


    //if (m_bSaveEngineImg)
    {
        QString str;
        str.sprintf(("%d_MeasureAlignSrc.jpg"), 120);
        SaveOutImage(src, nullptr, str);
    }

    if (nMesAlingNum == 3) //  Measure 얼라인
    {
        struct {
            RoiObject* pData;
            double dEdge;
        } pos[2][3]; // [v=0,h=1] [3]

        int nVert = 0, nHoriz = 0;
        int nDirection;	//("Left2Right,Right2Left,Top2Bottom,Bottom2Top")
        for (int i = 0; i < 3; i++) {
            RoiObject* pData = vecAlign[i];
            if (pData->getIntParam(("Direction"), nDirection) != 0)
                continue;

            IplImage* croppedImage;
            QRectF rect = pData->bounds();	// Area로 등록된 ROI
            cv::Point2f left_top = cv::Point2f(rect.left(), rect.top());
            cvSetImageROI(src, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
            croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), IPL_DEPTH_8U, 1);
            if (src->nChannels == 3)
                cvCvtColor(src, croppedImage, CV_RGB2GRAY);
            else
                cvCopy(src, croppedImage);
            cvResetImageROI(src);
            double dEdge = SingleROISubPixEdgeWithThreshold(croppedImage, pData, rect);
            switch (nDirection)
            {
            case 0: // Left2Right 세로선
            case 1: // Right2Left
                dEdge += rect.left();
                pos[0][nVert].pData = pData;
                pos[0][nVert].dEdge = dEdge * dResX;
                nVert++;
                break;
            case 2: // 가로선
            case 3:
                dEdge += rect.top();
                pos[1][nHoriz].pData = pData;
                pos[1][nHoriz].dEdge = dEdge * dResY;
                nHoriz++;
                break;
            }

            cvReleaseImage(&croppedImage);
            qDebug() << "dEdge: " << dEdge;

        }

        double dRetAngle = 0;
        CvPoint2D32f center;
        center.x = (src->width / 2);
        center.y = (src->height / 2);
        if (nVert == 2 && nHoriz == 1) {
            // 수직으로 두개의 ROI가 있는데 Y 순서대로 배열하기 위해
            RoiObject* pData = pos[0][0].pData;
            RoiObject* pData1 = pos[0][1].pData;
            if (pData->bounds().top() > pData1->bounds().top()) {
                RoiObject* pTmpData = pos[0][0].pData;
                pos[0][0].pData = pos[0][1].pData;
                pos[0][1].pData = pTmpData;
                double dTmpEdge = pos[0][0].dEdge;
                pos[0][0].dEdge = pos[0][1].dEdge;
                pos[0][1].dEdge = dTmpEdge;
            }
            pData = pos[0][0].pData;
            pData1 = pos[0][1].pData;
            RoiObject* pData2 = pos[1][0].pData; // 가로 Measure ROI

            double dDistX = fabs(pData1->bounds().top() - pData->bounds().top()) * dResX;
            double dDistY = (pos[0][0].dEdge - pos[0][1].dEdge);
            dRetAngle = (atan2(dDistY, dDistX) * 180.0 / PI); // degree
            //dRetAngle = -dRetAngle;

            // 가로 Measure ROI의 중심점이 dRetAngle 많큼 회전했을때 이동 위치를 산출해서 Rotate 회전 중심을 바꾸어준다.
            // 회전후 Shift된 위치를 계산해서 이동시키기위함.
            double a = (dRetAngle * PI) / 180; // radian
            double x1 = (double)pData2->bounds().center().x() * dResX;
            double y1 = (double)pData2->bounds().center().y() * dResY;
            double x0 = ((double)src->width / 2) * dResX; // 회전의 중심
            double y0 = ((double)src->height / 2) * dResY;
            double x2 = cos(a) * (x1 - x0) - (-sin(a) * (y1 - y0)) + x0;
            double y2 = (-sin(a) * (x1 - x0)) + cos(a) * (y1 - y0) + y0;

            center.x -= ((x1 - x2) / 2) / dResX;
            center.y -= ((y1 - y2) / 2) / dResY;
        }
        else if (nVert == 1 && nHoriz == 2)
        {
            // 수평으로 두개의 ROI가 있는데 X 순서대로 배열하기 위해
            RoiObject* pData = pos[1][0].pData;
            RoiObject* pData1 = pos[1][1].pData;
            if (pData->bounds().left() > pData1->bounds().left()) {
                RoiObject* pTmpData = pos[1][0].pData;
                pos[1][0].pData = pos[1][1].pData;
                pos[1][1].pData = pTmpData;
                double dTmpEdge = pos[1][0].dEdge;
                pos[1][0].dEdge = pos[1][1].dEdge;
                pos[1][1].dEdge = dTmpEdge;
            }
            pData = pos[1][0].pData;
            pData1 = pos[1][1].pData;
            RoiObject* pData2 = pos[0][0].pData; // 세로 Measure ROI

            double dDistX = fabs((pData1->bounds().left() - pData->bounds().left())) * dResX;
            double dDistY = (pos[1][0].dEdge - pos[1][1].dEdge);
            dRetAngle = (atan2(dDistY, dDistX) * 180.0 / PI); // degree
            dRetAngle = -dRetAngle;

            // 세로 Measure ROI의 중심점이 dRetAngle 많큼 회전했을때 이동 위치를 산출해서 Rotate 회전 중심을 바꾸어준다.
            // 회전후 Shift된 위치를 계산해서 이동시키기위함.
            double a = (dRetAngle * PI) / 180; // radian
            double x1 = (double)pData2->bounds().center().x() * dResX;
            double y1 = (double)pData2->bounds().center().y() * dResY;
            double x0 = ((double)src->width / 2) * dResX; // 회전의 중심
            double y0 = ((double)src->height / 2) * dResY;
            double x2 = cos(a) * (x1 - x0) - (-sin(a) * (y1 - y0)) + x0;
            double y2 = (-sin(a) * (x1 - x0)) + cos(a) * (y1 - y0) + y0;

            center.x -= ((x1 - x2) / 2) / dResX;
            center.y -= ((y1 - y2) / 2) / dResY;
        }
        else {
            nErrorType = 2;
            return nErrorType;
        }

        qDebug() << "RotateImage: " << dRetAngle << center.x << center.y;
        RotateImage(src, dRetAngle, center); // degree 입력

        //if (m_bSaveEngineImg)
        {
            QString str;
            str.sprintf(("%d_MeasureAlignDst.jpg"), 121);
            SaveOutImage(src, nullptr, str);
        }
    }
    else if (nMesAlingNum) // nMesAlingNum == 1 이면 가로 또는 세로위치만 이동한다. (한개만 사용)
    {
        int nDirection;
        RoiObject* pData = vecAlign[0];
        if (pData->getIntParam(("Direction"), nDirection) == 0) {

            IplImage* croppedImage;
            QRectF rect = pData->bounds();	// Area로 등록된 ROI
            cv::Point2f left_top = cv::Point2f(rect.left(), rect.top());
            cvSetImageROI(src, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
            croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), IPL_DEPTH_8U, 1);
            if (src->nChannels == 3)
                cvCvtColor(src, croppedImage, CV_RGB2GRAY);
            else
                cvCopy(src, croppedImage);
            cvResetImageROI(src);
            double dEdge = SingleROISubPixEdgeWithThreshold(croppedImage, pData, rect);
            cvReleaseImage(&croppedImage);

            if (nDirection <= 1) { // Vertical edge
                double dShift = pData->bounds().x() - dEdge; // 좌우 shift만 적용

                int sx = (int)round(dShift);
                if (sx < 0) { // shift left
                    sx = abs(sx);
                    cv::Mat m = shiftFrame(cv::cvarrToMat(src), abs(sx), ShiftLeft);
                    IplImage iplImage = m;
                    cvCopy(&iplImage, src, nullptr);
                }
                else if (sx > 0) { // shift right
                    sx = abs(sx);
                    cv::Mat m = shiftFrame(cv::cvarrToMat(src), abs(sx), ShiftRight);
                    IplImage iplImage = m;
                    cvCopy(&iplImage, src, nullptr);
                }
            }

            //if (m_bSaveEngineImg)
            {
                QString str;
                str.sprintf(("%d_MeasureAlignDst.jpg"), 122);
                SaveOutImage(src, nullptr, str);
            }
        }
    }

    return nErrorType;
}


int CImgAlign::GetCornerAlignPoint(IplImage* graySearchImg)
{
    //QString str;
    DocumentView* v = theMainWindow->currentView();

    // inspsect image
    for (const Layer *layer : v->mRoi->objectGroups()) {
        const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
        for (const Qroilib::RoiObject *roiObject : objectGroup) {
            Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;

                IplImage* croppedImage;
                QRectF rect = mObject->bounds();	// Area로 등록된 ROI
                QRectF rect1 = rect;
                rect.normalized();
                if (rect.left() < 0)	rect.setLeft(0);
                if (rect.top() < 0)	rect.setTop(0);
                if (rect.right() >= graySearchImg->width)
                    rect.setRight(graySearchImg->width);
                if (rect.bottom() >= graySearchImg->height)
                    rect.setBottom(graySearchImg->height);
                mObject->setBounds(rect);

                cv::Point2f left_top = cv::Point2f(rect.left(), rect.top());
                cvSetImageROI(graySearchImg, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
                croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), graySearchImg->depth, graySearchImg->nChannels);
                cvCopy(graySearchImg, croppedImage);
                cvResetImageROI(graySearchImg);

                mObject->m_vecDetectResult.clear();
                SingleROICorner(croppedImage, mObject, rect);
                mObject->setBounds(rect1);

                cvReleaseImage(&croppedImage);

                int size = mObject->m_vecDetectResult.size();
                if (size > 0)
                {
                    DetectResult *prst = &mObject->m_vecDetectResult[0];
                    prst->pt.x += rect1.left();
                    prst->pt.y += rect1.top();
                    insppt.push_back(prst->pt);
                }

        }
    }


    return 0;
}


//
// 에지의 Gray변화가 많은 부분의 위치를 추출
// Threshold를 이용하여 Edge의 경계면을 추출하고 경계면으로부터 Subpixel edge를 추출한다.
// * Hessian Matrix 를 이용한것보다 좀더 경사진면에서의 edge를 구할수 있다.
//
double CImgAlign::SingleROISubPixEdgeWithThreshold(IplImage* croppedImage, RoiObject *pData, QRectF rect)
{
    QString str;
    IplImage* grayImg = cvCloneImage(croppedImage);
    vector<cv::Point2f> vecEdges;

    int nThresholdValue = 0;
    CParam *pParam = pData->getParam(("High Threshold"));
    if (pParam)
        nThresholdValue = pParam->Value.toDouble();

    ThresholdRange(pData, grayImg, 211);

    NoiseOut(pData, grayImg, _ProcessValue1, 212);

    //cvNot(grayImg, grayImg);


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

    int nDir = 0;
    pParam = pData->getParam(("Direction")); // ("Left2Right,Right2Left,Top2Bottom,Bottom2Top")),
    if (pParam)
        nDir = (int)pParam->Value.toDouble();

    int nColor;
    int nPolarity = 0;
    int nDetectMethod = 0;
    pParam = pData->getParam(("Polarity")); // ("White2Black, Black2White")
    if (pParam)
        nPolarity = (int)pParam->Value.toDouble();
    if (nPolarity == 0)
        nColor = 255;
    else
        nColor = 0;
    pParam = pData->getParam(("Detect method")); // ("Average,First")
    if (pParam)
        nDetectMethod = (int)pParam->Value.toDouble();

    uchar* data = (uchar*)grayImg->imageData;
    int widthStep = grayImg->widthStep;
    int cx = grayImg->width;
    int cy = grayImg->height;

    // Threshold한 결과 이미지의 경계면을 구한다. Left2Right,Right2Left,Top2Bottom,Bottom2Top에 따라서
    bool bChange;
    switch (nDir)
    {
    case 0: //Left2Right
        for (int y = 0; y < cy; y++)
        {
            bChange = false;
            for (int x = 0; x < cx; x++)
            {
                int index = x + y*widthStep;
                if (data[index] == nColor)
                    bChange = true;
                if (bChange && data[index] != nColor) {
                    vecEdges.push_back(cv::Point2f(x, y));
                    break;
                }
            }
        }
        break;
    case 1: //Right2Left
        for (int y = 0; y < cy; y++)
        {
            bChange = false;
            for (int x = cx - 1; x >= 0; x--)
            {
                int index = x + y*widthStep;
                if (data[index] == nColor)
                    bChange = true;
                if (bChange && data[index] != nColor) {
                    vecEdges.push_back(cv::Point2f(x, y));
                    break;
                }
            }
        }
        break;
    case 2: //Top2Bottom
        for (int x = 0; x < cx; x++)
        {
            bChange = false;
            for (int y = 0; y < cy; y++)
            {
                int index = x + y*widthStep;
                if (data[index] == nColor)
                    bChange = true;
                if (bChange && data[index] != nColor) {
                    vecEdges.push_back(cv::Point2f(x, y));
                    break;
                }
            }
        }
        break;
    case 3: //Bottom2Top
        for (int x = 0; x < cx; x++)
        {
            bChange = false;
            for (int y = cy - 1; y >= 0; y--)
            {
                int index = x + y*widthStep;
                if (data[index] == nColor)
                    bChange = true;
                if (bChange && data[index] != nColor) {
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
    int first = sz * 0.4;
    int last = sz - (sz * 0.4);
    if (first < 0)
        first = 0;
    if (last < 0)
        last = 0;

    if (nDetectMethod == 0)
    {
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
    }
    else {
        //제일 처음 만나는 edge를 구한다
        if (sz > 0) {
            if (nDir == 0)  // Left2Right 세로선
                dVal += vecEdges[0].x;
            else if (nDir == 1)  // Right2Left 세로선
                dVal += vecEdges[sz - 1].x;
            else if (nDir == 2)  // Top2Bottom 가로선
                dVal += vecEdges[0].y;
            else				//Bottom2Top
                dVal += vecEdges[sz - 1].y;
        }
    }
    //

    cvReleaseImage(&grayImg);
    //// 여기까지 Threshold를 이용한 edge를 구하였다.


    //
    // dVal이 Threshold를 이용한 edge값인데, 이값을 기준으로 Ramp edge를 구한다.
    //
    int width = 6;
    pParam = pData->getParam(("Ramp width")); // default : 6
    if (pParam)
        width = (int)pParam->Value.toDouble();



    int start = 0;
    int end = 0;
    IplImage* edgeImage = nullptr;
    switch (nDir) // ("Left2Right,Right2Left,Top2Bottom,Bottom2Top")),
    {
    case 0: //Left2Right,Right2Left
    case 1:
        start = dVal - width;
        end = dVal + width;
        if (start < 0) start = 0;
        if (end > cx) end = cx;

        cvSetImageROI(croppedImage, cvRect((int)start, (int)0, end - start, croppedImage->height));
        edgeImage = cvCreateImage(cvSize(end - start, croppedImage->height), croppedImage->depth, croppedImage->nChannels);
        cvCopy(croppedImage, edgeImage);
        cvResetImageROI(croppedImage);

        //if (m_bSaveEngineImg)
        {
            SaveOutImage(edgeImage, pData, ("251_RampImg.jpg"), false);
        }
        break;
    case 2: //Top2Bottom,Bottom2Top
    case 3:
        start = dVal - width;
        end = dVal + width;
        if (start < 0) start = 0;
        if (end > cy) end = cy;

        cvSetImageROI(croppedImage, cvRect((int)0, (int)start, croppedImage->width, end - start));
        edgeImage = cvCreateImage(cvSize(croppedImage->width, end - start), croppedImage->depth, croppedImage->nChannels);
        cvCopy(croppedImage, edgeImage);
        cvResetImageROI(croppedImage);

        //if (m_bSaveEngineImg)
        {
            SaveOutImage(edgeImage, pData, ("252_RampImg.jpg"), false);
        }
        break;
    }

    double dEdge = SubPixelRampEdgeImage(edgeImage, nDir);
    //if (m_bSaveEngineImg)
    {
        SaveOutImage(edgeImage, pData, ("260_SubPixelRampEdgeImageIn.jpg"), false);
    }

    if (edgeImage) cvReleaseImage(&edgeImage);

    //DetectResult result;
    switch (nDir)
    {
    case 0: // Left2Right 세로선
    case 1: // Right2Left
        dEdge += start;
        m_DetectResult.pt.x = dEdge;
        m_DetectResult.pt.y = rect.height() / 2;
        break;
    case 2: // 가로선
    case 3:
        dEdge += start;
        m_DetectResult.pt.y = dEdge;
        m_DetectResult.pt.x = rect.width() / 2;
        break;
    }

    pData->m_vecDetectResult.push_back(m_DetectResult);


    return dEdge;
}

int CImgAlign::SingleROICorner(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect)
{
    Q_UNUSED(rect);
    int nCornerType = 1;
    int nFindMethod = 1; // 0 - 코너 찾기, 1-가로,세로 Line으로 코너 찾기
    nCornerType = 0;	// 좌상단 코너
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Corner"));
        if (pParam)
            nCornerType = pParam->Value.toDouble();
        pParam = pData->getParam(("Method"));
        if (pParam)
            nFindMethod = pParam->Value.toDouble();
    }


    CvPoint2D32f outCorner;

    int nRet2 = 0;
    if (nFindMethod == 0)
        nRet2 = EdgeCorner(pData, croppedImage, nCornerType, outCorner);
    else
        nRet2 = EdgeCornerByLine(pData, croppedImage, nCornerType, outCorner);

    if (nRet2 == 0)	// Blob처리를 한 Cornet 찾기
    {
        CvPoint2D32f pt;

        pt = outCorner;

        //DetectResult result;
        m_DetectResult.pt = pt;
        pData->m_vecDetectResult.push_back(m_DetectResult);

    }
    return 0;
}

//
// CornerType : 0 - Upper Left, 1 - Upper Right, 2 - Bottom Left, 3 - Bottom Right
//
int CImgAlign::EdgeCorner(Qroilib::RoiObject *pData, IplImage* graySearchImgIn, int CornerType, CvPoint2D32f &outCorner)
{
    outCorner = CvPoint2D32f(0, 0);
    QString str;
    IplImage* graySearchImg = cvCloneImage(graySearchImgIn);

    //if (m_bSaveEngineImg)
    {
        SaveOutImage(graySearchImg, pData, ("310_EdgeCornerSrc.jpg"));
    }

    int nThresholdValue = 0;
    CParam *pParam = pData->getParam(("High Threshold"));
    if (pParam)
        nThresholdValue = pParam->Value.toDouble();

    ThresholdRange(pData, graySearchImg, 311);

    NoiseOut(pData, graySearchImg, -1, 312);

    CBlobResult blobs;
    blobs = CBlobResult(graySearchImg, nullptr, 0);	// Use a black background color.
    int n = blobs.GetNumBlobs();

#if 1
    // 가장큰 Blob 2개만 남김
    n = blobs.GetNumBlobs();
    if (n > 1)
    {
        double dLargeArea = 0;
        double_stl_vector area = blobs.GetSTLResult(CBlobGetArea());
        std::stable_sort(area.begin(), area.end(), [](const double lhs, const double rhs)->bool {
            return lhs > rhs;
        });
        if (area.size() >= 2)
            dLargeArea = area[1];
        else if (area.size() >= 1)
            dLargeArea = area[0];
        blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, dLargeArea);
    }

    // 정사각형에 가까운 큰 Blob 2개만 남김
    n = blobs.GetNumBlobs();
    if (n > 1)
    {
        int nPos = 0;
        double_stl_vector AxisRatio = blobs.GetSTLResult(CBlobGetAxisRatio());
        std::stable_sort(AxisRatio.begin(), AxisRatio.end(), [](const double lhs, const double rhs)->bool {
            return lhs < rhs;
        });
        if (AxisRatio.size() >= 2)
            nPos = 1;
        blobs.Filter(blobs, B_INCLUDE, CBlobGetAxisRatio(), B_LESS_OR_EQUAL, AxisRatio[nPos]); // 정사각형에 가까운  blob 2개만 남김
    }
#endif

    cvZero(graySearchImg);
    for (int i = 0; i < blobs.GetNumBlobs(); i++) {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(graySearchImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
    }

    //if (m_bSaveEngineImg)
    {
        SaveOutImage(graySearchImg, pData, ("316_imageLargeBlobs.jpg"));
    }

#if 1
    // 코너위치를 보고 가까운 Blob만 남긴다.
    typedef struct _tagBlobInfo {
        cv::Point2f pt;
        double area;
    } BLOBSEL;
    vector<BLOBSEL*> vecBlobs;
    BLOBSEL *pBlobSel;
    cvZero(graySearchImg);
    for (int i = 0; i < blobs.GetNumBlobs(); i++) {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(graySearchImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.

        CvRect rect = currentBlob->GetBoundingBox();
        pBlobSel = new BLOBSEL;
        pBlobSel->area = currentBlob->Area();
        pBlobSel->pt.x = (rect.x + rect.width / 2) / 4; // Blob의 중앙점
        pBlobSel->pt.y = (rect.y + rect.height / 2) / 4;
        vecBlobs.push_back(pBlobSel);
    }

    int size = vecBlobs.size();
    if (size > 0)
    {
        switch (CornerType)
        {
        case 0: // UpperLeft - 위쪽 Blob, 왼쪽 Blob만 남긴다
            std::stable_sort(vecBlobs.begin(), vecBlobs.end(), [](const BLOBSEL* lhs, const BLOBSEL* rhs)->bool {
                if (lhs->pt.x > rhs->pt.x)
                    return true;
                return false;
            });
            pBlobSel = vecBlobs[0];
            blobs.Filter(blobs, B_INCLUDE, CBlobGetArea(), B_EQUAL, pBlobSel->area);
            break;
        case 2: // BottomLeft
            std::stable_sort(vecBlobs.begin(), vecBlobs.end(), [](const BLOBSEL* lhs, const BLOBSEL* rhs)->bool {
                if (lhs->pt.x > rhs->pt.x)
                    return true;
                return false;
            });
            pBlobSel = vecBlobs[0];
            blobs.Filter(blobs, B_INCLUDE, CBlobGetArea(), B_EQUAL, pBlobSel->area);
            break;
        case 1: // UpperRight
            std::stable_sort(vecBlobs.begin(), vecBlobs.end(), [](const BLOBSEL* lhs, const BLOBSEL* rhs)->bool {
                if (lhs->pt.x < rhs->pt.x)
                    return true;
                return false;
            });
            pBlobSel = vecBlobs[0];
            blobs.Filter(blobs, B_INCLUDE, CBlobGetArea(), B_EQUAL, pBlobSel->area);
            break;
        case 3: // BottomRight - 오른쪽 Blob, 아래쪽 blob만 남긴다.
            std::stable_sort(vecBlobs.begin(), vecBlobs.end(), [](const BLOBSEL* lhs, const BLOBSEL* rhs)->bool {
                if (lhs->pt.x < rhs->pt.x)
                    return true;
                return false;
            });
            pBlobSel = vecBlobs[0];
            blobs.Filter(blobs, B_INCLUDE, CBlobGetArea(), B_EQUAL, pBlobSel->area);
            break;
        }
    }
    size = vecBlobs.size();
    for (int i = 0; i < size; i++)
        delete vecBlobs[i];
#endif

    // Filter된  blob을 이미지로 변환
    cvZero(graySearchImg);
    for (int i = 0; i < blobs.GetNumBlobs(); i++) {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(graySearchImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
    }

    //if (m_bSaveEngineImg)
    {
        SaveOutImage(graySearchImg, pData, ("317_imageLargeBlobs.jpg"));
    }


    // 2. 윤곽선 표시 및 윤곽선 영상 생성

    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* contours = 0;
    cvFindContours(graySearchImg, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    //if (m_bSaveEngineImg)
    {
        cvZero(graySearchImg);
        //cvDrawContours( graySearchImg, contours, CV_RGB (255, 255, 0), CV_RGB (0, 255, 0), -1, 1, CV_AA, cvPoint (blob.m_recBlobs[i].x, blob.m_recBlobs[i].y));
        cvDrawContours(graySearchImg, contours, CV_RGB(255, 255, 255), CV_RGB(255, 255, 255), 2, 1, 8, cvPoint(0, 0));
        SaveOutImage(graySearchImg, pData, ("323_cvDrawContours.jpg"));
    }

    // 3. 꼭지점 추출

    std::vector<cv::Point2f> corners;
    int ret = -1;

    CvSeq* approxDP_seq = nullptr;
    double d = 0;
    if (contours != nullptr && contours->total >= 4)
    {
        while (1) {
            approxDP_seq = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, d, 1);
            d++;
            if (approxDP_seq->total > 4)
                continue;
            break;
        }

        //if (m_bSaveEngineImg)
        {
            IplImage *tempImage = cvCreateImage(cvSize(graySearchImg->width, graySearchImg->height), graySearchImg->depth, 3);
            cvDrawContours(tempImage, approxDP_seq, CVX_RED, CVX_RED, 1, 1, 8); // 외곽선 근사화가 잘되었는지 테스트
            SaveOutImage(tempImage, pData, ("324_cvApproxPoly.jpg"));
            cvReleaseImage(&tempImage);
        }

        for (int i=0; i<approxDP_seq->total; i++) {
            CvPoint* pt = (CvPoint *)cvGetSeqElem(approxDP_seq, i);
            corners.push_back(*pt);
        }
    }

    cv::Point2f center(0, 0);
    // Get mass center
    for (int i = 0; i < (int)corners.size(); i++)
        center += corners[i];
    center *= (1. / corners.size());

    cv::Point2f pt;
    if (corners.size() > 0)
    {
        pt = getCorner(corners, center, CornerType);
        outCorner = pt;
        ret = 0;
    }
    else
        ret = -1;

    //if (m_bSaveEngineImg)
    {
        cv::Point2f pt1 = cv::Point2f(pt.x - 30, pt.y - 30);
        cv::Point2f pt2 = cv::Point2f(pt.x + 30, pt.y + 30);
        cvRectangle(graySearchImg, pt1, pt2, CV_RGB(0, 84, 255), 3, 8, 0);
        //cvSetImageROI(graySearchImg, cvRect((int)pt1.x, (int)pt1.y, 200, 200));
        //cvAddS(graySearchImg, cvScalar(150), graySearchImg);
        //cvResetImageROI(graySearchImg);

        cvCircle(graySearchImg, pt, 3, CV_RGB(255, 255, 255), 2);

        SaveOutImage(graySearchImg, pData, ("335_Corners.jpg"));
    }

    //CvFont font;
    //cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, .5, .5, 0, 1, 8);

    cvReleaseMemStorage(&storage);
    cvReleaseImage(&graySearchImg);

    //cvReleaseStructuringElement(&element);
    return ret;
}

//
// CornerType : 0 - Upper Left, 1 - Upper Right, 2 - Bottom Left, 3 - Bottom Right
//
int CImgAlign::EdgeCornerByLine(Qroilib::RoiObject *pData, IplImage* grayCroppedImgIn, int CornerType, CvPoint2D32f &outCorner)
{
    outCorner = CvPoint2D32f();
    QString str;
    IplImage* grayCroppedImg = cvCloneImage(grayCroppedImgIn);

    //if (m_bSaveEngineImg)
    {
        str.sprintf(("209_GrayImage.jpg"));
        SaveOutImage(grayCroppedImg, pData, str);
    }

    int nThresholdValue = 0;
    CParam *pParam = pData->getParam(("High Threshold"));
    if (pParam)
        nThresholdValue = pParam->Value.toDouble();

    ThresholdRange(pData, grayCroppedImg, 211);

    NoiseOut(pData, grayCroppedImg, -1, 212);

    IplImage* croppedImageVerify = cvCloneImage(grayCroppedImg);

    /////////////////////////
    // 가장큰 Blob만 남김
    ////////////////////////
    CBlobResult blobs;
    blobs = CBlobResult(grayCroppedImg, nullptr, 0);	// Use a black background color.
    double dLargeArea = 0;
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++) {
        CBlob *p = blobs.GetBlob(i);
        double dArea = p->Area();
        if (dLargeArea < dArea) {
            dLargeArea = dArea;
        }
    }
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, dLargeArea);
    int NumOfBlob = blobs.GetNumBlobs();
    if (NumOfBlob < 1)
    {
        if (grayCroppedImg) cvReleaseImage(&grayCroppedImg);
        return -1;
    }
    cvZero(grayCroppedImg);
    // large blobs.
    CBlob *currentBlob = blobs.GetBlob(0);
    currentBlob->FillBlob(grayCroppedImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
    CvRect rect = currentBlob->GetBoundingBox();

    //if (m_bSaveEngineImg)
    {
        str.sprintf(("217_imageLargeBlobs.jpg"));
        SaveOutImage(grayCroppedImg, pData, str);
    }

    rect.x -= 5;
    rect.y -= 5;
    rect.width += 5;
    rect.height += 5;
    if (rect.x < 0)
        rect.x = 0;
    if (rect.y < 0)
        rect.y = 0;
    if (rect.width+rect.x >= grayCroppedImg->width)
        rect.width = grayCroppedImg->width - rect.x;
    if (rect.height+rect.y >= grayCroppedImg->height)
        rect.height = grayCroppedImg->height - rect.y;
    cvSetImageROI(grayCroppedImg, cvRect((int)rect.x, (int)rect.y, rect.width, rect.height));
    IplImage* workImg = cvCreateImage(cvSize(rect.width, rect.height), IPL_DEPTH_8U, 1);
    cvCopy(grayCroppedImg, workImg);
    cvResetImageROI(grayCroppedImg);

    //if (m_bSaveEngineImg)
    {
        str.sprintf(("218_workImg.jpg"));
        SaveOutImage(workImg, pData, str);
    }

    double dRejectLow = 0.45; // 양쪽 45%를 버리고 중앙 10%만 사용한다.
    double dRejectHigh = 0.45;

    int w = (int)((float)workImg->width * 0.3); // ROI영역 30%만 사용한다.
    int h = workImg->height;

    cv::Point2f pt = cv::Point2f(0,0);
    //Corner Position 에 따라 Line Edge 추출 방향이 다르다.
    switch (CornerType) {
    case 0 : // Upper Left // White to Black edge, 중앙에서 좌, 중앙에서 위
        pt.x = ROIPixEdge(workImg, 1, dRejectLow, dRejectHigh); // Right2Left
        pt.y = ROIPixEdge(workImg, 3, dRejectLow, dRejectHigh); // Bottom2Top
        break;
    case 1: // Upper Right
        pt.x = ROIPixEdge(workImg, 0, dRejectLow, dRejectHigh); // Left2Right
        pt.y = ROIPixEdge(workImg, 3, dRejectLow, dRejectHigh); // Bottom2Top
        break;
    case 2: // Bottom Left
        pt.x = ROIPixEdge(workImg, 1, dRejectLow, dRejectHigh); // Right2Left
        pt.y = ROIPixEdge(workImg, 2, dRejectLow, dRejectHigh); // Top2Bottom
        break;
    case 3: // Bottom Right
        pt.x = ROIPixEdge(workImg, 0, dRejectLow, dRejectHigh); // Left2Right
        pt.y = ROIPixEdge(workImg, 2, dRejectLow, dRejectHigh); // Top2Bottom
        break;
    }
    int ret = 0;
    w = workImg->width - 1;
    h = workImg->height - 1;
    if (pt.x <= 1 || pt.x >= w || pt.y <= 1 || pt.y >= h) {
        str.sprintf(("Edge position is out range of ROI"));
        //g_cLog->AddLog(_LOG_LIST_SYS, str);
        qDebug() << str;
        ret = -1;
    }

    pt.x += rect.x;
    pt.y += rect.y;

    outCorner = pt;
    m_DetectResult.pt = pt;

    //if (m_bSaveEngineImg)
    {
        cv::Point2f pt1 = cv::Point2f(pt.x - 20, pt.y - 20);
        cv::Point2f pt2 = cv::Point2f(pt.x + 20, pt.y + 20);
        cvRectangle(grayCroppedImg, pt1, pt2, CV_RGB(0, 84, 255), 3, 8, 0);

        cvSetImageROI(grayCroppedImg, cvRect((int)pt1.x, (int)pt1.y, 40, 40));
        cvAddS(grayCroppedImg, cvScalar(150), grayCroppedImg);
        cvResetImageROI(grayCroppedImg);

        cvCircle(grayCroppedImg, pt, 3, CVX_RED, 2);
        str.sprintf(("235_Corners.jpg"));
        SaveOutImage(grayCroppedImg, pData, str);
    }

    if (croppedImageVerify) cvReleaseImage(&croppedImageVerify);
    if (grayCroppedImg) cvReleaseImage(&grayCroppedImg);
    if (workImg) cvReleaseImage(&workImg);
    return ret;
}


//
// 사용자가 설정한 값으로 Threshold를 실행
//
int CImgAlign::ThresholdRange(RoiObject *pData, IplImage* grayImg, int nDbg)
{
    QString str;

    int nThresholdLowValue = 70;
    int nThresholdHighValue = 255;

    //if (m_bSaveEngineImg)
    {
        str.sprintf(("%03d_cvThresholdRange.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }

    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Low Threshold"));
        if (pParam)
            nThresholdLowValue = pParam->Value.toDouble();
        pParam = pData->getParam(("High Threshold"));
        if (pParam)
            nThresholdHighValue = pParam->Value.toDouble();
    }
    if (nThresholdHighValue == 0 && nThresholdLowValue == 0)
        return -1;

    cvInRangeS(grayImg, cv::Scalar(nThresholdLowValue), cv::Scalar(nThresholdHighValue), grayImg);

    //if (m_bSaveEngineImg)
    {
        str.sprintf(("%03d_cvThresholdRange.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }

    return 0;
}

//
// 모폴리지를 이용하여 잡음제거.
//
int CImgAlign::NoiseOut(RoiObject *pData, IplImage* grayImg, int t, int nDbg, int h)
{
    QString str;

    if (t < 0)
        t = _ProcessValue1;

    // 1. Template이미지의 노이즈 제거.
    int filterSize = 3;  // 필터의 크기를 3으로 설정 (Noise out area)
    IplConvKernel *element = nullptr;
    if (filterSize <= 0)
        filterSize = 1;
    if (filterSize % 2 == 0)
        filterSize++;
    element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, nullptr);

    int nNoiseout = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Noise out 1"),t);
        if (pParam)
            nNoiseout = pParam->Value.toDouble();
    }
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_OPEN, -nNoiseout);
        else //if (nNoiseout > 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);
    }

    nNoiseout = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Noise out 2"),t);
        if (pParam)
            nNoiseout = pParam->Value.toDouble();
    }
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_OPEN, -nNoiseout);
        else //if (nNoiseout > 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);
    }

    nNoiseout = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Noise out 3"),t);
        if (pParam)
            nNoiseout = pParam->Value.toDouble();
    }
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_OPEN, -nNoiseout);
        else //if (nNoiseout > 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);
    }

    {
        if (h >= 0)
            str.sprintf(("%d_%03d_cvClose.jpg"), h, nDbg);
        else str.sprintf(("%03d_cvClose.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }

    cvReleaseStructuringElement(&element);
    return 0;
}


void CImgAlign::SaveOutImage(IplImage* pImgOut, RoiObject *pData, QString strMsg, bool bClear/*=false*/)
{
    QString str = ("");
    if (pData != nullptr)
        str.sprintf("./[%s]%s_%s", pData->groupName().toStdString().c_str(), pData->name().toStdString().c_str(), strMsg.toStdString().c_str());
    else
        str.sprintf("./%s", strMsg.toStdString().c_str());
    cvSaveImage((const char *)str.toStdString().c_str(), pImgOut);
    if (bClear) cvZero(pImgOut);
}
