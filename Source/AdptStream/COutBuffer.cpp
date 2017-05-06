/*******************************************************************************
	File:		COutBuffer.cpp

	Contains:	output buffer implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "COutBuffer.h"

#include "UYYDataFunc.h"

COutBuffer::COutBuffer (void)
	: CBaseObject ()
	, m_pBA (NULL)
	, m_pProgFunc (NULL)
	, m_uTrackID (0X1001)
	, m_llOffsetTime (0)
	, m_llVAddLastTime (0)
	, m_llVGetLastTime (0)
	, m_llAAddLastTime (0)
	, m_llAGetLastTime (0)
	, m_llSeekPos (0)
	, m_bNeedBuff (false)
	, m_bEOS (false)
{
	SetObjectName ("COutBuffer");

	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));
}

COutBuffer::~COutBuffer(void)
{
	if (m_fmtVideo.pHeadData != NULL)
		delete []m_fmtVideo.pHeadData;
	if (m_fmtAudio.pHeadData != NULL)
		delete []m_fmtAudio.pHeadData;
}

int COutBuffer::GetSample (YY_BUFFER * pBuff)
{
	return VO_RET_SOURCE2_FAIL;
}

int COutBuffer::AddSample (YY_BUFFER * pBuff)
{
	return VO_RET_SOURCE2_FAIL;
}

int COutBuffer::AddBuffer (VO_PARSER_OUTPUT_BUFFER * pBuff, int nFlag)
{
	return VO_RET_SOURCE2_FAIL;
}

int COutBuffer::GetBufferInfo (int nTrackType, VO_S64 * llDur, VO_U32 * nCount)
{
	return VO_RET_SOURCE2_FAIL;
}

int COutBuffer::GetBuffTime (VO_S64 * pGetBuffTime, VO_S64 * pAddBuffTime)
{
	if (pGetBuffTime != NULL)
		*pGetBuffTime = m_llVGetLastTime > m_llAGetLastTime ? m_llVGetLastTime : m_llAGetLastTime;

	if (pAddBuffTime != NULL)
		*pAddBuffTime = m_llVAddLastTime > m_llAAddLastTime ? m_llVAddLastTime : m_llAAddLastTime;

	return VO_ERR_NONE;
}

bool COutBuffer::IsBuffering (void)
{
	return false;
}

int COutBuffer::AddTrackInfo (VO_PARSER_STREAMINFO * pInfo)
{
	if (m_pProgFunc == NULL)
		return VO_RET_SOURCE2_OK;

	VO_U32					nRC = 0;
	int						nIndex = 0;
	VO_SOURCE2_STREAM_INFO	stmInfo;
	memset (&stmInfo, 0, sizeof (VO_SOURCE2_STREAM_INFO));
	nRC = m_pProgFunc->GetStream (m_pProgFunc->hHandle, VOS2_PROGINFO_BYSELECT, 0, &stmInfo);
	if (nRC != VO_ERR_NONE)
		return VO_RET_SOURCE2_OK;

	VO_U32					uMuxStreamID = -1;
	VO_SOURCE2_TRACK_INFO	trkInfo;
	memset (&trkInfo, 0, sizeof (VO_SOURCE2_TRACK_INFO));
	nIndex = 0;
	while (true)
	{
		if (m_pProgFunc->GetTrack (m_pProgFunc->hHandle, VOS2_PROGINFO_BYINDEX, stmInfo.uStreamID, nIndex, &trkInfo, VO_FALSE) != VO_ERR_NONE)
			break;
		if (trkInfo.uTrackType == VO_SOURCE2_TT_STREAM)
		{
			uMuxStreamID = trkInfo.uTrackID;
			break;
		}
		nIndex++;
	}

	VO_SOURCE2_TRACK_INFO * pTrackInfo = NULL;
	if (pInfo->eMediaType == VO_PARSER_MEDIA_TYPE_EX_VIDEO)
	{
		pTrackInfo = new VO_SOURCE2_TRACK_INFO ();
		memset (pTrackInfo, 0, sizeof (VO_SOURCE2_TRACK_INFO));

		pTrackInfo->uTrackID = m_uTrackID++;
		pTrackInfo->uSelInfo = VO_SOURCE2_SELECT_SELECTED;
		pTrackInfo->uTrackType = VO_SOURCE2_TT_VIDEO;
		pTrackInfo->uCodec = pInfo->nVideoCodecType;
		pTrackInfo->sVideoInfo.sFormat.Width  = pInfo->VideoFormat.width;
		pTrackInfo->sVideoInfo.sFormat.Height  = pInfo->VideoFormat.height;
		if (pInfo->nVideoExtraSize > 0)
		{
			pTrackInfo->uHeadSize = pInfo->nVideoExtraSize;
			pTrackInfo->pHeadData = (VO_PBYTE) pInfo->pVideoExtraData;
		}
		pTrackInfo->nMuxTrackID = uMuxStreamID;

		m_pProgFunc->SetTrack (m_pProgFunc->hHandle, stmInfo.uStreamID, pTrackInfo, VOS2_PROGINFO_NEW);
	
		m_pProgFunc->SetTrack (m_pProgFunc->hHandle, stmInfo.uStreamID, pTrackInfo, VOS2_PROGINFO_SELECT);

		delete pTrackInfo;
	}
	else if (pInfo->eMediaType == VO_PARSER_MEDIA_TYPE_EX_AUDIO)
	{
		pTrackInfo = new VO_SOURCE2_TRACK_INFO ();
		memset (pTrackInfo, 0, sizeof (VO_SOURCE2_TRACK_INFO));
		pTrackInfo->uTrackID = m_uTrackID++;
		pTrackInfo->uTrackType = VO_SOURCE2_TT_AUDIO;
		pTrackInfo->uCodec = pInfo->nAudioCodecType;
		pTrackInfo->sAudioInfo.sFormat.SampleRate  = pInfo->AudioFormat.sample_rate;
		pTrackInfo->sAudioInfo.sFormat.Channels  = pInfo->AudioFormat.channels;
		pTrackInfo->sAudioInfo.sFormat.SampleBits  = pInfo->AudioFormat.sample_bits;

		if (pInfo->nAudioExtraSize > 0)
		{
			pTrackInfo->uHeadSize = pInfo->nVideoExtraSize;
			pTrackInfo->pHeadData = (VO_PBYTE) pInfo->pVideoExtraData;
		}

		VOS2_PROGINFO_COMMAND	cmdType = VOS2_PROGINFO_NEW;
		memset (&trkInfo, 0, sizeof (VO_SOURCE2_TRACK_INFO));
		nIndex = 0;
		while (true)
		{
			if (m_pProgFunc->GetTrack (m_pProgFunc->hHandle, VOS2_PROGINFO_BYINDEX, stmInfo.uStreamID, nIndex, &trkInfo, VO_FALSE) != VO_ERR_NONE)
				break;

			if (trkInfo.uTrackType == VO_SOURCE2_TT_AUDIO && trkInfo.nMuxTrackID == uMuxStreamID)
			{
				pTrackInfo->uTrackID = trkInfo.uTrackID;
				strcpy (pTrackInfo->sAudioInfo.chLanguage, trkInfo.sAudioInfo.chLanguage);
				cmdType = VOS2_PROGINFO_UPDATE;
			}
			nIndex++;
		}
		pTrackInfo->nMuxTrackID = uMuxStreamID;

		m_pProgFunc->SetTrack (m_pProgFunc->hHandle, stmInfo.uStreamID, pTrackInfo, cmdType);

		cmdType = VOS2_PROGINFO_SELECT;
		m_pProgFunc->SetTrack (m_pProgFunc->hHandle, stmInfo.uStreamID, pTrackInfo, cmdType);
	
		delete pTrackInfo;
	}

	return VO_RET_SOURCE2_OK;
}
