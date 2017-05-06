/*******************************************************************************
	File:		CStreamDownLoad.h

	Contains:	The stream data download header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __CStreamDownLoad_H__
#define __CStreamDownLoad_H__

#include "voSource2.h"
#include "voAdaptiveStreamParser.h"

#include "CBaseObject.h"
#include "UThreadFunc.h"

class CStreamDemux;
class CStreamParser;
class CSourceIO;
class COutBuffer;
class CStreamBA;
class CStreamDRM;

class CStreamDownLoad : public CBaseObject
{
public:
	CStreamDownLoad(CStreamParser * pParser);
	virtual ~CStreamDownLoad(void);

	virtual void	SetOutBuffer (COutBuffer * pOutBuff) {m_pOutBuffer = pOutBuff;}
	virtual void	SetStreamBA (CStreamBA * pBA) {m_pBA = pBA;}
	virtual void	SetStreamDRM (CStreamDRM * pDRM) {m_pDRM = pDRM;}
	virtual void	SetProgInfoFunc (VOS2_ProgramInfo_Func * pFunc);

	virtual int		SetPos (long long llPos);

	virtual void	Start (int nChunkType);
	virtual void	Stop(void);

protected:
	virtual int		Download (VO_ADAPTIVESTREAMPARSER_CHUNK * pChunk);
	virtual int		Demux (int nFlag, VO_PBYTE pBuff, VO_U32 nSize);

	static	int		DownLoadProc (void * pParam);
	virtual int		DownLoadLoop (void);

protected:
	static VO_S32	ProgInfoNotifyEvent (VO_PTR pUserData, VO_U32 nID, VO_U32 uStatus, VO_PTR pValue);

protected:
	CStreamParser *			m_pParser;
	CSourceIO *				m_pIO;
	CStreamDemux *			m_pDemux;
	COutBuffer*				m_pOutBuffer;
	CStreamBA *				m_pBA;
	CStreamDRM *			m_pDRM;
	VOS2_ProgramInfo_Func *	m_pProgFunc;
	VOS2_ProgInfo_Notify	m_sProgNotify;
	
	int						m_nChunkType;
	int						m_nStreamFlag;
	bool					m_bEOS;
	bool					m_bStreamChanged;

	yyThreadHandle			m_hThread;
	YYPLAY_STATUS			m_nStatus;
	bool					m_bWorking;

	VO_PBYTE				m_pBuffChunk;
	int						m_nReadSize;
	int						m_nDLChunkCount;

	int						m_nDbgReadSize;
}; //__CStreamDownLoad_H__

#endif
