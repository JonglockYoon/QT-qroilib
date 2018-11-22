#pragma once

#include <roilib_export.h>

struct CAMERA_INFOMATION {
    double	dResX;
    double	dResY;

    CAMERA_INFOMATION(){
        dResX = 0.010;
        dResY = 0.010;
    };
};

class CConfig
{
public:
    CConfig();
    CConfig(QString *pszPath);
    virtual ~CConfig(void);

    CAMERA_INFOMATION * m_pCamInfo;

    QString m_sFileName;	//설정파일에 대한 경로
    void ReadConfig();		//설정파일 불러오는 함수
    void WriteConfig();		//설정파일 저장하는 함수

    int	m_nCamNumber;		//연결되어야할 카메라 갯수
    int    m_nUserRole; //  _Role_Guru
    bool	m_bSaveEngineImg;
    bool m_bLogSave;
    QString RootPath;

    QString m_sSaveImageDir;	// GrabImage 및 디버깅 이미지 정장 Path, 공백이면 Recipe디렉토리에 생성된다.
    QString m_sLastRecipeName;

    QString m_sCamera1;
    QString m_sCamera2;

    int m_nCamExposure;
    int m_nCamFocus;

};

extern CConfig gCfg;//외부(다른 클래스)에서 접근하기 위한 변수
