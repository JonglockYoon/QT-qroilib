// imgprocengine.cpp
//
// Qroilib를 이용할때 이용가능한 opencv함수들을 모아놓은 module이다.
// application개발시 필요한 함수들만 다시 구성해서 새로운 engine module을 만드는것이
// 유지보수에 이롭다.
//
#include <stdio.h>
#include <QDir>
#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>

#include "imgprocengine.h"
#include "objectgroup.h"
//#include "config.h"
#include "recipedata.h"
#include "mainwindow.h"


#define Cam_ResX 0.007
#define Cam_ResY 0.007

//using namespace cv;

CImgProcEngine::CImgProcEngine()
{
    qDebug() << "CImgProcEngine";

    memset(&m_DetectResult, 0, sizeof(m_DetectResult));
    //m_sDebugPath = ".";
    m_bSaveEngineImg = true;

    QString str;

    //CConfig *pCfg = &gCfg;

    str = QString("./Engine");//.arg(gCfg.m_sSaveImageDir);
    QDir dir;
    dir.mkdir(str);
    m_sDebugPath = str;
    curImg = nullptr;

}

CImgProcEngine::~CImgProcEngine(void)
{
}

int CImgProcEngine::InspectOneItem(IplImage* img, RoiObject *pData)
{
    QString str;
    str.sprintf("InspectOneItem type=%d", pData->mInspectType);
    theMainWindow->DevLogSave(str.toLatin1().data());
	m_DetectResult.dRadius = 0;
    m_DetectResult.ngBlobImg = nullptr;

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
        str.sprintf(("0_%d_grayImage.jpg"), 110);
        SaveOutImage(graySearchImg, pData, str, false);
	}

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

	switch (pData->mInspectType)
	{

    case _Inspect_Roi_EdgeWithThreshold:
        SingleROIEdgeWithThreshold(croppedImage, pData, rect);
		break;
    case _Inspect_Roi_SubpixelEdge:
        SingleROISubPixelEdge(croppedImage, pData, rect);
        break;
    case _Inspect_Roi_Corner:
        SingleROICorner(croppedImage, pData, rect);
        break;

    }
	cvReleaseImage(&croppedImage);
	cvReleaseImage(&graySearchImg);

	return 0;
}

//
// Threshold를 이용하여 Edge의 경계면을 추출한다.
//
double CImgProcEngine::SingleROIEdgeWithThreshold(IplImage* croppedImage, RoiObject *pData, QRectF rect)
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

    int nDir = 0;
    pParam = pData->getParam(("Direction")); // ("Left2Right,Right2Left,Top2Bottom,Bottom2Top")),
    if (pParam)
        nDir = (int)pParam->Value.toDouble();

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

	cvReleaseImage(&grayImg);

    return dVal;
}

double CImgProcEngine::SingleROISubPixelEdge(IplImage* croppedImage, Qroilib::RoiObject *pData, QRectF rect)
{
    CParam *pParam;
    int nDir = 0;
    pParam = pData->getParam(("Direction")); // ("Left2Right,Right2Left,Top2Bottom,Bottom2Top")),
    if (pParam)
        nDir = (int)pParam->Value.toDouble();


    double d = SubPixelRampEdgeImage(croppedImage, nDir);

    return d;
}

bool CImgProcEngine::SetROIAreaForCriteriaPosition(RoiObject *pData, QString strCriteriaROI)
{
    Q_UNUSED(strCriteriaROI);
	QString str;
    bool bFoundCriteriaPosition = false;
    double dCriteriaX = 0.0;
    double dCriteriaY = 0.0;

    ViewMainPage *viewMain = (ViewMainPage *)theMainWindow->viewMainPage();
    Qroilib::DocumentView* v = viewMain->currentView();
    if (v != nullptr) {
        RoiObject *pCriData = nullptr;
        for (const Layer *layer : v->mRoi->objectGroups()) {
            const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
            for (const Qroilib::RoiObject *roiObject : objectGroup) {
                pCriData = (RoiObject *)roiObject;
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
    const double dResX = Cam_ResX;//gCfg.m_pCamInfo[0].dResX;
    const double dResY = Cam_ResY;//gCfg.m_pCamInfo[0].dResY;

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

    if (nRet2 == 0)	// Blob처리를 한 Corner 찾기
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

    NoiseOut(pData, graySearchImg, -1, 312);
    Expansion(pData, graySearchImg, -1, 313);

    CBlobResult blobs;
    blobs = CBlobResult(graySearchImg, nullptr);
    int n = blobs.GetNumBlobs();

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

    cvZero(graySearchImg);
    for (int i = 0; i < blobs.GetNumBlobs(); i++) {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(graySearchImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
    }

    if (m_bSaveEngineImg){
        SaveOutImage(graySearchImg, pData, ("316_imageLargeBlobs.jpg"));
    }

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
    PostExpansion(pData, graySearchImg, -1, 319);

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

    NoiseOut(pData, grayCroppedImg, -1, 212);
    Expansion(pData, grayCroppedImg, -1, 213);
    FilterLargeArea(grayCroppedImg);

    CBlobResult blobs;
    blobs = CBlobResult(grayCroppedImg, nullptr);
    int NumOfBlob = blobs.GetNumBlobs();
    if (NumOfBlob < 1) {
        if (grayCroppedImg) cvReleaseImage(&grayCroppedImg);
        return -1;
    }
    CBlob *p = blobs.GetBlob(0);
    CvRect rect = p->GetBoundingBox();

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

    if (grayCroppedImg) cvReleaseImage(&grayCroppedImg);
    if (workImg) cvReleaseImage(&workImg);
    return ret;
}

//
//
RoiObject *CImgProcEngine::GetCriteriaROI(RoiObject *pData, QString title)
{
	CParam *pParam = pData->getParam(title);
	if (pParam) {
        ViewMainPage *viewMain = (ViewMainPage *)theMainWindow->viewMainPage();
        Qroilib::DocumentView* v = viewMain->currentView();
        if (v == nullptr)
            return nullptr;

        int n = pParam->Value.toDouble();
		if (n > 0) { // 기준Mask ROI가 설정되어 있다.
            if (0 == pParam->m_vecDetail.size()){
                pParam->m_vecDetail.push_back(("No Parent/Criteria Roi"));

                RoiObject *pdata = nullptr;
                for (const Layer *layer : v->mRoi->objectGroups()) {
                    const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
                    for (const Qroilib::RoiObject *roiObject : objectGroup) {
                        pdata = (RoiObject *)roiObject;
                        if (pdata->groupName() == pData->groupName())
                        {
                            if (pParam->Value == pdata->name()) {
                                //QString str;
                                //str.sprintf(("%d"), i + 1);
                                //pParam->Value = str;
                            }
                            pParam->m_vecDetail.push_back(pdata->name());
                        }
                    }
                }

            }

			if (0 < pParam->m_vecDetail.size()){
                QString strCriteriaROI = pParam->m_vecDetail[n]; // str이 기준 Mask ROI 이다.
                if (strCriteriaROI != pData->name().toStdString().c_str()) {
                    RoiObject *pCriData = nullptr;
                    for (const Layer *layer : v->mRoi->objectGroups()) {
                        const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
                        for (const Qroilib::RoiObject *roiObject : objectGroup) {
                            pCriData = (RoiObject *)roiObject;

                            if (pCriData->isVisible() && pCriData->name() == strCriteriaROI) {
                                return pCriData;
                            }
                        }
                    }
				}
			}
		}
	}

    return nullptr;
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

//
// 사용자가 설정한 값으로 Threshold를 실행
//
int CImgProcEngine::Threshold(RoiObject *pData, IplImage* grayImg, int nDbg)
{
    QString str;

    int nThresholdValue = 70;
    int nThresholdMaxVal = 255;
    int bInvert = 0;
    if (pData != nullptr) {
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

    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Low Threshold"));
        if (pParam)
            nThresholdLowValue = pParam->Value.toDouble();
        pParam = pData->getParam(("High Threshold"));
        if (pParam)
            nThresholdHighValue = pParam->Value.toDouble();
        pParam = pData->getParam(("Invert?"));
        if (pParam)
            bInvert = (int)pParam->Value.toDouble();
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


int CImgProcEngine::PostNoiseOut(RoiObject *pData, IplImage* grayImg, int t, int nDbg, int h)
{
    QString str;

    if (t < 0)
        t = _PostProcessValue1;
    //IplImage* tmp = cvCreateImage(cvGetSize(grayImg), 8, 1);

    // 노이즈 제거 필터
    int filterSize = 3;  // 필터의 크기를 3으로 설정 (Noise out area)
    //if (pData != nullptr) {
    //	CParam *pParam = pData->getParam(("Noise out area"));
    //	if (pParam)
    //		filterSize = pParam->Value.toDouble();
    //}
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

        FilterLargeArea(grayImg);
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

        FilterLargeArea(grayImg);
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

int CImgProcEngine::PostExpansion(RoiObject *pData, IplImage* grayImg, int t, int nDbg, int h)
{
    Q_UNUSED(h);
    QString str;

    if (t < 0)
        t = _PostProcessValue1;
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

    FilterLargeArea(grayImg);

    if (nExpansion2 < 0)
        cvErode(grayImg, grayImg, nullptr, -nExpansion2);
    else //if (nExpansion > 0)
        cvDilate(grayImg, grayImg, nullptr, nExpansion2);

    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvExpansion.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }

    FilterLargeArea(grayImg);

    //cvReleaseImage(&tmp);
    return 0;
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

        //double x = prst->pt.x;
        //double y = prst->pt.y;

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

void CImgProcEngine::SaveOutImage(IplImage* pImgOut, RoiObject *pData, QString strMsg, bool bClear/*=false*/)
{
//    if (!gCfg.m_bSaveEngineImg)
//        return;
    QString str = ("");
    if (pData != nullptr)
        str.sprintf("%s/[%s]%s_%s", m_sDebugPath.toStdString().c_str(), pData->groupName().toStdString().c_str(), pData->name().toStdString().c_str(), strMsg.toStdString().c_str());
    else
        str.sprintf("%s/%s", m_sDebugPath.toStdString().c_str(), strMsg.toStdString().c_str());
    cvSaveImage((const char *)str.toStdString().c_str(), pImgOut);
    if (bClear) cvZero(pImgOut);

    //qDebug() << "SaveOutImage:" << str;
}

const int WinSize = 37;
const int AreaSize = 7;

int CImgProcEngine::AdaptiveThreshold(IplImage* grayImg, IplImage* outImg)
{
    QString str;
    int nBlkSize = WinSize;//Local variable binarize window size
    int C = AreaSize;//Local variable binarize threshold

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
    cvCopy(&iplImage, outImg);

    return 0;
}

int CImgProcEngine::MakeMask(RoiObject *pData, IplImage* grayImg, int nDbg)
{
    QString str;
    int width = grayImg->width;
    int height = grayImg->height;

    AdaptiveThreshold(grayImg, grayImg);

    int filterSize = 3;
    IplConvKernel *element = nullptr;
    element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, nullptr);
    cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, 1);
    cvReleaseStructuringElement(&element);

    if (m_bSaveEngineImg){
        str.sprintf(("%03d_Mask1.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }

    CBlobResult blobs;
    blobs = CBlobResult(grayImg, nullptr);
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++)
    {
        CBlob *p = blobs.GetBlob(i);
        CvRect r = p->GetBoundingBox();

        // 경계와 걸쳐 있는 Blob들을 삭제.
        if (r.x == 0 || r.y == 0)
            p->ClearContours();
        else if (r.width+r.x == width)
            p->ClearContours();
        else if (r.height+r.y == height)
            p->ClearContours();

        // Area가 500이하의 Blob들을 삭제.
        double area = p->Area();
        if (area < 500.0)
            p->ClearContours();
    }
    nBlobs = blobs.GetNumBlobs();
    cvZero(grayImg);
    for (int i = 0; i < nBlobs; i++)
    {
        CBlob *p = blobs.GetBlob(i);
        if (p->IsEmpty()) // Deleted blob.
            continue;
        CvRect r = p->GetBoundingBox();
        cvDrawRect(grayImg, CvPoint(r.x, r.y),CvPoint(r.x+r.width, r.y+r.height), CvScalar(255,255,255), CV_FILLED);
    }
    //cvDilate(grayImg, grayImg, nullptr, 2);

    if (m_bSaveEngineImg){
        str.sprintf(("%03d_Mask2.jpg"), nDbg+1);
        SaveOutImage(grayImg, pData, str, false);
    }

    return 0;
}

int CImgProcEngine::MeasureByCannyEdge(RoiObject *pData, IplImage* grayImg)
{
    QString str;
    try {
        cvSmooth(grayImg, grayImg, CV_GAUSSIAN, 7, 7);
    } catch (...) {
        qDebug() << "Error cvSmooth()";
        return -1;
    }

    cvCanny(grayImg, grayImg, 11, 25);

    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* contours = 0;
    cvFindContours(grayImg, storage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    IplImage* tmp = cvCloneImage(grayImg);
    cvZero(tmp);
    while (contours)
    {
         CvSeq *c = contours;
         for (int i = 0; i < c->total; i++)
         {
             CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, c, i);
         }
         contours = contours->h_next;

         //Scalar color = Scalar( 255,255,255);
         //cvDrawContours(tmp, contours, color, color, 2, 1, 8, cvPoint(0, 0));
     }
     cvReleaseMemStorage(&storage);

#if 0

     vector<Point> ConvexHullPoints =  contoursConvexHull(contours);

     polylines( drawing, ConvexHullPoints, true, Scalar(0,0,255), 2 );
     imshow("Contours", drawing);

     polylines( src, ConvexHullPoints, true, Scalar(0,0,255), 2 );
#endif

}

int CImgProcEngine::MeasureBySubpixelEdge(RoiObject *pData, IplImage* grayImg, CvPoint2D32f &pt1, CvPoint2D32f &pt2)
{
    //x : 50 ~ 60
    //y : 40 ~ 150

    IplImage* croppedImage;

    cvSetImageROI(grayImg, cvRect(0, 40, 12, 100));
    croppedImage = cvCreateImage(cvSize(12, 100), grayImg->depth, grayImg->nChannels);
    cvCopy(grayImg, croppedImage);
    cvResetImageROI(grayImg);
    pt1.x = SubPixelRampEdgeImage(croppedImage, 0); // Left2Right
    cvReleaseImage(&croppedImage);

    cvSetImageROI(grayImg, cvRect(grayImg->width-12, 40, 12, 100));
    croppedImage = cvCreateImage(cvSize(12, 100), grayImg->depth, grayImg->nChannels);
    cvCopy(grayImg, croppedImage);
    cvResetImageROI(grayImg);
    pt2.x = SubPixelRampEdgeImage(croppedImage, 1); // Right2Left
    pt2.x += (grayImg->width-12);
    cvReleaseImage(&croppedImage);

    cvSetImageROI(grayImg, cvRect(50, 0, 10, 14));
    croppedImage = cvCreateImage(cvSize(10, 14), grayImg->depth, grayImg->nChannels);
    cvCopy(grayImg, croppedImage);
    cvResetImageROI(grayImg);
    pt1.y = SubPixelRampEdgeImage(croppedImage, 2); // Top2Bottom
    cvReleaseImage(&croppedImage);

    cvSetImageROI(grayImg, cvRect(50, grayImg->height-14, 10, 14));
    croppedImage = cvCreateImage(cvSize(10, 14), grayImg->depth, grayImg->nChannels);
    cvCopy(grayImg, croppedImage);
    cvResetImageROI(grayImg);
    pt2.y = SubPixelRampEdgeImage(croppedImage, 3); // Bottom2Top
    pt2.y += (grayImg->height-14);
    cvReleaseImage(&croppedImage);

    return 0;
}

int CImgProcEngine::MeasureLCDPixelSize(RoiObject *pData, IplImage* iplImg)
{    
    QString str;
    IplImage* croppedImage;
    QRectF rect = pData->bounds();	// Area로 등록된 ROI.
    rect.normalized();

    if (rect.left() < 0)	rect.setLeft(0);
    if (rect.top() < 0)	rect.setTop(0);
    if (rect.right() >= iplImg->width) rect.setRight(iplImg->width);
    if (rect.bottom() >= iplImg->height) rect.setBottom(iplImg->height);
    pData->setBounds(rect);

    cv::Point2f left_top = cv::Point2f(rect.left(), rect.top());
    cvSetImageROI(iplImg, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
    croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), iplImg->depth, iplImg->nChannels);
    cvCopy(iplImg, croppedImage);
    cvResetImageROI(iplImg);

    if (m_bSaveEngineImg){
        str.sprintf(("%03d_Source.jpg"), 100);
        SaveOutImage(croppedImage, pData, str, false);
    }

    // 마스크를 생성합니다.
    IplImage* mask = cvCloneImage(croppedImage);
    MakeMask(pData, mask, 200);

#define LM 6
#define RM 4

    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);

    // nBlobs 만큼의 Thread를 생성해서 동작시켜면 CPU의 효율을 최대로 활용할수 있습니다.
    // 너무 많은 Thread를 생성하면 역 효과가 나타날수 있으므로,
    // 사용하는 System의 성능에 따라 생성하는 Thread의 갯수는 제한 하는것이 바람직합니다.
    CBlobResult blobs;
    blobs = CBlobResult(mask, nullptr);
    int nBlobs = blobs.GetNumBlobs();
    IplImage* tmp;
    for (int i = 0; i < nBlobs; i++)
    {
        CBlob *p = blobs.GetBlob(i);
        CvRect r = p->GetBoundingBox();
        r.x -= LM;
        r.y -= LM;
        r.width += (LM+RM);
        r.height += (LM*2);

        cvSetImageROI(croppedImage, cvRect(r.x, r.y, r.width, r.height));
        tmp = cvCreateImage(cvSize(r.width, r.height), croppedImage->depth, croppedImage->nChannels);
        cvCopy(croppedImage, tmp);
        cvResetImageROI(croppedImage);


        if (m_bSaveEngineImg){
            str.sprintf(("%03d_Pixel.jpg"), 300+i);
            SaveOutImage(tmp, pData, str, false);
        }

        CvPoint2D32f pt1;
        CvPoint2D32f pt2;
        MeasureBySubpixelEdge(pData, tmp, pt1, pt2);

        cv:Mat m = cvarrToMat(iplImg);
        pt1.x += (r.x + rect.left());
        pt1.y += (r.y + rect.top());
        pt2.x += (r.x + rect.left());
        pt2.y += (r.y + rect.top());
        cv::arrowedLine(m, pt1, pt2, CV_RGB(0, 255, 0), 1, 8, 0, 0.05);
        cv::arrowedLine(m, pt2, pt1, CV_RGB(0, 255, 0), 1, 8, 0, 0.05);

        QString text;
        text.sprintf("%.2f", pt2.x-pt1.x);
        cvPutText(iplImg, text.toLatin1(), cvPoint(pt1.x, (pt1.y+pt2.y)/2-14), &font, cvScalar(0, 0, 0, 0));
        text.sprintf("%.2f", pt2.y-pt1.y);
        cvPutText(iplImg, text.toLatin1(), cvPoint(pt1.x, (pt1.y+pt2.y)/2+14), &font, cvScalar(0, 0, 0, 0));


        MeasureByCannyEdge(pData, tmp);
        if (m_bSaveEngineImg){
            str.sprintf(("%03d_PixelCanny.jpg"), 400+i);
            SaveOutImage(tmp, pData, str, false);
        }

        if (tmp) cvReleaseImage(&tmp);
    }

    if (mask) cvReleaseImage(&mask);

    //cvShowImage("iplImg", iplImg);
    theMainWindow->outWidget("result", iplImg);

     return 0;
}

/*

import cv2
import numpy as np

def find_if_close(cnt1,cnt2):
    row1,row2 = cnt1.shape[0],cnt2.shape[0]
    for i in xrange(row1):
        for j in xrange(row2):
            dist = np.linalg.norm(cnt1[i]-cnt2[j])
            if abs(dist) < 50 :
                return True
            elif i==row1-1 and j==row2-1:
                return False

img = cv2.imread('dspcnt.jpg')
gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
ret,thresh = cv2.threshold(gray,127,255,0)
contours,hier = cv2.findContours(thresh,cv2.RETR_EXTERNAL,2)

LENGTH = len(contours)
status = np.zeros((LENGTH,1))

for i,cnt1 in enumerate(contours):
    x = i
    if i != LENGTH-1:
        for j,cnt2 in enumerate(contours[i+1:]):
            x = x+1
            dist = find_if_close(cnt1,cnt2)
            if dist == True:
                val = min(status[i],status[x])
                status[x] = status[i] = val
            else:
                if status[x]==status[i]:
                    status[x] = i+1

unified = []
maximum = int(status.max())+1
for i in xrange(maximum):
    pos = np.where(status==i)[0]
    if pos.size != 0:
        cont = np.vstack(contours[i] for i in pos)
        hull = cv2.convexHull(cont)
        unified.append(hull)

cv2.drawContours(img,unified,-1,(0,255,0),2)
cv2.drawContours(thresh,unified,-1,255,-1)

 */

/*

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

using namespace cv;
using namespace std;

vector<Point> contoursConvexHull( vector<vector<Point> > contours )
{
    vector<Point> result;
    vector<Point> pts;
    for ( size_t i = 0; i< contours.size(); i++)
        for ( size_t j = 0; j< contours[i].size(); j++)
            pts.push_back(contours[i][j]);
    convexHull( pts, result );
    return result;
}

int main( int, char** argv )
{
    Mat src, srcGray,srcBlur,srcCanny;

    src = imread( argv[1], 1 );
    cvtColor(src, srcGray, CV_BGR2GRAY);
    blur(srcGray, srcBlur, Size(3, 3));

    Canny(srcBlur, srcCanny, 0, 100, 3, true);

    vector<vector<Point> > contours;

    findContours( srcCanny, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

    Mat drawing = Mat::zeros(srcCanny.size(), CV_8UC3);

    for (int i = 0; i< contours.size(); i++)
    {
        Scalar color = Scalar( 255,255,255);
        drawContours( drawing, contours, i, color, 2 );
    }

    vector<Point> ConvexHullPoints =  contoursConvexHull(contours);

    polylines( drawing, ConvexHullPoints, true, Scalar(0,0,255), 2 );
    imshow("Contours", drawing);

    polylines( src, ConvexHullPoints, true, Scalar(0,0,255), 2 );
    imshow("contoursConvexHull", src);
    waitKey();
    return 0;
}

 */
