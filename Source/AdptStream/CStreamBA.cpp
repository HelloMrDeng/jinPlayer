/*******************************************************************************
	File:		CStreamBA.cpp

	Contains:	stream bitrate adjust implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "CStreamBA.h"

#include "CSourceIO.h"
#include "CStreamParser.h"
#include "COutBuffer.h"

#include "USystemFunc.h"
#include "yyCfgBA.h"
#include "yyLog.h"

CStreamBA::CStreamBA(void)
	: m_pDataIO (NULL)
	, m_pParser (NULL)
	, m_pOutBuff (NULL)
	, m_pProgInfo (NULL)
	, m_nLastTime (0)
	, m_nBAChgNum (0) 
	, m_llDLSize (0)
	, m_nUsedTime (1)
	, m_nDLStart (0)
	, m_nChkStart (0)
	, m_nStreamNum (0)
	, m_pStreamInfo (NULL)
	, m_nBitrateSel (-1)
	, m_nBitrateMax (-1)
	, m_nBitrateMin (-1)
	, m_nTstLastTime (0)
	, m_nTstChangeNum (0)
{
	SetObjectName ("CStreamBA");
}

CStreamBA::~CStreamBA(void)
{
	YYDLInfo * pInfo = m_lstSpeedInfo.RemoveHead ();
	while (pInfo != NULL)
	{
		delete pInfo;
		pInfo = m_lstSpeedInfo.RemoveHead ();
	}

	pInfo = m_lstChunkInfo.RemoveHead ();
	while (pInfo != NULL)
	{
		delete pInfo;
		pInfo = m_lstChunkInfo.RemoveHead ();
	}

	YY_DEL_A (m_pStreamInfo);
}

int CStreamBA::MonitorPlayBuffer (YY_BUFFER * pBuff)
{
	CAutoLock lock (&m_mtLock);
	if (m_nStreamNum <= 1)
		return YY_ERR_NONE;
#if 1
	if (pBuff->llTime > YYCFG_BA_STARTCHECK_TIME)
	{
		int nSpeed = GetDLSpeed (YYCFG_BA_LASTTIME_DOWNLOAD);
		SelectStreamBySpeed (nSpeed, pBuff->llTime);
	}
#else
	TestStreamChange (pBuff->llTime);
#endif // 000
    return YY_ERR_NONE;
}

int CStreamBA::SelectStreamBySpeed (int nSpeed, long long llTime)
{
	if (yyGetSysTime () - m_nLastTime < 2000)
		return YY_ERR_RETRY;
	m_nLastTime = yyGetSysTime ();
		
	nSpeed = (nSpeed / 100) * 75;
//	YYLOGI ("Speed: % 8d,  Time: % 8d, max % 8d,  Min % 8d", nSpeed, (int)llTime, m_nBitrateMax, m_nBitrateMin);
	
	int	nStreamSel = 0;
	if (nSpeed >= m_nBitrateMax)
	{
		nStreamSel = m_nStreamNum - 1;
	}
	else if (nSpeed <= m_nBitrateMin)
	{
		nStreamSel = 0;
	}
	else
	{
		for (int i = 0; i < m_nStreamNum; i++)
		{
			if (nSpeed < (int)m_pStreamInfo[i].uBitrate)
			{
				nStreamSel = i - 1;
				if (nStreamSel < 0)
					nStreamSel = 0;
				break;
			}
		}
	}	
	if (nStreamSel != m_nBitrateSel)
	{
		if (m_nBAChgNum <= 4 && nStreamSel > m_nBitrateSel)
		{
			m_nBAChgNum++;
			if (nSpeed < m_pStreamInfo[nStreamSel].uBitrate * 2)
				return YY_ERR_NONE;
		}
					
		if (m_pParser->SelectStream (m_pStreamInfo[nStreamSel].uStreamID) == VO_ERR_NONE)
		{
			long long	llDur = 0;
			VO_U32		nCount = 0;
			m_pOutBuff->GetBufferInfo (VO_SOURCE2_TT_VIDEO, &llDur, &nCount);
			if (nStreamSel > m_nBitrateSel)
			{
				VO_U64 uTime = llTime;
				m_pParser->Seek (&uTime, VO_ADAPTIVESTREAMPARSER_SEEKMODE_OBSOLUTE);
				YYLOGI ("YYBA: Change stream to % 6d  Seek: % 6d  Speed: % 8d  Buff  % 6d", 
						m_pStreamInfo[nStreamSel].uBitrate, (int)uTime, nSpeed, (int)llDur);
			}
			else
			{
				YYLOGI ("YYBA: Change stream to % 8d DL Speed: % 8d  Buff  % 8d", m_pStreamInfo[nStreamSel].uBitrate, nSpeed, (int)llDur);
			}
			m_nBitrateSel = nStreamSel;
			m_nBAChgNum++;
		}
		else
		{
			YYLOGI ("Select stream id % 8d failed!", m_pStreamInfo[nStreamSel].uStreamID);
		}		
	}

	return YY_ERR_NONE;
}

int CStreamBA::DownloadStart (VO_ADAPTIVESTREAMPARSER_CHUNK * pChunk)
{
	CAutoLock lock (&m_mtLock);
	if (m_nStreamNum == 0)
		InitStreamInfo ();

	m_nDLStart = yyGetSysTime ();
	m_nChkStart = yyGetSysTime ();

	return YY_ERR_NONE;
}

int CStreamBA::Downloading (int nSize)
{
	CAutoLock lock (&m_mtLock);
	if (m_nStreamNum <= 1)
		return YY_ERR_NONE;

	YYDLInfo * pInfo = NULL;
	if (m_lstSpeedInfo.GetCount () > YYCFG_BA_MAXNUM_DLINFO)
		pInfo = m_lstSpeedInfo.RemoveHead ();
	else
		pInfo = new YYDLInfo ();
	pInfo->nStartTime = m_nDLStart;
	pInfo->nUsedTime = yyGetSysTime () - m_nDLStart;
	pInfo->nSize = nSize;
	m_lstSpeedInfo.AddTail (pInfo);
//	YYLOGI ("Info: Start % 6d, Used % 6d, Size % 6d", pInfo->nStartTime, pInfo->nUsedTime, pInfo->nSize);
	m_nDLStart = yyGetSysTime ();

	m_llDLSize += nSize;
	m_nUsedTime += pInfo->nUsedTime;

	return YY_ERR_NONE;
}

int CStreamBA::DownloadFinish (int nSize, int nRC)
{
	if (m_nStreamNum <= 1)
		return YY_ERR_NONE;

#if 0
	YYDLInfo * pInfo = new YYDLInfo ();
	pInfo->nStartTime = m_nChkStart;
	pInfo->nUsedTime = yyGetSysTime () - m_nChkStart;
	pInfo->nSize = nSize;
	m_lstChunkInfo.AddTail (pInfo);
#endif // 0
	return YY_ERR_NONE;
}

int CStreamBA::GetDLSpeed (int nLastTime)
{
	int nSpeed = 0;

	long long	llSize = 0;
	int			nTime = 1;
	
	YYDLInfo *	pInfo = NULL;
	NODEPOS	pos = m_lstSpeedInfo.GetTailPositionI ();
	while (pos != NULL)
	{
		pInfo = m_lstSpeedInfo.GetPrev (pos);
		llSize += pInfo->nSize;
		nTime += pInfo->nUsedTime;
		if (nTime >= nLastTime)
			break;
	}

	if (nTime == 0)
		nSpeed =  0X7FFFFFFF;
	else
		nSpeed = (int)((llSize * 1000 * 8) / nTime);

//	YYLOGI ("Count: % 8d   DLSpeed is % 8d   Total % 8d", m_lstSpeedInfo.GetCount (), nSpeed, (int)(m_llDLSize * 1000 * 8 / m_nUsedTime));

	return nSpeed;
}

#ifdef _OS_WIN32
int __cdecl CStreamBA::compare_bitrate (const void *arg1, const void *arg2)
#else
int CStreamBA::compare_bitrate (const void *arg1, const void *arg2)
#endif // _OS_WIN32
{
	return  ((VO_SOURCE2_STREAM_INFO *)arg1)->uBitrate - ((VO_SOURCE2_STREAM_INFO *)arg2)->uBitrate;
}

int CStreamBA::InitStreamInfo (void)
{
	int nIndex = 0;
	int nPlayBitrate = 0;
	int nRC = VO_ERR_NONE;
	VO_SOURCE2_STREAM_INFO	strmInfo;
	m_nStreamNum = 0;
	m_nBitrateMin = 0X7FFFFFFF;
	m_nBitrateMax = 0;
	while (nRC == YY_ERR_NONE)
	{
		nRC = m_pProgInfo->GetStream (m_pProgInfo->hHandle, VOS2_PROGINFO_BYINDEX, nIndex, &strmInfo);
		if (nRC == YY_ERR_NONE)
			m_nStreamNum++;
		nIndex++;
	}

	YY_DEL_A (m_pStreamInfo);
	m_pStreamInfo = new VO_SOURCE2_STREAM_INFO[m_nStreamNum];
	nIndex = 0;
	nRC = m_pProgInfo->GetStream (m_pProgInfo->hHandle, VOS2_PROGINFO_BYINDEX, nIndex, &strmInfo);
	while (nRC == YY_ERR_NONE)
	{
		memcpy (&m_pStreamInfo[nIndex], &strmInfo, sizeof (strmInfo));
		if (m_nBitrateMax < (int)strmInfo.uBitrate)
			m_nBitrateMax = strmInfo.uBitrate;
		if (m_nBitrateMin > (int)strmInfo.uBitrate)
			m_nBitrateMin = strmInfo.uBitrate;
		if (strmInfo.uSelInfo > 0)
		{
			m_nBitrateSel = nIndex;
			nPlayBitrate = strmInfo.uBitrate;
		}
		nIndex++;
		nRC = m_pProgInfo->GetStream (m_pProgInfo->hHandle, VOS2_PROGINFO_BYINDEX, nIndex, &strmInfo);
	}

	qsort (m_pStreamInfo, m_nStreamNum, sizeof(VO_SOURCE2_STREAM_INFO), compare_bitrate);
	
	for (int i = 0; i < m_nStreamNum; i++)
	{
		if (nPlayBitrate == (int)m_pStreamInfo[i].uBitrate)
		{
			m_nBitrateSel = i;
			break;
		}
	}	
	
	for (int i = 0; i < m_nStreamNum; i++)
		YYLOGI ("Stream % 4d   Bitrate: % 8d", i, m_pStreamInfo[i].uBitrate);
	YYLOGI ("Playing % 3d   Bitrate: % 8d", m_nBitrateSel, nPlayBitrate);

	return YY_ERR_NONE;
}

int CStreamBA::TestStreamChange (long long llPlayTime)
{
	if (m_nTstLastTime == 0)
		m_nTstLastTime = yyGetSysTime ();

	if (yyGetSysTime () - m_nTstLastTime >= 10000)
	{
		m_nTstLastTime = yyGetSysTime ();
		m_nTstChangeNum++;

//		int		nStream = m_nTstChangeNum % m_nStreamNum;
		int		nStream = m_nBitrateSel;
		while (nStream == m_nBitrateSel)
			nStream = rand () % m_nStreamNum;
		m_nBitrateSel = nStream;
//		if (nStream == 1)
//			nStream = m_nStreamNum - 1;
		VO_U32	uRC = m_pParser->SelectStream (m_pStreamInfo[nStream].uStreamID);
		if (uRC != VO_ERR_NONE)
			return YY_ERR_FAILED;

		VO_U64 uTime = llPlayTime;
		uRC = m_pParser->Seek (&uTime, VO_ADAPTIVESTREAMPARSER_SEEKMODE_OBSOLUTE);
		YYLOGI ("YYBA: Test Change stream to % 4d Bitrate: % 8d Seek to % 8d", nStream, m_pStreamInfo[nStream].uBitrate, (int)uTime);
	}

	return YY_ERR_NONE;
}