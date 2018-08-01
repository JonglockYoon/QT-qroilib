
#include <QDir>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>

#include "imgprocengine.h"
#include "voronoithinner.h"
#include "objectgroup.h"
#include "config.h"
#include "recipedata.h"
#include "mainwindow.h"

//using namespace cv;

static bool sort_using_greater_than(double u, double v)
{
	return u > v;
}
static bool sort_using_less_than(double u, double v)
{
	return u < v;
}


CImgProcEngine::CImgProcEngine()
{
    m_sDebugPath = ".";
    m_bSaveEngineImg = true;

    QString str;
    if (gCfg.m_sSaveImageDir.isEmpty())
        str = QString("%1/Engine").arg(gCfg.RootPath);
    else
        str = QString("%1/Engine").arg(gCfg.m_sSaveImageDir);
    QDir dir;
    dir.mkdir(str);
    m_sDebugPath = str;

}

CImgProcEngine::~CImgProcEngine()
{

}

cv::Mat CImgProcEngine::shiftFrame(cv::Mat frame, int pixels, eShiftDirection direction)
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


int CImgProcEngine::AlignImage(RoiObject* pData, IplImage* src)
{
	QString str;
	int nErrorType = 0;
	int nAlignIndex[4];
	int nMesAlingNum = 0;
	int nVerticalAlignPattNum = 0;
	int nHorizontalAlignPattNum = 0;
	int nTwoPointAlignType = 0; // 0 : None, 1 : Vertical, 2 : Horizontal

    int size = g_cRecipeData->m_vecRoiObject.size();
    vector<RoiObject*> *pVecRoiData = &g_cRecipeData->m_vecRoiObject;
	for (int i = 0; i<size; i++){
        RoiObject *pWorkData = (*pVecRoiData)[i];
        if (!pWorkData->objectGroup()->isVisible() || !pWorkData->isVisible())
            continue;

		if (pWorkData->mInspectType == _Inspect_Patt_VerticalAlign){
			nAlignIndex[nVerticalAlignPattNum] = i;
			nVerticalAlignPattNum++;
		}
		else if (pWorkData->mInspectType == _Inspect_Patt_HorizontalAlign){
			nAlignIndex[nHorizontalAlignPattNum] = i;
			nHorizontalAlignPattNum++;
		}
		else if (pWorkData->mInspectType == _Inspect_Roi_MeasureAlign){
			nAlignIndex[nMesAlingNum] = i;
			nMesAlingNum++;
		}
	}

	if (nVerticalAlignPattNum && nVerticalAlignPattNum != 2){ // 패턴은 가로 두개 또는 세로 두개 이어야한다.
		nErrorType = 1;
        return nErrorType;
	}
	else if (nHorizontalAlignPattNum && nHorizontalAlignPattNum != 2){
		nErrorType = 1;
        return nErrorType;
	}
	else if (nVerticalAlignPattNum == 0 && nHorizontalAlignPattNum == 0 && nMesAlingNum == 0){
		nErrorType = 1;
        return nErrorType;
	}

	double *pAngle = NULL;

    if (m_bSaveEngineImg)
	{
		QString str;
		str.sprintf(("%d_AlignSrc.BMP"), 100);
        SaveOutImage(src, pData, str);
	}

	if (nVerticalAlignPattNum || nHorizontalAlignPattNum) // 패턴 얼라인
	{
        RoiObject* pData = (*pVecRoiData)[nAlignIndex[0]];
        RoiObject* pData1 = (*pVecRoiData)[nAlignIndex[1]];

        //g_cLog->AddLog(_LOG_LIST_SYS, ("TwoPoint Align Start"));

		if ((nVerticalAlignPattNum > 0) && (nHorizontalAlignPattNum == 0)){
			// 1 : Vertical
			nTwoPointAlignType = 1;
		}
		else if ((nVerticalAlignPattNum == 0) && (nHorizontalAlignPattNum > 0)){
			// 2 : Horizontal
			nTwoPointAlignType = 2;
		}

		// 2016.6.6 jlyoon 향후 구현필요
		// 6월8일현재 Measure Align 기능만 구현해 놓았다.
        //if (nErrorType = TwoPointAlign(src, dst, (RoiObject*)pRoiData, (RoiObject*)pRoiData1, pAngle, nTwoPointAlignType))
		{
			nErrorType = 2;
            //g_cLog->AddLog(_LOG_LIST_SYS, ("[AlignSet] Image Align Set Error..."));
            return nErrorType;
		}
        //g_cLog->AddLog(_LOG_LIST_SYS, ("TwoPoint Align End..."));

	} //if(nVerticalAlignPattNum || nHorizontalAlignPattNum)
	else if (nMesAlingNum == 3) //  Measure 얼라인
	{
		struct {
            RoiObject* pData;
			double dOrgDiff; // 기준 X,Y위치
			double dEdge;
		} pos[2][3]; // [v=0,h=1] [3]

		int nVert = 0, nHoriz = 0;
		int nDirection;	//("Left2Right,Right2Left,Top2Bottom,Bottom2Top")
		for (int i = 0; i < 3; i++) {
            RoiObject* pData = (*pVecRoiData)[nAlignIndex[i]];

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
			str.sprintf(("%d_AlignDst.BMP"), 101);
            SaveOutImage(src, pData, str);
		}

        //g_cLog->AddLog(_LOG_LIST_SYS, ("Measure Align End..."));

	} 
	else if (nMesAlingNum) // nMesAlingNum == 1 이면 가로 또는 세로위치만 이동한다. (한개만 사용)
	{
		int nDirection;
        RoiObject* pData = (*pVecRoiData)[nAlignIndex[0]];
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
                    cvCopy(&iplImage, src, NULL);
				}
				else if (sx > 0) { // shift right
					sx = abs(sx);
					cv::Mat m = shiftFrame(cvarrToMat(src), abs(sx), ShiftRight);
                    IplImage iplImage = m;
                    cvCopy(&iplImage, src, NULL);
				}
			}

		}
	}

	return nErrorType;
}

int CImgProcEngine::InspectOneItem(IplImage* img, RoiObject *pData)
{
	QString str;
	m_DetectResult.dRadius = 0;
	m_DetectResult.ngBlobImg = NULL;

	if (pData == NULL)
		return -1;

    curImg = img;

    if (m_bSaveEngineImg)
    {
        str.sprintf("%d_srcImage.jpg", 100);
        SaveOutImage(img, pData, str, false);
    }

	int size = pData->m_vecDetectResult.size();
	for (int i = 0; i < size; i++) {
		DetectResult *prst = &pData->m_vecDetectResult[i];
		if (prst->ngBlobImg)
			cvReleaseImage(&prst->ngBlobImg);
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

    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_grayImage.jpg"), 110);
        SaveOutImage(graySearchImg, pData, str, false);
	}

	IplImage* croppedImage;
    QRectF rect = pData->bounds();	// Area로 등록된 ROI

    QRectF rect1 = rect;
    float w = rect1.width() / 2;
    float h = rect1.height() / 2;
    rect.normalized();
	switch (pData->mInspectType)
	{
	case _Inspect_Roi_BondingCircle: // ROI 영역보다 큰 영역을 잡는다.
	case _Inspect_Roi_BondingRect:
        rect.setLeft(rect.left() - w);
        rect.setRight(rect.right() + w);
        rect.setTop(rect.top() - h);
        rect.setBottom(rect.bottom() + h);
		break;
	default:
		break;
	}

    if (rect.left() < 0)	rect.setLeft(0);
    if (rect.top() < 0)	rect.setTop(0);
    if (rect.right() >= graySearchImg->width) rect.setRight(graySearchImg->width - 1);
    if (rect.bottom() >= graySearchImg->height) rect.setBottom(graySearchImg->height - 1);

    Point2f left_top = Point2f(rect.left(), rect.top());
    cvSetImageROI(graySearchImg, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
    croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), graySearchImg->depth, graySearchImg->nChannels);
	cvCopy(graySearchImg, croppedImage);
	cvResetImageROI(graySearchImg);

	int nDirection;
	switch (pData->mInspectType)
	{
    case _Inspect_Patt_Identify:
        //strLog.Format(("[%s] InspectType : _Inspect_Patt_Identify"), pData->m_sName);
        //MSystem.DevLogSave(("%s"), strLog);
        SinglePattIdentify(croppedImage, pData, rect);
        break;

//	case _Inspect_Roi_MeasureAlign:
//		if (pData->getIntParam(("Direction"), nDirection) == 0) {
//			if (nDirection == 4)  { //  (nCh == 4) 상부 Align은 원의 무게 줌심을 가지고 처리한다.
//                //AlignImageCH4(pData, graySearchImg);
//                //break;
//			}
//		}
//        SingleROISubPixEdgeWithThreshold(croppedImage, pData, rect);
//		break;
    case _Inspect_Roi_CenterOfPlusMark:
        SingleROICenterOfPlusMark(croppedImage, pData, rect);
        break;
	case _Inspect_Roi_BondingCircle:
        SingleROIBondingCircleCheck(croppedImage, pData, rect);
		break;
	case _Inspect_Roi_BondingRect:
        SingleROIBondingRectCheck(croppedImage, pData, rect);
		break;
	case _Inspect_Roi_SubpixelEdgeWithThreshold:
        //strLog.sprintf(("[%s] InspectType : _Inspect_Roi_SubpixelEdgeWithThreshold"), pData->name().toStdString().c_str());
        //g_cLog->AddLog(strLog, _LOG_LIST_INSP);
        SingleROISubPixEdgeWithThreshold(croppedImage, pData, rect);
		break;
	case _Inspect_Roi_Circle_With_Threshold:
        SingleROICircleWithThreshold(croppedImage, pData, rect);
		break;
	case _Inspect_Roi_Bonding_Position:
        SingleROIBondingPosition(croppedImage, pData, rect);
		break;
	case _Inspect_Roi_Bonding_Hole:
        SingleROIBondingHole(croppedImage, pData, rect);
		break;
    case _Inspect_Roi_Screw:
        SingleROIScrew(croppedImage, pData, rect);
        break;
    case _Inspect_Roi_Bonding_HLine:
        SingleROIBondingLine(croppedImage, pData, rect);
		break;
	case _Inspect_Roi_Side3_LaserPoint:
        SingleROISide3LaserPoint(croppedImage, pData, rect);
		break;
	case _Inspect_Roi_Pin_ErrorCheck:
        SingleROIPinErrorCheck(croppedImage, pData, rect);
		break;
	}
	cvReleaseImage(&croppedImage);
	cvReleaseImage(&graySearchImg);

	return 0;
}

//
//
int CImgProcEngine::SingleROICenterOfPlusMark(IplImage* croppedImageIn, RoiObject *pData, QRectF rectIn)
{
	QString str;
	IplImage* drawImage = NULL;
	IplImage* croppedImage = cvCloneImage(croppedImageIn);
    int MinMomentSize = 6, MaxMomentSize = 600;
	int nMinCircleRadius = 300;
	int nMaxCircleRadius = 1000;

	m_DetectResult.pt.x = 0;
	m_DetectResult.pt.y = 0;

	if (pData != NULL)
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

		NoiseOut(pData, croppedImage, 131);
	}
	else {
		IplConvKernel *element;
		int filterSize = 3;
		//CBlobResult blobs;
		//int n;

		cvInRangeS(croppedImage, cvScalar(200), cvScalar(255), croppedImage);

		filterSize = 3;
		element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, NULL);
		cvMorphologyEx(croppedImage, croppedImage, NULL, element, CV_MOP_OPEN, 2);
		cvMorphologyEx(croppedImage, croppedImage, NULL, element, CV_MOP_CLOSE, 2);
		cvReleaseStructuringElement(&element);

        if (m_bSaveEngineImg)
		{
			str.sprintf(("%d_Preprocess.jpg"), 132);
            SaveOutImage(croppedImage, NULL, str, false);
		}
	}

    if (m_bSaveEngineImg) {
		drawImage = cvCreateImage(cvSize(croppedImage->width, croppedImage->height), croppedImage->depth, 3);
        cvZero(drawImage);
    }
    ExcludeLargeBlob(croppedImage, pData, nMaxCircleRadius*2, 133);

    FilterLargeArea(croppedImage);
    if (m_bSaveEngineImg)
	{
        str.sprintf(("134_cvApproxInImage.jpg"), m_sDebugPath.toStdString().c_str());
        SaveOutImage(croppedImage, pData, str, false);
	}

    Expansion(pData, croppedImage, 134);



	// 외곽선 추적 및 근사화 변수 초기화
	CvMemStorage* m_storage = cvCreateMemStorage(0); // 배열 자료(점의 좌표가 들어간다)
	CvSeq* m_seq = 0;         // 경계 계수를 저장할 변수
	CvSeq* m_approxDP_seq = 0;
    //CvSeq* m_dominant_points = 0;    // 특징점 찾기 위한 변수
	CvSeq* ptseq;
	CvSeq* defect = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);

	cvFindContours(croppedImage, m_storage, &m_seq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// (2) 외곽선 근사화
	////////////////////////////////////////////////////////////////////////////////////////////////////
	int testcount = -1;
	int iContoursSize = 0;

	if (m_seq != 0)
	{
		testcount = 0;

		m_approxDP_seq = cvApproxPoly(m_seq, sizeof(CvContour), m_storage, CV_POLY_APPROX_DP, 3, 1);
		if (m_approxDP_seq == NULL)
		{
			if (m_storage) cvReleaseMemStorage(&m_storage);
			if (croppedImage) cvReleaseImage(&croppedImage);
			if (drawImage) cvReleaseImage(&drawImage);
			return -1;
		}

        if (m_bSaveEngineImg)
		{
			cvDrawContours(drawImage, m_approxDP_seq, CVX_RED, CVX_RED, 1, 1, 8); // 외곽선 근사화가 잘되었는지 테스트
            str.sprintf(("135_cvApproxPoly.jpg"), m_sDebugPath.toStdString().c_str());
            SaveOutImage(drawImage, pData, str, false);
		}

        //int nSeq = 0;
		for (CvSeq* c = m_approxDP_seq; c != NULL; c = c->h_next)  // 엣지의 링크드리스트를 순회하면서 각 엣지들에대해서 출력한다.
		{
			float radius;
			CvPoint2D32f center;
			cvMinEnclosingCircle(c, &center, &radius);

			// 반지름이 설정치보다 적거나 크면 제외
            qDebug() << nMinCircleRadius << nMaxCircleRadius << radius;
			if (nMinCircleRadius > radius || nMaxCircleRadius < radius)
				continue;

            ptseq = cvCreateSeq((CV_SEQ_KIND_CURVE | CV_SEQ_ELTYPE_POINT | CV_SEQ_FLAG_CLOSED) | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);
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
                    str.sprintf(("138_moments.jpg"), m_sDebugPath.toStdString().c_str());
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
	if (defect) cvClearSeq(defect);
	if (m_storage) cvReleaseMemStorage(&m_storage);

	if (croppedImage) cvReleaseImage(&croppedImage);
	if (drawImage) cvReleaseImage(&drawImage);

	if (m_DetectResult.pt.x == 0 || m_DetectResult.pt.y == 0) return -1;
	if (iContoursSize <= 0) return -1;

    m_DetectResult.rect = rectIn;// pData->m_RoiArea;
    pData->m_vecDetectResult.push_back(m_DetectResult);


	return iContoursSize;
}


//CrossPointOfThinner
Point2f CImgProcEngine::CrossPointOfThinner(RoiObject *pData, IplImage* srcimg, CvSeq* ptseq)
{
	IplImage* timg = NULL;
	timg = cvCreateImage(cvSize(srcimg->width, srcimg->height), IPL_DEPTH_8U, 1);
	cvZero(timg);
	cvDrawContours(timg, ptseq, CVX_WHITE, CVX_WHITE, 1, CV_FILLED, 8);
	cvDilate(timg, timg, NULL, 1);

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
	if (pData != NULL)
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
	cvDilate(timg, timg, NULL, 4);
    m0.release();

    if (m_bSaveEngineImg)
	{
        SaveOutImage(timg, pData, ("310_Thinner1.jpg"), false);
	}

    //IplImage* timgChk = cvCloneImage(timg);

	double cX = -1, cY = -1;
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
	//	NULL, : 관심영역 지정
	//	3, : 미분 계수의 자기상관행렬을 계산할 때 사용되는 평균 블록 크기
	//	1, : 해리스코너 사용 : use_harris로 0이 아니면 해리스 코너 사용. 0 : Shi - Tomasi방법 사용
	//	0.01 : 위 use_harris를 0이 아닌값으로 설정하였다면 이값은 코너 응답 함수 식에서 대각합에 대한 가중치로 작용
	//	);
	//cvGoodFeaturesToTrack(timg, eig_img, temp_img, corners, &corner_count, 0.1, 15, NULL, 3, 1, 0.01);
	cvGoodFeaturesToTrack(timg, eig_img, temp_img, corners, &corner_count, 0.5, 15, NULL, 3, 0, 0.01);
	cvFindCornerSubPix(timg, corners, corner_count, cvSize(3, 3), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
	for (int i = 0; i < corner_count; i++) {
		pt = cvPointFrom32f(corners[i]);
		//cvCircle(timg, pt, 3, CV_RGB(0, 0, 255), 2);

		// 코너가 "+"의 중심이 아닐수 있으므로 blob으로 재확인
		//int nRet2 = WACenterVerifyBlob(pData, timgChk, pt);
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
        str.sprintf(("375_centerofcontour.jpg"), m_sDebugPath.toStdString().c_str());
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
    blobs = CBlobResult(croppedImage, NULL, 0);	// Use a black background color.
    int nBlobs = blobs.GetNumBlobs();
    if (nBlobs > 0) {
        CBlob *p = blobs.GetBlob(0);
        CvRect rect = p->GetBoundingBox();
        if (rect.width < nsize || rect.height < nsize) {
            ret = -1;

            str.sprintf(("Center of plus mark blob size wrong"));
            qDebug() << str;
        }
    }
    else {
        ret = -1;

        str.sprintf(("Center of plus mark not found"));
        qDebug() << str;
    }
    cvReleaseImage(&croppedImage);
    return ret;
}

//
// VCM 본딩기 본딩 Circle 영역 검사
//
int CImgProcEngine::SingleROIBondingCircleCheck(IplImage* croppedImage, RoiObject *pData, QRectF rectIn)
{
	QString str;
	int n = 0;
	int nThresholdValue = 0;

    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_Src.jpg"), 210);
        SaveOutImage(croppedImage, pData, str, false);
	}

	CParam *pParam = pData->getParam(("High Threshold"));
	if (pParam)
        nThresholdValue = pParam->Value.toDouble();

	if (nThresholdValue == 0)
		ThresholdOTSU(pData, croppedImage, 211);
	else
		ThresholdRange(pData, croppedImage, 211);

	//AdaptiveThreshold(pData, croppedImage, 211);
	//cvNot(croppedImage, croppedImage);
    //if (m_bSaveEngineImg)
	//{
	//	str.sprintf(("%d_not.jpg"), 212);
    //	SaveOutImage(croppedImage, pData, str, false);
	//}

	NoiseOut(pData, croppedImage, 213);


	CBlobResult blobs;
	blobs = CBlobResult(croppedImage, NULL, 0);	// Use a black background color.
	int blobCount = blobs.GetNumBlobs();
	// Fill Blob
	if (blobCount > 0) {
		cvZero(croppedImage);
		for (int i = 0; i < blobs.GetNumBlobs(); i++) {
			CBlob *currentBlob = blobs.GetBlob(i);
			currentBlob->FillBlob(croppedImage, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
		}
	}
    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_fillblob.jpg"), 214);
        SaveOutImage(croppedImage, pData, str, false);
	}

	Expansion(pData, croppedImage, 215);
	//Smooth(pData, croppedImage, 216);

	// croppedImage에 들어 있는 Blob의 크기로 양불판정

	double dMinAreaP = 0.8;
	double dMaxAreaP = 1.3;
	pParam = pData->getParam(("Minimun Area(%)"));
	if (pParam)
        dMinAreaP = pParam->Value.toDouble() / 100;
	pParam = pData->getParam(("Maximun Area(%)"));
	if (pParam)
        dMaxAreaP = pParam->Value.toDouble() / 100;

    double dMinWidth = pData->bounds().width() * dMinAreaP;
    double dMaxWidth = pData->bounds().width() * dMaxAreaP;
    double dMinHeight = pData->bounds().height() * dMinAreaP;
    double dMaxHeight = pData->bounds().height() * dMaxAreaP;

	// 직사각형의 면적 기준
	//double dMinArea = dMinWidth * dMinHeight;
	//double dMaxArea = dMaxWidth * dMaxHeight;
	// 타원면적 기준
	double dMinArea = 3.1415926535 * (dMinWidth / 2) * (dMinHeight / 2);
	double dMaxArea = 3.1415926535 * (dMaxWidth / 2) * (dMaxHeight / 2);

	blobs = CBlobResult(croppedImage, NULL, 0);	// Use a black background color.
	blobCount = blobs.GetNumBlobs();

	QString s;
	double dLargeArea = 0;
	int nIndexLargeBlob = -1;
	QString s1;
	for (int i = 0; i < blobCount; i++) // 제일큰 Blob 한개만 남긴다
	{
		CBlob *currentBlob = blobs.GetBlob(i);
		double dArea = currentBlob->Area();
		if (dLargeArea < dArea) {
			// 가로세로비가 원형인것만 남긴다
			CvRect r = currentBlob->GetBoundingBox();
			const double circleError = 0.40;
			double longAxis = max(r.width, r.height);
			double smallAxis = min(r.width, r.height);
			if (longAxis > 0 && smallAxis > 0)
			{
				float circleMeasure = (float)((longAxis - smallAxis) / longAxis);
				if (circleMeasure > circleError) {// 원형이 아니면
                    //str.sprintf(("CH:%d Circle Rate Error = %.2f"), pData, circleMeasure);
                    ////g_cLog->AddLog(_LOG_LIST_INSP, str);
					continue;
				}
			}
			dLargeArea = dArea;
			nIndexLargeBlob = i;
			s1.sprintf(("%.0f "), dArea);
		}
		//s.sprintf(("%.0f "), dArea);
		//s1 += s;
	}
    s.sprintf(("%.0f <  %s < %.0f ?"), dMinArea, s1.toStdString().c_str(), dMaxArea);
    //str.sprintf(("CH:%d Circle Area = %s"), pData, s);
    //g_cLog->AddLog(_LOG_LIST_INSP, str);

	if (blobCount > 0 && nIndexLargeBlob >= 0) {
		cvZero(croppedImage);
		//for (int i = 0; i < blobs.GetNumBlobs(); i++) {
		CBlob *currentBlob = blobs.GetBlob(nIndexLargeBlob);
		currentBlob->FillBlob(croppedImage, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
		//}
	}
	else {
		cvZero(croppedImage);
	}
    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_procImage.jpg"), 217);
        SaveOutImage(croppedImage, pData, str, false);
	}

	m_DetectResult.nResult = 1; // OK
	blobs = CBlobResult(croppedImage, NULL, 0);	// Use a black background color.
	blobCount = blobs.GetNumBlobs();

	double w = 0;
	double h = 0;
	if (blobCount > 0)
	{
		CBlob *currentBlob = blobs.GetBlob(0);
		CvRect rect = currentBlob->GetBoundingBox();
		w = rect.width;
		h = rect.height;
		if (w <  dMinWidth || h < dMinHeight) // 본딩영역 Circle 가로 세로가 조건보다 크거나 적으면
			m_DetectResult.nResult = 2;
		if (w > dMaxWidth || h > dMaxHeight) {
			m_DetectResult.nResult = 2;
		}
	}

	if (dLargeArea < dMinArea || dLargeArea > dMaxArea) { // 본딩영역 Area 범위 밖이면
		m_DetectResult.nResult = 2;
        //str.sprintf(("%s Circle Area NG : %.0f <  %.0f < %.0f ? "), strChs[gCfg.m_nFrontRear][pData], dMinArea, dLargeArea, dMaxArea);
        str.sprintf(("CH:%d Circle Area NG"), pData);
	}
	else
        str.sprintf(("CH:%d Circle Area OK"), pData);
    //g_cLog->AddLog(_LOG_LIST_INSP, str);

	s.sprintf(("%.0f,%.0f <  %.0f,%.0f < %.0f,%.0f ?"), dMinWidth, dMinHeight, w, h, dMaxWidth, dMaxHeight);
    //str.sprintf(("CH:%d Circle Length = %s"), pData, s);
    //g_cLog->AddLog(_LOG_LIST_INSP, str);

    str.sprintf(("CH:%d Circle Length Result : "), pData);
	if (dMinWidth > w || dMaxWidth < w)
		str += ("NG");
	else if (dMinHeight > h || dMaxHeight < h)
		str += ("NG");
	else
		str += ("OK");
    //g_cLog->AddLog(_LOG_LIST_INSP, str);

	m_DetectResult.ngBlobImg = cvCloneImage(croppedImage); // Overflow Check전에 Blob저장

#if 1 // 
    str.sprintf(("CH:%s Overflow bonding Result : "), pData->groupName().toStdString().c_str());
    QRectF rect = pData->bounds();	// Area로 등록된 ROI
    rect.setLeft(rect.width() / 2);
    rect.setTop(rect.height() / 2);
    cvSetImageROI(croppedImage, cvRect((int)rect.left(), (int)rect.top(), rect.width(), rect.height()));
    cvRectangle(croppedImage, CvPoint(0, 0), CvPoint(rect.width(), rect.height()), CV_RGB(0, 0, 0), -1);
	cvResetImageROI(croppedImage);

	int filterSize = 3;
	IplConvKernel *element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, NULL);
	//cvMorphologyEx(croppedImage, croppedImage, NULL, element, CV_MOP_CLOSE, 2);
	cvMorphologyEx(croppedImage, croppedImage, NULL, element, CV_MOP_OPEN, 1); // 백색 잡음제거
	cvReleaseStructuringElement(&element);

	blobs = CBlobResult(croppedImage, NULL, 0);	// Use a black background color.
	blobCount = blobs.GetNumBlobs();
	if (blobCount > 0) {
		str += ("NG");
		m_DetectResult.nResult = 2;
	}
	else {
		str += ("OK");
	}
    //g_cLog->AddLog(_LOG_LIST_INSP, str);
#endif

    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_procImage.jpg"), 218);
		SaveOutImage(croppedImage, pData, str);
	}

    str.sprintf(("CH:%s %s Result : "), pData->groupName().toStdString().c_str(), pData->name().toStdString().c_str());
	if (m_DetectResult.nResult == 1)
		str += ("OK");
	else
		str += ("NG");
    //g_cLog->AddLog(_LOG_LIST_INSP, str);

	if (m_DetectResult.nResult == 1) {
		cvZero(croppedImage);
		//blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_EQUAL, dLargeArea);
	}

#if 0 // 2016.11.19현재 한개의 본딩Blob의 영역크기만 Chekc한다
	//  틘곳 Check는 주변의 음영지역이 많에 Check가 힘듬.
	if (blobCount == 1) { // 튄곳이 없으면 양품
		if (m_DetectResult.nResult != 2)
			m_DetectResult.nResult = 1;
	}
	else
		m_DetectResult.nResult = 2;
#endif

    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_result.jpg"), 500);
		SaveOutImage(croppedImage, pData, str);
	}

	m_DetectResult.pt = { 0, 0 };
    //m_DetectResult.pt1 = { 0, 0 };
	m_DetectResult.rect = rectIn;// pData->m_RoiArea;

    //m_DetectResult.nCh = pData;
	pData->m_vecDetectResult.push_back(m_DetectResult);

	return  n;
}


//
// VCM 본딩기 본딩 Rect 영역 검사
//
int CImgProcEngine::SingleROIBondingRectCheck(IplImage* croppedImage, RoiObject *pData, QRectF rectIn)
{
	QString str;
	int n = 0;
	int nThresholdValue = 0;
	CParam *pParam = pData->getParam(("High Threshold"));
	if (pParam)
        nThresholdValue = pParam->Value.toDouble();

	//if (nThresholdValue == 0)
	//	ThresholdOTSU(pData, croppedImage, 211);
	//else
	//	ThresholdRange(pData, croppedImage, 211);

	AdaptiveThreshold(pData, croppedImage, 211);
	cvNot(croppedImage, croppedImage);
    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_not.jpg"), 212);
		SaveOutImage(croppedImage, pData, str);
	}

	NoiseOut(pData, croppedImage, 213);


	CBlobResult blobs;
	blobs = CBlobResult(croppedImage, NULL, 0);	// Use a black background color.
	int blobCount = blobs.GetNumBlobs();
	// Fill Blob
	if (blobCount > 0) {
		cvZero(croppedImage);
		for (int i = 0; i < blobs.GetNumBlobs(); i++) {
			CBlob *currentBlob = blobs.GetBlob(i);
			currentBlob->FillBlob(croppedImage, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
		}
	}

    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_fillblob.jpg"), 214);
		SaveOutImage(croppedImage, pData, str);
	}

	Expansion(pData, croppedImage, 215);
	//Smooth(pData, croppedImage, 216);

	// croppedImage에 들어 있는 Blob의 크기로 양불판정

	double dMinAreaP = 0.8;
	double dMaxAreaP = 1.3;
	pParam = pData->getParam(("Minimun Area(%)"));
	if (pParam)
        dMinAreaP = pParam->Value.toDouble() / 100;
	pParam = pData->getParam(("Maximun Area(%)"));
	if (pParam)
        dMaxAreaP = pParam->Value.toDouble() / 100;

	double dMinRectP = 0.8;
	double dMaxRectP = 1.3;
	pParam = pData->getParam(("Minimun Rect(%)"));
	if (pParam)
        dMinRectP = pParam->Value.toDouble() / 100;
	pParam = pData->getParam(("Maximun Rect(%)"));
	if (pParam)
        dMaxRectP = pParam->Value.toDouble() / 100;


    double dMinWidth = pData->bounds().width() * dMinAreaP;
    double dMaxWidth = pData->bounds().width() * dMaxAreaP;
    double dMinHeight = pData->bounds().height() * dMinAreaP;
    double dMaxHeight = pData->bounds().height() * dMaxAreaP;

	// 직사각형의 면적 기준
	double dMinArea = dMinWidth * dMinHeight;
	double dMaxArea = dMaxWidth * dMaxHeight;

	blobs = CBlobResult(croppedImage, NULL, 0);	// Use a black background color.
	blobCount = blobs.GetNumBlobs();

	QString s;
	double dLargeArea = 0;
	int nIndexLargeBlob = -1;
	QString s1;
	for (int i = 0; i < blobCount; i++) // 제일큰 Blob 한개만 남긴다
	{
		CBlob *currentBlob = blobs.GetBlob(i);
		double dArea = currentBlob->Area();
		if (dLargeArea < dArea) {
			dLargeArea = dArea;
			nIndexLargeBlob = i;
			s1.sprintf(("%.0f "), dArea);
		}
		//s.sprintf(("%.0f "), dArea);
		//s1 += s;
	}
    //s.sprintf(("%.0f <  %s < %.0f ?"), dMinArea, s1, dMaxArea);
    //str.sprintf(("CH:%d Rect Area = %s"), pData, s);
    //g_cLog->AddLog(_LOG_LIST_INSP, str);

	if (blobCount > 0 && nIndexLargeBlob >= 0) {
		cvZero(croppedImage);
		//for (int i = 0; i < blobs.GetNumBlobs(); i++) {
		CBlob *currentBlob = blobs.GetBlob(nIndexLargeBlob);
		currentBlob->FillBlob(croppedImage, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
		//}
	}
	else {
		cvZero(croppedImage);
	}

    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_procImage.jpg"), 215);
		SaveOutImage(croppedImage, pData, str);
	}

	m_DetectResult.nResult = 1; // OK
	blobs = CBlobResult(croppedImage, NULL, 0);	// Use a black background color.
	blobCount = blobs.GetNumBlobs();
	double w = 0;
	double h = 0;
	if (blobCount > 0)
	{
		//if (nIndexLargeBlob >= 0 && blobCount > nIndexLargeBlob) {
		CBlob *currentBlob = blobs.GetBlob(0);
		CvRect rect = currentBlob->GetBoundingBox();
		w = rect.width;
		h = rect.height;
		//if (w <  dMinWidth || h < dMinHeight) // 본딩영역 Circle 가로 세로가 조건보다 크거나 적으면
		//	m_DetectResult.nResult = 2; // NG
		//if (w > dMaxWidth || h > dMaxHeight)
		//	m_DetectResult.nResult = 2; // NG
	}
	if (dLargeArea < dMinArea || dLargeArea > dMaxArea) {// 본딩영역 Area 범위 밖이면
		m_DetectResult.nResult = 2;
        //str.sprintf(("CH:%s Rect Area NG"), pData->groupName());
	}
    //else
    //    str.sprintf(("CH:%s Rect Area OK"), pData->groupName());
    //g_cLog->AddLog(_LOG_LIST_INSP, str);

	// 여기서 부터는 가로 세로 길이로 NG판단
    dMinWidth = pData->bounds().width() * dMinRectP;
    dMaxWidth = pData->bounds().width() * dMaxRectP;
    dMinHeight = pData->bounds().height() * dMinRectP;
    dMaxHeight = pData->bounds().height() * dMaxRectP;
	if (w <  dMinWidth || h < dMinHeight) // 본딩영역 Rect 가로 세로가 조건보다 크거나 적으면
		m_DetectResult.nResult = 2; // NG
	if (w > dMaxWidth || h > dMaxHeight)
		m_DetectResult.nResult = 2; // NG
	s.sprintf(("%.0f,%.0f <  %.0f,%.0f < %.0f,%.0f ?"), dMinWidth, dMinHeight, w, h, dMaxWidth, dMaxHeight);
    //str.sprintf(("CH:%d Rect Diameter = %s"), pData, s);
    //g_cLog->AddLog(_LOG_LIST_INSP, str);

    str.sprintf(("CH:%d Rect Length Result : "), pData);
	if (dMinWidth > w || dMaxWidth < w)
		str += ("NG");
	else if (dMinHeight > h || dMaxHeight < h)
		str += ("NG");
	else
		str += ("OK");
    //g_cLog->AddLog(_LOG_LIST_INSP, str);

	m_DetectResult.ngBlobImg = cvCloneImage(croppedImage); // Overflow Check전에 Blob저장

    str.sprintf(("CH:%d Overflow bonding Result : "), pData);
    QRectF rect = pData->bounds();	// Area로 등록된 ROI
    rect.setLeft(rect.width() / 2);
    rect.setTop(rect.height() / 2);
    cvSetImageROI(croppedImage, cvRect((int)rect.left(), (int)rect.top(), rect.width(), rect.height()));
    cvRectangle(croppedImage, CvPoint(0, 0), CvPoint(rect.width(), rect.height()), CV_RGB(0, 0, 0), -1);
	cvResetImageROI(croppedImage);
	blobs = CBlobResult(croppedImage, NULL, 0);	// Use a black background color.
	blobCount = blobs.GetNumBlobs();
	if (blobCount > 0) {
		str += ("NG");
		m_DetectResult.nResult = 2;
	} else {
		str += ("OK");
	}
    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_procImage.jpg"), 216);
		SaveOutImage(croppedImage, pData, str);
	}

	if (m_DetectResult.nResult == 1) {
		cvZero(croppedImage);
		//blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_EQUAL, dLargeArea);
	}

#if 0 // 2016.11.19현재 한개의 본딩Blob의 영역크기만 Chekc한다
	//  틘곳 Check는 주변의 음영지역이 많아 Check가 힘듬.
	if (blobCount == 1) { // 튄곳이 없으면 양품
		if (m_DetectResult.nResult != 2)
			m_DetectResult.nResult = 1;
	}
	else
		m_DetectResult.nResult = 2;
#endif

    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_result.jpg"), 500);
		SaveOutImage(croppedImage, pData, str);
	}

	m_DetectResult.pt = { 0, 0 };
    //m_DetectResult.pt1 = { 0, 0 };
	m_DetectResult.rect = rectIn;// pData->m_RoiArea;

	//m_DetectResult.pt = CvPoint2D32f(0,0);
	//m_DetectResult.pt1 = CvPoint2D32f(0,0);
	//m_DetectResult.dAngle = 0;
    //m_DetectResult.nCh = pData;
	pData->m_vecDetectResult.push_back(m_DetectResult);

//    str.sprintf(("CH:%s %s Result : "), pData->groupName(), pData->name().toStdString().c_str());
//	if (m_DetectResult.nResult == 1)
//		str += ("OK");
//	else
//		str += ("NG");
    //g_cLog->AddLog(_LOG_LIST_INSP, str);

	return  n;
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

	NoiseOut(pData, grayImg, 212);
	Expansion(pData, grayImg, 213);

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
	IplImage* edgeImage = NULL;
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

	double dEdge = SubPixelRampEdgeImage(edgeImage, pData, nDir);
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


    //str.sprintf(("CH:%d %s Edge Result : %.3f"), pData, pData->name().toStdString().c_str(), dEdge);
    ////g_cLog->AddLog(_LOG_LIST_SYS, str);

	return dEdge;
}

int CImgProcEngine::SingleROICircleWithThreshold(IplImage* croppedImage, RoiObject *pData, QRectF rect)
{
	QString str;
	IplImage* grayImg = cvCloneImage(croppedImage);

	int ret = 0;
	CvMemStorage* storage = NULL;
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

	NoiseOut(pData, grayImg, 221);

	Smooth(pData, grayImg, 231);

	int nMinCircleRadius = 30;
	int nMaxCircleRadius = 500;
	int nMaximumThresholdCanny = 200;
	if (pData != NULL) {
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
		int cx, cy, radius;
		circle = (float*)cvGetSeqElem(Circles, 0);

        m_DetectResult.pt.x = circle[0];// + pData->bounds().left();
        m_DetectResult.pt.y = circle[1];// + pData->bounds().top();
		m_DetectResult.dRadius = circle[2];
		m_DetectResult.dAngle = 0;
        //m_DetectResult.nCh = pData;
		pData->m_vecDetectResult.push_back(m_DetectResult);
		ret = 1;
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
	QString str;
	IplImage* grayImg = cvCloneImage(croppedImage);
	int filterSize = 3;
	IplConvKernel *element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, NULL);

	int ret = 0;
	CvMemStorage* storage = NULL;
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
	cvMorphologyEx(grayImg, grayImg, NULL, element, CV_MOP_CLOSE, nNoiseout);

	cvNot(grayImg, grayImg);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 202);
		SaveOutImage(grayImg, pData, str);
	}


	//int nInvert = CV_THRESH_BINARY | CV_THRESH_OTSU;
	//double otsuThreshold = cvThreshold(grayImg, grayImg, 0, 255, nInvert);
	cvInRangeS(grayImg, Scalar(120), Scalar(255), grayImg); // 함체 및 AMP

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 203);
		SaveOutImage(grayImg, pData, str);
	}
	// Fill Blob
	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.

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
	cvMorphologyEx(grayImg, grayImg, NULL, element, CV_MOP_OPEN, nNoiseout);
	nNoiseout = 20; // 백색강조
	cvMorphologyEx(grayImg, grayImg, NULL, element, CV_MOP_CLOSE, nNoiseout);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 205);
		SaveOutImage(grayImg, pData, str);
	}

	/////////////////////////
	// 조건에 맞는 Blob만 남김
	////////////////////////

	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.
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

			CvSize2D32f f = currentBlob->GetEllipse().size;
			f.width;
			f.height;

			currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
		}
	}


	//cvDilate(grayImg, grayImg, NULL, 20);

	Smooth(pData, grayImg, 210);

	int nMinCircleRadius = 30;
	int nMaxCircleRadius = 500;
	int nMaximumThresholdCanny = 200;
	double dp = 1.2; // Accumulator resolution
	double min_dist = 100; // Minimum distance between the centers of the detected circles.
	double vote = 20;
	if (pData != NULL) {
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
		int cx, cy, radius;
		circle = (float*)cvGetSeqElem(Circles, 0);

        m_DetectResult.pt.x = circle[0];// + pData->bounds().left();
        m_DetectResult.pt.y = circle[1];// + pData->bounds().top();
		m_DetectResult.dRadius = circle[2];
		m_DetectResult.dAngle = 0;
        //m_DetectResult.nCh = pData;
		//pData->m_vecDetectResult.push_back(m_DetectResult);
		ret = 1;
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
    if (pData->mPattern->iplTemplate == NULL)
        return -1;
    QString str;

    CvSize searchSize = cvSize(grayImage->width, grayImage->height);
    IplImage* graySearchImg = cvCreateImage(searchSize, IPL_DEPTH_8U, 1);
    if (grayImage->nChannels == 3)
        cvCvtColor(grayImage, graySearchImg, CV_RGB2GRAY);
    else
        cvCopy(grayImage, graySearchImg);

    static CvPoint2D32f cog = { 0, 0 };

    CvSize templateSize = cvSize(pData->mPattern->iplTemplate->width, pData->mPattern->iplTemplate->height);
    IplImage* grayTemplateImg = cvCreateImage(templateSize, IPL_DEPTH_8U, 1);
    if (pData->mPattern->iplTemplate->nChannels == 3)
        cvCvtColor(pData->mPattern->iplTemplate, grayTemplateImg, CV_RGB2GRAY);
    else
        cvCopy(pData->mPattern->iplTemplate, grayTemplateImg);


    double dMatchShapes = 0;
    double MatchRate = 0, LimitMatchRate = 40;

    if (pData != NULL)
    {
        CParam *pParam = pData->getParam(("Pattern matching rate"));
        if (pParam) LimitMatchRate = pParam->Value.toDouble();
    }

    CvPoint left_top = { 0, 0 };

    CvSize size = cvSize(pData->bounds().width() - grayTemplateImg->width + 1, pData->bounds().height() - grayTemplateImg->height + 1);
    IplImage* C = cvCreateImage(size, IPL_DEPTH_32F, 1); // 상관계수를 구할 이미지(C)
    double min, max;
    double rate = LimitMatchRate / 100.0;
    std::vector<std::pair<double, double>> pairs;
    IplImage *clone = cvCloneImage(grayTemplateImg);
    for (double a = -30.0; a < 30.0; a=a+1.0) // 패턴을 -30 에서 30도까지 돌려가면서 매칭율이 가장좋은 이미지를 찾는다.
    {
        cvCopy(grayTemplateImg, clone);
        RotateImage(clone, a);
        cvMatchTemplate(graySearchImg, clone, C, CV_TM_CCOEFF_NORMED); // 제곱차 매칭
        cvMinMaxLoc(C, &min, &max, NULL, &left_top); // 상관계수가 최대값을 값는 위치 찾기
        if (max > rate) {
            pairs.push_back(std::pair<double, double>(a, max));
        }
    }
    cvReleaseImage(&C);

    std::stable_sort(pairs.begin(), pairs.end(), [=](const std::pair<double, double>& a, const std::pair<double, double>& b)
    {
        return a.second > b.second; // descending
    });

    std::pair<double, double> a = pairs[0];
    cvCopy(grayTemplateImg, clone);
    RotateImage(clone, a.first);
    cvCopy(clone, grayTemplateImg);
    qDebug() << "big match : " << a.first << a.second;

    cvReleaseImage(&clone);

    // Opencv Template Matching을 이용해서 Template 을 찾는다.
    // 이미지 Template Matching이기때문에 부정확한것은 cvMatchShapes()로 보완한다.
    QString strMsg;
    MatchRate = TemplateMatch(pData, graySearchImg, grayTemplateImg, left_top, dMatchShapes);
    if (LimitMatchRate <= MatchRate)
    {
        //strMsg.Format(("TemplateMatch Result Success ===> : %.2f%%"), MatchRate);
        //MSystem.DevLogSave(("%s"), strMsg);

        if (m_DetectResult.nResult == 1)
        {
            m_DetectResult.pt.x = 0;
            m_DetectResult.pt.y = 0;
            m_DetectResult.rect.setLeft(pData->bounds().x());// + left_top.x);
            m_DetectResult.rect.setTop(pData->bounds().y());// + left_top.y);
            m_DetectResult.rect.setRight(m_DetectResult.rect.left() + pData->mPattern->iplTemplate->width);
            m_DetectResult.rect.setBottom(m_DetectResult.rect.top() + pData->mPattern->iplTemplate->height);
            m_DetectResult.dRadius = 0;
            m_DetectResult.dAngle = 0;
            pData->m_vecDetectResult.push_back(m_DetectResult);
        }
    }
    else {
        left_top = { 0, 0 };
    }

    str.sprintf(("SinglePattIdentify MatchRate:%.1f MatchShapes:%.1f (%s)"),
                MatchRate, dMatchShapes, m_DetectResult.nResult == 1 ? ("OK") : ("NG"));
    qDebug() << str;
    //MSystem.m_pFormBottom->SetBottomMessage(str);

    return 0;
}

float CImgProcEngine::verifyCircle(cv::Mat dt, cv::Point2f center, float radius, std::vector<cv::Point2f> & inlierSet)
{
	unsigned int counter = 0;
	unsigned int inlier = 0;
	float minInlierDist = 2.0f;
	float maxInlierDistMax = 100.0f;
	float maxInlierDist = radius / 25.0f;
	if (maxInlierDist<minInlierDist) maxInlierDist = minInlierDist;
	if (maxInlierDist>maxInlierDistMax) maxInlierDist = maxInlierDistMax;

	// choose samples along the circle and count inlier percentage
	for (float t = 0; t<2 * 3.14159265359f; t += 0.05f)
	{
		counter++;
		float cX = radius*cos(t) + center.x;
		float cY = radius*sin(t) + center.y;

		if (cX < dt.cols)
			if (cX >= 0)
				if (cY < dt.rows)
					if (cY >= 0)
						if (dt.at<float>(cY, cX) < maxInlierDist)
						{
							inlier++;
							inlierSet.push_back(cv::Point2f(cX, cY));
						}
	}

	return (float)inlier / float(counter);
}

inline void CImgProcEngine::getCircle(cv::Point2f& p1, cv::Point2f& p2, cv::Point2f& p3, cv::Point2f& center, float& radius)
{
	float x1 = p1.x;
	float x2 = p2.x;
	float x3 = p3.x;

	float y1 = p1.y;
	float y2 = p2.y;
	float y3 = p3.y;

	// PLEASE CHECK FOR TYPOS IN THE FORMULA :)
	center.x = (x1*x1 + y1*y1)*(y2 - y3) + (x2*x2 + y2*y2)*(y3 - y1) + (x3*x3 + y3*y3)*(y1 - y2);
	center.x /= (2 * (x1*(y2 - y3) - y1*(x2 - x3) + x2*y3 - x3*y2));

	center.y = (x1*x1 + y1*y1)*(x3 - x2) + (x2*x2 + y2*y2)*(x1 - x3) + (x3*x3 + y3*y3)*(x2 - x1);
	center.y /= (2 * (x1*(y2 - y3) - y1*(x2 - x3) + x2*y3 - x3*y2));

	radius = sqrt((center.x - x1)*(center.x - x1) + (center.y - y1)*(center.y - y1));
}

std::vector<cv::Point2f> CImgProcEngine::getPointPositions(cv::Mat binaryImage)
{
	std::vector<cv::Point2f> pointPositions;

	for (unsigned int y = 0; y<binaryImage.rows; ++y)
	{
		//unsigned char* rowPtr = binaryImage.ptr<unsigned char>(y);
		for (unsigned int x = 0; x<binaryImage.cols; ++x)
		{
			//if(rowPtr[x] > 0) pointPositions.push_back(cv::Point2i(x,y));
			if (binaryImage.at<unsigned char>(y, x) > 0) pointPositions.push_back(cv::Point2f(x, y));
		}
	}

	return pointPositions;
}

int CImgProcEngine::Find_RANSAC_Circle(IplImage* grayImgIn, RoiObject *pData, int retry, int nMax)
{
	QString str;
	CParam *pParam;
	float bestCirclePercentage = 0.8f;
	pParam = pData->getParam(("Roundness accuracy rate"));
	if (pParam)
        bestCirclePercentage = pParam->Value.toDouble() / 100.0;
	float nMinCircleRadius = 50;// nLow;
	float nMaxCircleRadius = 100;// nHigh;
	pParam = pData->getParam(("Minimum circle radius"));
	if (pParam)
        nMinCircleRadius = pParam->Value.toDouble();
	str.sprintf(("Maximum circle radius%d"), retry+1);
	pParam = pData->getParam(str);
	if (pParam)
        nMaxCircleRadius = pParam->Value.toDouble();

	//cv::Point2f bestCircleCenter;
	//float bestCircleRadius;
	//float bestCirclePercentage = minCirclePercentage;
	//float minRadius = 10.0;   // TODO: ADJUST THIS PARAMETER TO YOUR NEEDS, otherwise smaller circles wont be detected or "small noise circles" will have a high percentage of completion

	IplImage* grayImg = cvCreateImage(cvGetSize(grayImgIn), IPL_DEPTH_8U, 1);
	CBlobResult blobs = CBlobResult(grayImgIn, NULL, 0);	// Use a black background color.
	int nBlobs = blobs.GetNumBlobs();
	for (int i = 0; i < nBlobs; i++)	// 여러개의 Blob이 있을때 한개씩 뽑아서 RANSAC을 돌린다.
	{
		CBlob *p = blobs.GetBlob(i);
		cvZero(grayImg);
		p->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.

		//RANSAC
		cv::Mat canny;
		cv::Mat gray = cv::cvarrToMat(grayImg);

		cv::Mat mask;

		float canny1 = 100;
		float canny2 = 20;

		cv::Canny(gray, canny, canny1, canny2);
		//cv::imshow("canny",canny);

		mask = canny;

		std::vector<cv::Point2f> edgePositions;
		edgePositions = getPointPositions(mask);

		// create distance transform to efficiently evaluate distance to nearest edge
		cv::Mat dt;
		cv::distanceTransform(255 - mask, dt, CV_DIST_L1, 3);

		//TODO: maybe seed random variable for real random numbers.

		//unsigned int nIterations = 0;

		//float minCirclePercentage = 0.2f;
		//float minCirclePercentage = 0.05f;  // at least 5% of a circle must be present? maybe more...

		int maxNrOfIterations = edgePositions.size();   // TODO: adjust this parameter or include some real ransac criteria with inlier/outlier percentages to decide when to stop

		RANSACIRCLE tmprclc;
		tmprclc.cPerc = 0.0;
		tmprclc.radius = 0.0;
		for (unsigned int its = 0; its < maxNrOfIterations; ++its)
		{
			//RANSAC: randomly choose 3 point and create a circle:
			//TODO: choose randomly but more intelligent,
			//so that it is more likely to choose three points of a circle.
			//For example if there are many small circles, it is unlikely to randomly choose 3 points of the same circle.
			unsigned int idx1 = rand() % edgePositions.size();
			unsigned int idx2 = rand() % edgePositions.size();
			unsigned int idx3 = rand() % edgePositions.size();

			// we need 3 different samples:
			if (idx1 == idx2) continue;
			if (idx1 == idx3) continue;
			if (idx3 == idx2) continue;

			// create circle from 3 points:
			cv::Point2f center; float radius;
			getCircle(edgePositions[idx1], edgePositions[idx2], edgePositions[idx3], center, radius);

			// inlier set unused at the moment but could be used to approximate a (more robust) circle from alle inlier
			std::vector<cv::Point2f> inlierSet;

			//verify or falsify the circle by inlier counting:
			float cPerc = verifyCircle(dt, center, radius, inlierSet);

			// update best circle information if necessary
			if (radius >= nMinCircleRadius && radius <= nMaxCircleRadius)
			{
				RANSACIRCLE rclc;
				rclc.center = center;
				rclc.radius = radius;
				rclc.cPerc = cPerc;
				if (cPerc >= bestCirclePercentage)
				{
					int cx = grayImg->width;
					int cy = grayImg->height;
					uchar* imageData = (uchar*)grayImg->imageData;
					int imgStep = grayImg->widthStep;
					if (imageData[imgStep * (int)center.y + (int)center.x] == 255)
					{
						//bestCirclePercentage = cPerc;
						//bestCircleRadius = radius;
						//bestCircleCenter = center;
						//cv::circle(color, center, radius, cv::Scalar(255, 255, 0), 1);
						//qDebug() << "cPerc: " << cPerc << "radius: " << radius;
						str.sprintf(("circle(retry:%d): cPerc:%.2f center : %.1f,%.1f  radius : %.1f"), retry, cPerc, center.x, center.y, radius);
                        ////g_cLog->AddLog(_LOG_LIST_SYS, str);
						//TRACE(("%s\n"), str);

						vecRansicCircle.push_back(rclc);
						if (vecRansicCircle.size() > nMax)
							break;
					}
				}
				else {
					if (tmprclc.cPerc < rclc.cPerc)
						tmprclc = rclc;
				}
			}
		}

		//if (vecRansicCircle.size() == 0) {
		//	if (tmprclc.cPerc > 0.0) {
		//		str.sprintf(("fail circle match(retry:%d): cPerc:%.1f center : %.1f,%.1f radius : %.1f"), retry, tmprclc.cPerc, tmprclc.center.x, tmprclc.center.y, tmprclc.radius);
        //		//g_cLog->AddLog(_LOG_LIST_SYS, str);
		//		//TRACE(("%s\n"), str);
		//	}
		//}
	}
	cvReleaseImage(&grayImg);

	return 0;
}


int CImgProcEngine::FilterBlobLength(IplImage* grayImg, RoiObject *pData, int nMinLength, int nMaxLength)
{
	IplImage *pMask = cvCloneImage(grayImg);
	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.

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

int CImgProcEngine::FilterBlobBoundingBoxLength(IplImage* grayImg, RoiObject *pData, int nMinLength, int nMaxLength)
{
	IplImage *pMask = cvCloneImage(grayImg);
	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.
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
// VCM본딩 전용으로 라인이 두개 이상일때 가장 아래라인으로 사용한다.
//
int CImgProcEngine::FilterBlobBoundingBoxXLength(IplImage* grayImg, RoiObject *pData, int nMinLength, int nMaxLength)
{
	int index = 0;
	int y = 0;

	IplImage *pMask = cvCloneImage(grayImg);
	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.
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
			else {
				if (rect.y > y) {
					y = rect.y;
					index = i;
				}
			}
		}
	}

	cvZero(grayImg);
	if (blobCount > 0) {
		for (int i = 0; i < blobCount; i++)
		{
			if (i == index)
			{
				CBlob *currentBlob = blobs.GetBlob(i);
				currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
			}
		}
	}
	cvAnd(grayImg, pMask, grayImg);
	cvReleaseImage(&pMask);

	return 0;
}

int CImgProcEngine::FilterIncludeLargeBlob(IplImage* grayImg, RoiObject *pData)
{
	IplImage *pMask = cvCloneImage(grayImg);
	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.
	double dLargeArea = 0;
	// 가장큰 Blob만 남기고 제거
	int n = blobs.GetNumBlobs();
	if (n > 1)
	{
		double_stl_vector area = blobs.GetSTLResult(CBlobGetArea());
		std::sort(area.begin(), area.end(), sort_using_greater_than);
		dLargeArea = area[0];
		blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, dLargeArea);
	}

	cvZero(grayImg);
	for (int i = 0; i < blobs.GetNumBlobs(); i++) {
		CBlob *currentBlob = blobs.GetBlob(i);
		currentBlob->FillBlob(grayImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
	}
	cvAnd(grayImg, pMask, grayImg);
    //if (m_bSaveEngineImg)
	//{
	//	str.sprintf(("%d_FilterIncludeLargeBlob.jpg"), 115);
    //	if (pData) SaveOutImage(pMaskImage, pData, str, false);
	//}
	cvReleaseImage(&pMask);
	return 0;
}

int CImgProcEngine::FilterExcludeLargeBlob(IplImage* grayImg, RoiObject *pData)
{
	IplImage *pMask = cvCloneImage(grayImg);
	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.
	double dLargeArea = 0;
	// 가장큰 Blob만 남기고 제거
	int n = blobs.GetNumBlobs();
	if (n > 1)
	{
		double_stl_vector area = blobs.GetSTLResult(CBlobGetArea());
		std::sort(area.begin(), area.end(), sort_using_greater_than);
		dLargeArea = area[0];
		blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_EQUAL, dLargeArea);
	}

	cvZero(grayImg);
	for (int i = 0; i < blobs.GetNumBlobs(); i++) {
		CBlob *currentBlob = blobs.GetBlob(i);
		currentBlob->FillBlob(grayImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
	}
	cvAnd(grayImg, pMask, grayImg);
    //if (m_bSaveEngineImg)
	//{
	//	str.sprintf(("%d_FilterIncludeLargeBlob.jpg"), 115);
    //	if (pData) SaveOutImage(pMaskImage, pData, str, false);
	//}
	cvReleaseImage(&pMask);
	return 0;
}

int CImgProcEngine::BondingHoleRANSAC(IplImage* grayImg, RoiObject *pData, QRectF rect, int retry)
{
	QString str;
	RANSACIRCLE *pCircle;
	vecRansicCircle.clear();
	CParam *pParam;

	int nMinCircleRadius = 30;// nLow;
	int nMaxCircleRadius = 200;// nHigh;
	pParam = pData->getParam(("Minimum circle radius"));
	if (pParam)
        nMinCircleRadius = (int)pParam->Value.toDouble();
	str.sprintf(("Maximum circle radius%d"), retry + 1);
	pParam = pData->getParam(str);
	if (pParam)
        nMaxCircleRadius = (int)pParam->Value.toDouble();


	IplImage* grayImg1 = cvCloneImage(grayImg);

	if (Find_RANSAC_Circle(grayImg, pData, retry, 3) == 0)	// RANSAC Max Count : 3
	{
		int size = vecRansicCircle.size();
		for (int i = size - 1; i >= 0; i--) {
			RANSACIRCLE *p = &vecRansicCircle[i];
			if (p->radius > nMaxCircleRadius || p->radius < nMinCircleRadius) {
				vecRansicCircle.erase(vecRansicCircle.begin() + i);
			}
		}

		size = vecRansicCircle.size();
		if (size > 1)
		{
			size = vecRansicCircle.size();
			vector<RANSACIRCLE> vecResultCircle;
			//TRACE(("RANSAC_Circle: size = %d\n"), size);
			if (size > 1) {
				std::stable_sort(vecRansicCircle.begin(), vecRansicCircle.end(), [](const RANSACIRCLE lhs, const RANSACIRCLE rhs)->bool {
					if (sqrt(lhs.center.x*lhs.center.x + lhs.center.y*lhs.center.y) < sqrt(rhs.center.x*rhs.center.x + rhs.center.y*rhs.center.y)) // assending
						return true;
					return false;
				});

				// 소팅을 한후 근접한 Circle(4 point 이내의 점들)은 한개로 그룹화 한다.
				RANSACIRCLE *pCircleBefore = &vecRansicCircle[0];
				RANSACIRCLE Circle = *pCircleBefore;
				for (int i = 1; i < size; i++)
				{
					pCircle = &vecRansicCircle[i]; // 다음 item
					double lpos = sqrt(pCircleBefore->center.x*pCircleBefore->center.x + pCircleBefore->center.y*pCircleBefore->center.y);
					double rpos = sqrt(pCircle->center.x*pCircle->center.x + pCircle->center.y*pCircle->center.y);
					if (fabs(lpos - rpos) <= 10.0)  // 4.0 -> 10.0
					{
						if (Circle.cPerc < pCircle->cPerc)
						{
							//Circle.center.x = (Circle.center.x + pCircle->center.x) / 2.0;
							//Circle.center.y = (Circle.center.y + pCircle->center.y) / 2.0;
							//Circle.radius = (Circle.radius + pCircle->radius) / 2;
							Circle.center.x = pCircle->center.x; // 원형도가 가장 좋은것을 취한다.
							Circle.center.y = pCircle->center.y;
							Circle.radius = pCircle->radius;
							Circle.cPerc = pCircle->cPerc;
						}
						pCircleBefore = &vecRansicCircle[i];

						continue;
					}
					vecResultCircle.push_back(Circle);
					pCircleBefore = &vecRansicCircle[i];
					Circle = *pCircleBefore;
				}


				if (Circle.cPerc < pCircle->cPerc)
				{
					//Circle.center.x = (Circle.center.x + pCircle->center.x) / 2.0;
					//Circle.center.y = (Circle.center.y + pCircle->center.y) / 2.0;
					//Circle.radius = (Circle.radius + pCircle->radius) / 2;
					Circle.center.x = pCircle->center.x; // 원형도가 가장 좋은것을 취한다.
					Circle.center.y = pCircle->center.y;
					Circle.radius = pCircle->radius;
					Circle.cPerc = pCircle->cPerc;
				}
				vecResultCircle.push_back(Circle);
			}
			else {
				vecResultCircle.push_back(vecRansicCircle[0]);
			}

			// 그룹화 하고 나서도 2개 이상의 서클이 보이면..
			size = vecResultCircle.size();
			//TRACE(("Find_Result_RANSAC_Circle: size = %d\n"), size);
			if (size > 1)
			{
				vector<RANSACIRCLE> vecCircle;

				for (int i = 0; i < size; i++)
				{
					pCircle = &vecResultCircle[i];
					if (pCircle->radius > 0)
					{
						int r = (int)pCircle->radius;
						int x = (int)pCircle->center.x - r;
						int y = (int)pCircle->center.y - r;
						if (x < 0) x = 0;
						if (y < 0) y = 0;

						int w = r * 2;
						int h = r * 2;
						if (grayImg1->width < (x + w))
							w -= ((x + w) - grayImg1->width);
						if (grayImg1->height < (y + h))
							h -= ((y + h) - grayImg1->height);

						cvSetImageROI(grayImg1, cvRect(x, y, w, h));
						IplImage *image = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
						cvCopy(grayImg1, image);
						cvResetImageROI(grayImg1);

						cvInRangeS(image, Scalar(230), Scalar(255), image);

                        if (m_bSaveEngineImg)
						{
							QString str; str.sprintf(("%d_Filter_%d.jpg"), 400, i);
							SaveOutImage(image, pData, str);
						}

						CBlobResult blobs;
						blobs = CBlobResult(image, NULL, 0);	// Use a black background color.
						int blobCount = blobs.GetNumBlobs();
						double dLargeArea = 0;
						for (int j = 0; j < blobCount; j++) {
							CBlob *p = blobs.GetBlob(j);
							double dArea = p->Area();
							if (dLargeArea < dArea) {
								dLargeArea = dArea;
							}
						}
						cvReleaseImage(&image);

						double dChkPerc = 0.5;
						if (pCircle->cPerc < 0.5)
							dChkPerc = pCircle->cPerc;
						// 원영역의 흰색영역이 50% 이상인지 check
						if (dLargeArea < (double)(w*h)*dChkPerc) {
							continue;
						}

						vecCircle.push_back(*pCircle);
					}
				}

				if (vecCircle.size() > 0)
				{
					std::stable_sort(vecCircle.begin(), vecCircle.end(), [](const RANSACIRCLE lhs, const RANSACIRCLE rhs)->bool {
						if (lhs.cPerc > rhs.cPerc) // descending
							return true;
						return false;
					});

                    m_DetectResult.pt.x = vecCircle[0].center.x;// + pData->bounds().left();
                    m_DetectResult.pt.y = vecCircle[0].center.y;// + pData->bounds().top();
					m_DetectResult.dRadius = vecCircle[0].radius;
					m_DetectResult.dAngle = vecCircle[0].cPerc; // dAngle에 원형도 값을 넣어 사용
                    //m_DetectResult.nCh = pData;
					pData->m_vecDetectResult.push_back(m_DetectResult);
					str.sprintf(("====> RANSAC Result Circle found1. : %.1f (%.1f,%.1f)"), vecCircle[0].radius, vecCircle[0].center.x, vecCircle[0].center.y);
                    //g_cLog->AddLog(_LOG_LIST_SYS, str);

				}

			} // if (vecResultCircle.size() > 1) 
			else if (size == 1)
			{
				pCircle = &vecResultCircle[0];
                m_DetectResult.pt.x = pCircle->center.x;// + pData->bounds().left();
                m_DetectResult.pt.y = pCircle->center.y;// + pData->bounds().top();
				m_DetectResult.dRadius = pCircle->radius;
				m_DetectResult.dAngle = pCircle->cPerc; // dAngle에 원형도 값을 넣어 사용
                //m_DetectResult.nCh = pData;
				pData->m_vecDetectResult.push_back(m_DetectResult);
				//str.sprintf(("====> RANSAC Result Circle found2. : %.1f (%.1f,%.1f)\n"), pCircle->radius, pCircle->center.x, pCircle->center.y);
                ////g_cLog->AddLog(_LOG_LIST_SYS, str);

			}
			size = vecResultCircle.size();
			////TRACE(("Find_Result_RANSAC_Circle: last size = %d\n"), size); 
		} // if (vecRansicCircle.size() > 1) 
		else if (size == 1)
		{
			pCircle = &vecRansicCircle[0];
            m_DetectResult.pt.x = pCircle->center.x;// + pData->bounds().left();
            m_DetectResult.pt.y = pCircle->center.y;// + pData->bounds().top();
			m_DetectResult.dRadius = pCircle->radius;
			m_DetectResult.dAngle = pCircle->cPerc; // dAngle에 원형도 값을 넣어 사용
            //m_DetectResult.nCh = pData;
			pData->m_vecDetectResult.push_back(m_DetectResult);
			//str.sprintf(("====> RANSAC Result Circle found3. : %.1f (%.1f,%.1f)\n"), pCircle->radius, pCircle->center.x, pCircle->center.y);
            ////g_cLog->AddLog(_LOG_LIST_SYS, str);


		}

		if (m_DetectResult.dRadius > 0.0) {
			//str.sprintf(("circle: cPerc:%.1f center : %.1f,%.1f  radius : %.1f"), m_DetectResult.dAngle, m_DetectResult.pt.x, m_DetectResult.pt.y, m_DetectResult.dRadius);
            ////g_cLog->AddLog(_LOG_LIST_SYS, str);
		}
	}


	if (grayImg1) cvReleaseImage(&grayImg1);
	return 0;
}

int CImgProcEngine::BondingHoleGravity(IplImage* grayImg, RoiObject *pData, QRectF rect, int retry)
{
	QString str;
	IplImage* drawImage = NULL;
	CvPoint2D32f cog;
	CParam *pParam;
    if (m_bSaveEngineImg) {
		drawImage = cvCreateImage(cvSize(grayImg->width, grayImg->height), grayImg->depth, 3);
	}

	CBlobResult blobs;
	int nMinCircleRadius = 30;// nLow;
	int nMaxCircleRadius = 200;// nHigh;
	pParam = pData->getParam(("Minimum circle radius"));
	if (pParam)
        nMinCircleRadius = (int)pParam->Value.toDouble();
	str.sprintf(("Maximum circle radius%d"), retry + 1);
	pParam = pData->getParam(str);
	if (pParam)
        nMaxCircleRadius = (int)pParam->Value.toDouble();

#if 1
	//CenterOfGravity(pData, grayImg, cog, retry+1);

	//CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.

	/////////////////////////
	// 반지름조건에 부합하는 blob만 남김
	////////////////////////
a1:
	int nBlobs = blobs.GetNumBlobs();
	for (int i = 0; i < nBlobs; i++) {
		CBlob *p = blobs.GetBlob(i);
		CvRect rect = p->GetBoundingBox();
		int _max = MAX(rect.width / 2, rect.height / 2);
		int _min = MIN(rect.width / 2, rect.height / 2);
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

	// blob이 두개 이상 남아 있으면 가운데 blob을 선택한다.
	nBlobs = blobs.GetNumBlobs();
	if (nBlobs >= 2) {
		int cw = grayImg->width/2;
		int ch = grayImg->height/2;
		double ml = 9999;
		int idx = -1;
		for (int i = 0; i < nBlobs; i++) {
			CBlob *p = blobs.GetBlob(i);
			CvRect rect = p->GetBoundingBox();
			double cx = cw - rect.width / 2;
			double cy = ch - rect.height / 2;
			double l = sqrt(cx*cx + cy*cy);
			if (ml > l) {
				ml = l;
				idx = i;
			}
		}
		if (idx >= 0) {
			for (int i = 0; i < nBlobs; i++) {
				CBlob *p = blobs.GetBlob(i);
				if (idx != i)
					p->ClearContours();
			}
		}
	}

	// filter blobs.
	cvZero(grayImg);
	int blobCount = blobs.GetNumBlobs();
	for (int i = 0; i < blobCount; i++)
	{
		CBlob *currentBlob = blobs.GetBlob(i);
		currentBlob->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.
	}
#endif

	{
		CvMemStorage* m_storage = cvCreateMemStorage(0); // 배열 자료(점의 좌표가 들어간다)
		CvSeq* m_seq = 0;         // 경계 계수를 저장할 변수
		cvFindContours(grayImg, m_storage, &m_seq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);


		cvZero(grayImg);
		//CvBox2D rect2;
		CvScalar color = CV_RGB(255, 255, 255);
		if (m_seq != NULL)
		{
			int nSeq = 0;
			for (CvSeq* c = m_seq; c != NULL; c = c->h_next)  // 엣지의 링크드리스트를 순회하면서 각 엣지들에대해서 출력한다.
			{
				RotatedRect rect2 = cvMinAreaRect2(c, 0);

				//int m1 = MIN(rect2.size.height, rect2.size.width);
				int _max = MAX(rect2.size.height/2, rect2.size.width/2);
				int _min = MIN(rect2.size.height/2, rect2.size.width/2);
				if (_min >= nMinCircleRadius && _max <= nMaxCircleRadius) {
					cvDrawContours(grayImg, c, color, color, -1, CV_FILLED, 8);
					str.sprintf(("RotareRect width=%.2f,height=%.2f\n"), rect2.size.width, rect2.size.height);
                    //g_cLog->AddLog(_LOG_LIST_SYS, str);
				}
			}
		}
		if (m_seq) cvClearSeq(m_seq);
		if (m_storage) cvReleaseMemStorage(&m_storage);
	}


    if (m_bSaveEngineImg)
	{
		str.sprintf(("313_cvApproxInImage.jpg"));
		SaveOutImage(grayImg, pData, str);
	}

	// 외곽선 추적 및 근사화 변수 초기화
	CvSeq* m_approxDP_seq = 0;
	CvSeq* m_dominant_points = 0;    // 특징점 찾기 위한 변수
	CvSeq* ptseq;
	CvMemStorage* m_storage = cvCreateMemStorage(0); // 배열 자료(점의 좌표가 들어간다)
	CvSeq* m_seq = 0;         // 경계 계수를 저장할 변수
	//CvSeq* defect = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);
	cvFindContours(grayImg, m_storage, &m_seq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

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

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// (2) 외곽선 근사화
	////////////////////////////////////////////////////////////////////////////////////////////////////
	int testcount = -1;
	int iContoursSize = 0;
	int MinMomentSize = 4, MaxMomentSize = 50;

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
		m_approxDP_seq = cvApproxPoly(m_seq, sizeof(CvContour), m_storage, CV_POLY_APPROX_DP, 3, 1);
		if (m_approxDP_seq != NULL)
		{
            if (m_bSaveEngineImg)
			{
				cvDrawContours(drawImage, m_approxDP_seq, CVX_RED, CVX_RED, 1, 1, 8); // 외곽선 근사화가 잘되었는지 테스트
				str.sprintf(("322_cvApproxPoly.jpg"));
				SaveOutImage(drawImage, pData, str);
			}

			int nSeq = 0;
			for (CvSeq* c = m_approxDP_seq; c != NULL; c = c->h_next)  // 엣지의 링크드리스트를 순회하면서 각 엣지들에대해서 출력한다.
			{
				float radius;
				CvPoint2D32f center;
				cvMinEnclosingCircle(c, &center, &radius);

                ptseq = cvCreateSeq((CV_SEQ_KIND_CURVE | CV_SEQ_ELTYPE_POINT | CV_SEQ_FLAG_CLOSED) | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);
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

                    //str.sprintf(("CH:%d Radius = %.3f)"), pData, radius);
                    ////g_cLog->AddLog(_LOG_LIST_SYS, str);

					cog.x = centerPoint.x;
					cog.y = centerPoint.y;

					m_DetectResult.dAngle = 0;

                    m_DetectResult.pt.x = centerPoint.x;// + pData->bounds().left();
                    m_DetectResult.pt.y = centerPoint.y;// + pData->bounds().top();

					pData->m_vecDetectResult.push_back(m_DetectResult);

					iContoursSize++;
				}
				cvClearSeq(ptseq);
				if (iContoursSize > 0) // 1개만 찾자
					break;
			}

			if (m_approxDP_seq) cvClearSeq(m_approxDP_seq);
		}
	}

	if (m_seq) cvClearSeq(m_seq);
	//if (defect) cvClearSeq(defect);
	if (m_storage) cvReleaseMemStorage(&m_storage);

	if (drawImage) cvReleaseImage(&drawImage);
	return 0;
}

int CImgProcEngine::SingleROIBondingHole(IplImage* croppedImage, RoiObject *pData, QRectF rect)
{
	QString str;
    int retry = 0;
	CParam *pParam;
	IplConvKernel *element;
	int filterSize = 3;

	IplImage* mask = cvCloneImage(croppedImage);

	int nMaskLow = 230;
	pParam = pData->getParam(("Mask Low"));
	if (pParam)
        nMaskLow = pParam->Value.toDouble();
	int nMaskHigh = 255;
	str.sprintf(("Mask High"));
	pParam = pData->getParam(str);
	if (pParam)
        nMaskHigh = pParam->Value.toDouble();

	if (nMaskHigh > 0)
	{
		element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, NULL);
		int nNoiseout = 0;
		if (pData != NULL) {
			CParam *pParam = pData->getParam(("Mask Noise out"));
			if (pParam)
                nNoiseout = pParam->Value.toDouble();
		}

		cvInRangeS(mask, Scalar(nMaskLow), Scalar(nMaskHigh), mask);

		if (nNoiseout < 0)
			cvMorphologyEx(mask, mask, NULL, element, CV_MOP_OPEN, -nNoiseout);
		else //if (nNoiseout > 0)
			cvMorphologyEx(mask, mask, NULL, element, CV_MOP_CLOSE, nNoiseout);
		cvReleaseStructuringElement(&element);

        if (m_bSaveEngineImg)
		{
			QString str; str.sprintf(("%d_Mask.jpg"), 150);
			SaveOutImage(mask, pData, str);
		}

		if (nMaskLow == 0)
			cvSub(croppedImage, mask, croppedImage);
		else
			cvAdd(croppedImage, mask, croppedImage);

        if (m_bSaveEngineImg)
		{
			QString str; str.sprintf(("%d_MaskOut.jpg"), 151);
			SaveOutImage(croppedImage, pData, str);
		}

		if (mask) cvReleaseImage(&mask);

	}

again:
	pData->m_vecDetectResult.clear();
	IplImage* grayImg = cvCloneImage(croppedImage);

	int ret = 0;
	//CvMemStorage* storage = NULL;
	//storage = cvCreateMemStorage(0);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 200);
		SaveOutImage(grayImg, pData, str);
	}

	int nThresholdLowValue = 80;
	pParam = pData->getParam(("Low Threshold"));
	if (pParam)
        nThresholdLowValue = pParam->Value.toDouble();
	int nThresholdHighValue = 255;
	str.sprintf(("High Threshold%d"), retry + 1);
	pParam = pData->getParam(str);
	if (pParam)
        nThresholdHighValue = pParam->Value.toDouble();
    if (nThresholdHighValue == 0) {
        if (grayImg) cvReleaseImage(&grayImg);
        return 0;
    }

	cvInRangeS(grayImg, Scalar(nThresholdLowValue), Scalar(nThresholdHighValue), grayImg);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Threshold_%d.jpg"), 203, retry);
		SaveOutImage(grayImg, pData, str);
	}

	NoiseOut(pData, grayImg, 212);
	Expansion(pData, grayImg, 213);

	PostNoiseOut(pData, grayImg, 231);
	PostExpansion(pData, grayImg, 232);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Noise_%d.jpg"), 234, retry);
		SaveOutImage(grayImg, pData, str);
	}

	IplImage *pMask = cvCloneImage(grayImg);
	IplImage* img2 = cvCloneImage(grayImg);

	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.
	QString s;
	double dLargeArea = 0;
	int nIndexLargeBlob = -1;
	QString s1;
	int blobCount = blobs.GetNumBlobs();
	for (int i = 0; i < blobCount; i++) // 제일큰 Blob 한개만 남긴다
	{
		CBlob *currentBlob = blobs.GetBlob(i);
		double dArea = currentBlob->Area();
		if (dLargeArea < dArea) {
			dLargeArea = dArea;
			nIndexLargeBlob = i;
		}
	}
	if (blobCount > 0 && nIndexLargeBlob >= 0) {
		cvZero(img2);
		CBlob *currentBlob = blobs.GetBlob(nIndexLargeBlob);
		currentBlob->FillBlob(img2, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
	}

	cvZero(grayImg);
	for (int i = 0; i < blobCount; i++) // img2를 제외한 blob
	{
		if (nIndexLargeBlob == i)
			continue;
		CBlob *currentBlob = blobs.GetBlob(i);
		currentBlob->FillBlob(grayImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
	}
    //if (m_bSaveEngineImg)
	//{
	//	QString str; str.sprintf(("%d_LargeBlob.jpg"), 240);
	//	SaveOutImage(grayImg, pData, str);
	//} 

	// 좌우 Trim
	int cx = img2->width;
	int cy = img2->height;
	uchar* imageData = (uchar*)img2->imageData;
	int imgStep = img2->widthStep;
	int tcy = (cy * 0.3) * 255;
	for (int i = 0; i < cx; i++) {	// 70% 이상 흰색이면 검정색으로채움
		unsigned long sum = 0;
		for (int j = 0; j < cy; j++)
			sum += (255 - imageData[imgStep * j + i]);
		if (sum < tcy) {
			for (int j = 0; j < cy; j++)
				imageData[imgStep * j + i] = 0;
		}
		else break;
	}
	for (int i = cx-1; i >= 0; i--) {
		unsigned long sum = 0;
		for (int j = 0; j < cy; j++)
			sum += (255 - imageData[imgStep * j + i]);
		if (sum < tcy) {
			for (int j = 0; j < cy; j++)
				imageData[imgStep * j + i] = 0;
		}
		else break;
	}

	// 상하 Trim
	int tcx = (cx * 0.3) * 255;
	for (int i = 0; i < cy; i++) {	// 70% 이상 흰색이면 검정색으로채움
		unsigned long sum = 0;
		for (int j = 0; j < cx; j++)
			sum += (255 - imageData[imgStep * i + j]);
		if (sum < tcx) {
			for (int j = 0; j < cx; j++)
				imageData[imgStep * i + j] = 0;
		}
		else break;
	}
	for (int i = cy - 1; i >= 0; i--) {
		unsigned long sum = 0;
		for (int j = 0; j < cx; j++)
			sum += (255 - imageData[imgStep * i + j]);
		if (sum < tcx) {
			for (int j = 0; j < cx; j++)
				imageData[imgStep * i + j] = 0;
		}
		else break;
	}

	cvOr(grayImg, img2, grayImg);
	
	cvAnd(grayImg, pMask, grayImg);
	cvReleaseImage(&pMask);


//	element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_ELLIPSE, NULL);
//	cvMorphologyEx(grayImg, grayImg, NULL, element, CV_MOP_OPEN, 5); // 백색 잡음제거
//	cvReleaseStructuringElement(&element);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Trim_%d.jpg"), 245, retry);
		SaveOutImage(grayImg, pData, str);
	}

	if (img2) cvReleaseImage(&img2);
	// Tirm 끝

	float bestCirclePercentage = 0.0;
	pParam = pData->getParam(("Roundness accuracy rate"));
	if (pParam)
        bestCirclePercentage = pParam->Value.toDouble() / 100.0;

	IplImage* grayImg1 = cvCloneImage(grayImg);

	if (bestCirclePercentage > 0.0)
	{
        BondingHoleRANSAC(grayImg, pData, rect, retry);

		if (pData->m_vecDetectResult.size() > 0)
		{
            if (m_bSaveEngineImg)
			{
				DetectResult *pRst = &pData->m_vecDetectResult[0];
				IplImage* Img = cvCloneImage(croppedImage);
                cvCircle(Img, cvPoint(pRst->pt.x,
                                      pRst->pt.y), pRst->dRadius, CV_RGB(128, 128, 128), 2);
				QString str; str.sprintf(("%d_RansacResult.jpg"), 500);
				SaveOutImage(Img, pData, str);
				if (Img) cvReleaseImage(&Img);
			}
		}
	}
	else
	{
        BondingHoleGravity(grayImg, pData, rect, retry);

	}

#if 1
	// 최종 윈의 위치 Blob White영역이 20% 이상인지 Check한다.
	if (pData->m_vecDetectResult.size() > 0) {
		DetectResult *pResult = &pData->m_vecDetectResult[0];

		double r = (pResult->dRadius);
        int x = (int)(pResult->pt.x - r);
        int y = (int)(pResult->pt.y - r);
		if (x < 0) x = 0;
		if (y < 0) y = 0;

		int w = r * 2;
		int h = r * 2;
		if (grayImg1->width < (x + w))
			w -= ((x + w) - grayImg1->width);
		if (grayImg1->height < (y + h))
			h -= ((y + h) - grayImg1->height);

		cvSetImageROI(grayImg1, cvRect(x, y, w, h));
		IplImage *image = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
		cvCopy(grayImg1, image);
		cvResetImageROI(grayImg1);

		cvInRangeS(image, Scalar(230), Scalar(255), image);

        if (m_bSaveEngineImg)
		{
			QString str; str.sprintf(("%d_Filter.jpg"), 910);
			SaveOutImage(image, pData, str);
		}

		CBlobResult blobs;
		blobs = CBlobResult(image, NULL, 0);	// Use a black background color.
		int blobCount = blobs.GetNumBlobs();
		double dLargeArea = 0;
		for (int j = 0; j < blobCount; j++) {
			CBlob *p = blobs.GetBlob(j);
			double dArea = p->Area();
			if (dLargeArea < dArea) {
				dLargeArea = dArea;
			}
		}
		cvReleaseImage(&image);

		double dSize = (w*h)*0.2;
		if (dLargeArea < dSize)
		{
			str.sprintf(("Delete detected circle because blob size %.0f < %.0f"), dLargeArea, dSize);
			pData->m_vecDetectResult.erase(pData->m_vecDetectResult.begin());
            //g_cLog->AddLog(_LOG_LIST_SYS, str);
		}
	}
#endif

	//cvReleaseMemStorage(&storage);
	if (grayImg) cvReleaseImage(&grayImg);
	if (grayImg1) cvReleaseImage(&grayImg1);

    retry++;
    if (m_DetectResult.dRadius == 0.0 && retry < 2) // retry 는 recipe상의 1,2 조건을 처리한다.
        goto again;

	if (pData->m_vecDetectResult.size() > 0) {

		DetectResult *pResult = &pData->m_vecDetectResult[0];

        //pResult->pt1.x = pResult->pt1.y = 0.0;
		double dOffsetX = 0.0;
		double dOffsetY = 0.0;
		pParam = pData->getParam(("X Offset(mm)"));
		if (pParam)
            dOffsetX = pParam->Value.toDouble();
		pParam = pData->getParam(("Y Offset(mm)"));
		if (pParam)
            dOffsetY = pParam->Value.toDouble();
        const double dResX = gCfg.m_pCamInfo[0].dResX;
        const double dResY = gCfg.m_pCamInfo[0].dResY;

		pResult->pt.x += (dOffsetX / dResX); // Offset Pixel 갯수
		pResult->pt.y += (dOffsetY / dResY); // Offset Pixel 갯수
		//str.sprintf(("circle : cPerc:%.21f center : %.1f,%.1f  radius : %.1f"), pResult->dAngle, pResult->pt.x, pResult->pt.y, pResult->dRadius);
		str.sprintf(("circle : center : %.1f,%.1f  radius : %.1f"), pResult->pt.x, pResult->pt.y, pResult->dRadius);
        //g_cLog->AddLog(_LOG_LIST_SYS, str);
	}

	return 0;
}


int CImgProcEngine::SingleROIScrew(IplImage* croppedImage, RoiObject *pData, QRectF rectIn)
{
    QString str;
//	int retry = 0;
//again:
    pData->m_vecDetectResult.clear();
    IplImage* grayImg = cvCloneImage(croppedImage);

    int ret = 0;
    //CvMemStorage* storage = NULL;
    //storage = cvCreateMemStorage(0);
    CParam *pParam;

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_Src.jpg"), 200);
        SaveOutImage(grayImg, pData, str);
    }

    //CannyEdgeSub();
    int nThresholdLowValue = 80;
    pParam = pData->getParam(("Low Threshold"));
    if (pParam)
        nThresholdLowValue = pParam->Value.toDouble();
    int nThresholdHighValue = 255;
    str = "High Threshold";
    pParam = pData->getParam(str);
    if (pParam)
        nThresholdHighValue = pParam->Value.toDouble();

    int nMinCircleRadius = 30;// nLow;
    int nMaxCircleRadius = 200;// nHigh;
    pParam = pData->getParam(("Minimum circle radius"));
    if (pParam)
        nMinCircleRadius = pParam->Value.toDouble();
    str = (("Maximum circle radius"));
    pParam = pData->getParam(str);
    if (pParam)
        nMaxCircleRadius = pParam->Value.toDouble();

    cvInRangeS(grayImg, Scalar(nThresholdLowValue), Scalar(nThresholdHighValue), grayImg);
    //if (nThresholdHighValue == 0)
    //	ThresholdOTSU(pData, grayImg, 202);
    //else
    //	ThresholdRange(pData, grayImg, 202);

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_Threshold.jpg"), 203);
        SaveOutImage(grayImg, pData, str);
    }

    NoiseOut(pData, grayImg, 212);
    Expansion(pData, grayImg, 213);

    PostNoiseOut(pData, grayImg, 231);
    PostExpansion(pData, grayImg, 232);

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_Noise.jpg"), 234);
        SaveOutImage(grayImg, pData, str);
    }

    ExcludeLargeBlob(grayImg, pData, nMaxCircleRadius*2, 235);

    IncludeRangeBlob(grayImg, pData, nMinCircleRadius, nMaxCircleRadius);
    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_SizeFilter.jpg"), 244);
        SaveOutImage(grayImg, pData, str);
    }

    RANSACIRCLE *pCircle;
    vecRansicCircle.clear();
    IplImage* grayImg1 = cvCloneImage(grayImg);

    if (Find_RANSAC_Circle(grayImg, pData, 0, 50) == 0)
    {
        int size = vecRansicCircle.size();
        for (int i = size - 1; i >= 0; i--) {
            RANSACIRCLE *p = &vecRansicCircle[i];
            if (p->radius > nMaxCircleRadius || p->radius < nMinCircleRadius) {
                vecRansicCircle.erase(vecRansicCircle.begin() + i);
            }
        }

        size = vecRansicCircle.size();
        if (size > 1)
        {
            size = vecRansicCircle.size();
            vector<RANSACIRCLE> vecResultCircle;
            //TRACE(("RANSAC_Circle: size = %d\n"), size);
            if (size > 1) {
                std::stable_sort(vecRansicCircle.begin(), vecRansicCircle.end(), [](const RANSACIRCLE lhs, const RANSACIRCLE rhs)->bool {
                    if (sqrt(lhs.center.x*lhs.center.x + lhs.center.y*lhs.center.y) < sqrt(rhs.center.x*rhs.center.x + rhs.center.y*rhs.center.y)) // assending
                        return true;
                    return false;
                });

                // 소팅을 한후 근접한 Circle(4 point 이내의 점들)은 한개로 그룹화 한다.
                RANSACIRCLE *pCircleBefore = &vecRansicCircle[0];
                RANSACIRCLE Circle = *pCircleBefore;
                for (int i = 1; i < size; i++)
                {
                    pCircle = &vecRansicCircle[i]; // 다음 item
                    double lpos = sqrt(pCircleBefore->center.x*pCircleBefore->center.x + pCircleBefore->center.y*pCircleBefore->center.y);
                    double rpos = sqrt(pCircle->center.x*pCircle->center.x + pCircle->center.y*pCircle->center.y);
                        if (fabs(lpos - rpos) <= 10.0)  // 4.0 -> 10.0
                        {
                            if (Circle.cPerc < pCircle->cPerc)
                        {
                            //Circle.center.x = (Circle.center.x + pCircle->center.x) / 2.0;
                            //Circle.center.y = (Circle.center.y + pCircle->center.y) / 2.0;
                            //Circle.radius = (Circle.radius + pCircle->radius) / 2;
                            Circle.center.x = pCircle->center.x; // 원형도가 가장 좋은것을 취한다.
                            Circle.center.y = pCircle->center.y;
                            Circle.radius = pCircle->radius;
                            Circle.cPerc = pCircle->cPerc;
                        }
                        pCircleBefore = &vecRansicCircle[i];

                        continue;
                    }
                    vecResultCircle.push_back(Circle);
                    pCircleBefore = &vecRansicCircle[i];
                    Circle = *pCircleBefore;
                }
                if (Circle.cPerc < pCircle->cPerc)
                {
                    //Circle.center.x = (Circle.center.x + pCircle->center.x) / 2.0;
                    //Circle.center.y = (Circle.center.y + pCircle->center.y) / 2.0;
                    //Circle.radius = (Circle.radius + pCircle->radius) / 2;
                    Circle.center.x = pCircle->center.x; // 원형도가 가장 좋은것을 취한다.
                    Circle.center.y = pCircle->center.y;
                    Circle.radius = pCircle->radius;
                    Circle.cPerc = pCircle->cPerc;
                }
                vecResultCircle.push_back(Circle);
            }
            else {
                vecResultCircle.push_back(vecRansicCircle[0]);
            }

            // 그룹화 하고 나서도 2개 이상의 서클이 보이면..
            size = vecResultCircle.size();
            //TRACE(("Find_Result_RANSAC_Circle: size = %d\n"), size);
            if (size > 1)
            {
                vector<RANSACIRCLE> vecCircle;

                for (int i = 0; i < size; i++)
                {
                    pCircle = &vecResultCircle[i];
                    if (pCircle->radius > 0)
                    {
                        int r = (int)pCircle->radius;
                        int x = (int)pCircle->center.x - r;
                        int y = (int)pCircle->center.y - r;
                        if (x < 0) x = 0;
                        if (y < 0) y = 0;

                        int w = r * 2;
                        int h = r * 2;
                        if (grayImg1->width < (x + w))
                            w -= ((x + w) - grayImg1->width);
                        if (grayImg1->height < (y + h))
                            h -= ((y + h) - grayImg1->height);

                        cvSetImageROI(grayImg1, cvRect(x, y, w, h));
                        IplImage *image = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
                        cvCopy(grayImg1, image);
                        cvResetImageROI(grayImg1);

                        cvInRangeS(image, Scalar(230), Scalar(255), image);

                        if (m_bSaveEngineImg)
                        {
                            QString str; str.sprintf(("%d_Filter_%d.jpg"), 800, i);
                            SaveOutImage(image, pData, str);
                        }

                        CBlobResult blobs;
                        blobs = CBlobResult(image, NULL, 0);	// Use a black background color.
                        int blobCount = blobs.GetNumBlobs();
                        double dLargeArea = 0;
                        for (int j = 0; j < blobCount; j++) {
                            CBlob *p = blobs.GetBlob(j);
                            double dArea = p->Area();
                            if (dLargeArea < dArea) {
                                dLargeArea = dArea;
                            }
                        }

                        double dChkPerc = 0.5;
                        if (pCircle->cPerc < 0.5)
                            dChkPerc = pCircle->cPerc;
                        // 원영역의 흰색영역이 50% 이상인지 check
                        if (dLargeArea < (double)(r*r)*dChkPerc)
                            continue;

                        vecCircle.push_back(*pCircle);
                        cvReleaseImage(&image);
                    }
                }


                if (vecCircle.size() > 0)
                {
                    std::stable_sort(vecCircle.begin(), vecCircle.end(), [](const RANSACIRCLE lhs, const RANSACIRCLE rhs)->bool {
                        if (lhs.cPerc > rhs.cPerc) // descending
                            return true;
                        return false;
                    });

                    pCircle = &vecCircle[0];
                    m_DetectResult.pt.x = vecCircle[0].center.x;// + pData->bounds().left();
                    m_DetectResult.pt.y = vecCircle[0].center.y;// + pData->bounds().top();
                    m_DetectResult.dRadius = vecCircle[0].radius;
                    m_DetectResult.dAngle = vecCircle[0].cPerc; // dAngle에 원형도 값을 넣어 사용.
                    //m_DetectResult.nCh = pData->m_nCh;
                    pData->m_vecDetectResult.push_back(m_DetectResult);
                    str.sprintf(("====> RANSAC Result Circle found1. : %.1f (%.1f,%.1f)\n"), vecCircle[0].radius, vecCircle[0].center.x, vecCircle[0].center.y);
                    //TRACE(str);

                    if (m_bSaveEngineImg)
                    {
                        IplImage* Img = cvCloneImage(croppedImage);
                        cvCircle(Img, cvPoint(vecCircle[0].center.x, vecCircle[0].center.y), vecCircle[0].radius, CV_RGB(128, 128, 128), 2);
                        QString str; str.sprintf(("%d_Result.jpg"), 900);
                        SaveOutImage(Img, pData, str);
                        if (Img) cvReleaseImage(&Img);
                    }
                }

            } // if (vecResultCircle.size() > 1)
            else if (size == 1)
            {
                pCircle = &vecResultCircle[0];
                m_DetectResult.pt.x = pCircle->center.x;// + pData->bounds().left();
                m_DetectResult.pt.y = pCircle->center.y;// + pData->bounds().top();
                m_DetectResult.dRadius = pCircle->radius;
                m_DetectResult.dAngle = pCircle->cPerc; // dAngle에 원형도 값을 넣어 사용.
                //m_DetectResult.nCh = pData->m_nCh;
                pData->m_vecDetectResult.push_back(m_DetectResult);
                str.sprintf(("====> RANSAC Result Circle found2. : %.1f (%.1f,%.1f)\n"), pCircle->radius, pCircle->center.x, pCircle->center.y);
                //TRACE(str);
            }
            size = vecResultCircle.size();
            //TRACE(("Find_Result_RANSAC_Circle: last size = %d\n"), size);
        } // if (vecRansicCircle.size() > 1)
        else if (size == 1)
        {
            pCircle = &vecRansicCircle[0];
            m_DetectResult.pt.x = pCircle->center.x;// + pData->bounds().left();
            m_DetectResult.pt.y = pCircle->center.y;// + pData->bounds().top();
            m_DetectResult.dRadius = pCircle->radius;
            m_DetectResult.dAngle = pCircle->cPerc; // dAngle에 원형도 값을 넣어 사용.
            //m_DetectResult.nCh = pData->m_nCh;
            pData->m_vecDetectResult.push_back(m_DetectResult);
            str.sprintf(("====> RANSAC Result Circle found3. : %.1f (%.1f,%.1f)\n"), pCircle->radius, pCircle->center.x, pCircle->center.y);
            //TRACE(str);
        }

    }

    //retry++;
    //if (m_DetectResult.dRadius == 0.0 && retry < 3)
    //	goto again;

    if (m_DetectResult.dRadius > 0.0 && pData->m_vecDetectResult.size() > 0) {
        str.sprintf(("Result circle: cPerc:%.1f center : %.1f,%.1f  radius : %.1f"), m_DetectResult.dAngle, m_DetectResult.pt.x, m_DetectResult.pt.y, m_DetectResult.dRadius);
        //MSystem.m_pFormBottom->SetBottomMessage(str);

        IplImage* rstimg = cvCreateImage(cvSize(rectIn.width(), rectIn.height()), IPL_DEPTH_8U, 1);
        DetectResult *pRst = &pData->m_vecDetectResult[0];
        cvSet(rstimg, 0);
        cvCircle(rstimg, cvPoint(pRst->pt.x,
                              pRst->pt.y), pRst->dRadius, 255, 2);
        //pRst->ngBlobImg = rstimg;
        pRst->rect = rectIn;

        if (m_bSaveEngineImg)
        {
            QString str; str.sprintf(("%d_RansacResult.jpg"), 920);
            SaveOutImage(rstimg, pData, str);
        }

    }

    //cvReleaseMemStorage(&storage);
    if (grayImg) cvReleaseImage(&grayImg);
    if (grayImg1) cvReleaseImage(&grayImg1);

    return 0;
}

int CImgProcEngine::SingleROIBondingLine(IplImage* croppedImage, RoiObject *pData, QRectF rect)
{
	QString str;

	pData->m_vecDetectResult.clear();
	IplImage* grayImg = cvCloneImage(croppedImage);

	int ret = 0;
	CParam *pParam;

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 200);
		SaveOutImage(grayImg, pData, str);
	}

	int nThresholdLowValue = 230;
	pParam = pData->getParam(("Low Threshold"));
	if (pParam)
        nThresholdLowValue = pParam->Value.toDouble();
	int nThresholdHighValue = 255;
	pParam = pData->getParam(("High Threshold"));
	if (pParam)
        nThresholdHighValue = pParam->Value.toDouble();

	int nMinLineLength = 10;// nLow;
	int nMaxLineLength = 100;// nHigh;
	pParam = pData->getParam(("Minimum line length"));
	if (pParam)
        nMinLineLength = (int)pParam->Value.toDouble();
	str.sprintf(("Maximum line length"));
	pParam = pData->getParam(str);
	if (pParam)
        nMaxLineLength = (int)pParam->Value.toDouble();

	cvInRangeS(grayImg, Scalar(nThresholdLowValue), Scalar(nThresholdHighValue), grayImg);
	//if (nThresholdHighValue == 0)
	//	ThresholdOTSU(pData, grayImg, 202);
	//else
	//	ThresholdRange(pData, grayImg, 202);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Threshold.jpg"), 203);
		SaveOutImage(grayImg, pData, str);
	}

	NoiseOut(pData, grayImg, 212);
	Expansion(pData, grayImg, 213);

	PostNoiseOut(pData, grayImg, 231);
	PostExpansion(pData, grayImg, 232);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Noise.jpg"), 234);
		SaveOutImage(grayImg, pData, str);
	}

	FilterBlobBoundingBoxXLength(grayImg, pData, nMinLineLength, nMaxLineLength);
	//FilterBlobLength(grayImg, pData, nMinLineLength, nMaxLineLength);
    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_FilterBlobLength.jpg"), 251);
		SaveOutImage(grayImg, pData, str);
	}

	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.
	int blobCount = blobs.GetNumBlobs();
	if (blobCount > 0) {
		for (int i = 0; i < blobCount; i++)
		{
			CBlob *currentBlob = blobs.GetBlob(i);
			
			CvRect rect = currentBlob->GetBoundingBox();
            //rect.x += pData->bounds().left();
            //rect.y += pData->bounds().top() + rect.height;
			m_DetectResult.pt = CvPoint2D32f(rect.x, rect.y);
            //m_DetectResult.pt1 = CvPoint2D32f(rect.x+rect.width, rect.y);
            //m_DetectResult.nCh = pData;
			pData->m_vecDetectResult.push_back(m_DetectResult);
			break;
		}
	}

	if (pData->m_vecDetectResult.size() > 0) {
		DetectResult *pResult = &pData->m_vecDetectResult[0];
		double dOffsetX1 = 0.0;
		double dOffsetX2 = 0.0;
		double dOffsetY = 0.0;
		pParam = pData->getParam(("X1 Offset(mm)"));
		if (pParam)
            dOffsetX1 = pParam->Value.toDouble();
		pParam = pData->getParam(("X2 Offset(mm)"));
		if (pParam)
            dOffsetX2 = pParam->Value.toDouble();
		pParam = pData->getParam(("Y Offset(mm)"));
		if (pParam)
            dOffsetY = pParam->Value.toDouble();
		const double dResX = gCfg.m_pCamInfo[0].dResX;
		const double dResY = gCfg.m_pCamInfo[0].dResY;

		pResult->pt.x += (dOffsetX1 / dResX); // Offset Pixel 갯수
		pResult->pt.y += (dOffsetY / dResY);
        //pResult->pt1.x += (dOffsetX2 / dResX);
        //pResult->pt1.y += (dOffsetY / dResY);
        //str.sprintf(("line : %.1f,%.1f   %.1f,%.1f"), pResult->pt.x, pResult->pt.y, pResult->pt1.x, pResult->pt1.y);
        //g_cLog->AddLog(_LOG_LIST_SYS, str);
	}


	return 0;
}

int CImgProcEngine::SingleROIBondingPosition(IplImage* croppedImage, RoiObject *pData, QRectF rect)
{
	QString str;

	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Criteria position ROI"));

		if (pParam) {
            int n = pParam->Value.toDouble();
			if (n > 0) { // 기준위치 ROI가 설정되어 있다.
				if (0 == pParam->m_vecDetail.size()){
					pParam->m_vecDetail.push_back(("No Parent/Criteria Roi"));
                    for (int i = 0; i < g_cRecipeData->m_vecRoiObject.size(); i++){
                        RoiObject* pdata = g_cRecipeData->m_vecRoiObject[i];
                        if (pdata->groupName() == pData->groupName())
						{
                            if (pParam->Value == pdata->name()) {
								QString str;
								str.sprintf(("%d"), i + 1);
								pParam->Value = str;
							}
                            pParam->m_vecDetail.push_back(pdata->name());
						}
					}
				}

				if (0 < pParam->m_vecDetail.size()){
                    QString str = pParam->m_vecDetail[n];
                    if (str != pData->name())
						SetROIAreaForCriteriaPosition(pData, str);
				}
			}
		}
	}


	return 0;
}



bool CImgProcEngine::SetROIAreaForCriteriaPosition(RoiObject *pData, QString strCriteriaROI)
{
	QString str;
    bool bFoundCriteriaPosition = false;
	double dCriteriaX;
	double dCriteriaY;

    for (int i = 0; i < (int)g_cRecipeData->m_vecRoiObject.size(); i++)
	{
        RoiObject *pCriData = g_cRecipeData->m_vecRoiObject[i];
        if (pCriData->isVisible() && pCriData->name() == strCriteriaROI) {

			// Priority에 의해 기준위치 ROI가 먼저 처리되거 설정 - 2017.7.2 jlyoon
			// 수동으로 처리할때는 항상 기준위치 ROI 처리 
			int size = pCriData->m_vecDetectResult.size();
			//if (size == 0)
            InspectOneItem(curImg, pCriData); // 기준위치의 ROI를 먼저 처리한다.

			if (size > 0) {
				DetectResult *prst = &pCriData->m_vecDetectResult[0];
				dCriteriaX = prst->pt.x;
				dCriteriaY = prst->pt.y;
	
                bFoundCriteriaPosition = true;
				break;
			}
		}
	}
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
    //g_cLog->AddLog(_LOG_LIST_SYS, str);

    return true;
}
int CImgProcEngine::SingleROISide3LaserPoint(IplImage* croppedImage, RoiObject *pData, QRectF rect)
{
	QString str;


	pData->m_vecDetectResult.clear();
	IplImage* grayImg = cvCloneImage(croppedImage);

	int ret = 0;
	CParam *pParam;

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 200);
		SaveOutImage(grayImg, pData, str);
	}

	int nThresholdLowValue = 230;
	pParam = pData->getParam(("Low Threshold"));
	if (pParam)
        nThresholdLowValue = pParam->Value.toDouble();
	int nThresholdHighValue = 255;
	pParam = pData->getParam(("High Threshold"));
	if (pParam)
        nThresholdHighValue = pParam->Value.toDouble();

	int nMinLineLength = 10;// nLow;
	int nMaxLineLength = 100;// nHigh;
	pParam = pData->getParam(("Minimum line length"));
	if (pParam)
        nMinLineLength = (int)pParam->Value.toDouble();
	str.sprintf(("Maximum line length"));
	pParam = pData->getParam(str);
	if (pParam)
        nMaxLineLength = (int)pParam->Value.toDouble();

	cvInRangeS(grayImg, Scalar(nThresholdLowValue), Scalar(nThresholdHighValue), grayImg);
	//if (nThresholdHighValue == 0)
	//	ThresholdOTSU(pData, grayImg, 202);
	//else
	//	ThresholdRange(pData, grayImg, 202);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Threshold.jpg"), 203);
		SaveOutImage(grayImg, pData, str);
	}

	NoiseOut(pData, grayImg, 212);
	Expansion(pData, grayImg, 213);

	PostNoiseOut(pData, grayImg, 231);
	PostExpansion(pData, grayImg, 232);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Noise.jpg"), 234);
		SaveOutImage(grayImg, pData, str);
	}

	FilterBlobBoundingBoxXLength(grayImg, pData, nMinLineLength, nMaxLineLength);
	//FilterBlobLength(grayImg, pData, nMinLineLength, nMaxLineLength);
    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_FilterBlobLength.jpg"), 251);
		SaveOutImage(grayImg, pData, str);
	}

	Thinner(pData, grayImg, 252);

	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.
	int blobCount = blobs.GetNumBlobs();
	if (blobCount > 0) {
		for (int i = 0; i < blobCount; i++)
		{
			CBlob *currentBlob = blobs.GetBlob(i);

			CvRect rect = currentBlob->GetBoundingBox();
            //rect.x += pData->bounds().left();
            //rect.y += pData->bounds().top() + rect.height;
			m_DetectResult.pt = CvPoint2D32f(rect.x, rect.y);
			pData->m_vecDetectResult.push_back(m_DetectResult);
			break;
		}
	}

	if (pData->m_vecDetectResult.size() > 0) {
		DetectResult *pResult = &pData->m_vecDetectResult[0];
		double dOffsetX = 0.0;
		double dOffsetY = 0.0;
		pParam = pData->getParam(("X Offset(mm)"));
		if (pParam)
            dOffsetX = pParam->Value.toDouble();
		pParam = pData->getParam(("Y Offset(mm)"));
		if (pParam)
            dOffsetY = pParam->Value.toDouble();
		const double dResX = gCfg.m_pCamInfo[0].dResX;
		const double dResY = gCfg.m_pCamInfo[0].dResY;

        //pResult->pt1.x = 0;
        //pResult->pt1.y = 0;
		pResult->pt.x += (dOffsetX / dResX); // Offset Pixel 갯수
		pResult->pt.y += (dOffsetY / dResY);
		str.sprintf(("laser point : %.1f,%.1f"), pResult->pt.x, pResult->pt.y);
        //g_cLog->AddLog(_LOG_LIST_SYS, str);
	}


	return 0;
}

int CImgProcEngine::SingleROIPinErrorCheck(IplImage* croppedImage, RoiObject *pData, QRectF rect)
{
	QString str;

	pData->m_vecDetectResult.clear();
	IplImage* grayImg = cvCloneImage(croppedImage);

	int ret = 0;
	CParam *pParam;

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Src.jpg"), 200);
		SaveOutImage(grayImg, pData, str);
	}

	int nThresholdLowValue = 230;
	pParam = pData->getParam(("Low Threshold"));
	if (pParam)
        nThresholdLowValue = pParam->Value.toDouble();
	int nThresholdHighValue = 255;
	pParam = pData->getParam(("High Threshold"));
	if (pParam)
        nThresholdHighValue = pParam->Value.toDouble();

	int nMinArea = 8700;
	pParam = pData->getParam(("Area(8700)"));
	if (pParam)
        nMinArea = (int)pParam->Value.toDouble();
	str.sprintf(("Area"));

	cvInRangeS(grayImg, Scalar(nThresholdLowValue), Scalar(nThresholdHighValue), grayImg);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Threshold.jpg"), 203);
		SaveOutImage(grayImg, pData, str);
	}

	NoiseOut(pData, grayImg, 212);
	Expansion(pData, grayImg, 213);

    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_Noise.jpg"), 234);
		SaveOutImage(grayImg, pData, str);
	}

	FilterIncludeLargeBlob(grayImg, pData);
    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_FilterIncludeLargeBlob.jpg"), 251);
		SaveOutImage(grayImg, pData, str);
	}

	pData->m_bVisionError = false;
	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.
	int blobCount = blobs.GetNumBlobs();
	if (blobCount > 0) {
		for (int i = 0; i < blobCount; i++)
		{
			CBlob *currentBlob = blobs.GetBlob(i);
			double dArea = currentBlob->Area();
			str.sprintf(("Pin area size :%.0f"), dArea);
            //g_cLog->AddLog(_LOG_LIST_SYS, str);
			if (dArea < nMinArea) {
				pData->m_bVisionError = true;
				str.sprintf(("Pin area error :%.0f < %.0f"), dArea, (double)nMinArea);
                //g_cLog->AddLog(_LOG_LIST_SYS, str);
			}
			break;
		}
	}

	return 0;
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
			if (0 == pParam->m_vecDetail.size()){
				pParam->m_vecDetail.push_back(("No Parent/Criteria Roi"));
                for (int i = 0; i < g_cRecipeData->m_vecRoiObject.size(); i++){
                    RoiObject* pdata = g_cRecipeData->m_vecRoiObject[i];
                    if (pdata->groupName() == pData->groupName())
					{
                        if (pParam->Value == pdata->name()) {
							QString str;
							str.sprintf(("%d"), i + 1);
							pParam->Value = str;
						}
                        pParam->m_vecDetail.push_back(pdata->name());
					}
				}
			}

			if (0 < pParam->m_vecDetail.size()){
                QString strCriteriaROI = pParam->m_vecDetail[n]; // str이 기준 Mask ROI 이다.
                if (strCriteriaROI != pData->name().toStdString().c_str()) {
                    for (int i = 0; i < (int)g_cRecipeData->m_vecRoiObject.size(); i++)
					{
                        RoiObject *pCriData = g_cRecipeData->m_vecRoiObject[i];
                        if (pCriData->isVisible() && pCriData->name() == strCriteriaROI) {
							return pCriData;
						}
					}
				}
			}
		}
	}

	return NULL;
}

double CImgProcEngine::SubPixelRampEdgeImage(IplImage* edgeImage, RoiObject *pData, int nDir)
{
	vector<cv::Point2f> vecEdges;

	int widthStep = edgeImage->widthStep;
	int cx = edgeImage->width;
	int cy = edgeImage->height;

	int width = 6;
	CParam *pParam = pData->getParam(("Ramp width")); // default : 6
	if (pParam)
        width = (int)pParam->Value.toDouble();

	if ((nDir % 2) == 1) {
		cvFlip(edgeImage, edgeImage, -1); // 상하, 좌우반전
	}
    if (m_bSaveEngineImg){
        SaveOutImage(edgeImage, pData, ("260_SubPixelRampEdgeImageIn.jpg"), false);
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

cv::Point2f CImgProcEngine::getCorner(std::vector<cv::Point2f>& corners, cv::Point2f center, int CornerType)
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
void CImgProcEngine::bhm_line(int x1, int y1, int x2, int y2, std::vector<cv::Point>* points)
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

cv::Point CImgProcEngine::getValueX(std::vector<cv::Point> points, int pos)
{
	Point v[3] = { { 0, 0 }, { 0, 0 }, { 0, 0 } };
	int cnt = 0;
	Point pt;

	pt.y = points[pos].y;
	int size = points.size();
	pos--;
	if (pos >= 0 && pos < size)
	{
		v[cnt].x = points[pos].x;
		cnt++;
	}
	pos++;
	if (pos >= 0 && pos < size)
	{
		v[cnt].x = points[pos].x;
		cnt++;
	}
	pos++;
	if (pos >= 0 && pos < size)
	{
		v[cnt].x = points[pos].x;
		cnt++;
	}
	pt.x = (v[0].x + v[1].x + v[2].x) / cnt;
	return pt;
}

cv::Point CImgProcEngine::getValueY(std::vector<cv::Point> points, int pos)
{
	Point v[3] = { { 0, 0 }, { 0, 0 }, { 0, 0 } };
	int cnt = 0;
	Point pt;

	pt.x = points[pos].x;
	int size = points.size();
	pos--;
	if (pos >= 0 && pos < size)
	{
		v[cnt].y = points[pos].y;
		cnt++;
	}
	pos++;
	if (pos >= 0 && pos < size)
	{
		v[cnt].y = points[pos].y;
		cnt++;
	}
	pos++;
	if (pos >= 0 && pos < size)
	{
		v[cnt].y = points[pos].y;
		cnt++;
	}
	pt.y = (v[0].y + v[1].y + v[2].y) / cnt;
	return pt;
}

bool operator<(const cv::Point& a, const cv::Point& b) {
	return a.x < b.x;
}

double CImgProcEngine::getObjectAngle(IplImage *src)
{
	CvMoments cm;
	cvMoments(src, &cm, true);
	double th = 0.5 * atan((2 * cm.m11) / (cm.m20 - cm.m02));
	return th * (180 / 3.14);
}
double CImgProcEngine::GetDistance2D(CvPoint p1, CvPoint p2)
{
	return sqrt(pow((float)p1.x - p2.x, 2) + pow((float)p1.y - p2.y, 2));
}
void CImgProcEngine::GetMidpoint(CvPoint p1, CvPoint p2, CvPoint *p3)
{
	p3->x = (int)((float)(p1.x + p2.x) / 2.0);
	p3->y = (int)((float)(p1.y + p2.y) / 2.0);
}

int CImgProcEngine::CenterOfGravity(RoiObject *pData, IplImage* croppedImageIn, CvPoint2D32f &cog, int nRetry)
{
	QString str;
	IplImage* drawImage = NULL;
	IplImage* croppedImage = cvCloneImage(croppedImageIn);
	int MinMomentSize = 5, MaxMomentSize = 50;

	int nMinCircleRadius = 30;
	int nMaxCircleRadius = 100;
	if (pData != NULL)
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

	NoiseOut(pData, croppedImage, 311);
	Expansion(pData, croppedImage, 312);

	CBlobResult blobs;
	blobs = CBlobResult(croppedImage, NULL, 0);	// Use a black background color.
#if 1
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
#else
	/////////////////////////
	// 가장큰 Blob만 남김
	////////////////////////
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
#endif

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
	CvSeq* m_dominant_points = 0;    // 특징점 찾기 위한 변수
	CvSeq* ptseq;
	CvSeq* defect = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);

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
		m_approxDP_seq = cvApproxPoly(m_seq, sizeof(CvContour), m_storage, CV_POLY_APPROX_DP, 3, 1);
		if (m_approxDP_seq == NULL)
		{
			if (m_storage) cvReleaseMemStorage(&m_storage);
			if (croppedImage) cvReleaseImage(&croppedImage);
			if (drawImage) cvReleaseImage(&drawImage);
			return -1;
		}

        if (m_bSaveEngineImg)
		{
			cvDrawContours(drawImage, m_approxDP_seq, CVX_RED, CVX_RED, 1, 1, 8); // 외곽선 근사화가 잘되었는지 테스트
			str.sprintf(("322_cvApproxPoly.jpg"));
			SaveOutImage(drawImage, pData, str);
		}

		int nSeq = 0;
		for (CvSeq* c = m_approxDP_seq; c != NULL; c = c->h_next)  // 엣지의 링크드리스트를 순회하면서 각 엣지들에대해서 출력한다.
		{
			float radius;
			CvPoint2D32f center;
			cvMinEnclosingCircle(c, &center, &radius);

            ptseq = cvCreateSeq((CV_SEQ_KIND_CURVE | CV_SEQ_ELTYPE_POINT | CV_SEQ_FLAG_CLOSED) | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);
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

                //str.sprintf(("CH:%d Radius = %.3f)"), pData, radius);
                ////g_cLog->AddLog(_LOG_LIST_SYS, str);

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
	if (defect) cvClearSeq(defect);
	if (m_storage) cvReleaseMemStorage(&m_storage);

	if (croppedImage) cvReleaseImage(&croppedImage);
	if (drawImage) cvReleaseImage(&drawImage);

	if (cog.x == 0 || cog.y == 0) return -1;
	if (iContoursSize <= 0) return -1;
	return iContoursSize;
}

//
// 사용자가 설정한 값으로 Threshold를 실행
//
int CImgProcEngine::Threshold(RoiObject *pData, IplImage* grayImg, int nDbg)
{
	QString str;

	int nThresholdValue = 70;
	int nThresholdMaxVal = 255;
	int bInvert = 0;
	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Brightness Threshold"));
		if (pParam)
            nThresholdValue = pParam->Value.toDouble();
		pParam = pData->getParam(("Brightness Max"));
		if (pParam)
            nThresholdMaxVal = pParam->Value.toDouble();
		pParam = pData->getParam(("Invert?"));
		if (pParam)
            bInvert = (int)pParam->Value.toDouble();
	}

	int nInvert = CV_THRESH_BINARY;
	if (bInvert == 1)
		nInvert = CV_THRESH_BINARY_INV;
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
	int bInvert = 0;
	//int bLargeBlob = 0;

    //if (m_bSaveEngineImg){
	//	str.sprintf(("%03d_cvThresholdRange.jpg"), nDbg);
    //	SaveOutImage(grayImg, pData, str, false);
	//}

	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Low Threshold"));
		if (pParam)
            nThresholdLowValue = pParam->Value.toDouble();
		pParam = pData->getParam(("High Threshold"));
		if (pParam)
            nThresholdHighValue = pParam->Value.toDouble();
		pParam = pData->getParam(("Invert?"));
		if (pParam)
            bInvert = (int)pParam->Value.toDouble();
		//pParam = pData->getParam(("Large Blob?"));
		//if (pParam)
        //	bLargeBlob = (int)pParam->Value.toDouble();
	}
	if (nThresholdHighValue == 0 && nThresholdLowValue == 0)
		return -1;

	cvInRangeS(grayImg, Scalar(nThresholdLowValue), Scalar(nThresholdHighValue), grayImg);

    //if (m_bSaveEngineImg){
	//	str.sprintf(("%03d_cvThresholdRange1.jpg"), nDbg);
    //	SaveOutImage(grayImg, pData, str, false);
	//}

	if (bInvert == 1)
		cvNot(grayImg, grayImg);
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
	if (pData != NULL) {
		CParam *pParam;// = pData->getParam(("Brightness Threshold"));
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
int CImgProcEngine::NoiseOut(RoiObject *pData, IplImage* grayImg, int nDbg)
{
	QString str;

	IplImage* tmp = cvCreateImage(cvGetSize(grayImg), 8, 1);

	// 1. Template이미지의 노이즈 제거
	int filterSize = 3;  // 필터의 크기를 6으로 설정 (Noise out area)
	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Noise out area"));
		if (pParam)
            filterSize = pParam->Value.toDouble();
	}
	IplConvKernel *element = NULL;
	if (filterSize <= 0)
		filterSize = 1;
	if (filterSize % 2 == 0)
		filterSize++;
	//element = cvCreateStructuringElementEx(filterSize, filterSize, (filterSize - 1) / 2, (filterSize - 1) / 2, CV_SHAPE_RECT, NULL);
	element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, NULL);

	int nNoiseout = 0;
	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Noise out 1"));
		if (pParam)
            nNoiseout = pParam->Value.toDouble();
	}
	if (nNoiseout != 0)
	{
		if (nNoiseout < 0)
			cvMorphologyEx(grayImg, tmp, NULL, element, CV_MOP_OPEN, -nNoiseout);
		else //if (nNoiseout > 0)
			cvMorphologyEx(grayImg, tmp, NULL, element, CV_MOP_CLOSE, nNoiseout);
	}

	nNoiseout = 0;
	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Noise out 2"));
		if (pParam)
            nNoiseout = pParam->Value.toDouble();
	}
	if (nNoiseout != 0)
	{
		if (nNoiseout < 0)
			cvMorphologyEx(tmp, grayImg, NULL, element, CV_MOP_OPEN, -nNoiseout);
		else //if (nNoiseout > 0)
			cvMorphologyEx(tmp, grayImg, NULL, element, CV_MOP_CLOSE, nNoiseout);
	}
    if (m_bSaveEngineImg){
		str.sprintf(("%03d_cvClose.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
	}
	cvReleaseImage(&tmp);

	cvReleaseStructuringElement(&element);
	return 0;
}


int CImgProcEngine::PostNoiseOut(RoiObject *pData, IplImage* grayImg, int nDbg)
{
	QString str;

	IplImage* tmp = cvCreateImage(cvGetSize(grayImg), 8, 1);

	// 노이즈 제거 필터
	int filterSize = 3;  // 필터의 크기를 3으로 설정 (Noise out area)
	//if (pData != NULL) {
	//	CParam *pParam = pData->getParam(("Noise out area"));
	//	if (pParam)
    //		filterSize = pParam->Value.toDouble();
	//}
	IplConvKernel *element = NULL;
	if (filterSize <= 0)
		filterSize = 1;
	if (filterSize % 2 == 0)
		filterSize++;
	//element = cvCreateStructuringElementEx(filterSize, filterSize, (filterSize - 1) / 2, (filterSize - 1) / 2, CV_SHAPE_RECT, NULL);
	element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, NULL);

	int nNoiseout = 0;
	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Noise out 3"));
		if (pParam)
            nNoiseout = pParam->Value.toDouble();
	}
	if (nNoiseout != 0)
	{
		if (nNoiseout < 0)
			cvMorphologyEx(grayImg, tmp, NULL, element, CV_MOP_OPEN, -nNoiseout);
		else //if (nNoiseout > 0)
			cvMorphologyEx(grayImg, tmp, NULL, element, CV_MOP_CLOSE, nNoiseout);
	}
	nNoiseout = 0;
	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Noise out 4"));
		if (pParam)
            nNoiseout = pParam->Value.toDouble();
	}
	if (nNoiseout != 0)
	{
		if (nNoiseout < 0)
			cvMorphologyEx(tmp, grayImg, NULL, element, CV_MOP_OPEN, -nNoiseout);
		else //if (nNoiseout > 0)
			cvMorphologyEx(tmp, grayImg, NULL, element, CV_MOP_CLOSE, nNoiseout);
	}
    if (m_bSaveEngineImg){
		str.sprintf(("%03d_cvClose.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
	}
	cvReleaseImage(&tmp);

	cvReleaseStructuringElement(&element);
	return 0;
}


//
// Dialate / Erode
//
int CImgProcEngine::Expansion(RoiObject *pData, IplImage* grayImg, int nDbg)
{
	QString str;

	int nExpansion1 = 0;
	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Expansion 1"));
		if (pParam)
            nExpansion1 = pParam->Value.toDouble();
	}
	int nExpansion2 = 0;
	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Expansion 2"));
		if (pParam)
            nExpansion2 = pParam->Value.toDouble();
	}
	if (nExpansion1 == 0 && nExpansion2 == 0)
		return 0;

	IplImage* tmp = cvCreateImage(cvGetSize(grayImg), 8, 1);
	if (nExpansion1 < 0)
		cvErode(grayImg, tmp, NULL, -nExpansion1);
	else  //if (nExpansion > 0)
		cvDilate(grayImg, tmp, NULL, nExpansion1);

	if (nExpansion2 < 0)
		cvErode(tmp, grayImg, NULL, -nExpansion2);
	else //if (nExpansion > 0)
		cvDilate(tmp, grayImg, NULL, nExpansion2);

    if (m_bSaveEngineImg){
		str.sprintf(("%03d_cvExpansion.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
	}
	cvReleaseImage(&tmp);
	return 0;
}

int CImgProcEngine::PostExpansion(RoiObject *pData, IplImage* grayImg, int nDbg)
{
	QString str;

	int nExpansion1 = 0;
	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Expansion 3"));
		if (pParam)
            nExpansion1 = pParam->Value.toDouble();
	}
	int nExpansion2 = 0;
	if (pData != NULL) {
		CParam *pParam = pData->getParam(("Expansion 4"));
		if (pParam)
            nExpansion2 = pParam->Value.toDouble();
	}
	if (nExpansion1 == 0 && nExpansion2 == 0)
		return 0;

	IplImage* tmp = cvCreateImage(cvGetSize(grayImg), 8, 1);
	if (nExpansion1 < 0)
		cvErode(grayImg, tmp, NULL, -nExpansion1);
	else  //if (nExpansion > 0)
		cvDilate(grayImg, tmp, NULL, nExpansion1);

	if (nExpansion2 < 0)
		cvErode(tmp, grayImg, NULL, -nExpansion2);
	else //if (nExpansion > 0)
		cvDilate(tmp, grayImg, NULL, nExpansion2);

    if (m_bSaveEngineImg){
		str.sprintf(("%03d_cvExpansion.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
	}
	cvReleaseImage(&tmp);
	return 0;
}

void CImgProcEngine::Smooth(RoiObject *pData, IplImage* ImgIn, int iImgID)
{
	int param1 = 3;
	int param2 = 0;
	double param3 = 2, param4 = 2;
	int smoothMethod = CV_GAUSSIAN;
	int smoothSize = 3;
	IplImage* image = NULL;
    bool bUse = true;

	if (pData != NULL)
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



// nAxisLength : 400
int CImgProcEngine::ExcludeLargeBlob(IplImage* grayImg, RoiObject *pData, int nAxisLength, int nDbg)
{
	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.
	blobs.Filter(blobs, B_EXCLUDE, CBlobGetMajorAxisLength(), B_GREATER, nAxisLength); // 블럽이 아주 큰것은 제거.
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
    if (m_bSaveEngineImg)
	{
        QString str; str.sprintf(("%d_ExcludeLargeBlob.jpg"), nDbg);
		SaveOutImage(grayImg, pData, str);
	}
	cvReleaseImage(&pMask);
	return 0;
}

int CImgProcEngine::IncludeRangeBlob(IplImage* grayImg, RoiObject *pData, int nMinCircleRadius, int nMaxCircleRadius)
{

	CBlobResult blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.

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
    if (m_bSaveEngineImg)
	{
		QString str; str.sprintf(("%d_IncludeRangeBlob.jpg"), 221);
		SaveOutImage(grayImg, pData, str);
	}
	return 0;
}

void CImgProcEngine::FilterLargeArea(IplImage* grayImg)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.

    /////////////////////////
    // 가장큰 Blob만 남김.
    ////////////////////////
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


void CImgProcEngine::FilterLargeDiameter(IplImage* grayImg)
{
	CBlobResult blobs;
	blobs = CBlobResult(grayImg, NULL, 0);	// Use a black background color.

	/////////////////////////
	// 가장큰 Blob만 남김.
	////////////////////////
	CvRect LargeRect(0, 0);
	int index = -1;
	int nBlobs = blobs.GetNumBlobs();
	for (int i = 0; i < nBlobs; i++) {
		CBlob *p = blobs.GetBlob(i);
		CvRect r = p->GetBoundingBox();
		if (r.height > LargeRect.height || r.width > LargeRect.width)
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
double CImgProcEngine::TemplateMatch(RoiObject *pData, IplImage* graySearchImgIn, IplImage* grayTemplateImg, CvPoint &left_top, double &dMatchShapes)
{
	QString str;
    QRectF srect = pData->bounds();
	IplImage* graySearchImg = cvCloneImage(graySearchImgIn);

    if (m_bSaveEngineImg)
	{
		str.sprintf(("%d_graySearchImg.jpg"), 200);
		SaveOutImage(graySearchImg, pData, str);

		str.sprintf(("%d_grayTemplateImg.jpg"), 201);
		SaveOutImage(grayTemplateImg, pData, str);
	}

    CvSize size = cvSize(srect.width() - grayTemplateImg->width + 1, srect.height() - grayTemplateImg->height + 1);
	IplImage* C = cvCreateImage(size, IPL_DEPTH_32F, 1); // 상관계수를 구할 이미지(C)
	double min, max;

	IplImage* g2 = cvCreateImage(cvSize(grayTemplateImg->width, grayTemplateImg->height), IPL_DEPTH_8U, 1);
	cvCopy(grayTemplateImg, g2);

	int nThresholdValue = 0;
	CParam *pParam = pData->getParam(("High Threshold"));
	if (pParam)
        nThresholdValue = pParam->Value.toDouble();

	if (nThresholdValue == 0)
		ThresholdOTSU(pData, g2, 211);
	else
		ThresholdRange(pData, g2, 211);

	NoiseOut(pData, g2, 212);
	Expansion(pData, g2, 213);

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

	PostNoiseOut(pData, g2, 231);
	PostExpansion(pData, g2, 232);
	cvCanny(g2, g2, 100, 300, 3);
    if (m_bSaveEngineImg){
		SaveOutImage(g2, pData, ("227_TemplateImageCany.jpg"));
	}

	float dMatchingRate = 0.5f;
	float dMatchShapesingRate = 0.7f;
	if (pData != NULL) {
        CParam *pParam = pData->getParam("Pattern matching rate");
		if (pParam)
            dMatchingRate = (float)pParam->Value.toDouble() / 100.0f;
	}
	if (pData != NULL) {
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
		cvMinMaxLoc(C, &min, &max, NULL, &left_top); // 상관계수가 최대값을 값는 위치 찾기 
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

			IplImage* g1 = cvCreateImage(cvSize(grayTemplateImg->width, grayTemplateImg->height), IPL_DEPTH_8U, 1);

			cvCopy(graySearchImg, g1); // cvSetImageROI() 만큼 Copy
			if (nThresholdValue == 0)
				ThresholdOTSU(pData, g1, 243);
			else
				ThresholdRange(pData, g1, 243);

			NoiseOut(pData, g1, 244); // 노이즈 제거
			Expansion(pData, g1, 245);

			// 가장큼 or 긴 blob만 남긴다.
			if (nFilterBlob == 1)
				FilterLargeArea(g1);
			else if (nFilterBlob == 2)
				FilterLargeDiameter(g1);
            if (m_bSaveEngineImg){
				SaveOutImage(g1, pData, ("248_FilterBlob.jpg"));
			}

			PostNoiseOut(pData, g1, 251);
			PostExpansion(pData, g1, 252);
			cvCanny(g1, g1, 100, 300, 3);
            if (m_bSaveEngineImg){
				str.sprintf(("255_TemplateImageCany%d.jpg"), nLoop);
				SaveOutImage(g1, pData, str);
			}

			// g1이 전부 0로 채워져있으면 cvMatchShapes()에서 0로 리턴되어서 zero image filtering
			IplImage* c1 = cvCloneImage(g1);
			CvSeq* contours = 0;
			cvFindContours(c1, storage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL);
			cvReleaseImage(&c1);

			double matching = 1.0;
			if (contours && contours->total > 0)
				matching = cvMatchShapes(g1, g2, CV_CONTOURS_MATCH_I2); // 작은 값 일수록 두 이미지의 형상이 가까운 (0은 같은 모양)라는 것이된다.
			cvReleaseImage(&g1);

			//str.sprintf(("MatchTemplate cvMatchShapes : %.3f"), matching);
            //g_cLog->AddLog(_LOG_LIST_SYS, str);

			Point2f pt2 = Point2f((float)grayTemplateImg->width, (float)grayTemplateImg->height);
			cvRectangle(graySearchImg, cvPoint(0, 0), pt2, CV_RGB(128, 128, 128), CV_FILLED); // filled rectangle.

			cvResetImageROI(graySearchImg);

			if (matching > 1.0)
				matching = 1.0;
			dMatchShapes = (1.0-matching) * 100.0;
			if (matching <= (1.0 - dMatchShapesingRate)) // || max > 0.9)
			{
				str.sprintf(("Template Shape Match(succ) ===> : %.2f%%"), dMatchShapes);
                //g_cLog->AddLog(_LOG_LIST_SYS, str);

				cvReleaseImage(&graySearchImg);
				cvReleaseImage(&C);
				cvReleaseImage(&g2);
				cvReleaseMemStorage(&storage);

				m_DetectResult.nResult = 1; // OK

				//left_top.x += srect.left;
				//left_top.y += srect.top;
				return (max*100);
			}
			else {
				str.sprintf(("Template Shape Match ===> : %.2f%%"), dMatchShapes);
                qDebug() << str;
                //g_cLog->AddLog(_LOG_LIST_SYS, str);
			}
			nLoop++;
			//if (nLoop > 10) {
			//	str.sprintf(("MatchShape failed"));
            //	//g_cLog->AddLog(_LOG_LIST_SYS, str);
			//	break;
			//}
		}
		else 
		{
			nLoop++;
			if (nLoop > 10) {
                //dMatchShapes = maxRate * 100;
				str.sprintf(("TemplateMatch Result Fail ===> : %.2f%%"), maxRate * 100);
                //g_cLog->AddLog(_LOG_LIST_SYS, str);
				max = maxRate;
				m_DetectResult.nResult = 2; // NG
				break;
			}
		}
	}

	if(graySearchImg) cvReleaseImage(&graySearchImg);
	if(C) cvReleaseImage(&C);
	if(g2) cvReleaseImage(&g2);
	if(storage) cvReleaseMemStorage(&storage);

	return max*100.0;// -1;
}


//Moment 로 중심점 계산 : 공간 모멘트를 통해 중심점 구하기
Point2f CImgProcEngine::CenterOfMoment(CvSeq* c)
{
	double M;
	int x_order, y_order;
	double cX, cY, m00;
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
// 라인이 직접 만나야지 true를 리턴(확장라인 계산하지않음)
//
bool CImgProcEngine::checkCross(const cv::Point& AP1, const cv::Point& AP2, const cv::Point& BP1, const cv::Point& BP2, cv::Point* IP)
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
//bool CImgProcEngine::intersection(Point2f o1, Point2f p1, Point2f o2, Point2f p2, Point2f &r)
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

double CImgProcEngine::isCross(Point v1, Point v2)
{
	return v1.x*v2.y - v1.y*v2.x;
}
bool CImgProcEngine::getIntersectionPoint(Point a1, Point a2, Point b1, Point b2, Point & intPnt)
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

#if 1
//
// 한점과 직선이 만나는 가장 가까운점
// 리턴값 : 길이
// nearX, nearY : 만나는점
// 수선의 조건이 충족되면 라인상의 가장 가까운점을 찾게되고, 아니면 시작점 또는 끝점이 된다.
//
double CImgProcEngine::Dist2LineSegment(double px, double py, double X1, double Y1, double X2, double Y2, double &nearX, double &nearY)
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

#else

// 직선과 한점으로 법선과 교점을 구하는 함수.
// eqline 직선방정식 파라미터, point 점의 좌표, nleq 계산된 법선방정식 파라미터, intspoint 계산된 교점좌표
//double point[2], nleq[3], intspoint[2];
//double p1[2], p2[2], leq[3];
//GetLineEq_PointToPoint(p1, p2, leq);
//point[0] = x;
//point[1] = y;			// point
//nleq[0] = 0;
//nleq[1] = 0;
//nleq[2] = 0;							// normal line equation
//intspoint[0] = 0;
//intspoint[1] = 0;						// intersection point
//GetLineEq_LineToPoint(leq, point, nleq, intspoint);
unsigned int CImgProcEngine::GetLineEq_LineToPoint(double eqline[], double point[], double nleq[], double intspoint[])
{
	if (eqline[1] == 0) {			// 직선이 y축 평행
		nleq[0] = 0;
		nleq[1] = 1;
		nleq[2] = -1 * point[1];
		intspoint[0] = -1 * eqline[2] / eqline[0];
		intspoint[1] = point[1];
	}
	else if (eqline[0] == 0) {	// 직선이 x축 평행
		nleq[0] = 1;
		nleq[1] = 0;
		nleq[2] = -1 * point[0];
		intspoint[0] = point[0];
		intspoint[1] = -1 * eqline[2] / eqline[1];
	}
	else {
		nleq[0] = -1 * eqline[1] / eqline[0];
		nleq[1] = 1;
		nleq[2] = point[0] * eqline[1] / eqline[0] - point[1];
		intspoint[0] = (eqline[1] * nleq[2] / nleq[1] - eqline[2]) / (eqline[0] - eqline[1] * nleq[0] / nleq[1]);
		intspoint[1] = -1 * eqline[0] * intspoint[0] / eqline[1] - eqline[2] / eqline[1];
	}

	return 0;
}


// 두점을 지나는 직선의 방정식
// p1, p2 점의 좌표, leq 계산된 직선방정식의 파라미터
unsigned int CImgProcEngine::GetLineEq_PointToPoint(double p1[], double p2[], double leq[])
{
	double m, k;

	if ((p2[0] - p1[0]) == 0) {				// Y축 평행
		leq[0] = 1;
		leq[1] = 0;
		leq[2] = -1 * p1[0];
	}
	else if ((p2[1] - p1[1]) == 0) {		// X축 평행
		leq[0] = 0;
		leq[1] = 1;
		leq[2] = -1 * p1[1];
	}
	else {
		m = (p2[1] - p1[1]) / (p2[0] - p1[0]);
		k = p1[1] - m * p1[0];
		leq[0] = -1 * m;
		leq[1] = 1;
		leq[2] = -1 * k;
	}

	return 0;
}

#endif

//
// Hessian matrix 알고리즘을 이용한 Subpixel Edge를 구함
// 여러 edge contour를 구한다.
//
int CImgProcEngine::SubPixelHessianEigenEdge(IplImage *src, vector<Contour> &contours)
{
	double alpha = 1.0;
	int low = 10;
	int high = 10;
	int mode = RETR_LIST; //retrieves all of the contours without establishing any hierarchical relationships.
	//char imagePath[256] = "C:\\Projects\\Vision\\희성_BLU합착기\\합착치수검사\\image\\test.bmp";

	Mat msrc = cvarrToMat(src);// imread(imagePath, IMREAD_GRAYSCALE);
	//vector<Contour> contours;
	vector<Vec4i> hierarchy;
	//int64 t0 = getCPUTickCount();

	// alpha - GaussianBlur = sigma이며 흐려지는 정도를 조절할 수 있다.
	// hierarchy, mode  - have the same meanings as in cv::findContours
	EdgesSubPix(msrc, alpha, low, high, contours, hierarchy, mode);

	/*
	int64 t1 = getCPUTickCount();

	//cout << "execution time is " << (t1 - t0) / (double)getTickFrequency() << " seconds" << endl;

	uchar* data = (uchar*)msrc.data;

	IplImage *dst = cvCreateImage(CvSize(msrc.cols, msrc.rows), IPL_DEPTH_8U, msrc.channels());
	for (size_t i = 0; i < contours.size(); ++i)
	{
		for (size_t j = 0; j < contours[i].points.size(); ++j)
		{
			cv::Point2f pt = contours[i].points[j];
			printf("%.1f ", pt.x);

			dst->imageData[((int)pt.y*dst->widthStep) + (int)pt.x] = 255;

			data[((int)pt.y*msrc.cols) + (int)pt.x] = 255;
		}

	}
	*/
	//cvShowImage("dst", dst);
	//imshow("src", msrc);

	//cvWaitKey(0);

	return 0;
}


//
// 경사진면의 edge를 구한다
// nStep : 1 or 2 - 2를 주면 한개 pixel을 건너띤 경사를 구한다.
// nCnt - threshold경계면부터 검색할 범위
//
// nDir = 0 : Left2Right, Top2Bottom
//        1 : Right2Left, Bottom2Top
double CImgProcEngine::SubPixelRampEdge(unsigned char *pixelData, int pCnt, int nStep)
{
	double dLoc = 0.0;
	int i;
	int ffx[50];
	int iSumfx;

	memset(ffx, 0, sizeof(ffx));
	iSumfx = 0;
	for (i = 0; i < pCnt - 1; i++)					// f'(x)
	{
		if (nStep == 2)
		{
			if (i < pCnt - 2)
			{
				ffx[i + 1] = abs(pixelData[i + 2] - pixelData[i]);
				iSumfx += ffx[i + 1];
			}
		}
		else
		{
			ffx[i + 1] = abs(pixelData[i + 1] - pixelData[i]);
			iSumfx += ffx[i + 1];
		}
	}

	for (i = 0; i < pCnt - 1; i++)
	{
		if (nStep == 2)
		{
			if (i < pCnt - 2)
			{
				dLoc += (double(i + 1.0) * double(ffx[i + 1]) / double(iSumfx));
				//	    dLoc +=        X     *               W(i)
			}
		}
		else
		{
			if (iSumfx > 0)
				dLoc += (double(i + 1.0) * double(ffx[i + 1]) / double(iSumfx));
		}
	}

	return dLoc;

}

//
// Subpixel을 이용하여 Corner Edge를 구한다.
//
int CImgProcEngine::SubPixelCorner(IplImage *src, vector<Point2f> &points)
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
double CImgProcEngine::angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

#if 0
int CImgProcEngine::find_threshold(IplImage* image)
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
int CImgProcEngine::find_thresholdOTSU(IplImage* image)
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
	double otsuThreshold= cvThreshold(im, finalIm, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU );

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

//
// 입력된 흑백 이미지 Thinner 처리
//
void CImgProcEngine::Thinner(RoiObject *pData, IplImage* grayImg, int nDbg)
{
	QString str;
	Mat1b m = Mat1b(cvarrToMat(grayImg).clone());

	IplImage *trans = NULL;
	VoronoiThinner thinner;
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

void CImgProcEngine::SaveOutImage(IplImage* pImgOut, RoiObject *pData, QString strMsg, bool bClear/*=false*/)
{
	QString str = ("");
	if (pData != NULL)
        str.sprintf("%s/[%s]%s_%s", m_sDebugPath.toStdString().c_str(), pData->groupName().toStdString().c_str(), pData->name().toStdString().c_str(), strMsg.toStdString().c_str());
	else
        str.sprintf("%s/%s", m_sDebugPath.toStdString().c_str(), strMsg.toStdString().c_str());
    cvSaveImage((const char *)str.toStdString().c_str(), pImgOut);
	if (bClear) cvZero(pImgOut);
}

//void CImgProcEngine::SaveOutImage(IplImage* pImgOut, QString strMsg, bool bClear/*=false*/)
//{
//	QString str = ("");
//	str.sprintf(("%s\\%s"), m_sDebugPath.toStdString().c_str(), strMsg);
//	CT2A ascii(str); cvSaveImage(ascii, pImgOut);
//	if (bClear) cvZero(pImgOut);
//}

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

void CImgProcEngine::AffineTransform(std::vector<Point2f> vec, cv::Point2f srcTri[], cv::Point2f dstTri[])
{
	Mat m(2, 3, CV_32F);
	m = getAffineTransform(srcTri, dstTri);
	transform(vec, vec, m);
}

void CImgProcEngine::WarpPerspectiveImage(IplImage* img, CvPoint2D32f srcTri[], CvPoint2D32f dstTri[])
{
	CvMat* warp_mat = cvCreateMat(3, 3, CV_32FC1);

	cvGetPerspectiveTransform(srcTri, dstTri, warp_mat);
	cvWarpPerspective(img, img, warp_mat);

	cvReleaseMat(&warp_mat);
}

void CImgProcEngine::WarpAffineImage(IplImage* img, CvPoint2D32f srcTri[], CvPoint2D32f dstTri[])
{
	CvMat* warp_mat = cvCreateMat(2, 3, CV_32FC1);

	cvGetAffineTransform(srcTri, dstTri, warp_mat); // 트랜스폼 매트릭스 생성 from SrcTri,dstTri
	cvWarpAffine(img, img, warp_mat);

	cvReleaseMat(&warp_mat);
}

void CImgProcEngine::RotateImage(IplImage* img, double angle, CvPoint2D32f center, double scale)
{
	CvMat* warp_mat = cvCreateMat(2, 3, CV_32FC1);
	//회전행렬 계산
	cv2DRotationMatrix(center, angle, scale, warp_mat); //중심점, 회전각도, 스케일을 직접 입력;
	cvWarpAffine(img, img, warp_mat);
	cvReleaseMat(&warp_mat);
}

// 중심점과 회전각도를 이용하여 이미지 변환 : 패턴매칭에서 Align에 이용
void CImgProcEngine::RotateImage(IplImage* img, double angle, CvPoint2D32f center)
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
void CImgProcEngine::RotateImage(IplImage* img, double angle)
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


void CImgProcEngine::DrawResultCrossMark(IplImage *iplImage, RoiObject *pData)
{
	if (iplImage == NULL) return;
    //if (gCfg.m_bSaveGrabImg == false) return;

	int size = pData->m_vecDetectResult.size();
	for (int i = 0; i < size; i++) {
		DetectResult *prst = &pData->m_vecDetectResult[i];
        qDebug() << "DrawResultCrossMark" << prst->pt.x << prst->pt.y;

        double x = prst->pt.x + pData->bounds().x();// / gCfg.m_pCamInfo[0].dResX;
        double y = prst->pt.y + pData->bounds().y();// / gCfg.m_pCamInfo[0].dResY;

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

//
// AutoFocus를 위한 함수들
//

bool CImgProcEngine::checkForBurryImage(cv::Mat matImage)
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

	int soglia = -6118750;

	pixels = NULL;
	finalImage.release();

	bool isBlur = (maxLap < kBlurThreshhold) ? true : false;
	return isBlur;
}

double CImgProcEngine::rateFrame(Mat frame)
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

double CImgProcEngine::correctFocus(FocusState & state, double rate)
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


// Blob의 중심 구하기
Point2f CImgProcEngine::FindCenterOfBlob(IplImage * pImage)
{
	Point2f point;
	CvSeq* m_seq = 0, *ptseq = 0;
	int mode = CV_RETR_LIST;
	int method = CV_CHAIN_APPROX_SIMPLE;

	CvMemStorage* storage = cvCreateMemStorage(0);
	cvFindContours(pImage, storage, &m_seq, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	if (m_seq != 0)
	{
		CvSeq* m_approxDP_seq = cvApproxPoly(m_seq, sizeof(CvContour), storage, CV_POLY_APPROX_DP, 0, 1); // 3->0
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
