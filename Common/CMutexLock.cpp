/*******************************************************************************
	File:		CMutexLock.cpp

	Contains:	Mutex lock implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "CMutexLock.h"

CMutexLock::CMutexLock(void)
{
	SetObjectName ("CMutexLock");

#ifdef _OS_WIN32
    InitializeCriticalSection(&m_CritSec);
#elif defined _OS_LNXPC
	pthread_mutexattr_t attr;
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init (&m_hMutex, &attr);
#elif defined _OS_NDK
	pthread_mutexattr_t attr = PTHREAD_MUTEX_RECURSIVE_NP;
	pthread_mutex_init (&m_hMutex, &attr);
#endif // _OS_WIN32

    m_nCurrentOwner = 0;
	m_nLockCount = 0;
}

CMutexLock::~CMutexLock(void)
{
#ifdef _OS_WIN32
    DeleteCriticalSection(&m_CritSec);
#elif defined _OS_LINUX
	pthread_mutex_destroy (&m_hMutex);
#endif // _OS_WIN32
}

void CMutexLock::Lock(void)
{
	unsigned int nNewOwner = 0;
#ifdef _OS_WIN32
    nNewOwner = GetCurrentThreadId();
    EnterCriticalSection(&m_CritSec);
#elif defined _OS_LINUX
	nNewOwner = pthread_self();
	pthread_mutex_lock (&m_hMutex);
#endif // _OS_WIN32

    if (0 == m_nLockCount++)
        m_nCurrentOwner = nNewOwner;
}

void CMutexLock::Unlock(void)
{
    if (0 == --m_nLockCount)
        m_nCurrentOwner = 0;
#ifdef _OS_WIN32
    LeaveCriticalSection(&m_CritSec);
#elif defined _OS_LINUX
	pthread_mutex_unlock (&m_hMutex);
#endif // _OS_WIN32
}

bool CMutexLock::IsLock (void)
{
	return m_nLockCount > 0 ? true : false;
}

