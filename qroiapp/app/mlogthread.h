#pragma once

#include <string>
#include <queue>
#include <QThread>
#include <QMutex>

using namespace std;

struct LogWorkData
{
    string strFilePath;
    string strTime;
    string strLog;
    string strFlag;
};

using namespace std;

class MLogThread : public QThread
{
public:
	MLogThread();
	virtual ~MLogThread();

public:
    int Add(string lpszFilePath, string lpszTime, string lpszLog, string lpszFlag = "at");

    queue<LogWorkData> m_Data;

    void DeletePastLogFiles(QDateTime t);

protected:
    QMutex m_sync;
	int ThreadStart();
	int ThreadStop();
    virtual void run() Q_DECL_OVERRIDE;
    bool Thread();

	int m_iExitFlag;
};
