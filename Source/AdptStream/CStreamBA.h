/*******************************************************************************
	File:		CStreamBA.h

	Contains:	The stream bitrate adjust header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __CStreamBA_H__
#define __CStreamBA_H__

#include "voSource2.h"
#include "voAdaptiveStreamParser.h"
#include "voSource2_ProgramInfo.h"

#include "yyData.h"
#include "CBaseObject.h"
#include "CNodeList.h"
#include "CMutexLock.h"

class CSourceIO;
class CStreamParser;
class COutBuffer;

class CStreamBA : public CBaseObject
{
public:
struct YYDLInfo
{
	int		nStartTime;
	int		nUsedTime;	
	int		nSize;	
};

public:
    CStreamBA (void);
    virtual ~CStreamBA (void);

	virtual void	SetIO (CSourceIO* pSourceIO) { m_pDataIO = pSourceIO; }
    virtual void	SetParser (CStreamParser* pParser) { m_pParser = pParser; }
    virtual void	SetOutBuffer (COutBuffer* pOutBuff) { m_pOutBuff = pOutBuff; }
    virtual void	SetProgInfo (VOS2_ProgramInfo_Func* pProgInfo) { m_pProgInfo = pProgInfo; }

    virtual int		MonitorPlayBuffer (YY_BUFFER * pBuff);

	virtual int		DownloadStart (VO_ADAPTIVESTREAMPARSER_CHUNK * pChunk);
	virtual int		Downloading (int nSize);
	virtual int		DownloadFinish (int nSize, int nRC);

	virtual int		GetDLSpeed (int nLastTime);

protected:
	virtual int		InitStreamInfo (void);
	virtual int		SelectStreamBySpeed (int nSpeed, long long llTime);

	virtual int		TestStreamChange (long long llPlayTime);

#ifdef _OS_WIN32
	static int __cdecl 	compare_bitrate (const void *arg1, const void *arg2);
#else
	static int 			compare_bitrate (const void *arg1, const void *arg2);
#endif // _OS_WIN32	

protected:
    CSourceIO*				m_pDataIO;
    CStreamParser*			m_pParser;
	COutBuffer *			m_pOutBuff;
	VOS2_ProgramInfo_Func *	m_pProgInfo;

	CMutexLock				m_mtLock;
	int						m_nLastTime;
	int						m_nBAChgNum;

	long long				m_llDLSize;
	int						m_nUsedTime;
	int						m_nDLStart;
	int						m_nChkStart;
	CObjectList <YYDLInfo>	m_lstSpeedInfo;
	CObjectList <YYDLInfo>	m_lstChunkInfo;

    int 					m_nStreamNum;
	VO_SOURCE2_STREAM_INFO* m_pStreamInfo;
	int						m_nBitrateSel;
	int						m_nBitrateMax;
	int						m_nBitrateMin;

	// for test ba 
	int						m_nTstLastTime;
	int						m_nTstChangeNum;
};

#endif
