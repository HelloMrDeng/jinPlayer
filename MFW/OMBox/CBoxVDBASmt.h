/*******************************************************************************
	File:		CBoxVDBASmt.h

	Contains:	The video dec box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-09-19		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxVDBASmt_H__
#define __CBoxVDBASmt_H__

#include "CBoxVideoDec.h"

class CBoxVDBASmt : public CBoxVideoDec
{
public:
	CBoxVDBASmt(void * hInst);
	virtual ~CBoxVDBASmt(void);

	virtual int		ReadBuffer (YY_BUFFER * pBuffer, bool bWait);
	virtual int		RendBuffer (YY_BUFFER * pBuffer, bool bRend);

	virtual int		SetPos (int nPos, bool bSeek);

protected:
	virtual int		ReadSource (YY_BUFFER * pBuffer);
	virtual int		GetSoftOutput (YY_BUFFER * pBuffer);
	virtual int		GetHardOutput (YY_BUFFER * pBuffer);

	virtual void	ResetInBuff (bool bDel);
	virtual void	ResetOutBuff (bool bDel);

protected:
	CBaseVideoDec *			m_pSoftDec;

	bool					m_bInEOS;
	CMutexLock				m_mtIn;
	CObjectList<YY_BUFFER>	m_lstInFree;
	CObjectList<YY_BUFFER>	m_lstInFull;

	bool					m_bOutEOS;
	CMutexLock				m_mtOut;
	CObjectList<YY_BUFFER>	m_lstOutFree;
	CObjectList<YY_BUFFER>	m_lstOutFull;
	YY_VIDEO_FORMAT			m_fmtVideo;


	bool					m_bThrdStop;
	static	int		SoftDecProc (void * pParam);
	virtual int		SoftDecLoop (void);
	yyThreadHandle			m_hThrdSoft;
	CMutexLock				m_mtSoft;
	CObjectList<YY_BUFFER>	m_lstInSoft;
	long long				m_llSoftSrc;
	long long				m_llSoftRnd;

	static	int		HardDecProc (void * pParam);
	virtual int		HardDecLoop (void);
	yyThreadHandle			m_hThrdHard;
	bool					m_bNewFormat;
	YY_BUFFER *				m_pLastFmtBuff;
};

#endif // __CBoxVDBASmt_H__
