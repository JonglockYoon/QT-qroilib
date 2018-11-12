// imgprocengine.cpp
// Copyright 2018 jerry1455@gmail.com
//
// Qroilib를 이용할때 이용가능한 opencv함수들을 모아놓은 module이다.
// application개발시 필요한 함수들만 다시 구성해서 새로운 engine module을 만드는것이
// 유지보수에 이롭다.
//
#include <tesseract/baseapi.h>  //  Includes Tesseract and Leptonica libraries
#include <leptonica/allheaders.h>

#include <stdio.h>
#include <QDir>
#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>

#include "imgprocengine.h"
#include "objectgroup.h"
#include "config.h"
#include "recipedata.h"
#include "mainwindow.h"
#include "MatToQImage.h"
#include "geomatch.h"

#include "QZXing.h"
#include <zxing/NotFoundException.h>
#include <zxing/ReaderException.h>

using namespace zxing;
using namespace tesseract;
//using namespace cv;

CImgProcEngine::CImgProcEngine()
{
    qDebug() << "CImgProcEngine";

    memset(&m_DetectResult, 0, sizeof(m_DetectResult));
    //m_sDebugPath = ".";
    m_bSaveEngineImg = false;

    QString str;

    //CConfig *pCfg = &gCfg;

    qDebug() << gCfg.RootPath;
    if (gCfg.m_sSaveImageDir.isEmpty())
        str = QString("%1/Engine").arg(gCfg.RootPath);
    else
        str = QString("%1/Engine").arg(gCfg.m_sSaveImageDir);
    QDir dir;
    dir.mkdir(str);
    m_sDebugPath = str;
    curImg = nullptr;


    tessApi = new tesseract::TessBaseAPI();
    init_tess_failed = tessApi->Init("./tessdata", "eng");
    if (init_tess_failed) {
        qDebug() << "Could not initialize tesseract.";
    }

}

CImgProcEngine::~CImgProcEngine(void)
{
    if (tessApi)
        tessApi->End();
}

int CImgProcEngine::InspectOneItem(IplImage* img, RoiObject *pData)
{
    if (pData->mParent != nullptr) {
        if (pData->mParent->mInspectType < _INSPACT_ROI_START)
            return -1;
    }

    QString str;
    str.sprintf("InspectOneItem type=%d", pData->mInspectType);
    theMainWindow->DevLogSave(str.toLatin1().data());
	m_DetectResult.dRadius = 0;
    m_DetectResult.img = nullptr;

    if (pData == nullptr)
		return -1;

    curImg = img;

    if (m_bSaveEngineImg)
    {
        str.sprintf("0_%d_srcImage.jpg", 100);
        SaveOutImage(img, pData, str, false);
    }

	int size = pData->m_vecDetectResult.size();
	for (int i = 0; i < size; i++) {
		DetectResult *prst = &pData->m_vecDetectResult[i];
        if (prst->img)
            cvReleaseImage(&prst->img);
	}
	pData->m_vecDetectResult.clear();

    CvSize searchSize = cvSize(img->width, img->height);
	IplImage* graySearchImg = cvCreateImage(searchSize, IPL_DEPTH_8U, 1);
	QString strLog;

    if (img->nChannels == 3)
		cvCvtColor(img, graySearchImg, CV_RGB2GRAY);
    else if (img->nChannels == 4) {
        if (strncmp(img->channelSeq, "BGRA", 4) == 0)
            cvCvtColor(img, graySearchImg, CV_BGRA2GRAY);
        else
            cvCvtColor(img, graySearchImg, CV_RGBA2GRAY);
    } else
		cvCopy(img, graySearchImg);

//    if (m_bSaveEngineImg)
//	{
//        str.sprintf(("0_%d_grayImage.jpg"), 110);
//        SaveOutImage(graySearchImg, pData, str, false);
//	}

	IplImage* croppedImage;
    QRectF rect = pData->bounds();	// Area로 등록된 ROI
    rect.normalized();

    if (rect.left() < 0)	rect.setLeft(0);
    if (rect.top() < 0)	rect.setTop(0);
    if (rect.right() >= graySearchImg->width) rect.setRight(graySearchImg->width);
    if (rect.bottom() >= graySearchImg->height) rect.setBottom(graySearchImg->height);
    pData->setBounds(rect);

    Point2f left_top = Point2f(rect.left(), rect.top());
    cvSetImageROI(graySearchImg, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
    croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), graySearchImg->depth, graySearchImg->nChannels);
	cvCopy(graySearchImg, croppedImage);
	cvResetImageROI(graySearchImg);

    //int nDirection;
	switch (pData->mInspectType)
	{
    case _Inspect_Patt_Identify:
        //strLog.Format(("[%s] InspectType : _Inspect_Patt_Identify"), pData->m_sName);
        //MSystem.DevLogSave(("%s"), strLog);
        SinglePattIdentify(croppedImage, pData, rect);
        break;
    case _Inspect_Patt_MatchShapes:
        //strLog.sprintf(("[%s] InspectType : _Inspect_Patt_MatchShapes"), pData->name().toStdString().c_str());
        //theMainWindow->DevLogSave(strLog.toLatin1().data());
        SinglePattMatchShapes(croppedImage, pData, rect);
        break;
    case _Inspect_Patt_FeatureMatch:
        SinglePattFeatureMatch(croppedImage, pData, rect);
        break;

    case _Inspect_Roi_CenterOfPlusMark:
        SingleROICenterOfPlusMark(croppedImage, pData, rect);
        break;
	case _Inspect_Roi_SubpixelEdgeWithThreshold:
        //strLog.sprintf(("[%s] InspectType : _Inspect_Roi_SubpixelEdgeWithThreshold"), pData->name().toStdString().c_str());
        //g_cLog->AddLog(strLog, _LOG_LIST_INSP);
        SingleROISubPixEdgeWithThreshold(croppedImage, pData, rect);
		break;
    case _Inspect_Roi_Circle_With_Threshold:
        SingleROICircleWithThreshold(croppedImage, pData, rect);
        break;
    case _Inspect_Roi_Find_Shape:
        SingleROIFindShape(croppedImage, pData, rect);
        break;
    case _Inspect_Roi_Corner:
        SingleROICorner(croppedImage, pData, rect);
        break;
    case _Inspect_Teseract:
        SingleROIOCR(croppedImage, pData, rect);
        break;
    case _Inspect_BarCode:
        SingleROIBarCode(croppedImage, pData, rect);
        break;
    }
	cvReleaseImage(&croppedImage);
	cvReleaseImage(&graySearchImg);

	return 0;
}



int CImgProcEngine::GetAlignPtWithMask(RoiObject* pData, IplImage* graySearchImg)
{
    QString str;

    // mask image
    char strTemp[256];
    sprintf(strTemp, ("%s\\TeachingData\\%s\\%s.jpg"), gCfg.RootPath.toLatin1().data(), gCfg.m_sLastRecipeName.toLatin1().data(), ("maskimage"));
    IplImage* maskImg = cvLoadImage(strTemp, CV_LOAD_IMAGE_GRAYSCALE);

    DocumentView* v = theMainWindow->currentView();
    // mask image
    if (maskImg)
    {
        for (const Layer *layer : v->mRoi->objectGroups()) {
            const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
            for (const Qroilib::RoiObject *roiObject : objectGroup) {
                Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;

                    IplImage* croppedImage;
                    QRectF rect = pData->bounds();	// Area로 등록된 ROI
                    QRectF rect1 = rect;
                    rect.normalized();

                    if (rect.left() < 0)	rect.setLeft(0);
                    if (rect.top() < 0)	rect.setTop(0);
                    if (rect.right() >= maskImg->width)
                        rect.setRight(maskImg->width);
                    if (rect.bottom() >= maskImg->height)
                        rect.setBottom(maskImg->height);
                    mObject->setBounds(rect);

                    Point2f left_top = Point2f(rect.left(), rect.top());
                    cvSetImageROI(maskImg, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
                    croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), maskImg->depth, maskImg->nChannels);
                    cvCopy(maskImg, croppedImage);
                    cvResetImageROI(maskImg);

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
                        alignpt.push_back(prst->pt);
                    }
                }
        }
    }

    // inspsect image
    for (const Layer *layer : v->mRoi->objectGroups()) {
        const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
        for (const Qroilib::RoiObject *roiObject : objectGroup) {
            Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;

                IplImage* croppedImage;
                QRectF rect = pData->bounds();	// Area로 등록된 ROI
                QRectF rect1 = rect;
                rect.normalized();
                if (rect.left() < 0)	rect.setLeft(0);
                if (rect.top() < 0)	rect.setTop(0);
                if (rect.right() >= maskImg->width)
                    rect.setRight(maskImg->width);
                if (rect.bottom() >= maskImg->height)
                    rect.setBottom(maskImg->height);
                mObject->setBounds(rect);

                Point2f left_top = Point2f(rect.left(), rect.top());
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


    if (maskImg) cvReleaseImage(&maskImg);

    return 0;
}

int CImgProcEngine::TowPointAlignImage(IplImage* src)
{
    QString str;

    vector<CvPoint2D32f>	alignpt;
    vector<CvPoint2D32f>	insppt;

    alignpt.clear();
    insppt.clear();

    DocumentView* v = theMainWindow->currentView();
    for (const Layer *layer : v->mRoi->objectGroups()) {
        const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
        for (const Qroilib::RoiObject *roiObject : objectGroup) {
            Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;
            if (mObject->mInspectType == _Inspect_Roi_Corner) {
                GetAlignPtWithMask(mObject, src);
                break;
            }
        }
    }

    //두점 사이의 각도 구하기.
#define CalculDegree(from, to)  -((double)atan2(to.y - from.y, to.x - from.x) * 180.0f / PI)
    //#ifndef PI
    //#define PI 3.141592653589793f
    //#endif
#define RADIAN(angle) angle *  PI /180


    std::stable_sort(alignpt.begin(), alignpt.end(), [](const CvPoint2D32f lhs, const CvPoint2D32f rhs)->bool {
        if (lhs.y < rhs.y) // assending
            return true;
        return false;
    });
    std::stable_sort(insppt.begin(), insppt.end(), [](const CvPoint2D32f lhs, const CvPoint2D32f rhs)->bool {
        if (lhs.y < rhs.y) // assending
            return true;
        return false;
    });

    // alignpt & insppt....
    const double dResX = gCfg.m_pCamInfo[0].dResX;
    const double dResY = gCfg.m_pCamInfo[0].dResY;

    for (int i = 0; i<alignpt.size(); i++) {
        //TRACE(_T("align %.3f %.3f\n"), alignpt[i].x, alignpt[i].y);

        alignpt[i].x = alignpt[i].x * dResX;
        alignpt[i].y = alignpt[i].y * dResY;
    }
    for (int i = 0; i<insppt.size(); i++) {
        //TRACE(_T("insp %.3f %.3f\n"), insppt[i].x, insppt[i].y);

        insppt[i].x = insppt[i].x * dResX;
        insppt[i].y = insppt[i].y * dResY;
    }

    if (alignpt.size() == 2 && insppt.size() == 2)
    {

        double dTheta1 = CalculDegree(alignpt[1], alignpt[0]); // 티칭각도 -> mask
        double dTheta2 = CalculDegree(insppt[1], insppt[0]); // 안착 각도 -> insp
        double dTheta = (dTheta1 - dTheta2);
        //TRACE(_T("Compensation theta value %.3f\n"), dTheta);

        // center of image
        CvPoint2D32f dPointCenter;
        dPointCenter.x = src->width / 2.0  * dResX;
        dPointCenter.y = src->height / 2.0 * dResY;


        //align(mask) image rotate.
        char strTemp[256];
        sprintf(strTemp, ("%s\\TeachingData\\%s\\%s.jpg"), gCfg.RootPath.toLatin1().data(), gCfg.m_sLastRecipeName.toLatin1().data(), ("maskimage"));
        IplImage* maskImg = cvLoadImage(strTemp, CV_LOAD_IMAGE_GRAYSCALE);

        //dTheta -= 1.0;
        RotateImage(maskImg, dTheta*-1.0);

        double a = RADIAN(dTheta);
        CvPoint2D32f tmp;
        CvPoint2D32f dTranslateP;

        tmp.x = alignpt[0].x;
        tmp.y = alignpt[0].y;
        dTranslateP.x = (cos(a) * (tmp.x - dPointCenter.x)) - (sin(a) * (tmp.y - dPointCenter.y)) + dPointCenter.x;
        dTranslateP.y = (sin(a) * (tmp.x - dPointCenter.x)) + (cos(a) * (tmp.y - dPointCenter.y)) + dPointCenter.y;
        alignpt[0].x = dTranslateP.x;
        alignpt[0].y = dTranslateP.y;


        tmp.x = alignpt[1].x;
        tmp.y = alignpt[1].y;
        dTranslateP.x = (cos(a) * (tmp.x - dPointCenter.x)) - (sin(a) * (tmp.y - dPointCenter.y)) + dPointCenter.x;
        dTranslateP.y = (sin(a) * (tmp.x - dPointCenter.x)) + (cos(a) * (tmp.y - dPointCenter.y)) + dPointCenter.y;
        alignpt[1].x = dTranslateP.x;
        alignpt[1].y = dTranslateP.y;

        // 회전시킨후 두개지점의 X,Y이동거리를 산출한다.
        double tx1 = (alignpt[0].x - insppt[0].x);
        double ty1 = (alignpt[0].y - insppt[0].y);
        double tx2 = (alignpt[1].x - insppt[1].x);
        double ty2 = (alignpt[1].y - insppt[1].y);

        double dsx = (double)(tx1 + tx2) / 2.0; // 티칭위치를 회전이동 시킨후 안착위치와 X,Y가 움직인 거리
        double dsy = (double)(ty1 + ty2) / 2.0;
        int sx = (int)(dsx / dResX) * -1;
        int sy = (int)(dsy / dResY) * -1;
        sy = sy + 4;
        //TRACE(_T("Compensation xy value %d %d\n"), sx, sy);

        Mat matIn = cv::cvarrToMat(maskImg);
#if 1
        Mat mat;

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
        cvInRangeS(trans, cvScalar(250), cvScalar(255), trans);

        if (m_bSaveEngineImg)
        {
            str.sprintf(("205_mask.jpg"));
            SaveOutImage(trans, nullptr, str);
        }


        cvErode(trans, trans, nullptr, 1);
        cvAnd(src, trans, src);

        if (m_bSaveEngineImg)
        {
            str.sprintf(("206_grapimg.jpg"));
            SaveOutImage(src, nullptr, str);
        }

        if (maskImg) cvReleaseImage(&maskImg);
    }

    return 0;
}

int CImgProcEngine::MeasureAlignImage(IplImage* src)
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

            if (mObject->mInspectType == _Inspect_Roi_MeasureAlign){
                vecAlign.push_back(mObject);
                nMesAlingNum++;
                if (nMesAlingNum >= 3)
                    break;
            }
        }
    }
    if (nMesAlingNum == 0)
        return -1;


    if (m_bSaveEngineImg)
    {
        QString str;
        str.sprintf(("%d_MeasureAlignSrc.jpg"), 120);
        SaveOutImage(src, nullptr, str);
    }

    if (nMesAlingNum == 3) //  Measure 얼라인
    {
        struct {
            RoiObject* pData;
            double dOrgDiff; // 기준 X,Y위치
            double dEdge;
        } pos[2][3]; // [v=0,h=1] [3]

        int nVert = 0, nHoriz = 0;
        int nDirection;	//("Left2Right,Right2Left,Top2Bottom,Bottom2Top")
        for (int i = 0; i < 3; i++) {
            RoiObject* pData = vecAlign[i];

            IplImage* croppedImage;
            QRectF rect = pData->bounds();	// Area로 등록된 ROI
            Point2f left_top = Point2f(rect.left(), rect.top());
            cvSetImageROI(src, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
            croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), IPL_DEPTH_8U, 1);
            if (src->nChannels == 3)
                cvCvtColor(src, croppedImage, CV_RGB2GRAY);
            else
                cvCopy(src, croppedImage);
            cvResetImageROI(src);
            double v = SingleROISubPixEdgeWithThreshold(croppedImage, pData, rect);
            cvReleaseImage(&croppedImage);

            if (pData->getIntParam(("Direction"), nDirection) == 0) {
                if (nDirection <= 1) { // Vertical edge
                    pos[0][nVert].pData = pData;
                    pos[0][nVert].dEdge = v;
                    pos[0][nVert].dOrgDiff = pData->bounds().x() - v;
                    nVert++;
                }
                else {
                    pos[1][nHoriz].pData = pData;
                    pos[1][nHoriz].dEdge = v;
                    pos[1][nHoriz].dOrgDiff = pData->bounds().y() - v;
                    nHoriz++;
                }
            }
        }
        //g_cLog->AddLog(_LOG_LIST_SYS, ("Measure Align Start"));

        double dRetAngle = 0;
        CvPoint2D32f center;
        center.x = src->width / 2;
        center.y = src->height / 2;
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
                double dtmpDiff = pos[0][0].dOrgDiff;
                pos[0][0].dOrgDiff = pos[0][1].dOrgDiff;
                pos[0][1].dOrgDiff = dtmpDiff;
            }
            pData = pos[0][0].pData;
            pData1 = pos[0][1].pData;
            RoiObject* pData2 = pos[1][0].pData; // 가로 Measure ROI

            double dDistX = fabs(pData1->bounds().top() - pData->bounds().top());
            double dDistY = (pos[0][0].dEdge - pos[0][1].dEdge);
            dRetAngle = (atan2(dDistY, dDistX) * 180.0 / PI); // degree
            //dRetAngle = -dRetAngle;

            // 가로 Measure ROI의 중심점이 dRetAngle 많큼 회전했을때 이동 위치를 산출해서 Rotate 회전 중심을 바꾸어준다.
            // 회전후 Shift된 위치를 계산해서 이동시키기위함.
            double a = (dRetAngle * PI) / 180; // radian
            double x1 = pData2->bounds().center().x();
            double y1 = pData2->bounds().center().y();
            double x0 = src->width / 2; // 회전의 중심
            double y0 = src->height / 2;
            double x2 = cos(a) * (x1 - x0) - (-sin(a) * (y1 - y0)) + x0;
            double y2 = (-sin(a) * (x1 - x0)) + cos(a) * (y1 - y0) + y0;

            center.x -= (pos[0][0].dOrgDiff + (x1 - x2) / 2);
            center.y -= (pos[1][0].dOrgDiff + (y1 - y2) / 2);
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
                double dtmpDiff = pos[1][0].dOrgDiff;
                pos[1][0].dOrgDiff = pos[1][1].dOrgDiff;
                pos[1][1].dOrgDiff = dtmpDiff;
            }
            pData = pos[1][0].pData;
            pData1 = pos[1][1].pData;
            RoiObject* pData2 = pos[0][0].pData; // 세로 Measure ROI

            double dDistX = fabs((pData1->bounds().left() - pData->bounds().left()));
            double dDistY = (pos[1][0].dEdge - pos[1][1].dEdge);
            dRetAngle = (atan2(dDistY, dDistX) * 180.0 / PI); // degree
            dRetAngle = -dRetAngle;

            // 세로 Measure ROI의 중심점이 dRetAngle 많큼 회전했을때 이동 위치를 산출해서 Rotate 회전 중심을 바꾸어준다.
            // 회전후 Shift된 위치를 계산해서 이동시키기위함.
            double a = (dRetAngle * PI) / 180; // radian
            double x1 = pData2->bounds().center().x();
            double y1 = pData2->bounds().center().y();
            double x0 = src->width / 2; // 회전의 중심
            double y0 = src->height / 2;
            double x2 = cos(a) * (x1 - x0) - (-sin(a) * (y1 - y0)) + x0;
            double y2 = (-sin(a) * (x1 - x0)) + cos(a) * (y1 - y0) + y0;

            center.x -= (pos[0][0].dOrgDiff + (x1 - x2) / 2);
            center.y -= (pos[1][0].dOrgDiff + (y1 - y2) / 2);
        }
        else {
            nErrorType = 2;
            //g_cLog->AddLog(_LOG_LIST_SYS, ("Measure Error..."));
            return nErrorType;
        }
        RotateImage(src, dRetAngle, center); // degree 입력

        if (m_bSaveEngineImg)
        {
            QString str;
            str.sprintf(("%d_MeasureAlignDst.jpg"), 121);
            SaveOutImage(src, nullptr, str);
        }

        //g_cLog->AddLog(_LOG_LIST_SYS, ("Measure Align End..."));

    }
    else if (nMesAlingNum) // nMesAlingNum == 1 이면 가로 또는 세로위치만 이동한다. (한개만 사용)
    {
        int nDirection;
        RoiObject* pData = vecAlign[0];
        if (pData->getIntParam(("Direction"), nDirection) == 0) {

            if (nDirection == 4)  { //  (nCh == 4) 상부 Align은 원의 무게 줌심을 가지고 처리한다.
                //AlignImageCH4(pData, src);
                //return 0;
            }

            IplImage* croppedImage;
            QRectF rect = pData->bounds();	// Area로 등록된 ROI
            Point2f left_top = Point2f(rect.left(), rect.top());
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
                double v = pData->bounds().x() - dEdge; // 좌우 shift만 적용

                int sx = (int)round(v);
                if (sx < 0) { // shift left
                    sx = abs(sx);
                    cv::Mat m = shiftFrame(cvarrToMat(src), abs(sx), ShiftLeft);
                    IplImage iplImage = m;
                    cvCopy(&iplImage, src, nullptr);
                }
                else if (sx > 0) { // shift right
                    sx = abs(sx);
                    cv::Mat m = shiftFrame(cvarrToMat(src), abs(sx), ShiftRight);
                    IplImage iplImage = m;
                    cvCopy(&iplImage, src, nullptr);
                }
            }

            if (m_bSaveEngineImg)
            {
                QString str;
                str.sprintf(("%d_MeasureAlignDst.jpg"), 122);
                SaveOutImage(src, nullptr, str);
            }
        }
    }

    return nErrorType;
}


//
//
int CImgProcEngine::SingleROICenterOfPlusMark(IplImage* croppedImageIn, RoiObject *pData, QRectF rectIn)
{
	QString str;
    IplImage* drawImage = nullptr;
	IplImage* croppedImage = cvCloneImage(croppedImageIn);
    int MinMomentSize = 6, MaxMomentSize = 600;
	int nMinCircleRadius = 300;
	int nMaxCircleRadius = 1000;

	m_DetectResult.pt.x = 0;
	m_DetectResult.pt.y = 0;

    if (pData != nullptr)
	{
		CParam *pParam = pData->getParam(("Minimum circle radius"));
        if (pParam)	nMinCircleRadius = (int)pParam->Value.toDouble();
		pParam = pData->getParam(("Maximum circle radius"));
        if (pParam)	nMaxCircleRadius = (int)pParam->Value.toDouble();
		int nThresholdValue = 0;
		pParam = pData->getParam(("High Threshold"));
		if (pParam)
            nThresholdValue = pParam->Value.toDouble();

		if (nThresholdValue == 0) {
			ThresholdOTSU(pData, croppedImage, 130);
		}
		else
			ThresholdRange(pData, croppedImage, 130);

        NoiseOut(pData, croppedImage, _ProcessValue1, 131);
	}
	else {
		IplConvKernel *element;
		int filterSize = 3;
		//CBlobResult blobs;
		//int n;

		cvInRangeS(croppedImage, cvScalar(200), cvScalar(255), croppedImage);

		filterSize = 3;
        element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, nullptr);
        cvMorphologyEx(croppedImage, croppedImage, nullptr, element, CV_MOP_OPEN, 2);
        cvMorphologyEx(croppedImage, croppedImage, nullptr, element, CV_MOP_CLOSE, 2);
		cvReleaseStructuringElement(&element);

        if (m_bSaveEngineImg)
		{
			str.sprintf(("%d_Preprocess.jpg"), 132);
            SaveOutImage(croppedImage, nullptr, str, false);
		}
	}

    if (m_bSaveEngineImg) {
		drawImage = cvCreateImage(cvSize(croppedImage->width, croppedImage->height), croppedImage->depth, 3);
        cvZero(drawImage);
    }
    FilterLargeBlob(croppedImage, nMaxCircleRadius*2);
    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("0_%d_ExcludeLargeBlob.jpg"), 133);
        SaveOutImage(croppedImage, pData, str);
    }

    FilterLargeArea(croppedImage);
    if (m_bSaveEngineImg)
	{
        str.sprintf(("134_cvApproxInImage.jpg"));
        SaveOutImage(croppedImage, pData, str, false);
	}

    Expansion(pData, croppedImage, _ProcessValue1, 134);



	// 외곽선 추적 및 근사화 변수 초기화
	CvMemStorage* m_storage = cvCreateMemStorage(0); // 배열 자료(점의 좌표가 들어간다)
	CvSeq* m_seq = 0;         // 경계 계수를 저장할 변수
	CvSeq* m_approxDP_seq = 0;
    //CvSeq* m_dominant_points = 0;    // 특징점 찾기 위한 변수
	CvSeq* ptseq;
//	CvSeq* defect = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), m_storage);

	cvFindContours(croppedImage, m_storage, &m_seq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// (2) 외곽선 근사화
	////////////////////////////////////////////////////////////////////////////////////////////////////
	int testcount = -1;
	int iContoursSize = 0;

	if (m_seq != 0)
	{
		testcount = 0;

        m_approxDP_seq = cvApproxPoly(m_seq, sizeof(CvContour), m_storage, CV_POLY_APPROX_DP, cvContourPerimeter(m_seq)*0.01, 1);
        if (m_approxDP_seq == nullptr)
		{
			if (m_storage) cvReleaseMemStorage(&m_storage);
			if (croppedImage) cvReleaseImage(&croppedImage);
			if (drawImage) cvReleaseImage(&drawImage);
			return -1;
		}

        if (m_bSaveEngineImg)
		{
            cvDrawContours(drawImage, m_approxDP_seq, CVX_WHITE, CVX_WHITE, 1, 1, 8); // 외곽선 근사화가 잘되었는지 테스트
            str.sprintf(("135_cvApproxPoly.jpg"));
            SaveOutImage(drawImage, pData, str, false);
		}

        //int nSeq = 0;
        for (CvSeq* c = m_approxDP_seq; c != nullptr; c = c->h_next)  // 엣지의 링크드리스트를 순회하면서 각 엣지들에대해서 출력한다.
		{
			float radius;
			CvPoint2D32f center;
			cvMinEnclosingCircle(c, &center, &radius);

			// 반지름이 설정치보다 적거나 크면 제외
            QString str;
            str.sprintf("%d %d %f", nMinCircleRadius, nMaxCircleRadius, radius);
            theMainWindow->DevLogSave(str.toLatin1().data());
            if (nMinCircleRadius > radius || nMaxCircleRadius < radius)
				continue;

            ptseq = cvCreateSeq(CV_SEQ_KIND_CURVE|CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);
			if (c->total >= MinMomentSize && c->total < MaxMomentSize)            // 외곽선을 이루는 점의 갯수가 이것보다 미만이면 잡음이라고 판단
			{
				for (int i = 0; i < c->total; ++i)
				{
					// CvSeq로부터좌표를얻어낸다.
					CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, c, i);
					CvPoint temp;
					temp.x = p->x;
					temp.y = p->y;

					// 컨백스헐을 구하기위해 좌표 저장
					cvSeqPush(ptseq, &temp);

                    if (m_bSaveEngineImg){	// (Test) 링크드리스트를 순회하면서 점을 잘찍나 테스트
						if (testcount == 0)
                            cvCircle(drawImage, temp, 2, CVX_RED, CV_FILLED);
						if (testcount == 1)
							cvCircle(drawImage, temp, 2, CVX_GREEN, CV_FILLED);
						if (testcount == 2)
							cvCircle(drawImage, temp, 2, CVX_YELLOW, CV_FILLED);
					}
				}

                if (m_bSaveEngineImg)
				{
                    str.sprintf(("136_SeqImage%d.jpg"), iContoursSize);
                    SaveOutImage(drawImage, pData, str, false);
				}

				////////////////////////////////////////////////////////////////////////////////////////////////////
				// (4) 중심점 계산
				////////////////////////////////////////////////////////////////////////////////////////////////////
                //Point2f centerPoint = CrossPointOfThinner(pData, croppedImageIn, ptseq);
                Point2f centerPoint = CenterOfMoment(ptseq);

                if (m_bSaveEngineImg)
				{
                    str.sprintf(("138_moments.jpg"));
					cvCircle(drawImage, cvPoint(cvRound((double)centerPoint.x), cvRound((double)centerPoint.y)), 3, CVX_WHITE, CV_FILLED);
                    SaveOutImage(drawImage, pData, str, false);
				}

				testcount++;

				//Calcalate to radius from circumference
				m_DetectResult.dRadius = radius;//  m_IIEngine.CalculRadiusFromCircumference(dCircumference);

				//Move Point
                m_DetectResult.pt.x = centerPoint.x;
                m_DetectResult.pt.y = centerPoint.y;

				iContoursSize++;
			}
			cvClearSeq(ptseq);
			if (iContoursSize > 0) // 1개만 찾자
				break;
		}

		if (m_approxDP_seq) cvClearSeq(m_approxDP_seq);
	}

	if (m_seq) cvClearSeq(m_seq);
//	if (defect) cvClearSeq(defect);
	if (m_storage) cvReleaseMemStorage(&m_storage);

	if (croppedImage) cvReleaseImage(&croppedImage);
	if (drawImage) cvReleaseImage(&drawImage);

	if (m_DetectResult.pt.x == 0 || m_DetectResult.pt.y == 0) return -1;
	if (iContoursSize <= 0) return -1;

    m_DetectResult.tl = CvPoint2D32f(rectIn.topLeft().x(), rectIn.topLeft().y());
    m_DetectResult.br = CvPoint2D32f(rectIn.bottomRight().x(), rectIn.bottomRight().y());
    pData->m_vecDetectResult.push_back(m_DetectResult);


	return iContoursSize;
}


//CrossPointOfThinner
Point2f CImgProcEngine::CrossPointOfThinner(RoiObject *pData, IplImage* srcimg, CvSeq* ptseq)
{
    IplImage* timg = nullptr;
	timg = cvCreateImage(cvSize(srcimg->width, srcimg->height), IPL_DEPTH_8U, 1);
	cvZero(timg);
	cvDrawContours(timg, ptseq, CVX_WHITE, CVX_WHITE, 1, CV_FILLED, 8);
    cvDilate(timg, timg, nullptr, 1);

	// "+"마크 모양이 일그러져 있을때 직선라인형태로 변환
	cv::Mat m0;
	m0 = cv::cvarrToMat(timg);
	std::vector<cv::Vec4i> lines;

	//cv::HoughLinesP(cv::Mat(s1), lines, nRho, CV_PI/180 * nThea, nThr, 5, nMaxLineGab );
	//rho : 1 , //Theta : PI / 180  - // 단계별 크기
	//Thr :  투표(vote) 최대 개수 - 낮게주면 많은 라인들이 그려짐
	//Min : 5
	//Gab : 80 -> 60으로 변경
	//cv::HoughLinesP(m0, lines, 1, 2 * CV_PI / 180, 100, 100, 50);

	double dMin = 90;
	double dMaxRadius = 0;
    if (pData != nullptr)
	{
		CParam *pParam = pData->getParam(("Maximum circle radius"));
		if (pParam)
            dMaxRadius = (double)pParam->Value.toDouble();
	}

    lines.reserve(1000);
	if (dMaxRadius > 100)
		dMin = 150;
	cv::HoughLinesP(m0, lines, 1, 2 * CV_PI / 180, 50, dMin, 50); // 100 -> dMin(50)으로 변경, 적은 '+'대응

	cvZero(timg);
	for (size_t i = 0; i < lines.size(); i++)
	{
		cvLine(timg, cv::Point(lines[i][0], lines[i][1]), cv::Point(lines[i][2], lines[i][3]), cv::Scalar(255, 255, 255), 1, 8, 0);
	}
    cvDilate(timg, timg, nullptr, 4);
    m0.release();

    if (m_bSaveEngineImg)
	{
        SaveOutImage(timg, pData, ("310_Thinner1.jpg"), false);
	}

    //IplImage* timgChk = cvCloneImage(timg);

    //double cX = -1, cY = -1;
	Point2f pt;
	Thinner(pData, timg, 311);

    int corner_count = 3; // 세개의 코너를 찾아서 "+"확인
	IplImage *eig_img, *temp_img;
	CvPoint2D32f *corners;
	eig_img = cvCreateImage(cvGetSize(timg), IPL_DEPTH_32F, 1);
	temp_img = cvCreateImage(cvGetSize(timg), IPL_DEPTH_32F, 1);
	corners = (CvPoint2D32f *)cvAlloc(corner_count * sizeof(CvPoint2D32f));
	//(src_img_gray, : 입력 영상, 8비트 or 32비트, 단일 채널 영상
	//	eig_img, : 32비트 단일 채널 영상, 영사의 각 원소에는 해당 픽셀에서 구해진 최소 고유값 저장
	//	temp_img,
	//	corners, : 코너 알고리즘이 실행된 후 결과 포인트가 저장(CvPoint2D32f)
	//	&corner_count, : 이 함수가 검출할 수 있는 코너점의 최대 개수
	//	0.1, : 코너의 풒질을 결정. 1을 넘어서는 안됨.보통 0.10또는 0.01 정도 사용
	//	15, : 반환되는 코너 점들 사이의 최소(유클리디안) 거리
    //	nullptr, : 관심영역 지정
	//	3, : 미분 계수의 자기상관행렬을 계산할 때 사용되는 평균 블록 크기
	//	1, : 해리스코너 사용 : use_harris로 0이 아니면 해리스 코너 사용. 0 : Shi - Tomasi방법 사용
	//	0.01 : 위 use_harris를 0이 아닌값으로 설정하였다면 이값은 코너 응답 함수 식에서 대각합에 대한 가중치로 작용
	//	);
    //cvGoodFeaturesToTrack(timg, eig_img, temp_img, corners, &corner_count, 0.1, 15, nullptr, 3, 1, 0.01);
    cvGoodFeaturesToTrack(timg, eig_img, temp_img, corners, &corner_count, 0.5, 15, nullptr, 3, 0, 0.01);
	cvFindCornerSubPix(timg, corners, corner_count, cvSize(3, 3), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
	for (int i = 0; i < corner_count; i++) {
		pt = cvPointFrom32f(corners[i]);
		//cvCircle(timg, pt, 3, CV_RGB(0, 0, 255), 2);

		// 코너가 "+"의 중심이 아닐수 있으므로 blob으로 재확인
        int nRet3 = CenterOfPlusmarkVerify(pData, timg, pt);
        if (nRet3 >= 0)
            break;
        else
            pt = cv::Point2f(0.0, 0.0);
	}
	cvFree(&corners);
    //cvReleaseImage(&timgChk);


    if (m_bSaveEngineImg)
	{
		QString str;
        str.sprintf(("375_centerofcontour.jpg"));
		cvCircle(timg, cvPoint(cvRound((double)pt.x), cvRound((double)pt.y)), 3, CVX_WHITE, CV_FILLED);
        SaveOutImage(timg, pData, str, false);
	}

	cvReleaseImage(&timg);
    cvReleaseImage(&eig_img);
    cvReleaseImage(&temp_img);

	return pt;
}


int CImgProcEngine::CenterOfPlusmarkVerify(RoiObject *pData, IplImage* imageIn, cv::Point2f cpt)
{
    int ret = 0;
    QString str;
    if (m_bSaveEngineImg)
    {
        str.sprintf(("340_CenterVerify.jpg"));
        SaveOutImage(imageIn, pData, str);
    }

    int nsize = 40;
    CvPoint pt;
    pt.x = (int)cpt.x - nsize / 2;
    pt.y = (int)cpt.y - nsize / 2;
    if (pt.x < 0)
        pt.x = 0;
    if (pt.y < 0)
        pt.y = 0;
    if ((pt.x + nsize) >= imageIn->width)
        pt.x = imageIn->width - nsize;
    if ((pt.y + nsize) >= imageIn->height)
        pt.y = imageIn->height - nsize;

    if (pt.x < 0) {
        nsize = nsize + pt.x;
        pt.x = 0;
    }
    if (pt.y < 0) {
        nsize = nsize + pt.y;
        pt.y = 0;
    }
    if (nsize < 0) {
        str.sprintf(("center of plus mark position wrong"));
        qDebug() << str;

        return -1;
    }


    IplImage* croppedImage = cvCreateImage(cvSize(nsize, nsize), imageIn->depth, imageIn->nChannels);
    CopyImageROI(imageIn, croppedImage, cvRect((int)pt.x, (int)pt.y, (int)nsize, (int)nsize));
    if (m_bSaveEngineImg)
    {
        str.sprintf(("345_CenterROI.jpg"));
        SaveOutImage(croppedImage, pData, str);
    }

    nsize = nsize - 2;
    /////////////////////////
    // blob에 "+"가 있는지 check
    ////////////////////////
    CBlobResult blobs;
    blobs = CBlobResult(croppedImage, nullptr);
    int nBlobs = blobs.GetNumBlobs();
    if (nBlobs > 0) {
        CBlob *p = blobs.GetBlob(0);
        CvRect rect = p->GetBoundingBox();
        if (rect.width < nsize || rect.height < nsize) {
            ret = -1;

            str.sprintf(("Center of plus mark blob size wrong"));
            theMainWindow->DevLogSave(str.toLatin1().data());
            qDebug() << str;
        }
    }
    else {
        ret = -1;

        str.sprintf(("Center of plus mark not found"));
        theMainWindow->DevLogSave(str.toLatin1().data());
        qDebug() << str;
    }
    cvReleaseImage(&croppedImage);
    return ret;
}

//
// 에지의 Gray변화가 많은 부분의 위치를 추출
// Threshold를 이용하여 Edge의 경계면을 추출하고 경계면으로부터 Subpixel edge를 추출한다.
// * Hessian Matrix 를 이용한것보다 좀더 경사진면에서의 edge를 구할수 있다.
//
double CImgProcEngine::SingleROISubPixEdgeWithThreshold(IplImage* croppedImage, RoiObject *pData, QRectF rect)
{
	QString str;
	IplImage* grayImg = cvCloneImage(croppedImage);
	vector<cv::Point2f> vecEdges;

	int nThresholdValue = 0;
	CParam *pParam = pData->getParam(("High Threshold"));
	if (pParam)
        nThresholdValue = pParam->Value.toDouble();

	if (nThresholdValue == 0) {
		ThresholdOTSU(pData, grayImg, 211);
	}
	else
		ThresholdRange(pData, grayImg, 211);

    NoiseOut(pData, grayImg, _ProcessValue1, 212);
    Expansion(pData, grayImg, _ProcessValue1, 213);

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
    // 여기까지 Threshold를 이용한 edge를 구하였다.


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

        if (m_bSaveEngineImg){
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

        if (m_bSaveEngineImg){
            SaveOutImage(edgeImage, pData, ("252_RampImg.jpg"), false);
		}
		break;
	}

    double dEdge = SubPixelRampEdgeImage(edgeImage, nDir);
    if (m_bSaveEngineImg){
        SaveOutImage(edgeImage, pData, ("260_SubPixelRampEdgeImageIn.jpg"), false);
    }

	if (edgeImage) cvReleaseImage(&edgeImage);

	//DetectResult result;
	switch (nDir)
	{
	case 0: // Left2Right 세로선
	case 1: // Right2Left
        dEdge += rect.left();
		dEdge += start;
		m_DetectResult.pt.x = dEdge;
        m_DetectResult.pt.y = rect.top() + rect.height() / 2;
		break;
	case 2: // 가로선
	case 3:
        dEdge += rect.top();
		dEdge += start;
		m_DetectResult.pt.y = dEdge;
        m_DetectResult.pt.x = rect.left() + rect.width() / 2;
		break;
	}

    //m_DetectResult.nCh = nCh;
	pData->m_vecDetectResult.push_back(m_DetectResult);


    str.sprintf(("%s Edge Result : %.3f"), pData->name().toStdString().c_str(), dEdge);
    theMainWindow->DevLogSave(str.toLatin1().data());

	return dEdge;
}

int CImgProcEngine::SingleROICircleWithThreshold(IplImage* croppedImage, RoiObject *pData, QRectF rect)
{
    Q_UNUSED(rect);
    //QString str;
	IplImage* grayImg = cvCloneImage(croppedImage);

    //int ret = 0;
    CvMemStorage* storage = nullptr;
	storage = cvCreateMemStorage(0);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 200);
		SaveOutImage(grayImg, pData, str);
	}

	int nThresholdValue = 0;
	CParam *pParam = pData->getParam(("High Threshold"));
	if (pParam)
        nThresholdValue = pParam->Value.toDouble();

	if (nThresholdValue == 0) {
		ThresholdOTSU(pData, grayImg, 211);
	}
	else
		ThresholdRange(pData, grayImg, 211);

    NoiseOut(pData, grayImg, _ProcessValue1, 221);

	Smooth(pData, grayImg, 231);

	int nMinCircleRadius = 30;
	int nMaxCircleRadius = 500;
	int nMaximumThresholdCanny = 200;
    if (pData != nullptr) {
		CParam *pParam = pData->getParam(("Minimum circle radius"));
		if (pParam)
            nMinCircleRadius = (int)pParam->Value.toDouble();
		pParam = pData->getParam(("Maximum circle radius"));
		if (pParam)
            nMaxCircleRadius = (int)pParam->Value.toDouble();
		pParam = pData->getParam(("Maximum Threshold Canny"));
		if (pParam)
            nMaximumThresholdCanny = (int)pParam->Value.toDouble();
	}


	double dp = 1.2; // Accumulator resolution
	//double min_dist = 100; // Minimum distance between the centers of the detected circles.
	double min_dist = 60;//  graySearchImg->height / 3;
	CvSeq* Circles = cvHoughCircles(grayImg, storage, CV_HOUGH_GRADIENT,
		dp,  // // 누적기 해상도(영상크기/2) //예를 들어 2로 지정하면 영상의 절반 크기인 누산기를 만듬.
		min_dist, // 두 원 간의 최소 거리
		nMaximumThresholdCanny, // 200 // 캐니 최대 경계값, 200정도면 확실한 원을 찾고, 100정도이면 근사치의 원을 모두 찾아준다.
		20, // 25 // 투표 최소 개수 //낮은 경계값은  높은 경계값  절반으로 설정.
		nMinCircleRadius,   // Minimum circle radius
		nMaxCircleRadius); // Maximum circle radius

	float* circle;
	if (Circles->total >= 1)
	{
        //int cx, cy, radius;
		circle = (float*)cvGetSeqElem(Circles, 0);

        m_DetectResult.pt.x = circle[0];// + pData->bounds().left();
        m_DetectResult.pt.y = circle[1];// + pData->bounds().top();
		m_DetectResult.dRadius = circle[2];
		m_DetectResult.dAngle = 0;
        //m_DetectResult.nCh = pData;
		pData->m_vecDetectResult.push_back(m_DetectResult);
        //ret = 1;
	}

    if (m_bSaveEngineImg){
		for (int k = 0; k<Circles->total; k++)
		{
			//m_DetectResult.dRadius -= 10; //test
			circle = (float*)cvGetSeqElem(Circles, k);
			cvCircle(grayImg, cvPoint(cvRound(circle[0]), cvRound(circle[1])), cvRound(m_DetectResult.dRadius), CV_RGB(128, 128, 128), 3);

			break;
		}
        SaveOutImage(grayImg, pData, ("241_Circles.jpg"), false);

	}

	cvReleaseMemStorage(&storage);
	if (grayImg) cvReleaseImage(&grayImg);

	return 0;
}

int CImgProcEngine::SingleROICircleWithEdge(IplImage* croppedImage, RoiObject *pData, QRectF rect)
{
    Q_UNUSED(rect);
    //QString str;
	IplImage* grayImg = cvCloneImage(croppedImage);
	int filterSize = 3;
    IplConvKernel *element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, nullptr);

    //int ret = 0;
    CvMemStorage* storage = nullptr;
	storage = cvCreateMemStorage(0);

	int nThresholdLowValue = 0;
	CParam *pParam = pData->getParam(("Low Threshold"));
	if (pParam)
        nThresholdLowValue = pParam->Value.toDouble();
	int nThresholdHighValue = 0;
	pParam = pData->getParam(("High Threshold"));
	if (pParam)
        nThresholdHighValue = pParam->Value.toDouble();

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 200);
		SaveOutImage(grayImg, pData, str);
	}

	//cvSubRS(grayImg, 255, grayImg);

	IplImage* tmp = cvCreateImage(cvSize(croppedImage->width, croppedImage->height), croppedImage->depth, croppedImage->nChannels);
	cvCanny(grayImg, tmp, nThresholdLowValue, nThresholdHighValue, 3);

	cvSub(grayImg, tmp, grayImg);
	//cvCopy(tmp, grayImg);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Canny.jpg"), 201);
		SaveOutImage(tmp, pData, str);
	}
	cvReleaseImage(&tmp);

	int nNoiseout = 2; // 백색강조
    cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);

	cvNot(grayImg, grayImg);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 202);
		SaveOutImage(grayImg, pData, str);
	}


	//int nInvert = CV_THRESH_BINARY | CV_THRESH_OTSU;
    //double otsuThreshold = cvThreshold(grayImg, grayImg, 0,   , nInvert);
	cvInRangeS(grayImg, Scalar(120), Scalar(255), grayImg); // 함체 및 AMP

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 203);
		SaveOutImage(grayImg, pData, str);
	}
	// Fill Blob
	CBlobResult blobs;
    blobs = CBlobResult(grayImg, nullptr);

	blobs.Filter(blobs, B_EXCLUDE, CBlobGetMajorAxisLength(), B_GREATER, 400); // 블럽이 아주 큰것은 제거

	cvZero(grayImg);
	int blobCount = blobs.GetNumBlobs();
	if (blobCount > 0) {
		for (int i = 0; i < blobCount; i++)
		{
			CBlob *currentBlob = blobs.GetBlob(i);
			currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
		}
	}

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 204);
		SaveOutImage(grayImg, pData, str);
	}



	nNoiseout = 2; // 흑색강조
	nNoiseout = 10; // 흑색강조 - pba
    cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_OPEN, nNoiseout);
	nNoiseout = 20; // 백색강조
    cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 205);
		SaveOutImage(grayImg, pData, str);
	}

	/////////////////////////
	// 조건에 맞는 Blob만 남김
	////////////////////////

    blobs = CBlobResult(grayImg, nullptr);
	blobs.Filter(blobs, B_EXCLUDE, CBlobGetMajorAxisLength(), B_GREATER, 140); // 200 - AMP HOLE Size(120)
	blobs.Filter(blobs, B_INCLUDE, CBlobGetMinorAxisLength(), B_GREATER, 80);
	//blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 30000);
	//blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_GREATER, 70000);
	//blobs.Filter(blobs, B_EXCLUDE, CBlobGetBreadth(), B_GREATER, 200);

	//double dLargeArea = 0;
	//int nBlobs = blobs.GetNumBlobs();
	//for (int i = 0; i < nBlobs; i++) {
	//	CBlob *p = blobs.GetBlob(i);
	//	double dArea = p->Area();
	//	if (dLargeArea < dArea) {
	//		dLargeArea = dArea;
	//	}
	//}
	//blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, dLargeArea);

	// filter blobs.
	cvZero(grayImg);
	blobCount = blobs.GetNumBlobs();
	if (blobCount > 0) {
		for (int i = 0; i < blobCount; i++)
		{
			CBlob *currentBlob = blobs.GetBlob(i);

            //CvSize2D32f f = currentBlob->GetEllipse().size;
            //f.width;
            //f.height;

			currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
		}
	}


    //cvDilate(grayImg, grayImg, nullptr, 20);

	Smooth(pData, grayImg, 210);

	int nMinCircleRadius = 30;
	int nMaxCircleRadius = 500;
	int nMaximumThresholdCanny = 200;
	double dp = 1.2; // Accumulator resolution
	double min_dist = 100; // Minimum distance between the centers of the detected circles.
	double vote = 20;
    if (pData != nullptr) {
		CParam *pParam = pData->getParam(("Minimum circle radius"));
		if (pParam)
            nMinCircleRadius = (int)pParam->Value.toDouble();
		pParam = pData->getParam(("Maximum circle radius"));
		if (pParam)
            nMaxCircleRadius = (int)pParam->Value.toDouble();

		pParam = pData->getParam(("min dist"));
		if (pParam)
            min_dist = (double)pParam->Value.toDouble();
		pParam = pData->getParam(("Maximum Threshold Canny"));
		if (pParam)
            nMaximumThresholdCanny = (int)pParam->Value.toDouble();
		pParam = pData->getParam(("vote"));
		if (pParam)
            vote = (double)pParam->Value.toDouble();
		pParam = pData->getParam(("dp"));
		if (pParam)
            dp = (double)pParam->Value.toDouble();

	}
	
	min_dist = grayImg->height / 8;
	nMaximumThresholdCanny = 100;
	vote = 21;

	CvSeq* Circles = cvHoughCircles(grayImg, storage, CV_HOUGH_GRADIENT,
		dp,  // // 누적기 해상도(영상크기/2) //예를 들어 2로 지정하면 영상의 절반 크기인 누산기를 만듬.
		min_dist, // 두 원 간의 최소 거리
		nMaximumThresholdCanny, // 200 // 캐니 최대 경계값, 200정도면 확실한 원을 찾고, 100정도이면 근사치의 원을 모두 찾아준다.
		vote, // 25 // 투표 최소 개수 //낮은 경계값은  높은 경계값  절반으로 설정.
		nMinCircleRadius,   // Minimum circle radius
		nMaxCircleRadius); // Maximum circle radius

	float* circle;
	if (Circles->total >= 1)
	{
        //int cx, cy, radius;
		circle = (float*)cvGetSeqElem(Circles, 0);

        m_DetectResult.pt.x = circle[0];// + pData->bounds().left();
        m_DetectResult.pt.y = circle[1];// + pData->bounds().top();
		m_DetectResult.dRadius = circle[2];
		m_DetectResult.dAngle = 0;
        //m_DetectResult.nCh = pData;
		//pData->m_vecDetectResult.push_back(m_DetectResult);
        //ret = 1;
	}

    if (m_bSaveEngineImg){
		for (int k = 0; k<Circles->total; k++)
		{
			//m_DetectResult.dRadius -= 10; //test
			circle = (float*)cvGetSeqElem(Circles, k);
			cvCircle(grayImg, cvPoint(cvRound(circle[0]), cvRound(circle[1])), cvRound(m_DetectResult.dRadius), CV_RGB(128, 128, 128), 3);

			//break;
		}
        SaveOutImage(grayImg, pData, ("241_Circles.jpg"), false);
		//cv::namedWindow("result"); cv::imshow("result", cv::cvarrToMat(grayImg));
	}


	cvReleaseStructuringElement(&element);
	cvReleaseMemStorage(&storage);
	if (grayImg) cvReleaseImage(&grayImg);

	return 0;
}

int CImgProcEngine::SinglePattIdentify(IplImage* grayImage, RoiObject *pData, QRectF rect)
{
    Q_UNUSED(rect);
    if (pData == nullptr)
        return -1;
    if (pData->iplTemplate == nullptr)
        return -1;
    QString str;

    CvSize searchSize = cvSize(grayImage->width, grayImage->height);
    IplImage* graySearchImg = cvCreateImage(searchSize, IPL_DEPTH_8U, 1);
    if (grayImage->nChannels == 3)
        cvCvtColor(grayImage, graySearchImg, CV_RGB2GRAY);
    else
        cvCopy(grayImage, graySearchImg);

    //static CvPoint2D32f cog = { 0, 0 };

    CvSize templateSize = cvSize(pData->iplTemplate->width, pData->iplTemplate->height);
    IplImage* grayTemplateImg = cvCreateImage(templateSize, IPL_DEPTH_8U, 1);
    if (pData->iplTemplate->nChannels == 3)
        cvCvtColor(pData->iplTemplate, grayTemplateImg, CV_RGB2GRAY);
    else
        cvCopy(pData->iplTemplate, grayTemplateImg);


    double dMatchShapes = 0;
    double MatchRate = 0, LimitMatchRate = 40;

    CParam *pParam = pData->getParam(("Pattern matching rate"));
    if (pParam) LimitMatchRate = pParam->Value.toDouble();

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_TemplateImage0.jpg"), 140);
        SaveOutImage(pData->iplTemplate, pData, str);
    }

    double dAngle = 0.0;
    double dAngleStep = 0.0;
    CvPoint left_top = { 0, 0 };
    pParam = pData->getParam(("Rotate angle"));
    if (pParam) dAngle = pParam->Value.toDouble();
    pParam = pData->getParam(("Angle step"));
    if (pParam) dAngleStep = pParam->Value.toDouble();

    if (dAngle > 0.0) // computing power가 낮은 시스템은 사용하지말자.
    {
        CvSize size = cvSize(pData->bounds().width() - grayTemplateImg->width + 1, pData->bounds().height() - grayTemplateImg->height + 1);
        IplImage* C = cvCreateImage(size, IPL_DEPTH_32F, 1); // 상관계수를 구할 이미지(C)
        double min, max;
        double rate = LimitMatchRate / 100.0;
        std::vector<std::pair<double, double>> pairs;
        IplImage *clone = cvCloneImage(grayTemplateImg);
        int cnt = 0;
        for (double a = -dAngle; a < dAngle; a=a+dAngleStep) // 패턴을 -30 에서 30도까지 돌려가면서 매칭율이 가장좋은 이미지를 찾는다.
        {
            cvCopy(grayTemplateImg, clone);
            RotateImage(clone, a);

            if (m_bSaveEngineImg)
            {
                cnt++;
                QString str; str.sprintf(("%d_Template%d.jpg"), 149, cnt);
                SaveOutImage(clone, pData, str);
            }

            cvMatchTemplate(graySearchImg, clone, C, CV_TM_CCOEFF_NORMED); // 제곱차 매칭
            cvMinMaxLoc(C, &min, &max, nullptr, &left_top); // 상관계수가 최대값을 값는 위치 찾기
            if (max > rate) {
                pairs.push_back(std::pair<double, double>(a, max));
            }
        }
        cvReleaseImage(&C);

        std::stable_sort(pairs.begin(), pairs.end(), [=](const std::pair<double, double>& a, const std::pair<double, double>& b)
        {
            return a.second > b.second; // descending
        });

        if (pairs.size() > 0) {
            std::pair<double, double> a = pairs[0];
            cvCopy(grayTemplateImg, clone);
            RotateImage(clone, a.first);
            cvCopy(clone, grayTemplateImg);
            QString str;
            str.sprintf("big match : %f %f", a.first, a.second);
            //qDebug() << "big match : " << a.first << a.second;
            theMainWindow->DevLogSave(str.toLatin1().data());
        }
        cvReleaseImage(&clone);
    }

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_TemplateImage1.jpg"), 150);
        SaveOutImage(grayTemplateImg, pData, str);
    }

    // Opencv Template Matching을 이용해서 Template 을 찾는다.
    // 이미지 Template Matching이기때문에 부정확한것은 cvMatchShapes()로 보완한다.
    //QString strMsg;
    MatchRate = TemplateMatch(pData, graySearchImg, grayTemplateImg, left_top, dMatchShapes);
    if (LimitMatchRate <= MatchRate)
    {
        //strMsg.Format(("TemplateMatch Result Success ===> : %.2f%%"), MatchRate);
        //MSystem.DevLogSave(("%s"), strMsg);

        if (m_DetectResult.result == true)
        {
            m_DetectResult.pt = CvPoint2D32f(left_top.x, left_top.y);
            m_DetectResult.tl = CvPoint2D32f(left_top.x, left_top.y);
            m_DetectResult.br = CvPoint2D32f(left_top.x + pData->iplTemplate->width, left_top.y + pData->iplTemplate->height);
            m_DetectResult.dRadius = 0;
            m_DetectResult.dAngle = 0;
            pData->m_vecDetectResult.push_back(m_DetectResult);
        }
    }
    else {
        left_top = { 0, 0 };
    }

    str.sprintf(("SinglePattIdentify MatchRate:%.1f MatchShapes:%.1f (%s)"),
                MatchRate, dMatchShapes, m_DetectResult.result == true ? ("OK") : ("NG"));
    qDebug() << str;
    //MSystem.m_pFormBottom->SetBottomMessage(str);
    theMainWindow->DevLogSave(str.toLatin1().data());

    return 0;
}

int CImgProcEngine::SinglePattMatchShapes(IplImage* croppedImage, RoiObject *pData, QRectF rectIn)
{
    Q_UNUSED(rectIn);
    if (pData->iplTemplate == nullptr)
        return -1;

    clock_t start_time1 = clock();

    QString str;
    int retry = 0;
    CParam *pParam;
    IplImage* grayImg = nullptr;
    int nThresholdHighValue;

    nThresholdHighValue = 255;
    pParam = pData->getParam("High Threshold", _ProcessValue1+retry);
    if (pParam)
        nThresholdHighValue = pParam->Value.toDouble();

    pData->m_vecDetectResult.clear();
    if (grayImg != nullptr)
        cvReleaseImage(&grayImg);
    grayImg = cvCloneImage(croppedImage);

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_Src.jpg"), 200);
        SaveOutImage(grayImg, pData, str);
    }

    if (nThresholdHighValue == 0)
        ThresholdOTSU(pData, grayImg, 211);
    else
        ThresholdRange(pData, grayImg, 211);

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_Threshold.jpg"), 203);
        SaveOutImage(grayImg, pData, str);
    }

    NoiseOut(pData, grayImg, _ProcessValue1, 212);
    Expansion(pData, grayImg, _ProcessValue1, 213);

    ///////////////////////////////////////////////////////////

    CvSize templateSize = cvSize(pData->iplTemplate->width, pData->iplTemplate->height);
    IplImage* grayTemplateImg = cvCreateImage(templateSize, IPL_DEPTH_8U, 1);
    if (pData->iplTemplate->nChannels == 3)
        cvCvtColor(pData->iplTemplate, grayTemplateImg, CV_RGB2GRAY);
    else
        cvCopy(pData->iplTemplate, grayTemplateImg);

     if (m_bSaveEngineImg)
    {
        str.sprintf(("%d_grayTemplateImg.jpg"), 250);
        SaveOutImage(grayTemplateImg, pData, str);
    }


    IplImage* g2 = cvCreateImage(cvSize(grayTemplateImg->width, grayTemplateImg->height), IPL_DEPTH_8U, 1);
    cvCopy(pData->iplTemplate, g2);

    if (nThresholdHighValue == 0)
        ThresholdOTSU(pData, g2, 256);
    else
        ThresholdRange(pData, g2, 256);
    NoiseOut(pData, g2, _ProcessValue1, 260);
    Expansion(pData, g2, _ProcessValue1, 261);
    FilterLargeArea(g2);

    int nGaussian = 3;
    try {
        cvSmooth(g2, g2, CV_GAUSSIAN,nGaussian,nGaussian);
    } catch (...) {
        qDebug() << "Error g2 cvSmooth()";
    }

    cvCanny(g2, g2, 100, 300, 3);
    if (m_bSaveEngineImg){
        SaveOutImage(g2, pData, ("265_TemplateImageCany.jpg"));
    }

    CvMemStorage *s2 = cvCreateMemStorage(0); //storage area for all contours 모든 형상들을 위한 저장공간.
    CvSeq* c2 = 0;         // 경계 계수를 저장할 변수
    cvFindContours(g2, s2, &c2, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    if (c2 == nullptr || c2->total <= 0) {
        if (grayTemplateImg) cvReleaseImage(&grayTemplateImg);
        if (c2) cvClearSeq(c2);
        if (s2) cvReleaseMemStorage(&s2);
        return 0;
    }

    CvRect boundbox2 = cvBoundingRect(c2);
    for (int i = 0; i < c2->total; ++i)
    {
        CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, c2, i);
        p->x = p->x - boundbox2.x;
        p->y = p->y - boundbox2.y;
    }

    if (m_bSaveEngineImg)
    {
        IplImage* drawImage = cvCreateImage(cvSize(g2->width, g2->height), grayImg->depth, grayImg->nChannels);
        cvZero(drawImage);
        cvDrawContours(drawImage, c2, CVX_WHITE, CVX_WHITE, 1, 1, 8); // 외곽선 근사화가 잘되었는지 테스트
        str.sprintf(("266_cvApproxPoly_Template.jpg"));
        SaveOutImage(drawImage, pData, str, false);
        if (drawImage) cvReleaseImage(&drawImage);
    }

    //int nGaussian = 3;
    try {
        cvSmooth(grayImg, grayImg, CV_GAUSSIAN,nGaussian,nGaussian);
    } catch (...) {
        qDebug() << "Error grayImg cvSmooth()";
    }

    ///////////////////////////////////////////////////////////

    CvMemStorage *storage = cvCreateMemStorage(0); //storage area for all contours 모든 형상들을 위한 저장공간.
    CvSeq* contours = 0;         // 경계 계수를 저장할 변수

    int seq = 0;
    cvFindContours(grayImg, storage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    while(contours)
    {

        // QThead Lambda를 이용하면 병렬 처리가 가능함.
        int iRst = OneMatchShapes(contours, c2, pData, seq);

        //obtain the next contour 다음 형상 가져오기
        contours = contours->h_next;
        seq++;
    }

    int size = pData->m_vecDetectResult.size();
    if (size >= 1) {
        std::stable_sort(pData->m_vecDetectResult.begin(), pData->m_vecDetectResult.end(), [](const DetectResult lhs, const DetectResult rhs)->bool {
            if (lhs.dMatchRate > rhs.dMatchRate) // descending
                return true;
            return false;
        });

        clock_t finish_time1 = clock();
        double total_time = (double)(finish_time1-start_time1)/CLOCKS_PER_SEC;
        //QString str;
        str.sprintf("Searching Time=%dms", (int)(total_time*1000));
        theMainWindow->DevLogSave(str.toLatin1().data());

        m_DetectResult = pData->m_vecDetectResult[0];
        pData->m_vecDetectResult.clear();
        pData->m_vecDetectResult.push_back(m_DetectResult);
        str.sprintf(("Selected template Shape Match ===> : %.2f%%"), m_DetectResult.dMatchRate);
        theMainWindow->DevLogSave(str.toLatin1().data());
    }

    if (contours) cvClearSeq(contours);
    if (storage) cvReleaseMemStorage(&storage);
    if (c2) cvClearSeq(c2);
    if (s2) cvReleaseMemStorage(&s2);


    if (grayImg) cvReleaseImage(&grayImg);
    //if (grayImg1) cvReleaseImage(&grayImg1);
    if (grayTemplateImg) cvReleaseImage(&grayTemplateImg);


    return 0;
}

int CImgProcEngine::OneMatchShapes(CvSeq* contours, CvSeq* templateseq, RoiObject *pData, int seq)
{
    int rst = -1;

    float dMatchShapesingRate = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Shape matching rate"));
        if (pParam)
            dMatchShapesingRate = (float)pParam->Value.toDouble() / 100.0f;
    }
    CvRect tbb = cvBoundingRect(templateseq);

    QString str;
    CvMemStorage* ptseqstorage = cvCreateMemStorage(0);
    CvSeq* ptseq = cvCreateSeq(CV_SEQ_KIND_CURVE|CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), ptseqstorage);
    CvRect boundbox = cvBoundingRect(contours);
    for (int i = 0; i < contours->total; ++i)
    {
        CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, contours, i);
        CvPoint p1;
        p1.x = p->x - boundbox.x;
        p1.y = p->y - boundbox.y;
        cvSeqPush(ptseq, &p1);
    }

    Point2f centerPoint = CenterOfMoment(ptseq);
    CvRect sbb = cvBoundingRect(ptseq);
    if (m_bSaveEngineImg)
    {
        IplImage* drawImage = cvCreateImage(cvSize(sbb.width+sbb.x, sbb.height+sbb.y), IPL_DEPTH_8U, 1);
        cvZero(drawImage);
        cvDrawContours(drawImage, ptseq, CVX_WHITE, CVX_WHITE, 1, 1, 8);
        str.sprintf(("270_cvApproxPoly_%d.jpg"), seq);
        SaveOutImage(drawImage, pData, str, false);
        if (drawImage) cvReleaseImage(&drawImage);
    }

    double matching = cvMatchShapes(ptseq, templateseq, CV_CONTOURS_MATCH_I1); // 작은 값 일수록 두 이미지의 형상이 가까운 (0은 같은 모양)라는 것이된다.
    //double matching2 = cvMatchShapes(searchSeq, templateSeq, CV_CONTOURS_MATCH_I2);
    //double matching3 = cvMatchShapes(searchSeq, templateSeq, CV_CONTOURS_MATCH_I3);
    //qDebug() << "matching" << matching << matching2 << matching3;
    double dMatchShapes = (1.0-matching) * 100.0;
    if (matching <= (1.0 - dMatchShapesingRate))
    {
        m_DetectResult.resultType = RESULTTYPE_RECT;
        m_DetectResult.dMatchRate = dMatchShapes;
        m_DetectResult.pt = centerPoint;
        m_DetectResult.tl = CvPoint2D32f(centerPoint.x-tbb.width/2,centerPoint.y-tbb.height/2);
        m_DetectResult.br = CvPoint2D32f(centerPoint.x+tbb.width/2,centerPoint.y+tbb.height/2);
        pData->m_vecDetectResult.push_back(m_DetectResult);
        rst = 0;
    }

    cvClearSeq(ptseq);
    cvReleaseMemStorage(&ptseqstorage);

    return rst;
}

int CImgProcEngine::SinglePattFeatureMatch(IplImage* croppedImage, RoiObject *pData, QRectF rectIn)
{
    Q_UNUSED(rectIn);
    //if (pData->iplTemplate == nullptr)
    //    return -1;

    clock_t start_time1 = clock();

    CParam *pParam;
    int nMethod = 0;
    pParam = pData->getParam("Method");
    if (pParam)
        nMethod = (int)pParam->Value.toDouble();
    double dParam1 = 0;
    pParam = pData->getParam("Param1");
    if (pParam)
        dParam1 = pParam->Value.toDouble();

    double kDistanceCoef = 0;
    pParam = pData->getParam("kDistanceCoef");
    if (pParam)
        kDistanceCoef = pParam->Value.toDouble();
    double kMaxMatchingSize = 0;
    pParam = pData->getParam("kMaxMatchingSize");
    if (pParam)
        kMaxMatchingSize = pParam->Value.toDouble();


    //std::string detectorName;
    //detectorName = ui->comboBoxDetector->currentText().toLatin1().data(); // "SURF";
    std::vector<cv::KeyPoint> keypoints_object, keypoints_scene;
    cv::Mat descriptors_object, descriptors_scene;
    vector<vector<cv::DMatch>> m_knnMatches;
    std::vector< cv::DMatch > good_matches;

    cv::Mat img_object = cv::cvarrToMat(pData->iplTemplate);
    cv::Mat img_scene =  cv::cvarrToMat(croppedImage);

    if (nMethod == 1) { // SIFT distance:10, MaxSize:200
        SIFTDetector sift(dParam1); //  nParam1=0
        sift(img_object, cv::Mat(), keypoints_object, descriptors_object);
        sift(img_scene, cv::Mat(), keypoints_scene, descriptors_scene);

        cv::BFMatcher matcher;
        matcher.knnMatch(descriptors_object, descriptors_scene, m_knnMatches, 2);
    }
    else if (nMethod == 0) { // SURF distance:5, MaxSize:200
        SURFDetector surf(dParam1); //  nParam1=800
        surf(img_object, cv::Mat(), keypoints_object, descriptors_object);
        surf(img_scene, cv::Mat(), keypoints_scene, descriptors_scene);

        cv::BFMatcher matcher;
        matcher.knnMatch(descriptors_object, descriptors_scene, m_knnMatches, 2);
    }
    else if (nMethod == 2) { // ORB distance:5, MaxSize:200
        ORBDetector orb(dParam1); // nParam1=2000
        orb(img_object, cv::Mat(), keypoints_object, descriptors_object);
        orb(img_scene, cv::Mat(), keypoints_scene, descriptors_scene);

        cv::BFMatcher matcher(cv::NORM_HAMMING); // use cv::NORM_HAMMING2 for ORB descriptor with WTA_K == 3 or 4 (see ORB constructor)
        matcher.knnMatch(descriptors_object, descriptors_scene, m_knnMatches, 2);
    }
    else return -1;

    for(int i = 0; i < min(img_scene.rows-1,(int) m_knnMatches.size()); i++) //THIS LOOP IS SENSITIVE TO SEGFAULTS
    {
        const cv::DMatch& bestMatch = m_knnMatches[i][0];
        if((int)m_knnMatches[i].size()<=2 && (int)m_knnMatches[i].size()>0)
        {
            good_matches.push_back(bestMatch);
        }
    }

    if (good_matches.size() == 0)
        return -1;

    std::sort(good_matches.begin(), good_matches.end());
    while (good_matches.front().distance * kDistanceCoef < good_matches.back().distance) {
        good_matches.pop_back();
    }
    while (good_matches.size() > kMaxMatchingSize) {
        good_matches.pop_back();
    }

    std::vector<cv::Point2f> corner;
    cv::Mat img_matches = drawGoodMatches(img_object, img_scene,
                                keypoints_object, keypoints_scene, good_matches, corner);

    //-- Show detected matches
    if (img_matches.rows > 0)
    {
        cv::namedWindow("knnMatch", 0);
        cv::imshow("knnMatch", img_matches);
    }

    if (corner.size() > 0)
    {
        cv::Point pt1 = corner[0];
        cv::Point pt2 = corner[1];
        cv::Point pt3 = corner[2];
        cv::Point pt4 = corner[3];

        m_DetectResult.resultType = RESULTTYPE_RECT4P;
        m_DetectResult.tl = CvPoint2D32f(pt1);
        m_DetectResult.tr = CvPoint2D32f(pt2);
        m_DetectResult.br = CvPoint2D32f(pt3);
        m_DetectResult.bl = CvPoint2D32f(pt4);
        pData->m_vecDetectResult.push_back(m_DetectResult);

//        cvLine(outImg, pt1, pt2, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//        cvLine(outImg, pt2, pt3, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//        cvLine(outImg, pt3, pt4, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//        cvLine(outImg, pt4, pt1, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//        m_DetectResult.rect = QRectF(QPointF(0,0),QPointF(0,0));
//        pData->m_vecDetectResult.push_back(m_DetectResult);

//        cvRectangle(iplImg2, pt1, pt3, cvScalar(255, 255, 255), CV_FILLED); // test
//        theMainWindow->outWidget("test1", iplImg2);


        clock_t finish_time1 = clock();
        double total_time = (double)(finish_time1-start_time1)/CLOCKS_PER_SEC;

        QString str;
        str.sprintf("Searching Time=%dms", (int)(total_time*1000));
        theMainWindow->DevLogSave(str.toLatin1().data());

    }

    return 0;
}

cv::Mat CImgProcEngine::drawGoodMatches(
    const cv::Mat& img1,
    const cv::Mat& img2,
    const std::vector<cv::KeyPoint>& keypoints1,
    const std::vector<cv::KeyPoint>& keypoints2,
    std::vector<cv::DMatch>& good_matches,
    std::vector<cv::Point2f>& scene_corners_
)
{
    cv::Mat img_matches;
    if (good_matches.size() == 0)
        return img_matches;

    cv::drawMatches(img1, keypoints1, img2, keypoints2,
        good_matches, img_matches, cv::Scalar::all(-1), cv::Scalar::all(-1),
        std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    //-- Localize the object
    std::vector<cv::Point2f> obj;
    std::vector<cv::Point2f> scene;

    for (size_t i = 0; i < good_matches.size(); i++)
    {
        //-- Get the keypoints from the good matches
        obj.push_back(keypoints1[good_matches[i].queryIdx].pt);
        scene.push_back(keypoints2[good_matches[i].trainIdx].pt);
    }
    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<cv::Point2f> obj_corners(4);
    obj_corners[0] = cv::Point(0, 0);
    obj_corners[1] = cv::Point(img1.cols, 0);
    obj_corners[2] = cv::Point(img1.cols, img1.rows);
    obj_corners[3] = cv::Point(0, img1.rows);
    std::vector<cv::Point2f> scene_corners(4);

    cv::Mat H;
    try {
        H = cv::findHomography(obj, scene, cv::RANSAC);
        perspectiveTransform(obj_corners, scene_corners, H);
    } catch (...) {
        qDebug() << ("Error <unknown> findHomography()");
        return img_matches;
    }

    scene_corners_ = scene_corners;

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
//    cv::Point pt1 = scene_corners[0] + cv::Point2f((float)img1.cols, 0);
//    cv::Point pt2 = scene_corners[1] + cv::Point2f((float)img1.cols, 0);
//    cv::Point pt3 = scene_corners[2] + cv::Point2f((float)img1.cols, 0);
//    cv::Point pt4 = scene_corners[3] + cv::Point2f((float)img1.cols, 0);
//    line(img_matches, pt1, pt2, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//    line(img_matches, pt2, pt3, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//    line(img_matches, pt3, pt4, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//    line(img_matches, pt4, pt1, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);

    return img_matches;
}

int CImgProcEngine::SingleROIFindShape(IplImage* croppedImage, RoiObject *pData, QRectF rectIn)
{
    Q_UNUSED(rectIn);
    QString str;
    int retry = 0;
    //RANSACIRCLE *pCircle;
    CParam *pParam;
    IplImage* grayImg = nullptr;
    int nThresholdLowValue;
    int nThresholdHighValue;
    int nMinArea;
    //int nMaxArea;
    //IplImage* grayImg1;

    pData->m_vecDetectResult.clear();
    if (grayImg != nullptr)
        cvReleaseImage(&grayImg);
    grayImg = cvCloneImage(croppedImage);

    nThresholdLowValue = 0;
    pParam = pData->getParam(("Low Threshold"), _ProcessValue1+retry);
    if (pParam)
        nThresholdLowValue = pParam->Value.toDouble();

    nThresholdHighValue = 255;
    pParam = pData->getParam("High Threshold", _ProcessValue1+retry);
    if (pParam)
        nThresholdHighValue = pParam->Value.toDouble();

    int nType = -1;
    nMinArea = 0;
    //nMaxArea = 100000;
    pParam = pData->getParam("Minimum area");
    if (pParam)
        nMinArea = (int)pParam->Value.toDouble();
    //pParam = pData->getParam("Maximum area");
    //if (pParam)
    //    nMaxArea = (int)pParam->Value.toDouble();
    pParam = pData->getParam(("Type"));
    if (pParam)
        nType = (int)pParam->Value.toDouble();

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_Src.jpg"), 200);
        SaveOutImage(grayImg, pData, str);
    }

    cvInRangeS(grayImg, Scalar(nThresholdLowValue), Scalar(nThresholdHighValue), grayImg);

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_Threshold.jpg"), 203);
        SaveOutImage(grayImg, pData, str);
    }

    NoiseOut(pData, grayImg, _ProcessValue1, 212);
    Expansion(pData, grayImg, _ProcessValue1, 213);

    CvMemStorage *storage = cvCreateMemStorage(0); //storage area for all contours 모든 형상들을 위한 저장공간.
    CvSeq* contours = 0;         // 경계 계수를 저장할 변수
    CvSeq* result;   //hold sequence of points of a contour 형상의 포인터의 시퀀스 잡기.

    cvFindContours(grayImg, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    //iterating through each contour 각형상별로 반복
    while(contours)
    {
        //obtain a sequence of points of contour, pointed by the variable 'contour' 형상의 점들의 시퀀스 가져오기, 인자 ‘contour’에 의해 지정된
        result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.01, 0);
        double area = cvContourArea(contours);
        if (area > nMinArea)  //면적이 일정크기 이상이어야 한다.
        {
            if (m_bSaveEngineImg)
            {
                IplImage* drawImage = cvCreateImage(cvSize(grayImg->width, grayImg->height), grayImg->depth, 3);
                cvZero(drawImage);
                cvDrawContours(drawImage, result, CVX_WHITE, CVX_WHITE, 1, 1, 8); // 외곽선 근사화가 잘되었는지 테스트
                str.sprintf(("220_cvApproxPoly.jpg"));
                SaveOutImage(drawImage, pData, str, false);
                if (drawImage) cvReleaseImage(&drawImage);
            }

            vector<Point> approx;
            int size = result->total;
            CvSeq* c = result;
            //for (CvSeq* c = result; c != nullptr; c = c->h_next)  // 엣지의 링크드리스트를 순회하면서 각 엣지들에대해서 출력한다.
            for (int i = 0; i < size; i++)
            {
                // CvSeq로부터좌표를얻어낸다.
                CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, c, i);
                approx.push_back(*p);
            }
            //모든 코너의 각도를 구한다.
            vector<int> angle;
            //cout << "===" << size << endl;
            for (int k = 0; k < size; k++) {
                int ang = GetAngleABC(approx[k], approx[(k + 1) % size], approx[(k + 2)%size]);
                //cout << k<< k+1<< k+2<<"@@"<< ang << endl;
                angle.push_back(ang);
            }

            std::sort(angle.begin(), angle.end());
            int minAngle = angle.front();
            int maxAngle = angle.back();
            int threshold = 8;

            int type = -1;
            //도형을 판정한다.
            if ( size == 3 ) // triangle
                type = 0;
            else if ( size == 4 && maxAngle < 120 ) // && minAngle >= 90- threshold && maxAngle <= 90+ threshold) // rectangle
            {
                IplImage* img = cvCreateImage(cvSize(20, 20), IPL_DEPTH_8U, 1);
                cvZero(img);
                cvRectangle(img, cvPoint(5, 5), cvPoint(15,15), CV_RGB(255, 225, 255), CV_FILLED); // filled rectangle.

                CvMemStorage *s2 = cvCreateMemStorage(0); //storage area for all contours 모든 형상들을 위한 저장공간.
                CvSeq* c2 = 0;         // 경계 계수를 저장할 변수

                cvFindContours(img, s2, &c2, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
                double matching = cvMatchShapes(c2, c, CV_CONTOURS_MATCH_I2); // 작은 값 일수록 두 이미지의 형상이 가까운 (0은 같은 모양)라는 것이된다.
                if (matching> 0.5)
                   str = "Parallelogram";
               else if (0.3 < matching && matching < 0.5)
                   str = "rectangle";
               else if (0.02 < matching && matching < 0.3)
                   str = "Rhombus";
               else
                   str = "Square";
               theMainWindow->DevLogSave(str.toLatin1().data());
               qDebug() << str;

               if (img) cvReleaseImage(&img);
               if (c2) cvClearSeq(c2);
               if (s2) cvReleaseMemStorage(&s2);

               type = 1;
            } else if (size == 5 && minAngle >= 108- threshold && maxAngle <= 108+ threshold) // pentagon
            {
               str = "pentagon";
               theMainWindow->DevLogSave(str.toLatin1().data());
               qDebug() << str;
               type = 2;
            } else if (size == 6 && minAngle >= 120 - threshold && maxAngle <= 140 + threshold) // hexagon
            {
               str = "hexagon";
               theMainWindow->DevLogSave(str.toLatin1().data());
               qDebug() << str;
               type = 3;
            } else {
                // if found convex area is ~ PI*r² then it is probably a circle
               float radius;
               CvPoint2D32f center;
               cvMinEnclosingCircle(c, &center, &radius);
               double tmp = 1.0 - (area / (PI * pow(radius, 2)));
               if (tmp < 0.5) {
                   str = "circle";
                   theMainWindow->DevLogSave(str.toLatin1().data());
                   qDebug() << str;
                   type = 4;
               }
            }

            if (type >= 0 && nType == type)
            {
                CvSeq* ptseq = cvCreateSeq(CV_SEQ_KIND_CURVE | CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), storage);
                for (int i = 0; i < size; ++i)
                {
                    CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, c, i);
                    cvSeqPush(ptseq, p);
                }
                Point2f centerPoint = CenterOfMoment(ptseq);
                cvClearSeq(ptseq);

                m_DetectResult.pt = centerPoint;
                pData->m_vecDetectResult.push_back(m_DetectResult);
                break;
            }
        }
        //obtain the next contour 다음 형상 가져오기
        contours = contours->h_next;
    }

    if (contours) cvClearSeq(contours);
    if (storage) cvReleaseMemStorage(&storage);


    if (grayImg) cvReleaseImage(&grayImg);

    return 0;
}

bool CImgProcEngine::SetROIAreaForCriteriaPosition(RoiObject *pData, QString strCriteriaROI)
{
    Q_UNUSED(strCriteriaROI);
	QString str;
    bool bFoundCriteriaPosition = false;
    double dCriteriaX = 0.0;
    double dCriteriaY = 0.0;

//    for (int i = 0; i < (int)g_cRecipeData->m_vecRoiObject.size(); i++)
//	{
//        RoiObject *pCriData = g_cRecipeData->m_vecRoiObject[i];
//        if (pCriData->isVisible() && pCriData->name() == strCriteriaROI) {

//			// Priority에 의해 기준위치 ROI가 먼저 처리되거 설정 - 2017.7.2 jlyoon
//			// 수동으로 처리할때는 항상 기준위치 ROI 처리
//			int size = pCriData->m_vecDetectResult.size();
//			//if (size == 0)
//            InspectOneItem(curImg, pCriData); // 기준위치의 ROI를 먼저 처리한다.

//			if (size > 0) {
//				DetectResult *prst = &pCriData->m_vecDetectResult[0];
//				dCriteriaX = prst->pt.x;
//				dCriteriaY = prst->pt.y;
	
//                bFoundCriteriaPosition = true;
//				break;
//			}
//		}
//	}
    if (bFoundCriteriaPosition == false)
        return false;

	double dOffsetX = 0.0;
	double dOffsetY = 0.0;
	CParam *pParam = pData->getParam(("X Offset(mm)"));
	if (pParam)
        dOffsetX = pParam->Value.toDouble();
	pParam = pData->getParam(("Y Offset(mm)"));
	if (pParam)
        dOffsetY = pParam->Value.toDouble();
	const double dResX = gCfg.m_pCamInfo[0].dResX;
	const double dResY = gCfg.m_pCamInfo[0].dResY;

	m_DetectResult.pt.x = dCriteriaX;
	m_DetectResult.pt.y = dCriteriaY;
	pData->m_vecDetectResult.push_back(m_DetectResult);

	DetectResult *pResult = &pData->m_vecDetectResult[0];
	pResult->pt.x += (dOffsetX / dResX); // Offset Pixel 갯수
	pResult->pt.y += (dOffsetY / dResY); // Offset Pixel 갯수


	str.sprintf(("bonding position point : %.2f,%.2f"), m_DetectResult.pt.x, m_DetectResult.pt.y);
    theMainWindow->DevLogSave(str.toLatin1().data());

    return true;
}


int CImgProcEngine::SingleROICorner(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect)
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
        //pt.x += rect.left();
        //pt.y += rect.top();

        //DetectResult result;
        m_DetectResult.pt = pt;
        pData->m_vecDetectResult.push_back(m_DetectResult);

        QString str;
        str.sprintf(("SingleROICorner point : %.2f,%.2f"), m_DetectResult.pt.x, m_DetectResult.pt.y);
        theMainWindow->DevLogSave(str.toLatin1().data());
    }
    return 0;
}

//
// CornerType : 0 - Upper Left, 1 - Upper Right, 2 - Bottom Left, 3 - Bottom Right
//
int CImgProcEngine::EdgeCorner(Qroilib::RoiObject *pData, IplImage* graySearchImgIn, int CornerType, CvPoint2D32f &outCorner)
{
    outCorner = CvPoint2D32f(0, 0);
    QString str;
    IplImage* graySearchImg = cvCloneImage(graySearchImgIn);

    if (m_bSaveEngineImg){
        SaveOutImage(graySearchImg, pData, ("310_EdgeCornerSrc.jpg"));
    }

    int nThresholdValue = 0;
    CParam *pParam = pData->getParam(("High Threshold"));
    if (pParam)
        nThresholdValue = pParam->Value.toDouble();

    if (nThresholdValue == 0) {
        ThresholdOTSU(pData, graySearchImg, 311);
    }
    else
        ThresholdRange(pData, graySearchImg, 311);

    NoiseOut(pData, graySearchImg, _ProcessValue1, 312);
    Expansion(pData, graySearchImg, _ProcessValue1, 313);

    CBlobResult blobs;
    blobs = CBlobResult(graySearchImg, nullptr);
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

    if (m_bSaveEngineImg){
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

    if (m_bSaveEngineImg){
        SaveOutImage(graySearchImg, pData, ("317_imageLargeBlobs.jpg"));
    }


    // 후처리
    Expansion(pData, graySearchImg, _PostProcessValue1, 319);

    // 2. 윤곽선 표시 및 윤곽선 영상 생성

    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* contours = 0;
    cvFindContours(graySearchImg, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    if (m_bSaveEngineImg){
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

        if (m_bSaveEngineImg)
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

    Point2f pt;
    if (corners.size() > 0)
    {
        pt = getCorner(corners, center, CornerType);
        outCorner = pt;
        ret = 0;
    }
    else
        ret = -1;

    if (m_bSaveEngineImg){
        Point2f pt1 = Point2f(pt.x - 30, pt.y - 30);
        Point2f pt2 = Point2f(pt.x + 30, pt.y + 30);
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
int CImgProcEngine::EdgeCornerByLine(Qroilib::RoiObject *pData, IplImage* grayCroppedImgIn, int CornerType, CvPoint2D32f &outCorner)
{
    outCorner = CvPoint2D32f();
    QString str;
    IplImage* grayCroppedImg = cvCloneImage(grayCroppedImgIn);

    if (m_bSaveEngineImg)
    {
        str.sprintf(("209_GrayImage.jpg"));
        SaveOutImage(grayCroppedImg, pData, str);
    }

    int nThresholdValue = 0;
    CParam *pParam = pData->getParam(("High Threshold"));
    if (pParam)
        nThresholdValue = pParam->Value.toDouble();

    if (nThresholdValue == 0)
        ThresholdOTSU(pData, grayCroppedImg, 211);
    else
        ThresholdRange(pData, grayCroppedImg, 211);

    NoiseOut(pData, grayCroppedImg, _ProcessValue1, 212);
    Expansion(pData, grayCroppedImg, _ProcessValue1, 213);

    IplImage* croppedImageVerify = cvCloneImage(grayCroppedImg);

    /////////////////////////
    // 가장큰 Blob만 남김
    ////////////////////////
    CBlobResult blobs;
    blobs = CBlobResult(grayCroppedImg, nullptr);
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

    if (m_bSaveEngineImg)
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

    if (m_bSaveEngineImg)
    {
        str.sprintf(("218_workImg.jpg"));
        SaveOutImage(workImg, pData, str);
    }

    double dRejectLow = 0.45; // 양쪽 45%를 버리고 중앙 10%만 사용한다.
    double dRejectHigh = 0.45;

    int w = (int)((float)workImg->width * 0.3); // ROI영역 30%만 사용한다.
    int h = workImg->height;

    Point2f pt = Point2f(0,0);
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

    if (m_bSaveEngineImg)
    {
        Point2f pt1 = Point2f(pt.x - 20, pt.y - 20);
        Point2f pt2 = Point2f(pt.x + 20, pt.y + 20);
        cvRectangle(grayCroppedImg, pt1, pt2, CV_RGB(0, 84, 255), 3, 8, 0);

        cvSetImageROI(grayCroppedImg, cvRect((int)pt1.x, (int)pt1.y, 40, 40));
        cvAddS(grayCroppedImg, cvScalar(150), grayCroppedImg);
        cvResetImageROI(grayCroppedImg);

        cvCircle(grayCroppedImg, pt, 3, CVX_RED, 2);
        str.sprintf(("235_Corners.jpg"));
        SaveOutImage(grayCroppedImg, pData, str);
    }
#if 0
    int nRet2 = dgeVerify(pData, croppedImageVerify, pt);
    if (nRet2 < 0) {
        str.sprintf(("EdgeVerify error : Edge not found"));
        //g_cLog->AddLog(_LOG_LIST_SYS, str);
        ret = -1;
    }

    int nRet3 = dgeVerifyBlob(pData, croppedImageVerify, pt);
    if (nRet3 < 0) {
        str.sprintf(("EdgeVerifyBlob error : blob is too small"));
        //g_cLog->AddLog(_LOG_LIST_SYS, str);
        ret = -1;
    }
#endif
    if (croppedImageVerify) cvReleaseImage(&croppedImageVerify);
    if (grayCroppedImg) cvReleaseImage(&grayCroppedImg);
    if (workImg) cvReleaseImage(&workImg);
    return ret;
}

//
// 에지의 Gray변화가 많은 부분의 위치를 추출
// 미리 만들어진 마스크의 경계면에서 Edge를 추출한다.
// * Hessian Matrix 를 이용한것보다 좀더 경사진면에서의 edge를 구할수 있다.
//
RoiObject *CImgProcEngine::GetCriteriaROI(RoiObject *pData, QString title)
{
    //int nCh = pData;
	CParam *pParam = pData->getParam(title);
	if (pParam) {
        int n = pParam->Value.toDouble();
		if (n > 0) { // 기준Mask ROI가 설정되어 있다.
//			if (0 == pParam->m_vecDetail.size()){
//				pParam->m_vecDetail.push_back(("No Parent/Criteria Roi"));
//                for (int i = 0; i < g_cRecipeData->m_vecRoiObject.size(); i++){
//                    RoiObject* pdata = g_cRecipeData->m_vecRoiObject[i];
//                    if (pdata->groupName() == pData->groupName())
//					{
//                        if (pParam->Value == pdata->name()) {
//							QString str;
//							str.sprintf(("%d"), i + 1);
//							pParam->Value = str;
//						}
//                        pParam->m_vecDetail.push_back(pdata->name());
//					}
//				}
//			}

			if (0 < pParam->m_vecDetail.size()){
                QString strCriteriaROI = pParam->m_vecDetail[n]; // str이 기준 Mask ROI 이다.
                if (strCriteriaROI != pData->name().toStdString().c_str()) {
//                    for (int i = 0; i < (int)g_cRecipeData->m_vecRoiObject.size(); i++)
//					{
//                        RoiObject *pCriData = g_cRecipeData->m_vecRoiObject[i];
//                        if (pCriData->isVisible() && pCriData->name() == strCriteriaROI) {
//							return pCriData;
//						}
//					}
				}
			}
		}
	}

    return nullptr;
}


int CImgProcEngine::CenterOfGravity(RoiObject *pData, IplImage* croppedImageIn, CvPoint2D32f &cog, int nRetry)
{
    QString str;
    IplImage* drawImage = nullptr;
    IplImage* croppedImage = cvCloneImage(croppedImageIn);
    int MinMomentSize = 5, MaxMomentSize = 50;

    int nMinCircleRadius = 30;
    int nMaxCircleRadius = 100;
    if (pData != nullptr)
    {
        CParam *pParam = pData->getParam(("Minimum circle radius"));
        if (pParam)	nMinCircleRadius = (int)pParam->Value.toDouble();
        if (nRetry == 1)
            pParam = pData->getParam(("Maximum circle radius1"));
        else if (nRetry == 2)
            pParam = pData->getParam(("Maximum circle radius2"));
        else
            pParam = pData->getParam(("Maximum circle radius"));
        if (pParam)	nMaxCircleRadius = (int)pParam->Value.toDouble();
    }

    if (m_bSaveEngineImg) {
        drawImage = cvCreateImage(cvSize(croppedImage->width, croppedImage->height), croppedImage->depth, 3);
        cvZero(drawImage);
    }

    int nThresholdValue = 0;
    CParam *pParam;
    if (nRetry == 1)
        pParam = pData->getParam(("High Threshold1"));
    else if (nRetry == 1)
        pParam = pData->getParam(("High Threshold2"));
    else
        pParam = pData->getParam(("High Threshold"));
    if (pParam)
        nThresholdValue = pParam->Value.toDouble();

    if (nThresholdValue == 0) {
        ThresholdOTSU(pData, croppedImage, 310);
    }
    else
        ThresholdRange(pData, croppedImage, 310);

    NoiseOut(pData, croppedImage, _ProcessValue1, 311);
    Expansion(pData, croppedImage, _ProcessValue1, 312);

    CBlobResult blobs;
    blobs = CBlobResult(croppedImage, nullptr);

    /////////////////////////
    // 반지름조건에 부합하는 blob만 남김
    ////////////////////////
a1:
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++) {
        CBlob *p = blobs.GetBlob(i);
        CvRect rect = p->GetBoundingBox();
        int rw = rect.width / 2;
        int rh = rect.height / 2;
        int _max = MAX(rw/2, rh/2);
        int _min = MIN(rw / 2, rh / 2);
        double dArea = p->Area();
        if (_max > nMaxCircleRadius) {
            blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_EQUAL, dArea);
            goto a1;
        }
        if (_min < nMinCircleRadius){
            blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_EQUAL, dArea);
            goto a1;
        }
    }

    // filter blobs.
    cvZero(croppedImage);
    int blobCount = blobs.GetNumBlobs();
    if (blobCount == 0) {
        if (croppedImage) cvReleaseImage(&croppedImage);
        if (drawImage) cvReleaseImage(&drawImage);
        return -1;
    }
    for (int i = 0; i < blobCount; i++)
    {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(croppedImage, CVX_WHITE);	// Draw the filtered blobs as white.
    }

    if (m_bSaveEngineImg)
    {
        str.sprintf(("313_cvApproxInImage.jpg"));
        SaveOutImage(croppedImage, pData, str);
    }

    // 외곽선 추적 및 근사화 변수 초기화
    CvMemStorage* m_storage = cvCreateMemStorage(0); // 배열 자료(점의 좌표가 들어간다)
    CvSeq* m_seq = 0;         // 경계 계수를 저장할 변수
    CvSeq* m_approxDP_seq = 0;
    //CvSeq* m_dominant_points = 0;    // 특징점 찾기 위한 변수
    CvSeq* ptseq;
  //  CvSeq* defect = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), m_storage);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // (1) 외곽선 추적
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // 1. CvArr* image,
    // 2. CvMemStorage* storage,
    // 3. CvSeq** first_contour,
    // 4. int header_size=sizeof(CvContour),
    // 5. int mode=CV_RETR_LIST,
    // 			 CV_RETR_EXTERNAL / CV_RETR_LIST / CV_RETR_CCOMP / CV_RETR_TREE
    // 6. int method=CV_CHAIN_APPROX_SIMPLE,
    // 			 CV_CHAIN_CODE / CV_CHAIN_APPROX_NONE / CV_CHAIN_APPROX_SIMPLE / CV_LINK_RUNS
    // 7. CvPoint offset=cvPoint(0,0)
    cvFindContours(croppedImage, m_storage, &m_seq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // (2) 외곽선 근사화
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    int testcount = -1;
    int iContoursSize = 0;

    if (m_seq != 0)
    {
        testcount = 0;

        // 1. CVSeq* seq = CVSeq 형태의 Contour sequence. 근사화 하기 위한 원본 윤곽선 정보
        // 2. int header_size = seq 의 헤더 크기
        // 3. CvMemStorage* storage = 연산을 위한 메모리 할당
        // 4. int method = CV_POLY_APPROX_DP
        // 5. double parameter = 근사화의 정확도를 의미
        // 6. int parameter2 = seq 가 CvMat* 형태의 배열일 경우 parameter2 == 0 이면 열린 곡선
        //    parameter2 == 0 이 아니면 닫힌 곡선
        m_approxDP_seq = cvApproxPoly(m_seq, sizeof(CvContour), m_storage, CV_POLY_APPROX_DP, cvContourPerimeter(m_seq)*0.01, 1);
        if (m_approxDP_seq == nullptr)
        {
            if (m_storage) cvReleaseMemStorage(&m_storage);
            if (croppedImage) cvReleaseImage(&croppedImage);
            if (drawImage) cvReleaseImage(&drawImage);
            return -1;
        }

        if (m_bSaveEngineImg)
        {
            cvDrawContours(drawImage, m_approxDP_seq, CVX_WHITE, CVX_WHITE, 1, 1, 8); // 외곽선 근사화가 잘되었는지 테스트
            str.sprintf(("322_cvApproxPoly.jpg"));
            SaveOutImage(drawImage, pData, str);
        }

        int nSeq = 0;
        for (CvSeq* c = m_approxDP_seq; c != nullptr; c = c->h_next)  // 엣지의 링크드리스트를 순회하면서 각 엣지들에대해서 출력한다.
        {
            float radius;
            CvPoint2D32f center;
            cvMinEnclosingCircle(c, &center, &radius);

            ptseq = cvCreateSeq(CV_SEQ_KIND_CURVE | CV_32SC2, sizeof(CvSeq), sizeof(CvPoint), m_storage);
            if (c->total >= MinMomentSize && c->total < MaxMomentSize)            // 외곽선을 이루는 점의 갯수가 이것보다 미만이면 잡음이라고 판단
            {
                for (int i = 0; i < c->total; ++i)
                {
                    // CvSeq로부터좌표를얻어낸다.
                    CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, c, i);
                    CvPoint temp;
                    temp.x = p->x;
                    temp.y = p->y;

                    // 컨백스헐을 구하기위해 좌표 저장
                    cvSeqPush(ptseq, &temp);

                    if (m_bSaveEngineImg){	// (Test) 링크드리스트를 순회하면서 점을 잘찍나 테스트
                        if (testcount == 0)
                            cvCircle(drawImage, temp, 2, CVX_RED, CV_FILLED);
                        if (testcount == 1)
                            cvCircle(drawImage, temp, 2, CVX_GREEN, CV_FILLED);
                        if (testcount == 2)
                            cvCircle(drawImage, temp, 2, CVX_YELLOW, CV_FILLED);
                    }
                }

                if (m_bSaveEngineImg)
                {
                    str.sprintf(("323_SeqImage%d.jpg"), nSeq++);
                    SaveOutImage(drawImage, pData, str);
                }

                ////////////////////////////////////////////////////////////////////////////////////////////////////
                // (4) cvMoment 로 중심점 계산
                ////////////////////////////////////////////////////////////////////////////////////////////////////
                Point2f centerPoint = CenterOfMoment(ptseq);

                if (m_bSaveEngineImg)
                {
                    str.sprintf(("370_moments.jpg"));
                    cvCircle(drawImage, cvPoint(cvRound((double)centerPoint.x), cvRound((double)centerPoint.y)), 3, CVX_WHITE, CV_FILLED); // 以묒떖??李띿뼱二쇨린
                    SaveOutImage(drawImage, pData, str);
                }

                testcount++;

                //Calcalate to radius from circumference
                m_DetectResult.dRadius = radius;//  m_IIEngine.CalculRadiusFromCircumference(dCircumference);

                str.sprintf(("CenterOfGravity Radius = %.3f)"), radius);
                theMainWindow->DevLogSave(str.toLatin1().data());

                cog.x = centerPoint.x;
                cog.y = centerPoint.y;

                //Move Point
                m_DetectResult.pt = centerPoint;

                iContoursSize++;
            }
            cvClearSeq(ptseq);
            if (iContoursSize > 0) // 1개만 찾자
                break;
        }

        if (m_approxDP_seq) cvClearSeq(m_approxDP_seq);
    }

    if (m_seq) cvClearSeq(m_seq);
    //if (defect) cvClearSeq(defect);
    if (m_storage) cvReleaseMemStorage(&m_storage);

    if (croppedImage) cvReleaseImage(&croppedImage);
    if (drawImage) cvReleaseImage(&drawImage);

    if (cog.x == 0 || cog.y == 0) return -1;
    if (iContoursSize <= 0) return -1;
    return iContoursSize;
}


void CImgProcEngine::Smooth(RoiObject *pData, IplImage* ImgIn, int iImgID)
{
    int param1 = 3;
    int param2 = 0;
    double param3 = 2, param4 = 2;
    int smoothMethod = CV_GAUSSIAN;
    int smoothSize = 3;
    //IplImage* image = nullptr;
    bool bUse = true;

    if (pData != nullptr)
    {
        CParam *pParam = pData->getParam(("Smooth method"));
        if (pParam) smoothMethod = pParam->Value.toDouble();
        pParam = pData->getParam(("Smooth size"));
        if (pParam) smoothSize = pParam->Value.toDouble();
        // 5x5 행렬
        if (smoothSize == 1) smoothSize = 5;
        // 7x7 행렬
        else if (smoothSize == 2) smoothSize = 7;
        // 9x9 행렬
        else if (smoothSize == 3) smoothSize = 9;
        // 11x11 행렬
        else if (smoothSize == 4) smoothSize = 11;
        // 13x13 행렬
        else if (smoothSize == 5) smoothSize = 13;
        // 15x15 행렬
        else if (smoothSize == 6) smoothSize = 15;
        // 3x3 행렬
        else smoothSize = 3;

        pParam = pData->getParam(("Smooth Use"));
        if (pParam) bUse = (bool)pParam->Value.toDouble();
    }

    if (!bUse) return;

    // This is done so as to prevent a lot of false circles from being detected
    //
    // 1. const CvArr* src
    // 2. CvArr* dst
    // 3. int smooththype=CV_GAUSSIAN
    // 4. int param1=3
    // 5. int param2=0
    // 6. double param3=0
    // 7. double param4=0
    switch (smoothMethod)
    {
    case CV_BLUR_NO_SCALE:
        param1 = smoothSize;
        break;
    case CV_BLUR:
        param1 = smoothSize;
        break;
    case CV_MEDIAN:
        param1 = smoothSize;
        param2 = smoothSize;
        break;
//	case CV_BILATERAL:
//		param1 = smoothSize;
//		param2 = smoothSize;
//#if 1
//		if (smoothSize == 11)
//		{
//			param3 = 5;
//			param4 = 5;
//		}
//		else if (smoothSize == 5)
//		{
//			param3 = 3;
//			param4 = 3;
//		}
//		else
//		{
//			param3 = 2;
//			param4 = 2;
//		}
//#endif
//		break;
    case CV_GAUSSIAN:
    default:
        param1 = smoothSize;
        param2 = smoothSize;
        break;
    }

    cvSmooth(ImgIn, ImgIn, smoothMethod, param1, param2, param3, param4);

    if (m_bSaveEngineImg)
    {
        QString str;
        str.sprintf(("%d_Smooth.jpg"), iImgID);
        SaveOutImage(ImgIn, pData, str);
    }
}

double CImgProcEngine::TemplateMatch(RoiObject *pData, IplImage* graySearchImgIn, IplImage* grayTemplateImg, CvPoint &left_top, double &dMatchShapes)
{
    QString str;
    QRectF srect = pData->bounds();
    IplImage* graySearchImg = cvCloneImage(graySearchImgIn);

    if (m_bSaveEngineImg)
    {
        //str.sprintf(("%d_graySearchImg.jpg"), 200);
        //SaveOutImage(graySearchImg, pData, str);

        str.sprintf(("%d_grayTemplateImg.jpg"), 201);
        SaveOutImage(grayTemplateImg, pData, str);
    }

    clock_t start_time1 = clock();

    CvSize size = cvSize(srect.width() - grayTemplateImg->width + 1, srect.height() - grayTemplateImg->height + 1);
    IplImage* C = cvCreateImage(size, IPL_DEPTH_32F, 1); // 상관계수를 구할 이미지(C)
    double min, max;

    IplImage* g2 = cvCreateImage(cvSize(grayTemplateImg->width, grayTemplateImg->height), IPL_DEPTH_8U, 1);
    cvCopy(pData->iplTemplate, g2);

    int nThresholdValue = 0;
    CParam *pParam = pData->getParam(("High Threshold"));
    if (pParam)
        nThresholdValue = pParam->Value.toDouble();

    if (nThresholdValue == 0)
        ThresholdOTSU(pData, g2, 211);
    else
        ThresholdRange(pData, g2, 211);

    NoiseOut(pData, g2, _ProcessValue2, 212);
    Expansion(pData, g2, _ProcessValue2, 213);

    int nFilterBlob = 0;
    pParam = pData->getParam(("Filter blob"));
    if (pParam)
        nFilterBlob = (int)pParam->Value.toDouble();
    // 가장큼 or 긴 blob만 남긴다.
    if (nFilterBlob == 1)
        FilterLargeArea(g2);
    else if (nFilterBlob == 2)
        FilterLargeDiameter(g2);
    if (m_bSaveEngineImg){
        SaveOutImage(g2, pData, ("226_FilterBlob.jpg"));
    }

    NoiseOut(pData, g2, _ProcessValue2, 231);
    Expansion(pData, g2, _ProcessValue2, 232);
    cvCanny(g2, g2, 100, 300, 3);
    if (m_bSaveEngineImg){
        SaveOutImage(g2, pData, ("227_TemplateImageCany.jpg"));
    }

    float dMatchingRate = 0.5f;
    float dMatchShapesingRate = 0.7f;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam("Pattern matching rate");
        if (pParam)
            dMatchingRate = (float)pParam->Value.toDouble() / 100.0f;
    }
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Shape matching rate"));
        if (pParam)
            dMatchShapesingRate = (float)pParam->Value.toDouble() / 100.0f;
    }

    CvMemStorage* storage = cvCreateMemStorage(0);

    dMatchShapes = 0.0;
    int nLoop = 0;
    double maxRate = 0;
    while (1)
    {
        cvMatchTemplate(graySearchImg, grayTemplateImg, C, CV_TM_CCOEFF_NORMED); // 제곱차 매칭
        cvMinMaxLoc(C, &min, &max, nullptr, &left_top); // 상관계수가 최대값을 값는 위치 찾기
        //str.sprintf(("MatchTemplate : %.3f"), max);
        //g_cLog->AddLog(_LOG_LIST_SYS, str);
        if (maxRate < max)
            maxRate = max;

        if (max >= dMatchingRate)	// OpenCV의 Template Matching 함수를 이용해서 유사한 패턴을 찾은다음, Shape 기능을 이용하여 한번더 검사한다.
        {
            str.sprintf(("TemplateMatch step%d : %.2f%%"), nLoop, max*100);
            //g_cLog->AddLog(_LOG_LIST_SYS, str);

            cvSetImageROI(graySearchImg, cvRect(left_top.x, left_top.y, grayTemplateImg->width, grayTemplateImg->height));
            if (m_bSaveEngineImg)
            {
                str.sprintf(("242_MatchImage%d.jpg"), nLoop);
                SaveOutImage(graySearchImg, pData, str);
            }

            if (dMatchShapesingRate > 0)
            {
                IplImage* g1 = cvCreateImage(cvSize(grayTemplateImg->width, grayTemplateImg->height), IPL_DEPTH_8U, 1);

                cvCopy(graySearchImg, g1); // cvSetImageROI() 만큼 Copy
                if (nThresholdValue == 0)
                    ThresholdOTSU(pData, g1, 243);
                else
                    ThresholdRange(pData, g1, 243);

                NoiseOut(pData, g1, _ProcessValue2, 244); // 노이즈 제거
                Expansion(pData, g1, _ProcessValue2, 245);

                // 가장큼 or 긴 blob만 남긴다.
                if (nFilterBlob == 1)
                    FilterLargeArea(g1);
                else if (nFilterBlob == 2)
                    FilterLargeDiameter(g1);
                if (m_bSaveEngineImg){
                    SaveOutImage(g1, pData, ("248_FilterBlob.jpg"));
                }

                NoiseOut(pData, g1, _ProcessValue2, 251);
                Expansion(pData, g1, _ProcessValue2, 252);
                cvCanny(g1, g1, 100, 300, 3);
                if (m_bSaveEngineImg){
                    str.sprintf(("256_SearchImageCany%d.jpg"), nLoop);
                    SaveOutImage(g1, pData, str);
                }

                // g1이 전부 0로 채워져있으면 cvMatchShapes()에서 0로 리턴되어서 zero image filtering
                //IplImage* c1 = cvCloneImage(g1);
                //CvSeq* contours = 0;
                //cvFindContours(c1, storage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL);
                //cvReleaseImage(&c1);

                double matching = 1.0;
                //if (contours && contours->total > 0)
                    matching = cvMatchShapes(g1, g2, CV_CONTOURS_MATCH_I1); // 작은 값 일수록 두 이미지의 형상이 가까운 (0은 같은 모양)라는 것이된다.
                cvReleaseImage(&g1);

                str.sprintf(("MatchTemplate cvMatchShapes : %.3f"), matching);
                theMainWindow->DevLogSave(str.toLatin1().data());

                Point2f pt2 = Point2f((float)grayTemplateImg->width, (float)grayTemplateImg->height);
                cvRectangle(graySearchImg, cvPoint(0, 0), pt2, CV_RGB(128, 128, 128), CV_FILLED); // filled rectangle.

                cvResetImageROI(graySearchImg);

                if (matching > 1.0)
                    matching = 1.0;
                dMatchShapes = (1.0-matching) * 100.0;
                if (matching <= (1.0 - dMatchShapesingRate)) // || max > 0.9)
                {
                    str.sprintf(("Template Shape Match(succ) ===> : %.2f%%"), dMatchShapes);
                    theMainWindow->DevLogSave(str.toLatin1().data());

//                    cvReleaseImage(&graySearchImg);
//                    cvReleaseImage(&C);
//                    cvReleaseImage(&g2);
//                    cvReleaseMemStorage(&storage);

                    m_DetectResult.result = true; // OK

                    //left_top.x += srect.left;
                    //left_top.y += srect.top;
                    //return (max*100);
                    break;
                }
                else {
                    str.sprintf(("Template Shape Match(Fail) ===> : %.2f%%"), dMatchShapes);
                    qDebug() << str;
                    theMainWindow->DevLogSave(str.toLatin1().data());
                    break;
                }
                nLoop++;
                //if (nLoop > 10) {
                //	str.sprintf(("MatchShape failed"));
                //	//g_cLog->AddLog(_LOG_LIST_SYS, str);
                //	break;
                //}
            } else {
                str.sprintf(("TemplateMatch Match(succ) ===> : %.2f%%"), maxRate * 100);
                theMainWindow->DevLogSave(str.toLatin1().data());
                m_DetectResult.result = true; // OK
                break;
            }
        }
        else
        {
            nLoop++;
            //if (nLoop > 10) {
                //dMatchShapes = maxRate * 100;
                str.sprintf(("TemplateMatch Result Fail ===> : %.2f%%"), maxRate * 100);
                theMainWindow->DevLogSave(str.toLatin1().data());
                max = maxRate;
                m_DetectResult.result = false; // NG
                break;
            //}
        }
    }

    clock_t finish_time1 = clock();
    double total_time = (double)(finish_time1-start_time1)/CLOCKS_PER_SEC;
    //QString str;
    str.sprintf("Searching Time=%dms", (int)(total_time*1000));
    theMainWindow->DevLogSave(str.toLatin1().data());


    if(graySearchImg) cvReleaseImage(&graySearchImg);
    if(C) cvReleaseImage(&C);
    if(g2) cvReleaseImage(&g2);
    if(storage) cvReleaseMemStorage(&storage);

    return max*100.0;// -1;
}


//
// 사용자가 설정한 값으로 Threshold를 실행
//
int CImgProcEngine::Threshold(RoiObject *pData, IplImage* grayImg, int nDbg)
{
    QString str;

    int nThresholdValue = 70;
    int nThresholdMaxVal = 255;

    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Brightness Threshold"));
        if (pParam)
            nThresholdValue = pParam->Value.toDouble();
        pParam = pData->getParam(("Brightness Max"));
        if (pParam)
            nThresholdMaxVal = pParam->Value.toDouble();
    }

    int nInvert = CV_THRESH_BINARY;
    cvThreshold(grayImg, grayImg, nThresholdValue, nThresholdMaxVal, nInvert);
    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvThreshold.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }

    return 0;
}

//
// 사용자가 설정한 값으로 Threshold를 실행
//
int CImgProcEngine::ThresholdRange(RoiObject *pData, IplImage* grayImg, int nDbg)
{
    QString str;

    int nThresholdLowValue = 70;
    int nThresholdHighValue = 255;

    //if (m_bSaveEngineImg){
    //	str.sprintf(("%03d_cvThresholdRange.jpg"), nDbg);
    //	SaveOutImage(grayImg, pData, str, false);
    //}

    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Low Threshold"), -1);
        if (pParam)
            nThresholdLowValue = pParam->Value.toDouble();
        pParam = pData->getParam(("High Threshold"), -1);
        if (pParam)
            nThresholdHighValue = pParam->Value.toDouble();
    }
    if (nThresholdHighValue == 0 && nThresholdLowValue == 0)
        return -1;

    cvInRangeS(grayImg, Scalar(nThresholdLowValue), Scalar(nThresholdHighValue), grayImg);

    //if (m_bSaveEngineImg){
    //	str.sprintf(("%03d_cvThresholdRange1.jpg"), nDbg);
    //	SaveOutImage(grayImg, pData, str, false);
    //}

    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvThresholdRange.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }

    return 0;
}

//
// OTUS 알고리즘을 이용하여 히스토그램 Threshold를 실행
//
double CImgProcEngine::ThresholdOTSU(RoiObject *pData, IplImage* grayImg, int nDbg)
{
    QString str;

    //int nThresholdValue = 70;
    int nThresholdMaxVal = 255;
    //int bInvert = 0;
    if (pData != nullptr) {
        //CParam *pParam;// = pData->getParam(("Brightness Threshold"));
        //if (pParam)
        //	nThresholdValue = pParam->Value.toDouble();
        //pParam = pData->getParam(("Brightness Max"));
        //if (pParam)
        //	nThresholdMaxVal = pParam->Value.toDouble();
        //pParam = pData->getParam(("Invert?"));
        //if (pParam)
        //	bInvert = (int)pParam->Value.toDouble();
    }

    int nInvert = CV_THRESH_BINARY | CV_THRESH_OTSU;
    //if (bInvert == 1)
    //	nInvert = CV_THRESH_BINARY_INV | CV_THRESH_OTSU;
    double otsuThreshold = cvThreshold(grayImg, grayImg, 0, nThresholdMaxVal, nInvert);
    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvThreshold.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }

    return otsuThreshold;
}

int CImgProcEngine::AdaptiveThreshold(RoiObject *pData, IplImage* grayImg, int nDbg)
{
    QString str;
    int nBlkSize = 41;// pRoiData->nParam(23); //Local variable binarize window size
    int C = 3;// pRoiData->nParam(24); //Local variable binarize threshold

    CParam *pParam = pData->getParam(("Binarize window size"));
    if (pParam)
        nBlkSize = pParam->Value.toDouble();
    pParam = pData->getParam(("Binarize area size"));
    if (pParam)
        C = pParam->Value.toDouble();

    int cx = grayImg->width;
    int cy = grayImg->height;

    static cv::Mat dst;
    //세번째 255는 최대값.  이렇게 하면 0과 255로 이루어진 영상으로 됨
    //마지막에서 두번째값은 threshold 계산 때 주변 픽셀 사용하는 크기.  3,5,7 식으로 홀수로 넣어줌
    //마지막은 Constant subtracted from mean or weighted mean. It may be negative

    //if (nBlkSize == 0)
    //	nBlkSize = 51;
    //if (C == 0)
    //	C = 11;
    if (nBlkSize > min(cx, cy) / 4)
        nBlkSize = min(cx, cy) / 4;
    if (nBlkSize % 2 == 0) // 강제로 홀수로 만든다.
        nBlkSize++;
    cv::Mat m1 = cv::cvarrToMat(grayImg); //cv::Mat(s)
    cv::adaptiveThreshold(m1, dst, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, nBlkSize, C);

    IplImage iplImage = dst;
    cvCopy(&iplImage, grayImg);

    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvAdaptiveThreshold.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }

    return 0;
}

//
// 모폴리지를 이용하여 잡음제거
//
int CImgProcEngine::NoiseOut(RoiObject *pData, IplImage* grayImg, int t, int nDbg, int h)
{
    QString str;

    if (t < 0)
        t = _ProcessValue1;
    //IplImage* tmp = cvCreateImage(cvGetSize(grayImg), 8, 1);

    // 1. Template이미지의 노이즈 제거
    int filterSize = 3;  // 필터의 크기를 6으로 설정 (Noise out area)
//    if (pData != nullptr) {
//        CParam *pParam = pData->getParam(("Noise out area"));
//        if (pParam)
//            filterSize = pParam->Value.toDouble();
//    }
    IplConvKernel *element = nullptr;
    if (filterSize <= 0)
        filterSize = 1;
    if (filterSize % 2 == 0)
        filterSize++;
    //element = cvCreateStructuringElementEx(filterSize, filterSize, (filterSize - 1) / 2, (filterSize - 1) / 2, CV_SHAPE_RECT, nullptr);
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

    if (m_bSaveEngineImg){
        if (h >= 0)
            str.sprintf(("%d_%03d_cvClose.jpg"), h, nDbg);
        else str.sprintf(("%03d_cvClose.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }
    //cvReleaseImage(&tmp);

    cvReleaseStructuringElement(&element);
    return 0;
}

//
// Dialate / Erode
//
int CImgProcEngine::Expansion(RoiObject *pData, IplImage* grayImg, int t, int nDbg, int h)
{
    Q_UNUSED(h);
    QString str;

    if (t < 0)
        t = _ProcessValue1;
    int nExpansion1 = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Expansion 1"),t);
        if (pParam)
            nExpansion1 = pParam->Value.toDouble();
    }
    int nExpansion2 = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Expansion 2"),t);
        if (pParam)
            nExpansion2 = pParam->Value.toDouble();
    }
    if (nExpansion1 == 0 && nExpansion2 == 0)
        return 0;

    //IplImage* tmp = cvCreateImage(cvGetSize(grayImg), 8, 1);
    if (nExpansion1 < 0)
        cvErode(grayImg, grayImg, nullptr, -nExpansion1);
    else  //if (nExpansion > 0)
        cvDilate(grayImg, grayImg, nullptr, nExpansion1);

    if (nExpansion2 < 0)
        cvErode(grayImg, grayImg, nullptr, -nExpansion2);
    else //if (nExpansion > 0)
        cvDilate(grayImg, grayImg, nullptr, nExpansion2);

    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvExpansion.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }
    //cvReleaseImage(&tmp);
    return 0;
}

//
// 입력된 흑백 이미지 Thinner 처리
//
void CImgProcEngine::Thinner(RoiObject *pData, IplImage* grayImg, int nDbg)
{
    QString str;
    Mat1b m = Mat1b(cvarrToMat(grayImg).clone());

    IplImage *trans = nullptr;
    bool ok = thinner.thin(m, IMPL_GUO_HALL_FAST, false);
    if (ok) {
        IplImage iplImage = thinner.get_skeleton();
        trans = &iplImage;
    }
    if (m_bSaveEngineImg){
        str.sprintf(("%03d_Thin.jpg"), nDbg);
        SaveOutImage(trans, pData, str, false);
    }

    if (trans)
        cvCopy(trans, grayImg);
    else
        cvZero(grayImg);
    m.release();
}

void CImgProcEngine::DrawResultCrossMark(IplImage *iplImage, RoiObject *pData)
{
    if (iplImage == nullptr) return;

    QRectF rect = pData->bounds();	// Area로 등록된 ROI
    rect.normalized();

    if (rect.left() < 0)	rect.setLeft(0);
    if (rect.top() < 0)	rect.setTop(0);
    if (rect.right() >= iplImage->width) rect.setRight(iplImage->width);
    if (rect.bottom() >= iplImage->height) rect.setBottom(iplImage->height);

    int size = pData->m_vecDetectResult.size();
    for (int i = 0; i < size; i++) {
        DetectResult *prst = &pData->m_vecDetectResult[i];
        qDebug() << "DrawResultCrossMark" << prst->pt.x << prst->pt.y;

        double x = prst->pt.x + rect.x();// / gCfg.m_pCamInfo[0].dResX;
        double y = prst->pt.y + rect.y();// / gCfg.m_pCamInfo[0].dResY;

        double w = fabs(prst->br.x - prst->tl.x);
        double h = fabs(prst->br.y - prst->tl.y);
        if (w + h > 0)
        {
            if (prst->resultType == RESULTTYPE_RECT4P)
            {
                x = y = 0;
                cvLine(iplImage, CvPoint(prst->tl.x,prst->tl.y), CvPoint(prst->tr.x,prst->tr.y), cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
                cvLine(iplImage, CvPoint(prst->tr.x,prst->tr.y), CvPoint(prst->br.x,prst->br.y), cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
                cvLine(iplImage, CvPoint(prst->br.x,prst->br.y), CvPoint(prst->bl.x,prst->bl.y), cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
                cvLine(iplImage, CvPoint(prst->bl.x,prst->bl.y), CvPoint(prst->tl.x,prst->tl.y), cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
            }
            else
            {
                Point2f pt2 = Point2f((float)x+w, (float)y+h);
                cvRectangle(iplImage, cvPoint(x, y), pt2, CV_RGB(255, 0, 0), 2);
                x += w/2;
                y += h/2;
            }
        }

        if (x > 0 && y > 0)
        {
            CvPoint pt1, pt2;
            pt1.x = x - 40;
            pt1.y = y;
            pt2.x = x + 40;
            pt2.y = y;
            cvLine(iplImage, pt1, pt2, CV_RGB(192, 192, 192), 2, 8, 0);
            pt1.x = x;
            pt1.y = y - 40;
            pt2.x = x;
            pt2.y = y + 40;
            cvLine(iplImage, pt1, pt2, CV_RGB(192, 192, 192), 2, 8, 0);
            }
    }
}

void CImgProcEngine::SaveOutImage(IplImage* pImgOut, RoiObject *pData, QString strMsg, bool bClear/*=false*/)
{
    if (!gCfg.m_bSaveEngineImg)
        return;
    QString str = ("");
    if (pData != nullptr)
        str.sprintf("%s/[%s]%s_%s", m_sDebugPath.toStdString().c_str(), pData->groupName().toStdString().c_str(), pData->name().toStdString().c_str(), strMsg.toStdString().c_str());
    else
        str.sprintf("%s/%s", m_sDebugPath.toStdString().c_str(), strMsg.toStdString().c_str());
    cvSaveImage((const char *)str.toStdString().c_str(), pImgOut);
    if (bClear) cvZero(pImgOut);

    //qDebug() << "SaveOutImage:" << str;
}

//void CImgProcEngine::SaveOutImage(IplImage* pImgOut, QString strMsg, bool bClear/*=false*/)
//{
//	QString str = ("");
//	str.sprintf(("%s\\%s"), m_sDebugPath.toStdString().c_str(), strMsg);
//	CT2A ascii(str); cvSaveImage(ascii, pImgOut);
//	if (bClear) cvZero(pImgOut);
//}


int CImgProcEngine::SingleROIOCR(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect)
{
    QString str;



    tessApi->SetVariable("tessedit_char_whitelist",
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    tessApi->SetVariable("tessedit_char_blacklist",
        "~!@#$%^&*()_+`\\[],.;':\"?><");

    // Set Page segmentation mode to PSM_AUTO (3)
    tessApi->SetPageSegMode(tesseract::PSM_AUTO);


    ThresholdRange(pData, croppedImage, 200);

    int iMinY = 0, iMaxY = 100000;
    if (pData != NULL) {
        CParam *pParam = pData->getParam(("Min Size Y"));
        if (pParam)
            iMinY = (int)(pParam->Value.toDouble());
        pParam = pData->getParam(("Max Size Y"));
        if (pParam)
            iMaxY = (int)(pParam->Value.toDouble());
    }
    FilterBlobBoundingBoxYLength(croppedImage, iMinY, iMaxY);
    if (gCfg.m_bSaveEngineImg) {
        str.sprintf(("%03d_Filter.BMP"), 201);
        SaveOutImage(croppedImage, pData, str, false);
    }

    NoiseOut(pData, croppedImage, _ProcessValue1, 202);
    Expansion(pData, croppedImage, _ProcessValue1, 204);

    NoiseOut(pData, croppedImage, _ProcessValue2, 206);
    Expansion(pData, croppedImage, _ProcessValue2, 208);

    if (gCfg.m_bSaveEngineImg) {
        str.sprintf(("%03d_cvClose.BMP"), 210);
        SaveOutImage(croppedImage, pData, str, false);
    }

    int bInvert = 0;
    if (pData != NULL) {
        CParam *pParam = pData->getParam(("Invert?"));
        if (pParam)
            bInvert = (int)pParam->Value.toDouble();
        if (bInvert == 1)
            cvNot(croppedImage, croppedImage);
        if (m_bSaveEngineImg){
            str.sprintf(("%03d_Invert.jpg"), 211);
            SaveOutImage(croppedImage, pData, str, false);
        }
    }
    double dSizeX = 1.0, dSizeY = 1.0;
    if (pData != NULL) {
        CParam *pParam = pData->getParam(("Size X(%)"));
        if (pParam)
            dSizeX = (pParam->Value.toDouble()) / 100.0;
        pParam = pData->getParam(("Size Y(%)"));
        if (pParam)
            dSizeY = (pParam->Value.toDouble()) / 100.0;
    }

    CvSize sz = CvSize(croppedImage->width * dSizeX, croppedImage->height * dSizeY);
    IplImage* tmp = cvCreateImage(sz, 8, 1);
    cvResize(croppedImage, tmp, CV_INTER_CUBIC);

    Smooth(pData, tmp, 220);

    if (gCfg.m_bSaveEngineImg) {
        str.sprintf(("%03d_cvTmp.BMP"), 300);
        SaveOutImage(croppedImage, pData, str, false);
    }

    // Open input image using OpenCV
    cv::Mat im = cv::cvarrToMat(croppedImage);

    // Set image data
    tessApi->SetImage(im.data, im.cols, im.rows, 1, im.step); // BW color

    // Run Tesseract OCR on image
    char* rst = tessApi->GetUTF8Text();
    //string outText = string();

    // print recognized text
    //cout << outText << endl; // Destroy used object and release memory ocr->End();


    qDebug() << "OCR Text:" << rst;
    m_DetectResult.str = std::string(rst);
    pData->m_vecDetectResult.push_back(m_DetectResult);

//    CvFont font;
//    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3, 0, 1, CV_AA);

//    cvRectangle(outImg, CvPoint(0,0), CvPoint(outImg->width,10), cvScalar(255, 255, 255), CV_FILLED);
//    cvPutText(outImg, text, cvPoint(10, 10), &font, cvScalar(128, 128, 128));

    //QString str;
    str.sprintf("OCR Text:%s", rst);
    theMainWindow->DevLogSave(str.toLatin1().data());

    return -1;
}

int CImgProcEngine::SingleROIBarCode(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect)
{
    QZXing qz;

    cv::Mat m = cv::cvarrToMat(croppedImage);
    QImage img;
    img = MatToQImage(m);


    QString decode;
    try {
        decode = qz.decodeImage(img);
    }
    catch(zxing::NotFoundException  &e){}
    catch(zxing::ReaderException  &e){}

    qDebug() << "Barcode :" << decode;

    QString str;
    str = "Barcode : " + decode;
    theMainWindow->DevLogSave(str.toLatin1().data());
    m_DetectResult.str = str.toLatin1().data();
    pData->m_vecDetectResult.push_back(m_DetectResult);

    return 0;
}
