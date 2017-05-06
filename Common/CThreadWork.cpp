/*******************************************************************************
	File:		CThreadWork.cpp

	Contains:	Mutex lock implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "CThreadWork.h"

#include "USystemFunc.h"
#include "yyLog.h"

CThreadWork::CThreadWork(yyThreadProc fProc, void * pParam)
	: CBaseObject ()
	, m_fProc (fProc)
	, m_pParam (pParam)
	, m_fProcStart (NULL)
	, m_fProcStop (NULL)
	, m_hThread (NULL)
	, m_nStatus (YYWORK_Init)
	, m_bWorking (false)
	, m_nPriority (YY_THREAD_PRIORITY_NORMAL)
	, m_bPriority (false)
{
	SetObjectName ("CThreadWork");
}

CThreadWork::~CThreadWork(void)
{
	Stop ();
}

int CThreadWork::Start (void)
{
	m_nStatus = YYWORK_Run;
	if (m_hThread == NULL)
	{
		int nID = 0;
		yyThreadCreate (&m_hThread, &nID, WorkProc, this, 0);
	}
	return YY_ERR_NONE;
}

int CThreadWork::Pause (void)
{
	m_nStatus = YYWORK_Pause;
	int nStart = yyGetSysTime ();
	int	nTryTimes = 0;
	while (m_bWorking)
	{
		yySleep (5000);
		nTryTimes++;
		if (yyGetSysTime () - nStart > 5000)
		{
			if (nTryTimes % 100 == 0)
				YYLOGW ("It can't Pause in work thread! It used Time % 8d", yyGetSysTime () - nStart);
		}
	}
	return YY_ERR_NONE;
}

int CThreadWork::Stop (void)
{
	m_nStatus = YYWORK_Stop;
	int nStart = yyGetSysTime ();
	int	nTryTimes = 0;
	while (m_hThread != NULL)
	{
		yySleep (5000);
		nTryTimes++;
		if (yyGetSysTime () - nStart > 5000)
		{
			if (nTryTimes % 100 == 0)
				YYLOGW ("It can't Stop in work thread! It used Time % 8d", yyGetSysTime () - nStart);
		}
	}
	return YY_ERR_NONE;
}

int CThreadWork::SetStartStopProc (yyThreadProc fStart, yyThreadProc fStop)
{
	m_fProcStart = fStart;
	m_fProcStop = fStop;
	return YY_ERR_NONE;
}

int CThreadWork::SetPriority (yyThreadPriority nPriority)
{
	if (m_nPriority == nPriority)
		return YY_ERR_NONE;

	m_nPriority = nPriority;
	m_bPriority = true;
	return YY_ERR_NONE;
}

int CThreadWork::WorkProc (void * pParam)
{
	CThreadWork * pWork = (CThreadWork *)pParam;
	pWork->WorkLoop ();
	return 0;
}

int CThreadWork::WorkLoop (void)
{
	if (m_fProcStart != NULL)
		m_fProcStart (m_pParam);
		
	while (m_nStatus == YYWORK_Run || m_nStatus == YYWORK_Pause)
	{
		m_bWorking = false;
		if (m_nStatus == YYWORK_Pause)
		{
			yySleep (5000);
			continue;
		}
		m_bWorking = true;

		if (m_bPriority)
		{
			yyThreadSetPriority (yyThreadGetCurHandle (), m_nPriority);
			m_bPriority = false;
		}

		if (m_fProc != NULL)
			m_fProc (m_pParam);
	}
	if (m_fProcStop != NULL)
		m_fProcStop (m_pParam);	
	m_hThread = NULL;
	return YY_ERR_NONE;
}
