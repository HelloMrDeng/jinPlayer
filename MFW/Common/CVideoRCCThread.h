/*******************************************************************************
	File:		CVideoRCCThread.h

	Contains:	The thread video color convert and resize header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-10-06		Fenger			Create file

*******************************************************************************/
#ifndef __CVideoRCCThread_H__
#define __CVideoRCCThread_H__

#include "CFFMpegVideoRCC.h"

#include "CThreadSem.h"
#include "UThreadFunc.h"

class CVideoRCCThread : public CFFMpegVideoRCC
{
public:
	CVideoRCCThread(void * hInst, int nID);
	virtual ~CVideoRCCThread(void);

	virtual int		ConvertBuff (YY_BUFFER * iBuff, YY_VIDEO_BUFF * oBuff);
	virtual bool	Finished (void);
	virtual bool	WaitTask (void) {return m_bWait;}

protected:
	int					m_nID;
	YY_BUFFER *			m_pInBuff;
	YY_VIDEO_BUFF *		m_pOutBuff;
	bool				m_bFinished;

protected:
	CThreadSem *		m_pThdSem;
	yyThreadHandle		m_hThread;
	bool				m_bStop;
	bool				m_bWait;

	static	int		RCCProc (void * pParam);
	virtual int		RCCLoop (void);

};

#endif // __CVideoRCCThread_H__
