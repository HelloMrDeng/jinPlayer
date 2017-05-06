/*******************************************************************************
	File:		COutBuffer.cpp

	Contains:	output buffer implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "COutSampleV2.h"

#include "UVV2FF.h"

#include "USystemFunc.h"
#include "UYYDataFunc.h"
#include "yyCfgBA.h"
#include "yyLog.h"

#define YYMIN_OUTBUFF_TIME	3000

COutSampleV2::COutSampleV2 (void)
	: COutBuffer ()
	, m_pFmtVideo (NULL)
	, m_pFmtAudio (NULL)
{
	SetObjectName ("COutSampleV2");
}

COutSampleV2::~COutSampleV2(void)
{
	DeleteAllSample ();
}

void COutSampleV2::Reset (void)
{
	CAutoLock lock (&m_mtBuff);
	FreeSampleList (&m_lstVideoFull, &m_lstVideoFree);
	FreeSampleList (&m_lstVideoBuff, &m_lstVideoFree);
	FreeSampleList (&m_lstAudioFull, &m_lstAudioFree);
	FreeSampleList (&m_lstAudioBuff, &m_lstAudioFree);

	yyDataDeleteVideoFormat (&m_pFmtVideo);
	yyDataDeleteAudioFormat (&m_pFmtAudio);
}

int COutSampleV2::GetSample (YY_BUFFER * pBuff)
{
	CAutoLock lock (&m_mtBuff);

	COutBuffer::GetSample (pBuff);
	if (IsBuffering ())
		return YY_ERR_BUFFERING;

	if (pBuff->nType == YY_MEDIA_Video)
		return GetVideoSample (pBuff);
	else if (pBuff->nType == YY_MEDIA_Audio)
		return GetAudioSample (pBuff);

	return YY_ERR_FAILED;
}

int	COutSampleV2::GetVideoSample (YY_BUFFER * pBuff)
{
	if (m_pBA != NULL)
		m_pBA->MonitorPlayBuffer(pBuff);

	if (m_lstVideoFull.GetCount () <= 0 && m_lstVideoBuff.GetCount () <= 0)
	{
		if (m_bEOS)
			pBuff->uFlag = YYBUFF_EOS;
		return m_bEOS ? YY_ERR_FINISH : YY_ERR_RETRY;
	}
	if (m_lstVideoBuff.GetCount () > 0)
	{
		YY_BUFFER * pTmpBuff = m_lstVideoBuff.GetTail ();
		if (pTmpBuff->llTime - m_llAGetLastTime > 2000)
		{
			YY_VIDEO_FORMAT *	pFmtVideo = NULL;
			YY_BUFFER *			pKeyBuff = NULL;
			NODEPOS pos = m_lstVideoBuff.GetHeadPosition ();
			while (pos != NULL)
			{
				pTmpBuff = m_lstVideoBuff.GetNext (pos);
				if (pTmpBuff->pFormat != NULL)
					pFmtVideo = (YY_VIDEO_FORMAT *)pTmpBuff->pFormat;
				if ((pTmpBuff->uFlag & YYBUFF_KEY_FRAME) == YYBUFF_KEY_FRAME)
				{
					if ((m_lstVideoFull.GetCount () <= 0) ||
						(pTmpBuff->llTime > m_llVGetLastTime && pTmpBuff->llTime < m_llVGetLastTime + 100))
					{
						pKeyBuff = pTmpBuff;
						break;
					}
				}
			}

			if (pKeyBuff != NULL)
			{
				FreeSampleList (&m_lstVideoFull, &m_lstVideoFree);
				pTmpBuff = m_lstVideoBuff.RemoveHead ();
				while (pTmpBuff != NULL)
				{
					if (pTmpBuff->llTime < pKeyBuff->llTime)
					{
						if (pTmpBuff->pFormat == pFmtVideo)
							pTmpBuff->pFormat = NULL;
						m_lstVideoFree.AddTail (pTmpBuff);
					}
					else
					{
						if (pTmpBuff == pKeyBuff)
						{
							pTmpBuff->pFormat = pFmtVideo;
							pTmpBuff->uFlag |= YYBUFF_NEW_FORMAT;
						}
						m_lstVideoFull.AddTail (pTmpBuff);
					}
					pTmpBuff = m_lstVideoBuff.RemoveHead ();
				}
				YYLOGI ("Switch Video at % 8d", (int)m_llAGetLastTime);
			}
		}
	}

	YY_BUFFER * pFullSample = m_lstVideoFull.RemoveHead ();
	if (pFullSample == NULL)
	{
		yySleep (5000);
		return YY_ERR_RETRY;
	}
		
	memcpy (pBuff, pFullSample, sizeof (YY_BUFFER));
	if (m_bEOS && m_lstVideoFull.GetCount () == 0)
		pBuff->uFlag |= YYBUFF_EOS;

	m_lstVideoFree.AddTail (pFullSample);
	m_llVGetLastTime = pFullSample->llTime;

	return YY_ERR_NONE;
}

int	COutSampleV2::GetAudioSample (YY_BUFFER * pBuff)
{
	if (m_lstAudioFull.GetCount () <= 0 && m_lstAudioBuff.GetCount () <= 0)
	{
		if (m_bEOS)
			pBuff->uFlag = YYBUFF_EOS;
		return m_bEOS ? YY_ERR_FINISH : YY_ERR_RETRY;
	}

	if (m_lstAudioBuff.GetCount () > 0)
	{
		YY_BUFFER * pTmpBuff = m_lstAudioBuff.GetTail ();
		if (pTmpBuff->llTime - m_llAGetLastTime > 2000 || m_lstAudioFull.GetCount () <= 0)
		{
			FreeSampleList (&m_lstAudioFull, &m_lstAudioFree);
			pTmpBuff = m_lstAudioBuff.RemoveHead ();
			while (pTmpBuff != NULL)
			{
				if (pTmpBuff->llTime <= m_llAGetLastTime)
					m_lstAudioFree.AddTail (pTmpBuff);
				else
					m_lstAudioFull.AddTail (pTmpBuff);
				pTmpBuff = m_lstAudioBuff.RemoveHead ();
			}
			YYLOGI ("Switch Audio at % 8d", (int)m_llAGetLastTime);
		}
	}

	YY_BUFFER * pFullSample = m_lstAudioFull.RemoveHead ();
	if (pFullSample == NULL)
	{
		yySleep (2000);
		return YY_ERR_RETRY;
	}
	memcpy (pBuff, pFullSample, sizeof (YY_BUFFER));
	if (m_bEOS && m_lstAudioFull.GetCount () == 0)
		pBuff->uFlag |= YYBUFF_EOS;

	m_lstAudioFree.AddTail (pFullSample);
	m_llAGetLastTime = pFullSample->llTime;

	if (m_lstAudioFull.GetCount () <= 1 && !m_bEOS)
		m_bNeedBuff = true;

	return YY_ERR_NONE;
}

int COutSampleV2::AddSample (YY_BUFFER * pBuff)
{
	CAutoLock lock (&m_mtBuff);
	YY_BUFFER * pNewBuff = NULL;
	YY_BUFFER * pTmpBuff = NULL;
	if (pBuff->nType == YY_MEDIA_Audio)
	{
		if (pBuff->pFormat != NULL)
		{
			yyDataCloneAudioFormat (&m_fmtAudio, (YY_AUDIO_FORMAT *)pBuff->pFormat);

			if (m_lstAudioFull.GetCount () > 0)
			{
				if (m_lstAudioBuff.GetCount () > 0)
					FreeSampleList (&m_lstAudioBuff, &m_lstAudioFree);
				pNewBuff = GetBuffer (&m_lstAudioFree);
				yyDataCloneBuffer (pNewBuff, pBuff);
				m_lstAudioBuff.AddTail (pNewBuff);
				return YY_ERR_NONE;
			}
		}
		if (m_lstAudioBuff.GetCount () > 0)
		{
			pNewBuff = GetBuffer (&m_lstAudioFree);
			yyDataCloneBuffer (pNewBuff, pBuff);
			m_lstAudioBuff.AddTail (pNewBuff);
			return YY_ERR_NONE;
		}

		pNewBuff = GetBuffer (&m_lstAudioFree);
		yyDataCloneBuffer (pNewBuff, pBuff);
		m_lstAudioFull.AddTail (pNewBuff);
	}
	else if (pBuff->nType == YY_MEDIA_Video)
	{
		if (pBuff->pFormat != NULL)
		{
			yyDataCloneVideoFormat (&m_fmtVideo, (YY_VIDEO_FORMAT *)pBuff->pFormat);
			
			if (m_lstVideoFull.GetCount () > 0)
			{
				if (m_lstVideoBuff.GetCount () > 0)
					FreeSampleList (&m_lstVideoBuff, &m_lstVideoFree);
				pNewBuff = GetBuffer (&m_lstVideoFree);
				yyDataCloneBuffer (pNewBuff, pBuff);
				m_lstVideoBuff.AddTail (pNewBuff);
				return YY_ERR_NONE;
			}
		}
		if (m_lstVideoBuff.GetCount () > 0)
		{
			pNewBuff = GetBuffer (&m_lstVideoFree);
			yyDataCloneBuffer (pNewBuff, pBuff);
			m_lstVideoBuff.AddTail (pNewBuff);
			return YY_ERR_NONE;
		}

		pNewBuff = GetBuffer (&m_lstVideoFree);
		yyDataCloneBuffer (pNewBuff, pBuff);
		m_lstVideoFull.AddTail (pNewBuff);
	}

	return YY_ERR_NONE;
}

int	COutSampleV2::SetPos (long long llPos)
{
	m_llSeekPos = llPos;

	Reset ();

	m_llOffsetTime = 0;
	m_llVAddLastTime = 0;
	m_llVGetLastTime = 0;
	m_llAAddLastTime = 0;
	m_llAGetLastTime = 0;

	return YY_ERR_NONE;
}

int COutSampleV2::GetBufferInfo (int nTrackType, VO_S64 * llDur, VO_U32 * nCount)
{
	CAutoLock lock (&m_mtBuff);

	if (llDur != NULL)
		*llDur = 0;
	if (nCount != NULL)
		*nCount = 0;

	if (IsBuffering ())
		return YY_ERR_NONE;

	VO_S64 llBufDur = 0;
	VO_U32 nBufNum = 0;
	if (nTrackType == VO_SOURCE2_TT_VIDEO)
	{
		GetListInfo (&m_lstVideoFull, 0, &llBufDur, &nBufNum);
		if (llDur != NULL)
			*llDur = llBufDur;
		if (nCount != NULL)
			*nCount = nBufNum;

		YY_BUFFER * pEndSample = m_lstVideoFull.GetTail ();
		if (pEndSample == NULL)
			GetListInfo (&m_lstVideoBuff, 0, &llBufDur, &nBufNum);
		else
			GetListInfo (&m_lstVideoBuff, pEndSample->llTime, &llBufDur, &nBufNum);
	}
	else if (nTrackType == VO_SOURCE2_TT_AUDIO)
	{
		GetListInfo (&m_lstAudioFull, 0, &llBufDur, &nBufNum);
		if (llDur != NULL)
			*llDur = llBufDur;
		if (nCount != NULL)
			*nCount = nBufNum;

		YY_BUFFER * pEndSample = m_lstAudioFull.GetTail ();
		if (pEndSample == NULL)
			GetListInfo (&m_lstAudioBuff, 0, &llBufDur, &nBufNum);
		else
			GetListInfo (&m_lstAudioBuff, pEndSample->llTime, &llBufDur, &nBufNum);
	}
	if (llDur != NULL)
		*llDur += llBufDur;
	if (nCount != NULL)
		*nCount += nBufNum;

	return VO_ERR_NONE;
}

int COutSampleV2::GetListInfo (CObjectList <YY_BUFFER> * pList, long long llStart, VO_S64 * llDur, VO_U32 * nCount)
{
	*llDur = 0;
	*nCount = 0;

	if (pList->GetCount () <= 0)
		return VO_ERR_FAILED;

	YY_BUFFER * pEnd = pList->GetTail ();
	if (pEnd->llTime <= llStart)
		return VO_ERR_FAILED;

	int nBufNum = 0;
	YY_BUFFER * pBuffer = NULL;
	NODEPOS pos = pList->GetHeadPosition ();
	while (pos != NULL)
	{
		pBuffer = pList->GetNext (pos);
		if (pBuffer->llTime > llStart)
			break;
		nBufNum++;
	}

	*llDur = pEnd->llTime - pBuffer->llTime;
	*nCount = pList->GetCount () - nBufNum;

	return YY_ERR_NONE;
}

bool COutSampleV2::IsBuffering (void)
{
	if (!m_bNeedBuff || m_bEOS)
		return false;
	if (m_lstAudioBuff.GetCount () >= 100)
	{
		m_bNeedBuff = false; 
		return false;
	}

	YY_BUFFER * pStartSample = m_lstAudioFull.GetHead ();
	YY_BUFFER * pEndSample = m_lstAudioFull.GetTail ();
	if (pStartSample == NULL || pEndSample == NULL)
		return true;

	if (pEndSample->llTime - pStartSample->llTime < YYMIN_OUTBUFF_TIME && 
		m_lstAudioFull.GetCount () < 100)
		return true;
	
	m_bNeedBuff = false; 

	return false;
}

YY_BUFFER *	COutSampleV2::GetBuffer (CObjectList <YY_BUFFER> * pListFree)
{
	YY_BUFFER * pNewBuff = NULL;

	if (pListFree->GetCount () > 2)
	{
		pNewBuff = pListFree->RemoveHead ();
		if (pNewBuff->pFormat != NULL)
			yyDataResetBuffer (pNewBuff, false);
	}
	if (pNewBuff == NULL)
	{
		pNewBuff = new YY_BUFFER ();
		memset (pNewBuff, 0, sizeof (YY_BUFFER));
	}

	return pNewBuff;
}

void COutSampleV2::FreeSampleList (CObjectList <YY_BUFFER> * pListFull, CObjectList <YY_BUFFER> * pListFree)
{
	YY_BUFFER * pBuffer = pListFull->RemoveHead ();
	while (pBuffer != NULL)
	{
		yyDataResetBuffer (pBuffer, false);
		pListFree->AddTail (pBuffer);
		pBuffer = pListFull->RemoveHead ();
	}
}

void COutSampleV2::DeleteAllSample (void)
{
	Reset ();

	YY_BUFFER * pBuffer = m_lstVideoFree.RemoveHead ();
	while (pBuffer != NULL)
	{
		yyDataResetBuffer (pBuffer, true);
		pBuffer = m_lstVideoFree.RemoveHead ();
	}

	pBuffer = m_lstAudioFree.RemoveHead ();
	while (pBuffer != NULL)
	{
		yyDataResetBuffer (pBuffer, true);
		pBuffer = m_lstAudioFree.RemoveHead ();
	}
}

