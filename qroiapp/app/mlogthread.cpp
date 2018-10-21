//
// mlogthread.cpp
//
#include <QDir>
#include "config.h"
#include <QDateTime>
#include <QStringList>
#include <QFile>
#include "mlogthread.h"
#include "common.h"

MLogThread::MLogThread()
{
    ThreadStart();
}

MLogThread::~MLogThread()
{
    ThreadStop();
}

int MLogThread::ThreadStart()
{
    m_iExitFlag = 0;
    start();
    return 0;
}

int MLogThread::ThreadStop()
{
    exit(0);
    return 0;
}

void MLogThread::run()
{
    Thread();
    return;
}

bool MLogThread::Thread()
{
    while(1)
    {
        m_sync.lock();
        if(m_Data.size() <= 0)
        {
            m_sync.unlock();
            if(m_iExitFlag) break;

            msleep(10);
            continue;
        }

        LogWorkData data = m_Data.front();
        m_Data.pop();
        m_sync.unlock();

        // 디렉토리 자동 생성
        string strDir = data.strFilePath;
        int idx = strDir.find_last_of ('/');
        if(idx >= 0)
            strDir = strDir.substr(0,idx+1);

        QDir dir = QDir::current();//::root();
        dir.mkpath(QString(strDir.c_str()));

        // 파일에 작성
        FILE* f = fopen(data.strFilePath.c_str(), data.strFlag.c_str());
        if(f == NULL) continue;
        fprintf(f, "[%s]%s\n", data.strTime.c_str(), data.strLog.c_str());
        fclose(f);

        msleep(10);


        QDateTime t =  QDateTime::currentDateTime();
        QDateTime t1 = t.addDays(-90); // delete files before 90 days
        DeletePastLogFiles(t1);
    }
    return false;
}

void MLogThread::DeletePastLogFiles(QDateTime t)
{
    string str = string_format("%s/Log", gCfg.RootPath.toLatin1().data());
    QDir dir;
    dir.cd(str.c_str());
    QStringList allFiles = dir.entryList(QDir::Files | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);
    int size = allFiles.size();
    string b = string_format("%s_DevLog.dat", t.toString("yyyyMMdd").toLatin1().data());

    for (int i=0; i<size; i++)
    {
        string f = string(allFiles[i].toLatin1().data());
        if (b > f) {
            string d = str;
            d.append("/");
            d.append(f);
            QFile fsrc(d.c_str());
            if (fsrc.exists())
                dir.remove(f.c_str());
        }
    }
}

int MLogThread::Add(string lpszFilePath, string lpszTime, string lpszLog, string lpszFlag)
{
    m_sync.lock();

    LogWorkData data;
    data.strFilePath = lpszFilePath;
    data.strTime = lpszTime;
    data.strLog = lpszLog;
    data.strFlag = lpszFlag;

    m_Data.push(data);

    m_sync.unlock();
    return 0;
}

