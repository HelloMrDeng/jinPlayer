/*******************************************************************************
	File:		CStreamDemuxTS.cpp

	Contains:	stream demux implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "CStreamDemuxTS.h"

#include "UYYDataFunc.h"
#include "UVV2FF.h"

CStreamDemuxTS::CStreamDemuxTS (void)
	: CStreamDemux ()
	, m_hDemux (NULL)
	, m_pFmtVideo (NULL)
	, m_pFmtAudio (NULL)
{
	SetObjectName ("CStreamDemuxTS");

	memset (&m_fAPI, 0, sizeof (m_fAPI));
	yyGetTsDemuxFunc (&m_fAPI);

	memset (&m_initInfo, 0, sizeof (m_initInfo));
	m_initInfo.pUserData = this;
	m_initInfo.pProc = demuxCallback;

	memset (&m_inputData, 0, sizeof (m_inputData));

	memset (&m_sBuffer, 0, sizeof (m_sBuffer));
}

CStreamDemuxTS::~CStreamDemuxTS(void)
{
    if (m_hDemux != NULL)
        m_fAPI.Close (m_hDemux);

	yyDataDeleteVideoFormat (&m_pFmtVideo);
	yyDataDeleteAudioFormat (&m_pFmtAudio);
}

int	CStreamDemuxTS::Demux (int nFlag, VO_PBYTE pBuff, VO_S32 nSize)
{
	if (m_hDemux == NULL)
		m_fAPI.Open (&m_hDemux, &m_initInfo);

	m_inputData.nBufLen = nSize;
	m_inputData.pBuf = pBuff;
	m_inputData.nFlag = 0;

	int nRC = VO_ERR_NONE;
	if ((nFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
	{
		//m_inputData.nFlag = VO_PARSER_FLAG_STREAM_CHANGED;
		m_inputData.nFlag = VO_PARSER_FLAG_STREAM_RESET_ALL;
		m_nStreamFlag = YYBUFF_NEW_FORMAT;
		m_bVideoChecked = false;
		m_bAudioChecked = false;
	}
	if ((nFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
	{
		m_nStreamFlag = m_nStreamFlag | YYBUFF_NEW_POS;
		m_bVideoChecked = false;
		m_bAudioChecked = false;
	}

	nRC = m_fAPI.Process (m_hDemux , &m_inputData);

	return nRC;
}

void CStreamDemuxTS::demuxCallback (VO_PARSER_OUTPUT_BUFFER* pData)
{
    CStreamDemuxTS* pDemux = (CStreamDemuxTS *)pData->pUserData;
	pDemux->CheckBuffer (pData);
}

void CStreamDemuxTS::CheckBuffer (VO_PARSER_OUTPUT_BUFFER* pData)
{
	VO_MTV_FRAME_BUFFER * pBuffer = NULL;
	if (pData->nType == VO_PARSER_OT_AUDIO)
	{
		pBuffer = (VO_MTV_FRAME_BUFFER *)pData->pOutputData;
		pBuffer->nStartTime += m_llStartTime;
		if (!m_bAudioChecked)
		{
			if (pBuffer->nStartTime > m_llLastAudioTime && pBuffer->nStartTime < m_llLastAudioTime + 100)
				return;

			if (pBuffer->nStartTime  < m_llSeekPos)
				return;

			m_bAudioChecked = true;
		}
		m_llLastAudioTime = pBuffer->nStartTime;
	}
	else if (pData->nType == VO_PARSER_OT_VIDEO)
	{
		pBuffer = (VO_MTV_FRAME_BUFFER *)pData->pOutputData;
		pBuffer->nStartTime += m_llStartTime;
		if (!m_bVideoChecked)
		{
			if (pBuffer->nStartTime > m_llLastVideoTime && pBuffer->nStartTime < m_llLastVideoTime + 100)
				return;

			if (pBuffer->nStartTime  < m_llSeekPos)
				return;

			if (pBuffer->nFrameType != VO_VIDEO_FRAME_I)
				return;
			m_bVideoChecked = true;
		}
		m_llLastVideoTime = pBuffer->nStartTime;
	}

	SendBuffer (pData);

    m_nStreamFlag = 0;
}

void CStreamDemuxTS::SendBuffer (VO_PARSER_OUTPUT_BUFFER* pData)
{
	if (m_pOutBuffer == NULL) 
		return;

	if (pData->nType == VO_PARSER_OT_STREAMINFO)
	{
		VO_PARSER_STREAMINFO *	pInfo = (VO_PARSER_STREAMINFO *)pData->pOutputData;
		if (pInfo->eMediaType == VO_PARSER_MEDIA_TYPE_EX_VIDEO)
		{
			yyDataDeleteVideoFormat (&m_pFmtVideo);
			m_pFmtVideo = new YY_VIDEO_FORMAT ();
			memset (m_pFmtVideo, 0, sizeof (YY_VIDEO_FORMAT));
			m_pFmtVideo->nSourceType = YY_SOURCE_VV;
			m_pFmtVideo->nCodecID = yyVV2FFVideoCodecID (pInfo->nVideoCodecType);
			m_pFmtVideo->nWidth = pInfo->VideoFormat.width;
			m_pFmtVideo->nHeight = pInfo->VideoFormat.height;
			if (pInfo->nVideoExtraSize > 0)
			{
				m_pFmtVideo->nHeadSize = pInfo->nVideoExtraSize;
				m_pFmtVideo->pHeadData = new VO_BYTE[m_pFmtVideo->nHeadSize];
				memcpy (m_pFmtVideo->pHeadData, pInfo->pVideoExtraData, m_pFmtVideo->nHeadSize);
			}
		}
		else if (pInfo->eMediaType == VO_PARSER_MEDIA_TYPE_EX_AUDIO)
		{
			yyDataDeleteAudioFormat (&m_pFmtAudio);
			m_pFmtAudio = new YY_AUDIO_FORMAT ();
			memset (m_pFmtAudio, 0, sizeof (YY_AUDIO_FORMAT));
			m_pFmtAudio->nSourceType = YY_SOURCE_VV;
			m_pFmtAudio->nCodecID = yyVV2FFAudioCodecID (pInfo->nAudioCodecType);
			m_pFmtAudio->nSampleRate = pInfo->AudioFormat.sample_rate;
			m_pFmtAudio->nChannels = pInfo->AudioFormat.channels;
			m_pFmtAudio->nBits = pInfo->AudioFormat.sample_bits;
			if (pInfo->nAudioExtraSize > 0)
			{
				m_pFmtAudio->nHeadSize = pInfo->nAudioExtraSize;
				m_pFmtAudio->pHeadData = new VO_BYTE[m_pFmtAudio->nHeadSize];
				memcpy (m_pFmtAudio->pHeadData, pInfo->pAudioExtraData, m_pFmtAudio->nHeadSize);
			}
		}
	}
	else if (pData->nType == VO_PARSER_OT_VIDEO)
	{
		VO_MTV_FRAME_BUFFER * pBuff = (VO_MTV_FRAME_BUFFER *)pData->pOutputData;

		m_sBuffer.nType = YY_MEDIA_Video;
		m_sBuffer.uFlag = YYBUFF_TYPE_DATA;
		m_sBuffer.pFormat = NULL;
		if (pBuff->nFrameType == VO_VIDEO_FRAME_I)
			m_sBuffer.uFlag |= YYBUFF_KEY_FRAME;
		if (m_pFmtVideo != NULL)
		{
			m_sBuffer.uFlag |= YYBUFF_NEW_FORMAT;
			m_sBuffer.pFormat = m_pFmtVideo;
		}

		m_sBuffer.pBuff = pBuff->pData;
		m_sBuffer.uSize = pBuff->nSize;
		m_sBuffer.llTime = pBuff->nStartTime;

		m_pOutBuffer->AddSample (&m_sBuffer);

		if (m_pFmtVideo != NULL)
			yyDataDeleteVideoFormat (&m_pFmtVideo);
	}
	else if (pData->nType == VO_PARSER_OT_AUDIO)
	{
		VO_MTV_FRAME_BUFFER * pBuff = (VO_MTV_FRAME_BUFFER *)pData->pOutputData;

		m_sBuffer.nType = YY_MEDIA_Audio;
		m_sBuffer.uFlag = YYBUFF_TYPE_DATA;
		m_sBuffer.pFormat = NULL;
		if (m_pFmtAudio != NULL)
		{
			m_sBuffer.uFlag |= YYBUFF_NEW_FORMAT;
			m_sBuffer.pFormat = m_pFmtAudio;
		}

		m_sBuffer.pBuff = pBuff->pData;
		m_sBuffer.uSize = pBuff->nSize;
		m_sBuffer.llTime = pBuff->nStartTime;

		m_pOutBuffer->AddSample (&m_sBuffer);

		if (m_pFmtAudio != NULL)
			yyDataDeleteAudioFormat (&m_pFmtAudio);
	}
}

