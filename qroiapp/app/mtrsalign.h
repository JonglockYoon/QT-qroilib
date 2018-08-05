#pragma once

#include <QMutex>
#include "mtrsbase.h"
#include "roiobject.h"
#include "minfoteachmanager.h"

typedef struct _tagCorners {
    int idx;
    cv::Point3d pt;
} CORNERS;


// CMTrsAlign

using namespace Qroilib;

class CMTrsAlign : public MTrsBase
{
        Q_OBJECT

public:
    CMTrsAlign(const char *lpszThreadName);
    virtual ~CMTrsAlign();

public:	
	void doRunStep();
    bool SetUnitInitialize();

    void PrepareCenterOfImage(int ch);
    void GetScrewAlignPoints(int ch);
    void V2S(double &x, double &y);
    void S2V(double &x, double &y);
    bool One_Point_Align_Calc_By_RegAlignmentPos();
    bool Two_Point_Align_Calc_By_RegAlignmentPos();

public:
    int point; // 체결기가 align 요구할때 현재위치.
    bool bDoAlign;
    bool bAllTeachingMode;
    std::vector<CORNERS> corners;

signals:
    void initPositionlist(int ch);
    void updatePositionlist(int ch);

private:
    QMutex	m_sync;
    int ch;
    QString str;

    SCREWITEM *pcurritem;
    int totsize;
    int curr;
    SCREWITEM ul;
    SCREWITEM lr;
    Qroilib::RoiObject *pData;
    DetectResult Result;
    double x, y, z;
    CORNERS		cpt;
    cv::Point3d dAlignOffset[3];
    MInfoTeachManager*	pInfo;
    vector<SCREWITEM> *pPoints;

    double dCenterX;
    double dCenterY;
};


