//
// mtrsalign.cpp : 구현 파일입니다.
//
#include <QDebug>

#include "mtrsbase.h"
#include "mtrsalign.h"
#include "mainwindow.h"

// CMTrsAlign

CMTrsAlign::CMTrsAlign(const char* lpszThreadName) : MTrsBase(lpszThreadName)
{
    m_strUnitName = lpszThreadName;
    if (m_strUnitName == "left")
        ch = 0;
    else
        ch = 1;
    bDoAlign = false;
    bAllTeachingMode = false;
    SetStep(0);
}

CMTrsAlign::~CMTrsAlign()
{
}

// CMTrsAlign 메시지 처리기입니다.

void CMTrsAlign::doRunStep()
{
    switch (m_iCurrentStep)
    {
    case 0:
        pInfo = theMainWindow->m_pInfoTeachManager;
        pPoints = &pInfo->m_vecScrewTable[ch];
        dCenterX = dCenterY = 0;
        PrepareCenterOfImage(ch);
        SetStep(100);
        break;
    case 100:
        if (bDoAlign) {
            theMainWindow->LightOn(ch); // ROI가 한개 운용이 되므로 ROI에 명시되어 잇는 Light value로 조명을 켠다.
            SetStep(500);
        }
        break;
    case 500:
        qDebug() << "mtrsalign" << gCfg.m_nAlignMode;
        GetScrewAlignPoints(ch);
        str.sprintf("%.2f %.2f - %.2f %.2f", ul.x, ul.y, lr.x, lr.y);
        qDebug() << str;

        if (gCfg.m_nAlignMode == 1)
            SetStep(1000);
        else if (gCfg.m_nAlignMode == 2)
            SetStep(2000);
        else if (gCfg.m_nAlignMode == 3)
            SetStep(3000);
        break;

    //
    // 1 point align 시작
    //
    case 1000:
        cpt.pt.x = ul.x;
        cpt.pt.y = ul.y;
        cpt.pt.z = ul.z;
        corners.push_back(cpt);
        x = ul.x;
        y = ul.y;
        S2V(x, y); // 비젼위치로 변환.
        z = ul.z + gCfg.vision_offset_z; // 체결위치에서 비젼 Z Offset을 뺀 높이로 이동.
        theMainWindow->itfport.Move(m_strUnitName, x, y, z);
        SetStep(1010);
        break;
    case 1010:
        if (theMainWindow->itfport.bMoveComplete[ch])
            SetStep(1020);
        if (m_MTimer.MoreThan(2.0))
            SetStep(9000);
        break;
    case 1020:
        pData = theMainWindow->FindScrewHole(ch);
        if (pData != nullptr)
        {
            int size = pData->m_vecDetectResult.size();
            if (size > 0) {
                Result = pData->m_vecDetectResult[0];
                x = Result.pt.x + pData->bounds().x();
                y = Result.pt.y + pData->bounds().y();
                SetStep(1030);
                break;
            }
        }
        SetStep(9000);
#ifdef SIMULATION
        SetStep(1030);
#endif
        break;
    case 1030:
#if 0//def SIMULATION
        x = dCenterX;
        y = dCenterY;
#endif
        theMainWindow->SetCameraPause(ch, false);

        x = dCenterX - x;
        y = dCenterY - y;
        dAlignOffset[0].x = (x * gCfg.m_pCamInfo[0].dResX); // mm 변환;
        dAlignOffset[0].y = -(y * gCfg.m_pCamInfo[0].dResX);
        SetStep(1100);
        break;
    case 1100: // 첫 Hole위치가 dAlignOffset[]에 담겨있다.
                // 기존 티칭위치는 corners[]에 있다.
        One_Point_Align_Calc_By_RegAlignmentPos();
        emit updatePositionlist(ch);
        SetStep(1210);
        break;

    case 1210: // 2 point align 완료
        theMainWindow->itfport.ResponseAlignScrewPos(m_strUnitName);
        SetStep(9000);
        break;

    //
    // 2 point align 시작
    //
    case 2000:
        cpt.pt.x = ul.x;
        cpt.pt.y = ul.y;
        cpt.pt.z = ul.z;
        corners.push_back(cpt);
        corners.push_back(cpt);
        cpt.pt.x = lr.x;
        cpt.pt.y = lr.y;
        cpt.pt.z = lr.z;
        corners.push_back(cpt);

        x = ul.x;
        y = ul.y;
        S2V(x, y); // 비젼위치로 변환.
        z = ul.z + gCfg.vision_offset_z; // 체결위치에서 비젼 Z Offset을 뺀 높이로 이동.
        theMainWindow->itfport.Move(m_strUnitName, x, y, z);
        SetStep(2010);
        break;
    case 2010:
        if (theMainWindow->itfport.bMoveComplete[ch])
            SetStep(2020);
        if (m_MTimer.MoreThan(2.0))
            SetStep(9000);
        break;
    case 2020:
        pData = theMainWindow->FindScrewHole(ch);
        if (pData != nullptr)
        {
            int size = pData->m_vecDetectResult.size();
            if (size > 0) {
                Result = pData->m_vecDetectResult[0];
                x = Result.pt.x + pData->bounds().x();
                y = Result.pt.y + pData->bounds().y();
                SetStep(2030);
                break;
            }
        }
        SetStep(9000);
#ifdef SIMULATION
        SetStep(2030);
#endif
        break;
    case 2030:
#ifdef SIMULATION
        x = dCenterX;
        y = dCenterY;
#endif
        theMainWindow->SetCameraPause(ch, false);

        x = dCenterX - x;
        y = dCenterY - y;
        dAlignOffset[0].x = (x * gCfg.m_pCamInfo[0].dResX); // mm 변환;
        dAlignOffset[0].y = -(y * gCfg.m_pCamInfo[0].dResX);
        SetStep(2100);
        break;
    case 2100:
        x = lr.x;
        y = lr.y;
        S2V(x, y); // 비젼위치로 변환.
        z = lr.z + gCfg.vision_offset_z; // 체결위치에서 비젼 Z Offset을 뺀 높이로 이동.
        theMainWindow->itfport.Move(m_strUnitName, x, y, z);
        SetStep(2110);
        break;
    case 2110:
        if (theMainWindow->itfport.bMoveComplete[ch])
            SetStep(2120);
        if (m_MTimer.MoreThan(2.0))
            SetStep(9000);
        break;
    case 2120:
        pData = theMainWindow->FindScrewHole(ch);
        if (pData != nullptr)
        {
            int size = pData->m_vecDetectResult.size();
            if (size > 0) {
                Result = pData->m_vecDetectResult[0];
                x = Result.pt.x + pData->bounds().x();
                y = Result.pt.y + pData->bounds().y();
                SetStep(2130);
                break;
            }
        }
        SetStep(9000);
#ifdef SIMULATION
        SetStep(2130);
#endif
        break;
    case 2130:
#ifdef SIMULATION
        x = dCenterX;
        y = dCenterY;
#endif
        x = dCenterX - x;
        y = dCenterY - y;
        dAlignOffset[2].x = (x * gCfg.m_pCamInfo[0].dResX); // mm 변환;
        dAlignOffset[2].y = -(y * gCfg.m_pCamInfo[0].dResX);
        SetStep(2200);
        break;
    case 2200: // 두개의 지점 Hole위치가 dAlignOffset[]에 담겨있다.
                // 기존 티칭위치는 corners[]에 있다.
        Two_Point_Align_Calc_By_RegAlignmentPos();
        emit updatePositionlist(ch);
        SetStep(2900);
        break;

    case 2900: // 2 point align 완료
        theMainWindow->itfport.ResponseAlignScrewPos(m_strUnitName);
        emit updatePositionlist(ch);
        SetStep(9000);
        break;

    //
    // all point align 시작
    //
    case 3000:
        totsize = pPoints->size();
        curr = 0;
        SetStep(3010);
        break;
    case 3010:
        if (curr >= totsize) {
            SetStep(3900);
            break;
        }
        theMainWindow->SetCameraPause(ch, false);
        pcurritem = &(*pPoints)[curr];
        x = pcurritem->x;
        y = pcurritem->y;
        S2V(x, y); // 비젼위치로 변환.
        z = lr.z + gCfg.vision_offset_z; // 체결위치에서 비젼 Z Offset을 뺀 높이로 이동.
        theMainWindow->itfport.Move(m_strUnitName, x, y, z);
        SetStep(3020);
        break;
    case 3020:
        if (theMainWindow->itfport.bMoveComplete[ch])
            SetStep(3030);
        if (m_MTimer.MoreThan(2.0))
            SetStep(9000);
        break;
    case 3030:
        theMainWindow->SetCameraPause(ch, true);
        pData = theMainWindow->FindScrewHole(ch);
        if (pData != nullptr)
        {
            int size = pData->m_vecDetectResult.size();
            if (size > 0) {
                Result = pData->m_vecDetectResult[0];
                x = Result.pt.x + pData->bounds().x();
                y = Result.pt.y + pData->bounds().y();
                SetStep(3040);
                break;
            }
        }
        SetStep(9000);
#ifdef SIMULATION
        SetStep(3040);
#endif
        break;
    case 3040:
#if 0//def SIMULATION
        x = dCenterX;
        y = dCenterY;
#endif
        pcurritem->dx = (dCenterX - x) * gCfg.m_pCamInfo[0].dResX; // mm 변환;        ;
        pcurritem->dy = (dCenterY - y) * gCfg.m_pCamInfo[0].dResY;
//        if (bAllTeachingMode == true)
//        {
//            pcurritem->x += pcurritem->dx;
//            pcurritem->y += pcurritem->dy;
//            pcurritem->dx = 0;
//            pcurritem->dy = 0;
//        }
        SetStep(3050);
        break;
    case 3050:
        curr++;
        SetStep(3010);
        break;
    case 3900:
        if (bAllTeachingMode == false)
            theMainWindow->itfport.ResponseAlignScrewPos(m_strUnitName);
        emit updatePositionlist(ch);
        SetStep(9000);
        break;

    case 9000:
        bDoAlign = false;
        bAllTeachingMode = false;
        theMainWindow->LightOff(ch);
        theMainWindow->SetCameraPause(ch, false);
        SetStep(0);
        break;
    default:
        break;
    }
}


bool CMTrsAlign::SetUnitInitialize()
{
    SetStep(0);
    return true;
}

void CMTrsAlign::PrepareCenterOfImage(int ch)
{
    ViewMainPage* pView = theMainWindow->viewMainPage();
    Qroilib::DocumentView* v = pView->view(ch);
    if (v == nullptr)
        return;

    const QImage *camimg = v->image();
    QSize imgsize = camimg->size();

    dCenterX = imgsize.width() / 2.0;
    dCenterY = imgsize.height() / 2.0;
}

void CMTrsAlign::GetScrewAlignPoints(int ch)
{
    //MInfoTeachManager*	pInfo = theMainWindow->m_pInfoTeachManager;
    int size = pPoints->size();
    if (size < 2) {
        return;
    }

    vector<SCREWITEM> vecTemp;
    vecTemp = *pPoints;

    std::stable_sort(vecTemp.begin(), vecTemp.end(), [](const SCREWITEM lhs, const SCREWITEM rhs)->bool {
        return sqrt(lhs.x*lhs.x+lhs.y*lhs.y) < sqrt(rhs.x*rhs.x+rhs.y*rhs.y);
    });

    ul = vecTemp[0]; // 가장작은 위치
    lr = vecTemp[size-1]; // 가장큰 위치
}


bool CMTrsAlign::One_Point_Align_Calc_By_RegAlignmentPos()
{
    double tx = dAlignOffset[0].x; // 우하단 모서리 : 안착좌표 - 회전이동된티칭좌표의 회전이동위치
    double ty = dAlignOffset[0].y;
    for (int i = 0; i < pPoints->size(); i++) { // Align 좌표 수정.
        SCREWITEM *pitem = &(*pPoints)[i];
        pitem->dx = tx;
        pitem->dy = ty;
    }
    return true;
}

bool CMTrsAlign::Two_Point_Align_Calc_By_RegAlignmentPos()
{
    //TRACE(_T("Two_Point_Align_Calc Seating Offset : first=%.4f,%.4f second=%.4f,%.4f \n"), dAlignOffset[0].x, dAlignOffset[0].y, dAlignOffset[2].x, dAlignOffset[2].y);

    cv::Point2d teachpos[2] = { { corners[0].pt.x, corners[0].pt.y }, { corners[2].pt.x, corners[2].pt.y } };	// 티칭좌표
    cv::Point2d seatpos[2] = { { corners[0].pt.x - (double)dAlignOffset[0].x, corners[0].pt.y - (double)dAlignOffset[0].y },
                            { corners[2].pt.x - (double)dAlignOffset[2].x, corners[2].pt.y - (double)dAlignOffset[2].y } }; // 안착좌표

    double dTheta1 = CalculDegree(teachpos[0], teachpos[1]); // 티칭각도
    double dTheta2 = CalculDegree(seatpos[0], seatpos[1]); // 안착 각도
    double dTheta = (dTheta1 - dTheta2);
    //TRACE(_T("Compensation theta value = %.4f\n"), dTheta);

    double x1 = 9999.0, y1 = 9999.0, x2 = 0.0, y2 = 0.0;
    for (int i = 0; i < 2; i++) {
        if (teachpos[i].x < x1) x1 = teachpos[i].x; // 가장적은 x,y
        if (teachpos[i].y < y1) y1 = teachpos[i].y;
        if (teachpos[i].x > x2) x2 = teachpos[i].x; // 가장큰 x,y
        if (teachpos[i].y > y2) y2 = teachpos[i].y;
    }

    // 체결기 로봇의 기준점은 우하측이다. (티칭포인터들의 정렬)
    cv::Point2d dPointCenter;
    dPointCenter.x = x1 + (x2 - x1) / 2;
    dPointCenter.y = y1 + (y2 - y1) / 2;

    vector<SCREWITEM> vecTemp;
    vecTemp = *pPoints;

    double a = RADIAN(dTheta);
    cv::Point2d tmp;
    cv::Point2d dTranslateP;
    for (int i = 0; i < vecTemp.size(); i++) { // 티칭 포인터들의 좌표를 회전 각도로 이동 시킨다.
        tmp.x = vecTemp[i].x;
        tmp.y = vecTemp[i].y;

        dTranslateP.x = (cos(a) * (tmp.x - dPointCenter.x)) - (sin(a) * (tmp.y - dPointCenter.y)) + dPointCenter.x;
        dTranslateP.y = (sin(a) * (tmp.x - dPointCenter.y)) + (cos(a) * (tmp.y - dPointCenter.y)) + dPointCenter.y;
        //TRACE(_T("Theta Translate(%.4f,%.4f)\n"), dTranslateP.dX, dTranslateP.dY);

        vecTemp[i].x = dTranslateP.x;
        vecTemp[i].y = dTranslateP.y;
    }

    // 티칭 포인터 회전 이동점을 산출한다.
    for (int i = 0; i < 2; i++) { // 티칭 포인터들의 좌표를 회전 각도로 이동 시킨다.
        tmp.x = teachpos[i].x;
        tmp.y = teachpos[i].y;

        dTranslateP.x = (cos(a) * (tmp.x - dPointCenter.x)) - (sin(a) * (tmp.y - dPointCenter.y)) + dPointCenter.x;
        dTranslateP.y = (sin(a) * (tmp.x - dPointCenter.x)) + (cos(a) * (tmp.y - dPointCenter.y)) + dPointCenter.y;
        //TRACE(_T("Theta Translate(%.4f,%.4f)\n"), dTranslateP.dX, dTranslateP.dY);

        teachpos[i].x = dTranslateP.x;
        teachpos[i].y = dTranslateP.y;
    }

    // 회전시킨후 두개지점의 X,Y이동거리를 산출한다.
    // seatpos - 안착회전이동위치, corners - 티칭위치
    double tx1 = (seatpos[0].x - teachpos[0].x); // 우하단 모서리 : 안착좌표 - 회전이동된티칭좌표의 회전이동위치
    double ty1 = (seatpos[0].y - teachpos[0].y);
    double tx2 = (seatpos[1].x - teachpos[1].x); // 좌상단 모서리 : 안착좌표 - 회전이동된티칭좌표의 회전이동위치
    double ty2 = (seatpos[1].y - teachpos[1].y);

    //TRACE(_T("tx1,tx2 = (%.4f,%.4f)\n"), tx1, tx2);
    //TRACE(_T("ty1,ty2 = (%.4f,%.4f)\n"), ty1, ty2);
    double tx = (tx1 + tx2) / 2.0; // 티칭위치를 회전이동 시킨후 안착위치와 X,Y가 움직인 거리
    double ty = (ty1 + ty2) / 2.0;
    //TRACE(_T("Compensation xy value = (%.4f,%.4f)\n"), tx, ty);

    for (int i = 0; i < vecTemp.size(); i++) { // align 좌표 수정.
        vecTemp[i].x += tx;
        vecTemp[i].y += ty;
        //TRACE(_T("Translate [%d] = (%.4f,%.4f)\n"), i, vecPoints[i].x, vecPoints[i].y);

        //SCREWITEM *pitem = &pInfo->m_vecScrewTable[ch][i];
        SCREWITEM *pitem = &(*pPoints)[i];
        x = pitem->x - vecTemp[i].x;
        y = pitem->y - vecTemp[i].y;
        pitem->dx = x;
        pitem->dy = y;
    }
    // 보정완료

    return true;
}


// Screw위치로변환
void CMTrsAlign::V2S(double &x, double &y)
{
    x -= gCfg.feducial_offset_x;
    y -= gCfg.feducial_offset_y;
}

// 비젼위치로 변환.
void CMTrsAlign::S2V(double &x, double &y)
{
    x += gCfg.feducial_offset_x;
    y += gCfg.feducial_offset_y;
}
