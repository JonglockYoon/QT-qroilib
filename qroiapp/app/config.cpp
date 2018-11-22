#include <QApplication>
#include <QString>
#include <QSettings>
#include "config.h"

CConfig gCfg;

CConfig::CConfig()
{
    m_sLastRecipeName = "";

    m_sFileName = "./config.ini";
    m_bLogSave = true;
    m_nCamNumber = 1;
    m_pCamInfo = new CAMERA_INFOMATION[m_nCamNumber]; // default


}
CConfig::CConfig(QString *pszPath)
{
    m_sLastRecipeName = "";
    m_sFileName.sprintf("%s/config.ini", pszPath->toStdString().c_str());
    if (m_pCamInfo)
        delete m_pCamInfo;
    m_nCamNumber = 1;
    m_pCamInfo = new CAMERA_INFOMATION[m_nCamNumber]; // default
}

CConfig::~CConfig(void)
{
}

void CConfig::ReadConfig()
{
    QString str;
    QString dumy;

    QSettings settings(m_sFileName, QSettings::IniFormat);

    m_sLastRecipeName = settings.value("GeneralApp/LastRecipe", "NoModel").toString();
    m_bSaveEngineImg = settings.value("GeneralApp/SAVEENGINEIMG", true).toBool();
    RootPath = settings.value("GeneralApp/ROOT_DIR", "").toString();
    if (RootPath.length() == 0)
        RootPath = QApplication::applicationDirPath();
    m_sSaveImageDir = settings.value("GeneralApp/SAVE_IMAGE_DIR", "").toString();
    m_bLogSave = settings.value("GeneralApp/SAVELOG", true).toBool();
    m_nUserRole = settings.value("GeneralApp/USER_ROLE", true).toBool();

    m_nCamNumber = settings.value("CamInfo/CamNumber", 1).toInt();
    m_sCamera1 = settings.value("CamInfo/Cam1", "").toString();
    m_sCamera2 = settings.value("CamInfo/Cam2", "").toString();
    m_nCamExposure = settings.value("CamInfo/Exposure", 0).toInt();
    m_nCamFocus = settings.value("CamInfo/Focus", 0).toInt();

    if (m_pCamInfo)
        delete m_pCamInfo;
    m_pCamInfo = new CAMERA_INFOMATION[m_nCamNumber];
    for(int i=0; i<m_nCamNumber; i++){
        dumy.sprintf("CAM_INFO_%02d/", i);
        m_pCamInfo[i].dResX = settings.value(dumy+"ResX", 0.008).toDouble();
        m_pCamInfo[i].dResY = settings.value(dumy+"ResY", 0.008).toDouble();
    }

}

void CConfig::WriteConfig()
{
    //QString str;
    QString dumy;

    QSettings settings(m_sFileName, QSettings::IniFormat);

    settings.setValue("GeneralApp/LastRecipe", m_sLastRecipeName);
    settings.setValue("GeneralApp/SAVEENGINEIMG", m_bSaveEngineImg);
    settings.setValue("GeneralApp/ROOT_DIR", RootPath);
    settings.setValue("GeneralApp/SAVE_IMAGE_DIR", m_sSaveImageDir);
    settings.setValue("GeneralApp/SAVELOG", m_bLogSave);
    settings.setValue("GeneralApp/USER_ROLE", m_nUserRole);

    settings.setValue("CamInfo/CamNumber", m_nCamNumber);
    settings.setValue("CamInfo/Cam1", m_sCamera1);
    settings.setValue("CamInfo/Cam2", m_sCamera2);
    settings.setValue("CamInfo/Exposure", m_nCamExposure);
    settings.setValue("CamInfo/Focus", m_nCamFocus);

    if(m_pCamInfo){
        for(int i=0; i<m_nCamNumber; i++){
            dumy.sprintf("CAM_INFO_%02d/", i);
            settings.setValue(dumy+"ResX", m_pCamInfo[i].dResX );
            settings.setValue(dumy+"ResY", m_pCamInfo[i].dResY );
        }
    }

}
