/*******************************************************************************
	File:		CThreadWork.h

	Contains:	the mutex lock header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-24		Fenger			Create file

*******************************************************************************/
#ifndef __CThreadWork_H__
#define __CThreadWork_H__

#include "CBaseObject.h"
#include "UThreadFunc.h"

typedef enum {
	YYWORK_Init		= 0,
	YYWORK_Run		= 1,
	YYWORK_Pause	= 2,
	YYWORK_Stop		= 3,
	YYWORK_MAX		= 0X7FFFFFFF
} YYWORK_STATUS;

class CThreadWork : public CBaseObject
{
public:
    CThreadWork(yyThreadProc fProc, void * pParam);
    virtual ~CThreadWork(void);

	virtual int		Start (void);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		SetStartStopProc (yyThreadProc fStart, yyThreadProc fStop);
	virtual int		SetPriority (yyThreadPriority nPriority);
	YYWORK_STATUS	GetStatus (void) {return m_nStatus;}

protected:
	static	int		WorkProc (void * pParam);
	virtual int		WorkLoop (void);

protected:
	yyThreadProc		m_fProc;
	void *				m_pParam;
	yyThreadProc 		m_fProcStart;
	yyThreadProc 		m_fProcStop;
	yyThreadHandle		m_hThread;
	YYWORK_STATUS		m_nStatus;
	bool				m_bWorking;

	yyThreadPriority	m_nPriority;
	bool				m_bPriority;
};



#endif //__CThreadWork_H__
