/*******************************************************************************
	File:		CBaseClock.cpp

	Contains:	The base audio dec implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#include "CBaseClock.h"

#include "USystemFunc.h"
#include "yyLog.h"

CBaseClock::CBaseClock(void)
	: CBaseObject ()
	, m_bRun (false)
	, m_nErrAdj (50)
	, m_nOldAdj (50)
	, m_llStart (0)
	, m_llSystem (0)
	, m_llLast (0)
	, m_llNow (0)
	, m_fSpeed (1.0)
	, m_nOffset (0)
{
	SetObjectName ("CBaseClock");
}

CBaseClock::~CBaseClock(void)
{
}

long long CBaseClock::GetTime (void)
{
	CAutoLock lock (&m_mtTime);
	if (m_bRun)
	{
		if (m_llStart <= 0)
			return 0;

		m_llNow = m_llStart + (int)((yyGetSysTime () - m_llSystem) * m_fSpeed) - m_nOffset;
		if (m_llNow < 0)
			return 0;
			
		return m_llNow;
	}
	else
	{
		return m_llLast;
	}
}

int CBaseClock::SetTime (long long llTime)
{
	CAutoLock lock (&m_mtTime);
	if (m_llStart > 0 && abs ((int)(GetTime () - llTime)) < (m_nErrAdj * m_fSpeed))
		return YY_ERR_NONE;

	m_llStart = llTime;
	if (m_llStart <= 0)
		m_llStart = 1;

	m_llSystem = yyGetSysTime ();

	m_llLast = m_llStart;

	return YY_ERR_NONE;
}

int CBaseClock::SetErrAdj (int nError)
{
	CAutoLock lock (&m_mtTime);

	m_nErrAdj = nError;

	return YY_ERR_NONE;
}

int CBaseClock::GetErrAdj (void)
{
	return m_nErrAdj;
}

int CBaseClock::SetSpeed (float fSpeed)
{
	CAutoLock lock (&m_mtTime);
	if (fSpeed <= 0)
		return YY_ERR_ARG;

	m_llStart = GetTime ();
	m_llSystem = yyGetSysTime ();

	m_llLast = m_llStart;

	m_fSpeed = fSpeed;

	if (m_fSpeed < 0.5)
	{
		if (m_nOldAdj != 5000)
			m_nOldAdj = m_nErrAdj;
		m_nErrAdj = 5000;
	}
	else
	{
		m_nErrAdj = m_nOldAdj;
	}

	return YY_ERR_NONE;
}

float CBaseClock::GetSpeed (void)
{
	CAutoLock lock (&m_mtTime);

	return m_fSpeed;
}

int CBaseClock::SetOffset (int nOffset)
{
	CAutoLock lock (&m_mtTime);

	m_nOffset = nOffset;
	
	return YY_ERR_NONE;	
}
	
int CBaseClock::GetOffset (void)
{
	CAutoLock lock (&m_mtTime);
	
	return m_nOffset;
}

int CBaseClock::Start (void)
{
	CAutoLock lock (&m_mtTime);

	m_bRun = true;

	m_llStart = m_llLast;
	m_llSystem = yyGetSysTime ();

	return YY_ERR_NONE;
}

int CBaseClock::Pause (void)
{
	CAutoLock lock (&m_mtTime);

	m_llLast = GetTime ();
	m_bRun = false;

	return YY_ERR_NONE;
}

bool CBaseClock::IsPaused (void)
{
	CAutoLock lock (&m_mtTime);

	if (m_bRun)
		return false;

	return true;
}