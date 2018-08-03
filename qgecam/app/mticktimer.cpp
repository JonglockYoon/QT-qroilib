//
// mticktimer.cpp: implementation of the TickTimer class.
//
//////////////////////////////////////////////////////////////////////

#include <QElapsedTimer>
#include "mticktimer.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MTickTimer::MTickTimer()
{
    myTimer.start();

    m_bIsStart = false;
    StartTimer();
}

MTickTimer::~MTickTimer()
{
}

/**
 * Timer를 시작 한다.
 *
 * @return 0 : 성공, others : 에러
 */
int MTickTimer::StartTimer()
{
    m_dwStart = myTimer.elapsed(); //::GetTickCount();
    m_bIsStart = true;
    return DEF_TIMER_SUCCESS;
}

/**
 * Timer를 시작 한다.
 *
 * @return 0 : 성공, others : 에러
 */
int MTickTimer::StopTimer()
{
    if(!m_bIsStart) return ERR_TIMER_NOT_STARTED;
    m_bIsStart = false;
    return DEF_TIMER_SUCCESS;
}

/**
 * Timer를 시작 한다.
 *
 * @return 0 : 성공, others : 에러
 */
int MTickTimer::PauseTimer()
{
    unsigned long	dwStop;		// Stop시의 값
    unsigned long	dwElasp;	// 경과 시간

    dwStop = myTimer.elapsed(); //::GetTickCount();

    if(!m_bIsStart) return 0;

    if(m_dwStart <= dwStop)
    {
        dwElasp = dwStop - m_dwStart;		//no wrap
    }
    else
    {
        dwElasp = (0xFFFFFFFFL - m_dwStart); //calculate time from start up to wrap
        dwElasp += 1;   						//time to wrap from FFFFFFFF to 0 is 1 tick
        dwElasp += dwStop; 					//add in time after  0
    }

    return (int)(dwElasp/1000.0);
}

/**
 * 경과 시간을 리턴한다.
 *
 *@ return 양수 : 경과 시간 (초)
 */
double	MTickTimer::GetElaspTime()
{
    unsigned long	dwStop;		// Stop시의 값
    unsigned long	dwElasp;	// 경과 시간

    dwStop = myTimer.elapsed(); //GetTickCount();

    if(!m_bIsStart)	return 0.;

    if (m_dwStart <= dwStop)			//check for wrap around condition
    {
        dwElasp = dwStop - m_dwStart;	//no wrap
    }
    else
    {
        dwElasp = (0xFFFFFFFFL - m_dwStart); //calculate time from start up to wrap
        dwElasp += 1;   //time to wrap from FFFFFFFF to 0 is 1 tick
        dwElasp += dwStop; //add in time after  0
    }

    return (double)dwElasp/1000.;
}

/**
 * 지정 시간을 초과 하지 않았는지 체크 한다.
 *
 * @param  dwMsec : 지정시간 msec
 * @return TRUE : 경과함
           FALSE : 경과하지 않음
 */
bool MTickTimer::LessThan(double dSec)
{
    if( GetElaspTime() < dSec ) return true;
    else return false;
}

/**
 * 지정 시간을 초과 했는지 체크 한다.
 *
 * @param  dwMsec : 지정시간 msec
 * @return TRUE : 경과하지 않음
           FALSE : 경과함
 */
bool MTickTimer::MoreThan(double dSec)
{
    if( GetElaspTime() > dSec )
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * Timer 시작 여부를 알려준다.
 */
bool MTickTimer::IsTimerStarted() const
{
    return m_bIsStart;
}

/* End Of Code */
