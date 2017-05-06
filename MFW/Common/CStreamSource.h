/*******************************************************************************
	File:		CStreamSource.h

	Contains:	The adaption stream source header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __CStreamSource_H__
#define __CStreamSource_H__

#include "CBaseSource.h"

#include "CSourceIO.h"
#include "CStreamParser.h"
#include "CProgInfo.h"
#include "CStreamDownLoad.h"
#include "COutBuffer.h"
#include "CStreamBA.h"
#include "CStreamDRM.h"

class CStreamSource : public CBaseSource
{
public:
	CStreamSource(void * hInst);
	virtual ~CStreamSource(void);

	virtual int		Open (const TCHAR * pSource, int nType);
	virtual int		Close (void);
	virtual int		ForceClose (void);

	virtual int		Start (void);
	virtual int		Stop (void);

	virtual int		ReadData (YY_BUFFER * pBuff);

	virtual int		SetPos (long long llPos);

	virtual int		GetMediaInfo (TCHAR * pInfo, int nSize);

protected:
	virtual int		FillPlayListData (VO_ADAPTIVESTREAM_PLAYLISTDATA * pData);

public:
	static VO_S32	parserNotifyEvent (VO_PTR pUserData, VO_U32 nID, VO_U32 nParam1, VO_U32 nParam2);
	static VO_S32	ProgInfoNotifyEvent (VO_PTR pUserData, VO_U32 nID, VO_U32 uStatus, VO_PTR pValue);

protected:
	char						m_szURL[2048];
	CSourceIO *					m_pIO;
	CProgInfo *					m_pProgInfo;
	VOS2_ProgramInfo_Func *		m_pProgFunc;
	CStreamParser *				m_pParser;
	CStreamDownLoad *			m_pDLVideo;
	COutBuffer *				m_pOutBuff;
	CStreamBA *					m_pBA;
	CStreamDRM *				m_pDRM;

	VO_SOURCE2_EVENTCALLBACK	m_cbParser;
	VOS2_ProgInfo_Notify		m_progNotify;

};

#endif // __CStreamSource_H__
