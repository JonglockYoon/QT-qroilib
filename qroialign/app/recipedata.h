#pragma once


#include <QString>
#include "roiobject.h"


enum  StepType{
    _MaskingValue,
    _ProcessValue1,
    _ProcessValue2,
    _ProcessValue3,
    _FilterValue,
    _PostProcessValue1,
    _PostProcessValue2,
    _DecideValue,
    _PriorityValue,
    _LightValue,
};

// 여기에 정의된 순서를 변경하면 모든 레시피 파일에 영향을 주므로
// 순서변경시에는 레시피 ini 파일의 nInspectType도 같이 변경해야한다.
typedef enum _tagInspectType
{
    ///////////////////////////////////////////////////////////
    _Inspect_Patt_Start = _INSPECT_PATT_START, // do not change.
    _Inspect_Patt_End,

    ///////////////////////////////////////////////////////////
    _Inspect_Roi_Start = _INSPACT_ROI_START, // do not change.
    _Inspect_Roi_Align_TowPoint,
    _Inspect_Roi_Align_Measure,
    _Inspect_Roi_End,			// 영역 마지막

    ///////////////////////////////////////////////////////////
    _Inspect_Point_Start = _INSPACT_POINT_START, // do not change.
    _Inspect_Point_Coordnation,
    _Inspect_Point_End,

    _Inspect_Type_End = _INSPACT_TYPE_END, // do not change.


} InspectType;

class MainWindow;

class CRecipeData
{
public:
    CRecipeData(MainWindow *pParent);
    virtual ~CRecipeData(void);

    QString	m_sInspList[_INSPACT_TYPE_END]; // ROI Type Table // _Inspect_Type_End

    void SaveTemplelateImage(Qroilib::RoiObject *pRoiObject, IplImage *iplImg);
    void InitParamData(void);
    bool LoadRecipeData(); //해당 함수를 호출하기전에 Object파일을 먼저 불러주어야 된다
    bool SaveRecipeData();
    void SaveBaseRecipeData();
    void LoadBaseRecipeData();

    QString m_sFileName;
    MainWindow *pMainWindow;

};



