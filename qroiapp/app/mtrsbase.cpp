//
// mtrsbase.cpp : 구현 파일입니다.
//

#include <QMutexLocker>
#include <qmessagebox.h>
#include <qevent.h>
#include "common.h"
#include "mtrsbase.h"
#include "mainwindow.h"

// MTrsBase
MTrsBase::MTrsBase(const char *lpszThreadName) : m_iIndex(-1)
{
    m_strThreadName.sprintf("%s", lpszThreadName);
    Init();
}

MTrsBase::MTrsBase(const char *lpszThreadName, int index) : m_iIndex(index)
{
    m_strThreadName.sprintf("%s%d", lpszThreadName, index + 1);
    Init();
}

void MTrsBase::Init()
{
    m_bThreadLife = m_bThreadTerminate = false;
    SetStep(0);
    m_ErrTimer.StartTimer();

    QObject::connect(this, SIGNAL(threadErrorOccur(void)), theMainWindow, SLOT(ThreadErrorDialog(void)));
}

MTrsBase::~MTrsBase()
{
}

EOPStatus MTrsBase::GetOPStatus()
{
    return m_eOPStatus;
}



void MTrsBase::SetOPStatus(EOPStatus eOPStatus)
{
    if (m_eOPStatus == eOPStatus) return;

    switch (eOPStatus)
    {
    case STEP_STOP:
        break;
    case START_RUN:
        break;
    case ERROR_STOP:
        break;
    case STEP_READY:
        break;
    }

    m_eOPStatus = eOPStatus;
}

// MTrsBase 메시지 처리기입니다.
void MTrsBase::ThreadRun()
{
    m_bThreadLife = true;
    start();
}

void MTrsBase::ThreadStop()
{
    m_bThreadLife = false;
    //Sleep(100);

    exit(0);

}

void MTrsBase::run()
{
    while (m_bThreadLife)
    {
        switch (GetOPStatus())
        {
        case ERROR_STOP:
            //HandleError();
            break;

        case STEP_STOP:
            break;

        case START_RUN:
            doRunStep();
            break;

        case STEP_READY:
            break;
        }
        msleep(10);
    }
}

void MTrsBase::SetStep(int iStep)
{
    m_iCurrentStep = iStep;
    m_MTimer.StartTimer();
}

void MTrsBase::doRunStep(void)
{
}

bool MTrsBase::SetUnitInitialize(void)
{
    SetStep(0);

    return 0;
}


