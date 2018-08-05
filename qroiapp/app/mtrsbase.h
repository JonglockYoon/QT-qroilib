#pragma once
#include "common.h"
#include "mticktimer.h"
#include <QThread>
#include <string>
#include <QMutex>
#include <QMessageBox>
#include <QWaitCondition>


// MTrsBase

class MTrsBase : public QThread
{
    Q_OBJECT

private:
    QMutex 	m_sync;

    string strLog;
	void Init();

public: // Base
	int m_iIndex;

    virtual void run() Q_DECL_OVERRIDE;
	void ThreadStop();
	void ThreadRun();
    MTrsBase(const char *lpszThreadName);
    MTrsBase(const char *lpszThreadName, int index);
	virtual ~MTrsBase();
    EOPStatus GetOPStatus();
    void SetOPStatus(EOPStatus eOPStatus);

public:
	int m_iCurrentStep;
	int m_iErrorCode;
	int m_iPrevErrorCode;
	MTickTimer m_MTimer;
	MTickTimer m_ErrTimer;

    QString m_strThreadName;
    QString m_strUnitName;

    int m_wAxisNo;

public: // Thread
    bool m_bThreadLife;
    bool m_bThreadTerminate;

public:
	virtual void doRunStep(void);
    virtual bool SetUnitInitialize(void);

	void SetStep(int iStep);

signals:
    void threadErrorOccur(void);

private:
    EOPStatus			m_eOPStatus;//OP Status
    //QMutex mutex;
};
