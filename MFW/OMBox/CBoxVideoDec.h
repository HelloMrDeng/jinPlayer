/*******************************************************************************
	File:		CBoxVideoDec.h

	Contains:	The video dec box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxVideoDec_H__
#define __CBoxVideoDec_H__

#include "CBoxBase.h"
#include "CBaseVideoDec.h"
#include "CNodeList.h"

#include "UThreadFunc.h"

//#define YYDBG_VD_OUTPUT

class CBoxVideoDec : public CBoxBase
{
public:
	typedef struct
	{
		long long		llTime;
		long long		llDelay;
		int				nFlag;
	} YYBOX_VBUFF_INFO;

public:
	CBoxVideoDec(void * hInst);
	virtual ~CBoxVideoDec(void);

	virtual int		SetSource (CBoxBase * pSource);
	virtual int		ReadBuffer (YY_BUFFER * pBuffer, bool bWait);
	virtual int		RendBuffer (YY_BUFFER * pBuffer, bool bRend);

	virtual int		Start (void);
	virtual int		Stop (void);
	virtual int		SetPos (int nPos, bool bSeek);

protected:
	static	int		DecProc (void * pParam);
	virtual int		DecLoop (void);

	virtual int		DoReadBuffer (YY_BUFFER * pBuffer, bool bWait);
	
	virtual void	UpdateListInfo (YY_BUFFER * pBuffer, bool bInput);
	virtual void	ResetListInfo (bool bDelete);
	
	virtual int		SwitchFFDec (void);

protected:
	CBaseVideoDec *				m_pDec;
	bool						m_bFF;

	// for multi thread
	CMutexLock					m_mtReadSrc;
	yyThreadHandle				m_hThread;
	bool						m_bExitThread;
	bool						m_bNewFormat;
	bool						m_bEOS;
	bool						m_bSetRndPrio;
	int							m_nReadSrcBufs;

	CMutexLock					m_mtBuffer;
	YY_BUFFER					m_bufOutput;
	int							m_nOutCount;

	// keep the small buffer in dec module
	CMutexLock						m_mtInfo;
	CObjectList<YYBOX_VBUFF_INFO>	m_lstFree;
	CObjectList<YYBOX_VBUFF_INFO>	m_lstFull;
	long long						m_lllastOutTime;
	long long						m_lllastChkTime;
	int								m_nLastSysTime;
	
	long long						m_llDbgRndTime;
	int								m_nDbgSysTime;
	
};

#endif // __CBoxVideoDec_H__
