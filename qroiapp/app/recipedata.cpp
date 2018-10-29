
#include <QRectF>
#include <opencv2/core.hpp>
#include <QUndoStack>
#include <QApplication>

#include "mainwindow.h"
#include "documentview/documentview.h"
#include "viewmainpage.h"
#include "roiobject.h"
#include "addremoveroiobject.h"
#include "recipedata.h"
#include "common.h"
#include "roireader.h"
#include "config.h"
#include "savefile.h"

using namespace Qroilib;

CRecipeData * g_cRecipeData = NULL;
Qroilib::ParamTable paramTable[] = {
    _Inspect_Patt_Start,   CParam(_ProcessValue1, (""), _IntValue, ("0")), // do not delete.

    _Inspect_Patt_VerticalAlign,  CParam(_ProcessValue1, ("Pattern matching rate"), _DoubleValue, ("60")),
    _Inspect_Patt_VerticalAlign,  CParam(_ProcessValue1, ("Low Threshold"), _IntValue, ("120")),
    _Inspect_Patt_VerticalAlign,  CParam(_ProcessValue1, ("High Threshold"), _IntValue, ("255")),
    _Inspect_Patt_VerticalAlign,  CParam(_ProcessValue1, ("Noise out 1"), _IntValue, ("-2")),	// -1 : Open - 작은 White blob 들을 없앤다
    _Inspect_Patt_VerticalAlign,  CParam(_ProcessValue1, ("Noise out 2"), _IntValue, ("2")),	// 1 : Close - White blob 들을 묶는다.
    _Inspect_Patt_VerticalAlign,  CParam(_ProcessValue1, ("Shape matching rate"), _DoubleValue, ("70")),
    _Inspect_Patt_VerticalAlign,  CParam(_PriorityValue, ("Priority"), _IntValue, ("5")),


    _Inspect_Patt_HorizontalAlign,  CParam(_ProcessValue1,  ("Pattern matching rate"), _DoubleValue, ("60")),
    _Inspect_Patt_HorizontalAlign,  CParam(_PriorityValue, ("Priority"), _IntValue, ("5")),

    _Inspect_Patt_Identify,  CParam(_ProcessValue1, ("Pattern matching rate"), _DoubleValue, ("60")),
    _Inspect_Patt_Identify,  CParam(_ProcessValue1, ("Low Threshold"), _IntValue, ("0")),
    _Inspect_Patt_Identify,  CParam(_ProcessValue1, ("High Threshold"), _IntValue, ("100")),
    _Inspect_Patt_Identify,  CParam(_ProcessValue1, ("Noise out 1"), _IntValue, ("5")),	// -1 : Open - 작은 White blob 들을 없앤다.
    _Inspect_Patt_Identify,  CParam(_ProcessValue1, ("Noise out 2"), _IntValue, ("-5")),	// 1 : Close - White blob 들을 묶는다..
    //_Inspect_Patt_Identify,  CParam(_ProcessValue1, ("Expansion 1"), _IntValue, ("0")),	// -1 : Erode -
    //_Inspect_Patt_Identify,  CParam(_ProcessValue1, ("Expansion 2"), _IntValue, ("0")),	// 1 : Dilate - 팽창연산.
    _Inspect_Patt_Identify,  CParam(_ProcessValue1, ("Noise out 3"), _IntValue, ("0")),	// -1 : Open - 작은 White blob 들을 없앤다.
    _Inspect_Patt_Identify,  CParam(_ProcessValue1, ("Shape matching rate"), _DoubleValue, ("70")),
    //_Inspect_Patt_Identify,  CParam(_ProcessValue1, ("Matching after Binarized"), _ComboValue, ("0"), ("No, Yes")),
    _Inspect_Patt_Identify,  CParam(_ProcessValue1, ("Filter blob"), _ComboValue, ("2"), ("None,LargeArea,LargeDiameter")),
    //_Inspect_Patt_Identify,  CParam(_LightValue, ("Light"), _IntValue, ("50")),

    _Inspect_Patt_MatchShapes,  CParam(_ProcessValue1, ("Low Threshold"), _IntValue, ("0")),
    _Inspect_Patt_MatchShapes,  CParam(_ProcessValue1, ("High Threshold"), _IntValue, ("100")),
    _Inspect_Patt_MatchShapes,  CParam(_ProcessValue1, ("Noise out 1"), _IntValue, ("1")),	// -1 : Open - 작은 White blob 들을 없앤다
    _Inspect_Patt_MatchShapes,  CParam(_ProcessValue1, ("Noise out 2"), _IntValue, ("-1")),	// 1 : Close - White blob 들을 묶는다.
    _Inspect_Patt_MatchShapes,  CParam(_ProcessValue1, ("Shape matching rate"), _DoubleValue, ("85")),
    //_Inspect_Patt_MatchShapes,  CParam(_PriorityValue, ("Priority"), _IntValue, ("5")),

    _Inspect_Roi_Start,   CParam(_ProcessValue1, (""), _IntValue, ("0")), // do not delete.

    //_Inspect_Roi_MeasureAlign,  CParam(_ProcessValue1, ("Measurement type"), _ComboValue, ("0"), ("SubpixelEdge,SubpixelEdgeWithThreshold,PeakEdge")),
    _Inspect_Roi_MeasureAlign,  CParam(_ProcessValue1, ("Direction"), _ComboValue, ("0"), ("Left2Right,Right2Left,Top2Bottom,Bottom2Top,CenterPoint")),
    _Inspect_Roi_MeasureAlign,  CParam(_ProcessValue1, ("Detect method"), _ComboValue, ("0"), ("Average, First")),
    _Inspect_Roi_MeasureAlign,  CParam(_ProcessValue1, ("Polarity"), _ComboValue, ("0"), ("White2Black, Black2White")),
    _Inspect_Roi_MeasureAlign,  CParam(_ProcessValue1, ("Low Threshold"), _IntValue, ("200")),
    _Inspect_Roi_MeasureAlign,  CParam(_ProcessValue1, ("High Threshold"), _IntValue, ("255")),
    _Inspect_Roi_MeasureAlign,  CParam(_ProcessValue1, ("Noise out 1"), _IntValue, ("-2")),	// -1 : Open - 작은 White blob 들을 없앤다
    _Inspect_Roi_MeasureAlign,  CParam(_ProcessValue1, ("Noise out 2"), _IntValue, ("2")),	// 1 : Close - White blob 들을 묶는다.
    _Inspect_Roi_MeasureAlign,  CParam(_ProcessValue1, ("Ramp width"), _IntValue, ("6")),
    _Inspect_Roi_MeasureAlign,  CParam(_ProcessValue1, ("Detect method"), _ComboValue, ("0"), ("Average, First")),
    _Inspect_Roi_MeasureAlign,  CParam(_PriorityValue, ("Priority"), _IntValue, ("5")),

    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Direction"), _ComboValue, ("0"), ("Left2Right,Right2Left,Top2Bottom,Bottom2Top")),
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Polarity"), _ComboValue, ("0"), ("White2Black,Black2White")),
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Low Threshold"), _IntValue, ("50")),
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("High Threshold"), _IntValue, ("255")),
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Large Blob?"), _ComboValue, ("0"), ("No,Yes")),
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Invert?"), _ComboValue, ("0"), ("No,Yes")),
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Noise out 1"), _IntValue, ("3")),	// -1 : Open - 작은 White blob 들을 없앤다
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Noise out 2"), _IntValue, ("0")),	// 1 : Close - White blob 들을 묶는다.
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Expansion 1"), _IntValue, ("0")),	// -1 : Erode - 침식연산
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Expansion 2"), _IntValue, ("0")),	// 1 : Dilate - 팽창연산
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Ramp width"), _IntValue, ("6")),
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Detect method"), _ComboValue, ("0"), ("Average,First")),
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Criteria position ROI"), _ComboValue, (""), ("${ROI}")), //${ROI} 는 ROI영역들의 이름이 나열된다.
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Criteria direction"), _ComboValue, ("1"), ("Horizontal,Veritical")),
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_ProcessValue1, ("Criteria offset(mm)"), _DoubleValue, ("-0.1")), // mm단위
    _Inspect_Roi_SubpixelEdgeWithThreshold,  CParam(_PriorityValue, ("Priority"), _IntValue, ("10")),

    _Inspect_Roi_Circle_With_Threshold,  CParam(_ProcessValue1, ("Low Threshold"), _IntValue, ("0")),
    _Inspect_Roi_Circle_With_Threshold,  CParam(_ProcessValue1, ("High Threshold"), _IntValue, ("60")),
    _Inspect_Roi_Circle_With_Threshold,  CParam(_ProcessValue1, ("Noise out 1"), _IntValue, ("3")),	// -1 : Open - 작은 White blob 들을 없앤다
    _Inspect_Roi_Circle_With_Threshold,  CParam(_ProcessValue1, ("Noise out 2"), _IntValue, ("0")),	// 1 : Close - White blob 들을 묶는다.
    _Inspect_Roi_Circle_With_Threshold,  CParam(_ProcessValue1, ("Maximum Threshold Canny"), _DoubleValue, ("200")),
    _Inspect_Roi_Circle_With_Threshold,  CParam(_ProcessValue1, ("Minimum circle radius"), _DoubleValue, ("10")),
    _Inspect_Roi_Circle_With_Threshold,  CParam(_ProcessValue1, ("Maximum circle radius"), _DoubleValue, ("1000")),

    _Inspect_Roi_CenterOfPlusMark,  CParam(_ProcessValue1, ("Low Threshold"), _IntValue, ("0")),
    _Inspect_Roi_CenterOfPlusMark,  CParam(_ProcessValue1, ("High Threshold"), _IntValue, ("150")),
    _Inspect_Roi_CenterOfPlusMark,  CParam(_ProcessValue1, ("Invert?"), _ComboValue, ("0"), ("No,Yes")),
    _Inspect_Roi_CenterOfPlusMark,  CParam(_ProcessValue1, ("Noise out 1"), _IntValue, ("1")),	// -1 : Open - 작은 White blob 들을 없앤다
    _Inspect_Roi_CenterOfPlusMark,  CParam(_ProcessValue1, ("Noise out 2"), _IntValue, ("-1")),	// 1 : Close - White blob 들을 묶는다.침식연산
    _Inspect_Roi_CenterOfPlusMark,  CParam(_PostProcessValue1, ("Expansion 1"), _IntValue, ("3")),	// -1 : Erode -
    _Inspect_Roi_CenterOfPlusMark,  CParam(_PostProcessValue1, ("Expansion 2"), _IntValue, ("0")),	// 1 : Dilate - 팽창연산
    _Inspect_Roi_CenterOfPlusMark,  CParam(_DecideValue, ("Minimum circle radius"), _DoubleValue, ("30")),
    _Inspect_Roi_CenterOfPlusMark,  CParam(_DecideValue, ("Maximum circle radius"), _DoubleValue, ("70")),
    _Inspect_Roi_CenterOfPlusMark,  CParam(_PriorityValue, ("Priority"), _IntValue, ("10")),
    _Inspect_Roi_CenterOfPlusMark,  CParam(_LightValue, ("Light"), _IntValue, ("50")),

    _Inspect_Roi_Find_Shape,  CParam(_ProcessValue1, ("Low Threshold"), _IntValue, ("0")),
    _Inspect_Roi_Find_Shape,  CParam(_ProcessValue1, ("High Threshold"), _IntValue, ("200")),
    _Inspect_Roi_Find_Shape,  CParam(_ProcessValue1, ("Noise out 1"), _IntValue, ("1")),	// -1 : Open - 작은 White blob 들을 없앤다.
    _Inspect_Roi_Find_Shape,  CParam(_ProcessValue1, ("Noise out 2"), _IntValue, ("-1")),	// 1 : Close - White blob 들을 묶는다..
    _Inspect_Roi_Find_Shape,  CParam(_ProcessValue1, ("Noise out 3"), _IntValue, ("0")),	// -1 : Open - 작은 White blob 들을 없앤다.
    _Inspect_Roi_Find_Shape,  CParam(_ProcessValue1, ("Type"), _ComboValue, ("1"), ("Triangle,Rectangle,Pentagon,Hexagon,Circle")),
    _Inspect_Roi_Find_Shape,  CParam(_DecideValue, ("Minimum area"), _IntValue, ("100")),
    _Inspect_Roi_Find_Shape,  CParam(_DecideValue, ("Maximum area"), _IntValue, ("0")),

    _Inspect_Roi_Corner,  CParam(_ProcessValue1, ("Low Threshold"), _IntValue, ("150")),
    _Inspect_Roi_Corner,  CParam(_ProcessValue1, ("High Threshold"), _IntValue, ("255")),
    _Inspect_Roi_Corner,  CParam(_ProcessValue1, ("Noise out 1"), _IntValue, ("1")),	// -1 : Open - 작은 White blob 들을 없앤다
    _Inspect_Roi_Corner,  CParam(_ProcessValue1, ("Noise out 2"), _IntValue, ("-1")),	// 1 : Close - White blob 들을 묶는다.
    _Inspect_Roi_Corner,  CParam(_ProcessValue1, ("Expansion 1"), _IntValue, ("0")),	// -1 : Erode - 침식연산
    _Inspect_Roi_Corner,  CParam(_ProcessValue1, ("Expansion 2"), _IntValue, ("0")),	// 1 : Dilate - 팽창연산
    _Inspect_Roi_Corner,  CParam(_ProcessValue1,  ("Corner"), _ComboValue, ("0"), ("UpperLeft,UpperRight,BottomLeft,BottomRight")),
    //_Inspect_Roi_Corner,  CParam(_ProcessValue1, ("Method"), _ComboValue, ("1"), ("Corner,Line")),
    _Inspect_Roi_Corner,  CParam(_PostProcessValue1,  ("Expansion 1"), _IntValue, ("0")),	// -1 : Erode - 침식연산
    _Inspect_Roi_Corner,  CParam(_PostProcessValue1,  ("Expansion 2"), _IntValue, ("0")),	// 1 : Dilate - 팽창연산
    //_Inspect_Roi_Corner,  CParam(_LightValue, ("Light"), _IntValue, ("0")),

    //_Inspect_Teseract, CParam(_ProcessValue1, _T("Type"), _ComboValue, _T("English")),
    _Inspect_Teseract, CParam(_ProcessValue1, ("Low Threshold"), _IntValue, ("100")),
    _Inspect_Teseract, CParam(_ProcessValue1, ("High Threshold"), _IntValue, ("255")),
    _Inspect_Teseract, CParam(_ProcessValue1, ("Min Size Y"), _IntValue, ("100")),
    _Inspect_Teseract, CParam(_ProcessValue1, ("Max Size Y"), _IntValue, ("10000")),
    _Inspect_Teseract, CParam(_ProcessValue1, ("Noise out 1"), _IntValue, ("-2")),	// -1 : Open - 작은 White blob 들을 없앤다.
    _Inspect_Teseract, CParam(_ProcessValue1, ("Noise out 2"), _IntValue, ("2")),	// 1 : Close - White blob 들을 묶는다.
    _Inspect_Teseract, CParam(_ProcessValue1, ("Expansion 1"), _IntValue, ("-2")),	// -1 : Erode -
    _Inspect_Teseract, CParam(_ProcessValue1, ("Expansion 2"), _IntValue, ("0")),	// 1 : Dilate - 팽창연산
    _Inspect_Teseract, CParam(_ProcessValue2, ("Noise out 1"), _IntValue, ("0")),	// -1 : Open - 작은 White blob 들을 없앤다.
    _Inspect_Teseract, CParam(_ProcessValue2, ("Noise out 2"), _IntValue, ("0")),	// 1 : Close - White blob 들을 묶는다.
    _Inspect_Teseract, CParam(_ProcessValue2, ("Expansion 1"), _IntValue, ("0")),	// -1 : Erode -
    _Inspect_Teseract, CParam(_ProcessValue2, ("Expansion 2"), _IntValue, ("0")),	// 1 : Dilate - 팽창연산
    _Inspect_Teseract, CParam(_ProcessValue3, ("Size X(%)"), _IntValue, ("100")),
    _Inspect_Teseract, CParam(_ProcessValue3, ("Size Y(%)"), _IntValue, ("100")),
    _Inspect_Teseract, CParam(_ProcessValue3, ("Invert?"), _ComboValue, ("0"), ("No,Yes")),
    _Inspect_Teseract, CParam(_ProcessValue3, ("Smooth Use"), _ComboValue, ("0"), ("No,Yes")),
    _Inspect_Teseract, CParam(_ProcessValue3, ("Smooth method"), _ComboValue, ("2"), ("BLUR_NO_SCALE,BLUR,GAUSSIAN,CV_MEDIAN,BILATERAL ")),
    _Inspect_Teseract, CParam(_ProcessValue3, ("Smooth size"), _IntValue, ("7")),

    _Inspect_BarCode, CParam(_ProcessValue1, ("Type"), _ComboValue, ("Multiformat")),

    _Inspect_Point_Start,   CParam(_ProcessValue1, (""), _IntValue, ("0")), // do not delete.
    _Inspect_Point_Coordnation,  CParam(_ProcessValue1, ("Mark color"), _IntValue, ("128")), // not yet implement.


    _Inspect_Type_End,  CParam(_FilterValue, (""), _IntValue, ("")), // 반드시 있어야한다.
};

CRecipeData::CRecipeData(MainWindow *pParent)
{
    pMainWindow = pParent;
    //m_sObjListFileName = ("");
    m_sFileName = ("");
    //m_nOldGrabNumber = 0;

    for (int i = 0; i<_Inspect_Point_End; i++){
        m_sInspList[i].sprintf((""));
    }

    InitParamData();
}

CRecipeData::~CRecipeData(void)
{
    ClearImageBuff();
}

void CRecipeData::InitParamData()
{
    m_sInspList[_Inspect_Patt_VerticalAlign].sprintf("[Pattern]Vertical align");
    m_sInspList[_Inspect_Patt_HorizontalAlign].sprintf("[Pattern]Horizontal align");
    m_sInspList[_Inspect_Roi_MeasureAlign].sprintf(("MeasureAlign"));
    m_sInspList[_Inspect_Patt_Identify].sprintf(("Pattern Identify"));
    m_sInspList[_Inspect_Patt_MatchShapes].sprintf(("Pattern Shape Match"));

    m_sInspList[_Inspect_Roi_CenterOfPlusMark].sprintf(("Center of plusmark"));
    m_sInspList[_Inspect_Roi_SubpixelEdgeWithThreshold].sprintf(("Subpixel Edge with Threshold"));
    m_sInspList[_Inspect_Roi_Find_Shape].sprintf(("Find shape"));

    m_sInspList[_Inspect_Roi_Circle_With_Threshold].sprintf(("Circle with Threshold"));

    m_sInspList[_Inspect_Roi_Corner].sprintf(("Corner"));
    m_sInspList[_Inspect_Teseract].sprintf(("OCR"));
    m_sInspList[_Inspect_BarCode].sprintf(("BarCode"));

    m_sInspList[_Inspect_Point_Coordnation].sprintf(("Point"));

}

void CRecipeData::ClearImageBuff()
{
}

bool CRecipeData::LoadRecipeData()
{
    ViewMainPage* pMainView = pMainWindow->viewMainPage();
     QString strRootDir = gCfg.RootPath;//QApplication::applicationDirPath();
    QString fileName, path;
    path.sprintf("%s/TeachingData/%s", strRootDir.toStdString().c_str(),
                     gCfg.m_sLastRecipeName.toStdString().c_str());
    fileName.sprintf("%s.roi", gCfg.m_sLastRecipeName.toStdString().c_str());
    ObjectGroup *pobjs = nullptr;

    RoiReader reader(paramTable);
    QString file;
    file = path + "/" + fileName;
    RoiMap* roimap = reader.readRoi(file);
    if (roimap == nullptr)
        return false;

    int seq = 0;
    while (true) {
        Qroilib::DocumentView* v = pMainView->view(seq);
        if (v == nullptr)
            break;

        for (Layer *layer : v->mRoi->layers()) {
            if (ObjectGroup *objs = layer->asObjectGroup()) {
                for (RoiObject* obj : objs->objects()) {
                    if (obj->objectGroup() != nullptr)
                       v->mUndoStack->push(new RemoveRoiObject(v, obj));
                }
            }
        }
        seq++;
    }

    for (Layer *layer : roimap->layers())
    {
        seq = 0;
        while (true) {
            Qroilib::DocumentView* v = pMainView->view(seq);
            if (v == nullptr)
                break;
            pobjs = nullptr;
            for (Layer *player : v->mRoi->layers())
            {
                if (layer->name() == player->name()) {
                    pobjs = dynamic_cast<ObjectGroup*>(player);
                    break;
                }
            }
            if (pobjs == nullptr) {
                seq++;
                continue;
            }

            if (ObjectGroup *objs = layer->asObjectGroup()) {
                for (RoiObject* obj : objs->objects()) {
                    v->undoStack()->push(new AddRoiObject(v, pobjs, obj));
                }
            }
            seq++;
        }
    }
    return true;
}


void CRecipeData::SaveTemplelateImage(Qroilib::RoiObject *pRoiObject, IplImage *iplImg)
{
    QString strTemp;

    IplImage *clone = cvCreateImage(cvSize(iplImg->width, iplImg->height), IPL_DEPTH_8U, 1);
    if (iplImg->nChannels == 3)
        cvCvtColor(iplImg, clone, CV_RGB2GRAY);
    else if (iplImg->nChannels == 4) {
        if (strncmp(iplImg->channelSeq, "BGRA", 4) == 0)
            cvCvtColor(iplImg, clone, CV_BGRA2GRAY);
        else
            cvCvtColor(iplImg, clone, CV_RGBA2GRAY);
    } else
        cvCopy(iplImg, clone);

    if (pRoiObject->iplTemplate){
        cvReleaseImage(&pRoiObject->iplTemplate);
        pRoiObject->iplTemplate = nullptr;
    }
    pRoiObject->iplTemplate = cvCreateImage(cvSize((int)pRoiObject->mPattern->width(), (int)pRoiObject->mPattern->height()), IPL_DEPTH_8U, 1);
    cvSetImageROI(clone, cvRect((int)pRoiObject->mPattern->x(), (int)pRoiObject->mPattern->y(), (int)pRoiObject->mPattern->width(), (int)pRoiObject->mPattern->height()));
    cvCopy(clone, pRoiObject->iplTemplate);
    cvResetImageROI(clone);

    if (pRoiObject->iplTemplate != nullptr)
    {
        QString name = pRoiObject->name();
        strTemp = QString("%1/TeachingData/%2").arg(gCfg.RootPath).arg(gCfg.m_sLastRecipeName);
        QDir m_dir;
        m_dir.mkdir(strTemp);
        if (name.isEmpty()) {
            QString time = QDateTime::currentDateTime().toString("MMddhhmmss");
            name = "noname" + time;
        }
        strTemp = QString("%1/TeachingData/%2/%3.bmp").arg(gCfg.RootPath).arg(gCfg.m_sLastRecipeName).arg(name);
        cvSaveImage(strTemp.toStdString().c_str(), pRoiObject->iplTemplate);
    }
}

bool CRecipeData::SaveRecipeData()
{
    QString strRootDir = gCfg.RootPath;//QApplication::applicationDirPath();
    QString fileName;
    fileName.sprintf("%s/TeachingData/%s/%s.roi", strRootDir.toStdString().c_str(),
                     gCfg.m_sLastRecipeName.toStdString().c_str(),
                 gCfg.m_sLastRecipeName.toStdString().c_str());

    RoiWriter writer;
    ViewMainPage* pMainView = theMainWindow->viewMainPage();

    Qroilib::DocumentView* v = pMainView->view(0);
    if (v) {
        writer.writeRoiGroupStart(v->mRoi, fileName);
    }
    else return false;

    int seq = 0;
    while (true) {
        Qroilib::DocumentView* v = pMainView->view(seq);
        if (v == nullptr)
            break;

        writer.writeRoiGroup(v->mRoi);
        seq++;
    }

    writer.writeRoiGroupEnd();

    return true;
}
