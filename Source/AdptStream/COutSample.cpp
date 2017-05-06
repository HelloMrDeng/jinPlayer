/*******************************************************************************
	File:		COutBuffer.cpp

	Contains:	output buffer implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "COutSample.h"

#include "UVV2FF.h"

#include "USystemFunc.h"
#include "UYYDataFunc.h"
#include "yyCfgBA.h"
#include "yyLog.h"

COutSample::COutSample (void)
	: COutBuffer ()
	, m_nVStreamFlag (0)
	, m_nAStreamFlag (0)
{
	SetObjectName ("COutSample");

	m_pLstAddVideo = &m_lstVideo1;
	m_pLstGetVideo = &m_lstVideo1;

	m_pLstAddAudio = &m_lstAudio1;
	m_pLstGetAudio = &m_lstAudio1;
}

COutSample::~COutSample(void)
{
	DeleteSampleAll ();
}

void COutSample::Reset (void)
{
	CAutoLock lock (&m_mtBuff);
	FreeSampleList (&m_lstVideo1, &m_lstVideoFree);
	FreeSampleList (&m_lstVideo2, &m_lstVideoFree);
	m_pLstAddVideo = &m_lstVideo1;
	m_pLstGetVideo = &m_lstVideo1;

	FreeSampleList (&m_lstAudio1, &m_lstAudioFree);
	FreeSampleList (&m_lstAudio2, &m_lstAudioFree);
	m_pLstAddAudio = &m_lstAudio1;
	m_pLstGetAudio = &m_lstAudio1;
}

int COutSample::GetSample (YY_BUFFER * pBuff)
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

int	COutSample::GetVideoSample (YY_BUFFER * pBuff)
{
	if (m_pBA != NULL)
		m_pBA->MonitorPlayBuffer(pBuff);

	if (m_pLstGetVideo != m_pLstAddVideo)
		SwitchSampleList ();
	if (m_pLstGetVideo->GetCount () <= 0)
	{
		if (m_bEOS)
			pBuff->uFlag = YYBUFF_EOS;
		return m_bEOS ? YY_ERR_FINISH : YY_ERR_RETRY;
	}
	YY_BUFFER * pFullSample = m_pLstGetVideo->GetHead ();
	if (pFullSample->pBuff == NULL)
		return YY_ERR_RETRY;

	pFullSample = m_pLstGetVideo->RemoveHead ();
	memcpy (pBuff, pFullSample, sizeof (YY_BUFFER));
	if (m_bEOS && m_pLstGetVideo->GetCount () == 0)
		pBuff->uFlag |= YYBUFF_EOS;

	m_lstVideoFree.AddTail (pFullSample);
	m_llVGetLastTime = pFullSample->llTime;

	return YY_ERR_NONE;
}

int	COutSample::GetAudioSample (YY_BUFFER * pBuff)
{
	if (m_pLstGetAudio->GetCount () <= 0)
	{
		if (m_bEOS)
			pBuff->uFlag = YYBUFF_EOS;
		return m_bEOS ? YY_ERR_FINISH : YY_ERR_RETRY;
	}
	YY_BUFFER * pFullSample = m_pLstGetAudio->GetHead ();
	if (pFullSample->pBuff == NULL)
		return YY_ERR_RETRY;

	pFullSample = m_pLstGetAudio->RemoveHead ();
	memcpy (pBuff, pFullSample, sizeof (YY_BUFFER));
	if (m_bEOS && m_pLstGetAudio->GetCount () == 0)
		pBuff->uFlag |= YYBUFF_EOS;

	m_lstAudioFree.AddTail (pFullSample);
	m_llAGetLastTime = pFullSample->llTime;

	if (m_pLstGetAudio->GetCount () <= 1 && !m_bEOS)
	{
		if (m_pLstGetAudio == m_pLstAddAudio)
			m_bNeedBuff = true;
	}

	return YY_ERR_NONE;
}

int COutSample::SwitchSampleList (void)
{
	long long		llPlayTime = 0;
	YY_BUFFER *		pPlayBuff = NULL;
	YY_BUFFER *		pFormatBuff = NULL;
	void *			pFmtVideo = NULL;

	if (m_nVStreamFlag > 0 || m_nAStreamFlag > 0)
		return YY_ERR_RETRY;

	pPlayBuff = m_pLstGetVideo->GetHead ();
	if (pPlayBuff == NULL)
		llPlayTime = m_llVGetLastTime;
	else
		llPlayTime = pPlayBuff->llTime;

	NODEPOS pos = m_pLstAddVideo->GetHeadPosition ();
	while (pos != NULL)
	{
		pPlayBuff = m_pLstAddVideo->GetNext (pos);
		if (pPlayBuff->pFormat != NULL)
		{
			pFmtVideo = pPlayBuff->pFormat;
			pFormatBuff = pPlayBuff;
		}

		if (m_pLstGetVideo->GetCount () > 0)
		{
			if (pPlayBuff->llTime > llPlayTime)
				break;
			if (abs((int) (pPlayBuff->llTime - llPlayTime)) > 50)
				continue;
		}

		if (pPlayBuff->uFlag & YYBUFF_KEY_FRAME)
		{
			FreeSampleList (m_pLstGetVideo, &m_lstVideoFree);

			pPlayBuff->pFormat = pFmtVideo;
			if (pPlayBuff != pFormatBuff)
				pFormatBuff->pFormat = NULL;

			YY_BUFFER *	pTempBuff = NULL;
			while (pPlayBuff != m_pLstAddVideo->GetHead ())
			{
				pTempBuff = m_pLstAddVideo->RemoveHead ();
				m_lstVideoFree.AddTail (pTempBuff);
			}	
			m_pLstGetVideo = m_pLstAddVideo;

			// switch the audio stream
			pPlayBuff = m_pLstGetAudio->GetHead ();
			if (pPlayBuff != NULL)
				llPlayTime = pPlayBuff->llTime;
			FreeSampleList (m_pLstGetAudio, &m_lstAudioFree);
			pTempBuff = m_pLstAddAudio->GetHead ();
			while (pTempBuff != NULL)
			{
				if (pTempBuff->llTime < llPlayTime)
				{
					pTempBuff = m_pLstAddAudio->RemoveHead ();
					m_lstAudioFree.AddTail (pTempBuff);
					pTempBuff = m_pLstAddAudio->GetHead ();
				}
				else
				{
					break;
				}
			}
			m_pLstGetAudio = m_pLstAddAudio;

			YYLOGI("Play new stream at Time % 6d ", (int)llPlayTime);
		
			return YY_ERR_NONE;
		}
	}

	return YY_ERR_FAILED;
}

int COutSample::AddBuffer (VO_PARSER_OUTPUT_BUFFER * pBuff, int nFlag)
{
	if (pBuff == NULL)
		return VO_ERR_INVALID_ARG;

	CAutoLock lock (&m_mtBuff);

	COutBuffer::AddBuffer (pBuff, nFlag);

	if (nFlag > 0)
	{
		m_nVStreamFlag = nFlag;
		m_nAStreamFlag = nFlag;
	}

	VO_PARSER_STREAMINFO *	pStreamInfo = NULL;
	int						nStreamType = 0;
	if (pBuff->nType == VO_PARSER_OT_STREAMINFO)
	{
		pStreamInfo = (VO_PARSER_STREAMINFO *)pBuff->pOutputData;
		if (pStreamInfo->eMediaType == VO_PARSER_MEDIA_TYPE_EX_VIDEO)
			nStreamType = VO_PARSER_OT_VIDEO;
		else if (pStreamInfo->eMediaType == VO_PARSER_MEDIA_TYPE_EX_AUDIO)
			nStreamType = VO_PARSER_OT_AUDIO;
	}

	if ((m_nVStreamFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
	{
		if (pBuff->nType == VO_PARSER_OT_VIDEO || nStreamType == VO_PARSER_OT_VIDEO)
			m_nVStreamFlag = 0;
	}
	else if ((m_nVStreamFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
	{
		if (pBuff->nType == VO_PARSER_OT_VIDEO || nStreamType == VO_PARSER_OT_VIDEO)
		{
			m_nVStreamFlag = 0;
			if (m_pLstGetVideo == m_pLstAddVideo)
			{
				if (m_pLstAddVideo == &m_lstVideo1)
					m_pLstAddVideo = &m_lstVideo2;
				else
					m_pLstAddVideo = &m_lstVideo1;
			}
			else
			{
				FreeSampleList (m_pLstAddVideo, &m_lstVideoFree);
			}
		}
	}

	if ((m_nAStreamFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
	{
		if (pBuff->nType == VO_PARSER_OT_AUDIO || nStreamType == VO_PARSER_OT_AUDIO)
			m_nAStreamFlag = 0;
	}
	else if ((m_nAStreamFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
	{
		if (pBuff->nType == VO_PARSER_OT_AUDIO || nStreamType == VO_PARSER_OT_AUDIO)
		{
			m_nAStreamFlag = 0;
			if (m_pLstGetAudio == m_pLstAddAudio)
			{
				if (m_pLstAddAudio == &m_lstAudio1)
					m_pLstAddAudio = &m_lstAudio2;
				else
					m_pLstAddAudio = &m_lstAudio1;
			}
			else
			{
				FreeSampleList (m_pLstAddAudio, &m_lstAudioFree);
			}
		}
	}

	if (pBuff->nType == VO_PARSER_OT_AUDIO)
	{
		VO_MTV_FRAME_BUFFER * pBuffer = (VO_MTV_FRAME_BUFFER *)pBuff->pOutputData;
		AddAudio (pBuffer);
	}
	else if (pBuff->nType == VO_PARSER_OT_VIDEO)
	{
		VO_MTV_FRAME_BUFFER * pBuffer = (VO_MTV_FRAME_BUFFER *)pBuff->pOutputData;
		AddVideo (pBuffer);
	}
	else if (pBuff->nType == VO_PARSER_OT_STREAMINFO)
	{
		VO_PARSER_STREAMINFO * pInfo = (VO_PARSER_STREAMINFO *)pBuff->pOutputData;
		AddTrackInfo (pInfo);
	}
	else if (pBuff->nType == VO_PARSER_OT_TRACKINFO)
	{
	}

	return 0;
}

int COutSample::AddVideo (VO_MTV_FRAME_BUFFER * pBuff)
{
	YY_BUFFER * pBuffer = NULL;
	bool		bAdd = false;
	pBuffer = m_pLstAddVideo->GetTail ();
	if (pBuffer == NULL)
	{
		if (m_lstVideoFree.GetCount () > 2)
		{
			pBuffer = m_lstVideoFree.RemoveHead ();
			if (pBuffer->pFormat != NULL)
				yyDataResetBuffer (pBuffer, false);
		}
		bAdd = true;
	}
	else if (pBuffer->pBuff != NULL )
	{
		pBuffer = NULL;
		if (m_lstVideoFree.GetCount () > 2)
		{
			pBuffer = m_lstVideoFree.RemoveHead ();
			if (pBuffer->pFormat != NULL)
				yyDataResetBuffer (pBuffer, false);
		}
		bAdd = true;
	}
	if (pBuffer == NULL)
	{
		pBuffer = new YY_BUFFER ();
		memset (pBuffer, 0, sizeof (YY_BUFFER));
	}

	if (pBuffer->pBuff != NULL)
	{
		if (pBuffer->uBuffSize < pBuff->nSize || pBuffer->uBuffSize > pBuff->nSize * 2)
		{
			delete []pBuffer->pBuff;
			pBuffer->pBuff = NULL;
			pBuffer->uBuffSize = 0;
		}
	}

	pBuffer->nType = YY_MEDIA_Video;
	if (bAdd)
		pBuffer->uFlag = 0;
	if (pBuff->nFrameType == VO_VIDEO_FRAME_I)
		pBuffer->uFlag |= YYBUFF_KEY_FRAME;

	pBuffer->uSize = pBuff->nSize;
	if (pBuffer->pBuff == NULL)
	{
		pBuffer->uBuffSize = pBuff->nSize * 3 / 2;
		pBuffer->pBuff = new VO_BYTE[pBuffer->uBuffSize];
	}
	memcpy (pBuffer->pBuff, pBuff->pData, pBuffer->uSize);

	// Check the timestamp to make sure it will be smooth
	if (m_llVAddLastTime > 0)
	{
		if (abs((int) (m_llVAddLastTime - pBuff->nStartTime - m_llOffsetTime)) > YYCFG_BA_BUFFERSTEP_TIME)
		{
			m_llOffsetTime = m_llVAddLastTime - pBuff->nStartTime - 40;
			YYLOGI ("The offset Time is % 8d", (int)m_llOffsetTime);
		}
	}
	pBuffer->llTime = pBuff->nStartTime + m_llOffsetTime;;
	m_llVAddLastTime = pBuffer->llTime;

	if (bAdd)
		m_pLstAddVideo->AddTail (pBuffer);

	if ((m_nVStreamFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
	{
		pBuffer->uFlag = YYBUFF_NEW_POS;
		m_nVStreamFlag = 0;
	}

	return 0;
}

int COutSample::AddAudio (VO_MTV_FRAME_BUFFER * pBuff)
{
	YY_BUFFER * pBuffer = NULL;
	bool		bAdd =  false;
	pBuffer = m_pLstAddAudio->GetTail ();
	if (pBuffer == NULL)
	{
		if (m_lstAudioFree.GetCount () > 2)
		{
			pBuffer = m_lstAudioFree.RemoveHead ();
			if (pBuffer->pFormat != NULL)
				yyDataResetBuffer (pBuffer, false);
		}
		bAdd = true;
	}
	else if (pBuffer->pBuff != NULL )
	{
		pBuffer = NULL;
		if (m_lstAudioFree.GetCount () > 2)
		{
			pBuffer = m_lstAudioFree.RemoveHead ();
			if (pBuffer->pFormat != NULL)
				yyDataResetBuffer (pBuffer, false);
		}
		bAdd = true;
	}
	if (pBuffer == NULL)
	{
		pBuffer = new YY_BUFFER ();
		memset (pBuffer, 0, sizeof (YY_BUFFER));
	}

	if (pBuffer->pBuff != NULL)
	{
		if (pBuffer->uBuffSize < pBuff->nSize || pBuffer->uBuffSize > pBuff->nSize * 2)
		{
			delete []pBuffer->pBuff;
			pBuffer->pBuff = NULL;
			pBuffer->uBuffSize = 0;
		}
	}

	pBuffer->nType = YY_MEDIA_Audio;
	if (bAdd)
		pBuffer->uFlag = 0;
	pBuffer->uSize = pBuff->nSize;
	if (pBuffer->pBuff == NULL)
	{
		pBuffer->uBuffSize = pBuff->nSize * 2;
		pBuffer->pBuff = new VO_BYTE[pBuffer->uBuffSize];
	}
	memcpy (pBuffer->pBuff, pBuff->pData, pBuffer->uSize);

	// Check the timestamp to make sure it will be smooth
	if (m_llAAddLastTime > 0)
	{
		if (abs((int) (m_llAAddLastTime - pBuff->nStartTime - m_llOffsetTime)) > YYCFG_BA_BUFFERSTEP_TIME)
		{
			m_llOffsetTime = m_llAAddLastTime - pBuff->nStartTime - 40;
			YYLOGI ("The offset Time is % 8d", (int)m_llOffsetTime);
		}
	}
	pBuffer->llTime = pBuff->nStartTime + m_llOffsetTime;;
	m_llAAddLastTime = pBuffer->llTime;

	if (bAdd)
		m_pLstAddAudio->AddTail (pBuffer);

	if ((m_nAStreamFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
	{
		pBuffer->uFlag = YYBUFF_NEW_POS;
		m_nAStreamFlag = 0;
	}

	return YY_ERR_NONE;
}

int	COutSample::AddTrackInfo (VO_PARSER_STREAMINFO * pInfo)
{
	COutBuffer::AddTrackInfo (pInfo);

	YY_BUFFER * pBuffer = NULL;
	YY_BUFFER * pNewBuff = NULL;
	if (pInfo->eMediaType == VO_PARSER_MEDIA_TYPE_EX_VIDEO)
	{
		if (m_lstVideoFree.GetCount () > 2)
			pBuffer = m_lstVideoFree.RemoveHead ();
	}
	else if (pInfo->eMediaType == VO_PARSER_MEDIA_TYPE_EX_AUDIO)
	{
		if (m_lstAudioFree.GetCount () > 2)
			pBuffer = m_lstAudioFree.RemoveHead ();
	}
	if (pBuffer == NULL)
	{
		pBuffer = new YY_BUFFER ();
		memset (pBuffer, 0, sizeof (YY_BUFFER));
		pNewBuff = pBuffer;
	}
	if (pBuffer->pBuff != NULL)
		delete []pBuffer->pBuff;
	pBuffer->pBuff = NULL;
	pBuffer->llTime = 0;

	if (pInfo->eMediaType == VO_PARSER_MEDIA_TYPE_EX_AUDIO)
	{
		YY_AUDIO_FORMAT * pFmtAudio = new YY_AUDIO_FORMAT ();
		memset (pFmtAudio, 0, sizeof (YY_AUDIO_FORMAT));

		pFmtAudio->nSourceType = YY_SOURCE_VV;
		pFmtAudio->nCodecID = yyVV2FFAudioCodecID (pInfo->nAudioCodecType);
		pFmtAudio->nSampleRate = pInfo->AudioFormat.sample_rate;
		pFmtAudio->nChannels = pInfo->AudioFormat.channels;
		pFmtAudio->nBits = pInfo->AudioFormat.sample_bits;
		if (pInfo->nAudioExtraSize > 0)
		{
			pFmtAudio->nHeadSize = pInfo->nAudioExtraSize;
			pFmtAudio->pHeadData = new VO_BYTE[pFmtAudio->nHeadSize];
			memcpy (pFmtAudio->pHeadData, pInfo->pAudioExtraData, pFmtAudio->nHeadSize);
		}

		pBuffer->nType = YY_MEDIA_Audio;
		pBuffer->uFlag = YYBUFF_NEW_FORMAT;
		pBuffer->pFormat = pFmtAudio;

		yyDataCloneAudioFormat (&m_fmtAudio, pFmtAudio);

		m_pLstAddAudio->AddTail (pBuffer);
	}
	else if (pInfo->eMediaType == VO_PARSER_MEDIA_TYPE_EX_VIDEO)
	{
		YY_VIDEO_FORMAT * pFmtVideo = new YY_VIDEO_FORMAT ();
		memset (pFmtVideo, 0, sizeof (YY_VIDEO_FORMAT));

		pFmtVideo->nSourceType = YY_SOURCE_VV;
		pFmtVideo->nCodecID = yyVV2FFVideoCodecID (pInfo->nVideoCodecType);
		pFmtVideo->nWidth = pInfo->VideoFormat.width;
		pFmtVideo->nHeight = pInfo->VideoFormat.height;
		if (pInfo->nAudioExtraSize > 0)
		{
			pFmtVideo->nHeadSize = pInfo->nAudioExtraSize;
			pFmtVideo->pHeadData = new VO_BYTE[pFmtVideo->nHeadSize];
			memcpy (pFmtVideo->pHeadData, pInfo->pAudioExtraData, pFmtVideo->nHeadSize);
		}

		pBuffer->nType = YY_MEDIA_Video;
		pBuffer->uFlag = YYBUFF_NEW_FORMAT;
		pBuffer->pFormat = pFmtVideo;

		yyDataCloneVideoFormat (&m_fmtVideo, pFmtVideo);

		m_pLstAddVideo->AddTail (pBuffer);
	}
	else
	{
		if (pNewBuff != NULL)
			delete pNewBuff;
	}

	return 0;
}

int	COutSample::SetPos (long long llPos)
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

int COutSample::GetBufferInfo (int nTrackType, VO_S64 * llDur, VO_U32 * nCount)
{
	CAutoLock lock (&m_mtBuff);

	if (llDur != NULL)
		*llDur = 0;
	if (nCount != NULL)
		*nCount = 0;

	if (nTrackType == VO_SOURCE2_TT_VIDEO)
	{
		YY_BUFFER * pStartSample = m_pLstGetVideo->GetHead ();
		YY_BUFFER * pEndSample = m_pLstGetVideo->GetTail ();
		if (pStartSample == NULL || pEndSample == NULL)
			return VO_ERR_FAILED;

		if (llDur != NULL)
			*llDur = pEndSample->llTime - pStartSample->llTime;
		if (nCount != NULL)
			*nCount = m_pLstGetVideo->GetCount ();
		if (m_pLstGetVideo != m_pLstAddVideo && m_pLstAddVideo->GetCount () > 0)
		{
			int nAddCount = 0;
			YY_BUFFER * pEndAddSample = m_pLstAddVideo->GetTail ();
			if (pEndAddSample->llTime > pEndSample->llTime  && llDur != NULL)
				*llDur = pEndAddSample->llTime - pStartSample->llTime;

			NODEPOS pos = m_pLstAddVideo->GetHeadPosition ();
			while (pos != NULL)
			{
				pStartSample = m_pLstAddVideo->GetNext (pos);
				if (pStartSample->llTime > pEndSample->llTime)
					nAddCount++;
			}
			if (nCount != NULL)
				*nCount += nAddCount;
		}
	}
	else if (nTrackType == VO_SOURCE2_TT_AUDIO)
	{
		YY_BUFFER * pStartSample = m_pLstGetAudio->GetHead ();
		YY_BUFFER * pEndSample = m_pLstGetAudio->GetTail ();
		if (pStartSample == NULL || pEndSample == NULL)
			return VO_ERR_FAILED;

		if (llDur != NULL)
			*llDur = pEndSample->llTime - pStartSample->llTime;
		if (nCount != NULL)
			*nCount = m_pLstGetAudio->GetCount ();
		if (m_pLstGetAudio != m_pLstAddAudio && m_pLstAddAudio->GetCount () > 0)
		{
			int nAddCount = 0;
			YY_BUFFER * pEndAddSample = m_pLstAddAudio->GetTail ();
			if (pEndAddSample->llTime > pEndSample->llTime && llDur != NULL)
				*llDur = pEndAddSample->llTime - pStartSample->llTime;

			NODEPOS pos = m_pLstAddAudio->GetHeadPosition ();
			while (pos != NULL)
			{
				pStartSample = m_pLstAddAudio->GetNext (pos);
				if (pStartSample->llTime > pEndSample->llTime)
					nAddCount++;
			}
			if (nCount != NULL)
				*nCount += nAddCount;
		}
	}

	return VO_ERR_NONE;
}

bool COutSample::IsBuffering (void)
{
	if (!m_bNeedBuff || m_bEOS)
		return false;

	if (m_pLstGetAudio != m_pLstAddAudio)
		return false;

	YY_BUFFER * pStartSample = m_pLstGetAudio->GetHead ();
	YY_BUFFER * pEndSample = m_pLstGetAudio->GetTail ();
	if (pStartSample == NULL || pEndSample == NULL)
		return true;

	if (pEndSample->llTime - pStartSample->llTime < 2000 && m_pLstGetAudio->GetCount () < 50)
		return true;
	
	m_bNeedBuff = false; 
	return false;
}

void COutSample::FreeSampleList (CObjectList <YY_BUFFER> * pListFull, CObjectList <YY_BUFFER> * pListFree)
{
	YY_BUFFER * pBuffer = pListFull->RemoveHead ();
	while (pBuffer != NULL)
	{
		yyDataResetBuffer (pBuffer, false);
		pListFree->AddTail (pBuffer);
		pBuffer = pListFull->RemoveHead ();
	}
}

void COutSample::DeleteSampleAll (void)
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

