//
// recipedata.cpp
//
//QROILIB : QT Vision ROI Library
//Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
//
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
#include "savefile.h"

using namespace Qroilib;

CRecipeData * g_cRecipeData = NULL;
Qroilib::ParamTable paramTable[] = {
    _Inspect_Patt_Start,   CParam(_ProcessValue1, (""), _IntValue, ("0")), // do not delete.

    _Inspect_Roi_Start,   CParam(_ProcessValue1, (""), _IntValue, ("0")), // do not delete.

    _Inspect_Point_Start,   CParam(_ProcessValue1, (""), _IntValue, ("0")), // do not delete.

    _Inspect_Type_End,  CParam(_FilterValue, (""), _IntValue, ("")), // do not delete.
};

CRecipeData::CRecipeData(MainWindow *pParent)
{
    pMainWindow = pParent;
    m_sFileName = ("");

    for (int i = 0; i<_Inspect_Point_End; i++){
        m_sInspList[i].sprintf((""));
    }

    InitParamData();
}

CRecipeData::~CRecipeData(void)
{
}

void CRecipeData::InitParamData()
{
}

bool CRecipeData::LoadRecipeData()
{
    ViewMainPage* pMainView = pMainWindow->viewMainPage();
    ObjectGroup *pobjs = nullptr;

    RoiReader reader(paramTable);
    QString file = "./RecipeData.roi";
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
        if (name.isEmpty()) {
            QString time = QDateTime::currentDateTime().toString("MMddhhmmss");
            name = "noname" + time;
        }
        strTemp = name + ".bmp";
        cvSaveImage(strTemp.toStdString().c_str(), pRoiObject->iplTemplate);
    }

}


bool CRecipeData::SaveRecipeData()
{
    QString fileName = "./RecipeData.roi";
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

