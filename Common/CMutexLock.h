/*******************************************************************************
	File:		CMutexLock.h

	Contains:	the mutex lock header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-24		Fenger			Create file

*******************************************************************************/
#ifndef __CMutexLock_H__
#define __CMutexLock_H__

#include "CBaseObject.h"
#ifdef _OS_LINUX
#include <pthread.h>
#endif // _OS_LINUX

class CMutexLock : public CBaseObject
{
public:
    CMutexLock(void);
    virtual ~CMutexLock(void);

    void	Lock(void);
    void	Unlock(void);
	bool	IsLock (void);

public:
#ifdef _OS_WIN32
    CRITICAL_SECTION	m_CritSec;
#elif defined _OS_LINUX
	pthread_mutex_t		m_hMutex;
#endif // _OS_WIN32

	unsigned int		m_nCurrentOwner;
	unsigned int		m_nLockCount;
};

// locks a critical section, and unlocks it automatically
// when the lock goes out of scope
class CAutoLock
{
protected:
    CMutexLock * m_pLock;

public:
    CAutoLock(CMutexLock * plock)
    {
        m_pLock = plock;
        if (m_pLock) {
            m_pLock->Lock();
        }
    };

    ~CAutoLock()
	{
        if (m_pLock) {
            m_pLock->Unlock();
        }
    };
};

#endif //__CMutexLock_H__
