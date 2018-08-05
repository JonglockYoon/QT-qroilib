//
// minfoteachmanager.cpp
//
#include "common.h"
#include <QSettings>
#include "mainwindow.h"
#include "minfoteachmanager.h"
#include <QMutexLocker>

MInfoTeachManager::MInfoTeachManager()
{
    std::vector<SCREWITEM> element;
    element.resize(0);
    for (int i = 0; i < 2; i++)
        m_vecScrewTable.push_back(element);
}
void MInfoTeachManager::ReadData()
{
    QMutexLocker ml(&m_sync);

    string str, strkey;
    string s1 = gCfg.m_sLastRecipeName.toLatin1().data();
    str = string_format(("TeachingData/%s/%s.dat"), s1.c_str(), s1.c_str());
    ChangePath(gCfg.RootPath.toLatin1().data(), str.c_str());

    ChangeSection(("SCREW_POSITION"));

    QString qstr;
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < MAX_SCREW; j++)
        {
            strkey = string_format(("SCREWPOINT_%02d_%02d"), i, j);
            GetString(strkey, str);
            if (str.length() == 0)
                break;
            qstr = QString::fromStdString(str);
            QStringList list = qstr.split(QRegExp(","), QString::KeepEmptyParts);
            int seq = 0;
            SCREWITEM item;
            for (QString token : list)
            {
                if (seq == 0) item.point = token.toInt();
                else if (seq == 1) item.x = token.toDouble();
                else if (seq == 2) item.y = token.toDouble();
                else if (seq == 3) item.z = token.toDouble();
                seq++;
            }
            m_vecScrewTable[i].push_back(item);
        }
    }
}

void MInfoTeachManager::WriteData()
{
    QMutexLocker ml(&m_sync);

    string str, strkey;
    string s1 = gCfg.m_sLastRecipeName.toLatin1().data();
    str = string_format(("TeachingData/%s/%s.dat"), s1.c_str(),s1.c_str());
    ChangePath(gCfg.RootPath.toLatin1().data(), str.c_str());

    ChangeSection(("SCREW_POSITION"));

    QString qstr, temp;
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < m_vecScrewTable[i].size(); j++)
        {
            SCREWITEM item =m_vecScrewTable[i][j];
            qstr = "";
            strkey = string_format(("SCREWPOINT_%02d_%02d"), i, j);
            temp.sprintf(("%2d,"), item.point); qstr += temp;
            temp.sprintf(("%.3f,"), item.x); qstr += temp;
            temp.sprintf(("%.3f,"), item.y); qstr += temp;
            temp.sprintf(("%.3f"), item.z); qstr += temp;

            std::string str = qstr.toUtf8().constData();
            SetString(strkey, str);
        }
    }
}

void MInfoTeachManager::GetInt(string szKey, int& iOut, int iDefault)
{
    string str;
    string strDefault;

    strDefault = string_format(("%d"), iDefault);
    GetPrivateProfileString(m_strSection, (char*)szKey.c_str(), (char*)strDefault.c_str(), (char*)str.c_str(), 256, m_strPath);

    iOut = atoi(str.c_str());
}

void MInfoTeachManager::GetDouble(string szKey, double& dOut, double dDefault)
{
    string str;
    string strDefault;

    strDefault = string_format(("%g"), dDefault);
    GetPrivateProfileString(m_strSection, (char*)szKey.c_str(), (char*)strDefault.c_str(), (char*)str.c_str(), 256, m_strPath);

    dOut = atof(str.c_str());
}

void MInfoTeachManager::GetString(string szKey, string& strOut, string szDefault)
{
    char value[4096];

    GetPrivateProfileString(m_strSection, (char*)szKey.c_str(), (char*)szDefault.c_str(), (char*)value, 4096, m_strPath);

    string str(value);
    strOut = str;
}

void MInfoTeachManager::SetInt(string szKey, int iData)
{
    SetStringFormat(szKey, ("%d"), iData);
}

void MInfoTeachManager::SetDouble(string szKey, double dData)
{
    SetStringFormat(szKey, ("%.3f"), dData);
}

void MInfoTeachManager::SetString(string szKey, string szData)
{
    SetStringFormat(szKey, ("%s"), szData.c_str());
}

void MInfoTeachManager::SetStringFormat(string szKey, string szFormat, ...)
{
    char str[4096] = {0, };

    va_list va;
    va_start(va, szFormat);
    int n = vsnprintf((char *)str, 4096 - 1, szFormat.c_str(), va);
    va_end(va);

    string strPrev;
    GetString(szKey, strPrev);

    WritePrivateProfileString(m_strSection, (char*)szKey.c_str(), str, m_strPath);

    if(strPrev != str)
    {
        // 값이 바뀐 경우
        theMainWindow->DevLogSave(("Save (Model): [%s][%s], Prev: [%s] -> New : [%s]"),
            m_strSection.c_str(), szKey.c_str(), strPrev.c_str(), str);
    }
}

