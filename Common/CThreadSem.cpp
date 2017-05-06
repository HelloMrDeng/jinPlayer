/*******************************************************************************
	File:		CThreadSem.cpp

	Contains:	Thread Semaphore implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-10-06		Fenger			Create file

*******************************************************************************/
#include "CThreadSem.h"

#ifndef _OS_WINCE
#include "errno.h"
#endif //_OS_WINCE

#include "string.h"

#ifdef _OS_LINUX
#include "sys/time.h"
#endif

CThreadSem::CThreadSem()
	: m_nSemCount (0)
	, m_bWaiting (false)
{
	SetObjectName ("CThreadSem");

#ifdef _OS_WIN32
	InitializeCriticalSection(&m_CritSec);
	m_hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
#elif defined _OS_LINUX
	pthread_cond_init (&m_hCondition, NULL);
	pthread_mutex_init (&m_hMutex, NULL);
#endif // _OS_WIN32

}

CThreadSem::~CThreadSem()
{	
#ifdef _OS_WIN32
	CloseHandle (m_hEvent);
	DeleteCriticalSection(&m_CritSec);
#elif defined _OS_LINUX
	pthread_cond_destroy (&m_hCondition);
	pthread_mutex_destroy (&m_hMutex);	 
#endif // _OS_WIN32
}

int CThreadSem::Down (int nWaitTime)
{
	m_bWaiting = true;

#ifdef _OS_WIN32
	int uRC = 0;
	if (m_nSemCount == 0)
	{
		uRC = WaitForSingleObject (m_hEvent, nWaitTime);
		if (uRC == WAIT_TIMEOUT)
			return YY_SEM_TIMEOUT;
	}

	EnterCriticalSection(&m_CritSec);
	ResetEvent (m_hEvent);

	if (m_nSemCount > 0)
		m_nSemCount--;
	
	LeaveCriticalSection(&m_CritSec);
#elif defined _OS_LINUX
	pthread_mutex_lock (&m_hMutex);
	while (m_nSemCount == 0)
	{
		pthread_cond_wait (&m_hCondition, &m_hMutex);
		pthread_mutex_unlock (&m_hMutex);		
/*		
		struct timespec t;
		// david modified to support waiting infinitely 2011-05-12, nWaitTime must be ms
		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		t.tv_sec = ts.tv_sec + nWaitTime / 1000;
		t.tv_nsec = ts.tv_nsec + nWaitTime % 1000 * 1000000;
		t.tv_sec += t.tv_nsec / 1000000000;
		t.tv_nsec %= 1000000000;
		int err = pthread_cond_timedwait (&m_hCondition, &m_hMutex, &t);
		if(0 != err)
		{
			pthread_mutex_unlock (&m_hMutex);
			return YY_SEM_TIMEOUT;
		}
*/		
	}
	m_nSemCount--;
	pthread_mutex_unlock (&m_hMutex);	 
#endif // _OS_WIN32

	m_bWaiting = false;

	return 0;
}

int CThreadSem::Up (void)
{
 #ifdef _OS_WIN32
	EnterCriticalSection(&m_CritSec);

	m_nSemCount++;
	SetEvent (m_hEvent);

	LeaveCriticalSection(&m_CritSec);
#elif defined _OS_LINUX
	pthread_mutex_lock (&m_hMutex);
	m_nSemCount++;
	pthread_cond_signal(&m_hCondition);
	pthread_mutex_unlock (&m_hMutex);
#endif // _OS_WIN32

	return 0;
}

int CThreadSem::Reset(void)
{
#ifdef _OS_WIN32
	EnterCriticalSection(&m_CritSec);

	m_nSemCount = 0;
	ResetEvent (m_hEvent);

	LeaveCriticalSection(&m_CritSec);
#elif defined _OS_LINUX
	pthread_mutex_lock (&m_hMutex);
	m_nSemCount = 0;
	pthread_mutex_unlock (&m_hMutex);	 
#endif // _OS_WIN32

	return 0;
}

int CThreadSem::Wait (int nWaitTime)
{
	int ret = YY_SEM_OK;

#ifdef _OS_WIN32
	if (WaitForSingleObject(m_hEvent, nWaitTime) == WAIT_TIMEOUT)
		return YY_SEM_TIMEOUT;
	ResetEvent (m_hEvent);
#elif defined _OS_LINUX
	pthread_mutex_lock (&m_hMutex);
	pthread_cond_wait (&m_hCondition, &m_hMutex);
	pthread_mutex_unlock (&m_hMutex);		
/*
	struct timespec t;
	// david modified to support waiting infinitely 2011-05-12, nWaitTime must be ms
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	t.tv_sec = ts.tv_sec + nWaitTime / 1000;
	t.tv_nsec = ts.tv_nsec + nWaitTime % 1000 * 1000000;
	t.tv_sec += t.tv_nsec / 1000000000;
	t.tv_nsec %= 1000000000;
	int err = pthread_cond_timedwait (&m_hCondition, &m_hMutex, &t);
	if(0 != err)
		ret = YY_SEM_TIMEOUT;

	pthread_mutex_unlock (&m_hMutex);
*/	
#endif // _OS_WIN32

	return ret;
}

int CThreadSem::Signal (void)
{
#ifdef _OS_WIN32
	SetEvent (m_hEvent);
#elif defined _OS_LINUX
	pthread_mutex_lock (&m_hMutex);
	pthread_cond_signal (&m_hCondition);
	pthread_mutex_unlock (&m_hMutex);	 
#endif // _OS_WIN32

	return 0;
}

int CThreadSem::Broadcast (void)
{
#ifdef _OS_WIN32
	SetEvent (m_hEvent);
#elif defined _OS_LINUX
	pthread_mutex_lock (&m_hMutex);
	pthread_cond_broadcast (&m_hCondition);
	pthread_mutex_unlock (&m_hMutex);	 
#endif // _OS_WIN32

	return 0;
}

int CThreadSem::Count (void)
{
	return m_nSemCount;
}

bool CThreadSem::Waiting (void)
{
	return (bool)(m_bWaiting&&m_nSemCount <= 0);
}
