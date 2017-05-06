/*******************************************************************************
	File:		CThreadSem.h

	Contains:	Thread Semaphore header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-10-06		Fenger			Create file

*******************************************************************************/
#ifndef __CThreadSem_H__
#define __CThreadSem_H__

#ifdef _OS_WIN32
#include <windows.h>
#elif defined _OS_LINUX
#include <pthread.h>
#include <time.h>
#endif // _OS_WIN32

#include "CBaseObject.h"

#define YY_SEM_TIMEOUT		0X80000001
#define YY_SEM_MAXTIME		0XFFFFFFFF
#define YY_SEM_OK			0X00000000

// wrapper for whatever critical section we have
class CThreadSem : public CBaseObject
{
public:
    CThreadSem(void);
    virtual ~CThreadSem(void);

    virtual int		Down (int nWaitTime = YY_SEM_MAXTIME);
    virtual int		Up (void);
    virtual int		Reset(void);
    virtual int		Wait (int nWaitTime = YY_SEM_MAXTIME);
    virtual int		Signal (void);
    virtual int		Broadcast (void);
	virtual int		Count (void);
	virtual bool	Waiting (void);

#ifdef _OS_WIN32
	CRITICAL_SECTION	m_CritSec;
	HANDLE				m_hEvent;
#elif defined _OS_LINUX
  pthread_cond_t		m_hCondition;
  pthread_mutex_t		m_hMutex;	
#endif // _OS_WIN32

public:
	int					m_nSemCount;
	bool				m_bWaiting;
};

#endif //__CThreadSem_H__