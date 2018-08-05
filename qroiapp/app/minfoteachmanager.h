//
// minfoteachmanager.h
//
#pragma once

#ifndef MINFOTEACHMANAGER_H
#define MINFOTEACHMANAGER_H

#define MAX_SCREW 64

#include <QString>
#include "common.h"


class MInfoTeachManager
{
public:
    MInfoTeachManager();

    void ReadData();
    void WriteData();

public:
    vector<vector<SCREWITEM>> m_vecScrewTable;

protected:
    QMutex	m_sync;
    string m_strPath;
    string m_strSection;

    void	ChangePath(string strRoot, string szPath)
    {
        m_strPath = string_format(("%s/%s"), strRoot.c_str(), szPath.c_str());
    }

    void	ChangeSection(string szSection)
    {
        m_strSection = szSection;
    }

    void	GetInt(string szKey, int& iOut, int iDefault = 0);
    void	GetDouble(string szKey, double& dOut, double dDefault = 0.0);
    void	GetString(string szKey, string& strOut, string szDefault = (""));

    void	SetInt(string szKey, int iData);
    void	SetDouble(string szKey, double dData);
    void	SetString(string szKey, string szData);
    void	SetStringFormat(string szKey, string szFormat, ...);

};

#endif // MINFOTEACHMANAGER_H
