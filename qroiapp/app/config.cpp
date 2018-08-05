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

    m_sInterfacePort = settings.value("Ports/InterfacePort").toString();
    m_sLightPort = settings.value("Ports/LightPort").toString();

    m_nCamNumber = settings.value("CamInfo/CamNumber", 1).toInt();
    m_sCamera1 = settings.value("CamInfo/Cam1", "").toString();
    m_sCamera2 = settings.value("CamInfo/Cam2", "").toString();
    m_nCamExposure = settings.value("CamInfo/Exposure", 0).toInt();

    if (m_pCamInfo)
        delete m_pCamInfo;
    m_pCamInfo = new CAMERA_INFOMATION[m_nCamNumber];
    for(int i=0; i<m_nCamNumber; i++){
        dumy.sprintf("CAM_INFO_%02d/", i);
        m_pCamInfo[i].dResX = settings.value(dumy+"ResX", 0.008).toDouble();
        m_pCamInfo[i].dResY = settings.value(dumy+"ResY", 0.008).toDouble();
    }

    m_nAlignMode = settings.value("Screw/AlignMode", 1).toInt();

    feducial_vision_x = settings.value("Feducial/VisionX", 0.0).toDouble();
    feducial_vision_y = settings.value("Feducial/VisionY", 0.0).toDouble();
    feducial_vision_z = settings.value("Feducial/VisionZ", 0.0).toDouble();
    feducial_screw_x = settings.value("Feducial/ScrewX", 0.0).toDouble();
    feducial_screw_y = settings.value("Feducial/ScrewY", 0.0).toDouble();
    feducial_screw_z = settings.value("Feducial/ScrewZ", 0.0).toDouble();
    feducial_offset_x = settings.value("Feducial/OffsetX", 0.0).toDouble();
    feducial_offset_y = settings.value("Feducial/OffsetY", 0.0).toDouble();

    vision_offset_z = settings.value("Vision/OffsetZ", 0.0).toDouble();
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

    settings.setValue("Ports/InterfacePort", m_sInterfacePort);
    settings.setValue("Ports/LightPort", m_sLightPort);

    settings.setValue("CamInfo/CamNumber", m_nCamNumber);
    settings.setValue("CamInfo/Cam1", m_sCamera1);
    settings.setValue("CamInfo/Cam2", m_sCamera2);
    settings.setValue("CamInfo/Exposure", m_nCamExposure);

    if(m_pCamInfo){
        for(int i=0; i<m_nCamNumber; i++){
            dumy.sprintf("CAM_INFO_%02d/", i);
            settings.setValue(dumy+"ResX", m_pCamInfo[i].dResX );
            settings.setValue(dumy+"ResY", m_pCamInfo[i].dResY );
        }
    }

    settings.setValue("Screw/AlignMode", m_nAlignMode);


    settings.setValue("Feducial/VisionX", feducial_vision_x);
    settings.setValue("Feducial/VisionY", feducial_vision_y);
    settings.setValue("Feducial/VisionZ", feducial_vision_z);
    settings.setValue("Feducial/ScrewX", feducial_screw_x);
    settings.setValue("Feducial/ScrewY", feducial_screw_y);
    settings.setValue("Feducial/ScrewZ", feducial_screw_z);
    settings.setValue("Feducial/OffsetX", feducial_offset_x);
    settings.setValue("Feducial/OffsetY", feducial_offset_y);

    settings.setValue("Vision/OffsetZ", vision_offset_z);
}
