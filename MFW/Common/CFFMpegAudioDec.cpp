/*******************************************************************************
	File:		CFFMpegAudioDec.cpp

	Contains:	The ffmpeg audio dec implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CFFMpegAudioDec.h"

#include "UFFMpegFunc.h"
#include "UYYDataFunc.h"

#include "yyConfig.h"
#include "yyLog.h"

CFFMpegAudioDec::CFFMpegAudioDec(void * hInst)
	: CBaseAudioDec (hInst)
	, m_pDecCtx (NULL)
	, m_pNewCtx (NULL)
	, m_pDecAudio (NULL)
	, m_pFrmAudio (NULL)
	, m_pPacket (NULL)
	, m_pPktData (NULL)
{
	SetObjectName ("CFFMpegAudioDec");
	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));

	av_init_packet (&m_pktData);
	m_pktData.data = NULL;
	m_pktData.size = 0;

	av_init_packet (&m_pktBuff);
	m_pktBuff.data = NULL;
	m_pktBuff.size = 0;
}

CFFMpegAudioDec::~CFFMpegAudioDec(void)
{
	Uninit ();
}

int CFFMpegAudioDec::Init (YY_AUDIO_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;

	Uninit ();

	m_pDecAudio = avcodec_find_decoder ((AVCodecID)pFmt->nCodecID);
	if (m_pDecAudio == NULL)
		return YY_ERR_AUDIO;

	m_pDecAudio->pExtParam = NULL;

	if (pFmt->pPrivateData != NULL)
		m_pDecCtx = (AVCodecContext *)pFmt->pPrivateData;
	else
	{
		m_pNewCtx = avcodec_alloc_context3 (m_pDecAudio);
		m_pDecCtx = m_pNewCtx;
	}
	if (m_pDecCtx == NULL)
		return YY_ERR_MEMORY;

	int nRC = avcodec_open2 (m_pDecCtx, m_pDecAudio, NULL);
	if (nRC < 0)
		return YY_ERR_AUDIO;

	m_pFrmAudio = avcodec_alloc_frame ();

	memcpy (&m_fmtAudio, pFmt, sizeof (m_fmtAudio));
	m_fmtAudio.nCodecID = AV_CODEC_ID_PCM_S16LE;
	m_fmtAudio.pHeadData = NULL;
	m_fmtAudio.nHeadSize = 0;
	m_fmtAudio.pPrivateData = NULL;

	av_init_packet (&m_pktData);
	m_pktData.data = NULL;
	m_pktData.size = 0;

	m_uBuffFlag = 0;

	return YY_ERR_NONE;
}

int CFFMpegAudioDec::Uninit (void)
{
	if (m_pFrmAudio != NULL)
		avcodec_free_frame (&m_pFrmAudio);
	m_pFrmAudio = NULL;

	if (m_pDecCtx != NULL)
		avcodec_close (m_pDecCtx);
	m_pDecCtx = NULL;
	if (m_pNewCtx != NULL)
		av_free (m_pNewCtx);
	m_pNewCtx = NULL;

	// it was not need to free the packet.
//	av_free_packet (&m_pktData);
	m_pktData.data = NULL;
	m_pktData.size = 0;

	YY_DEL_A (m_pPktData);

	return YY_ERR_NONE;
}

int CFFMpegAudioDec::Flush (void)
{
	CAutoLock lock (&m_mtBuffer);

	if (m_pDecAudio != NULL)
		avcodec_flush_buffers (m_pDecCtx);

	m_pktData.size = 0;

	return YY_ERR_NONE;
}

int CFFMpegAudioDec::SetBuff (YY_BUFFER * pBuff)
{
	CAutoLock lockBuff (&m_mtBuffer);
	if (pBuff == NULL)
		return YY_ERR_ARG;

	CBaseAudioDec::SetBuff (pBuff);

	if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		Flush ();

	if ((pBuff->uFlag & YYBUFF_TYPE_PACKET) != YYBUFF_TYPE_PACKET)
	{
		m_pktBuff.data = pBuff->pBuff;
		m_pktBuff.size = pBuff->uSize;
		m_pktBuff.pts = pBuff->llTime;

		m_pPacket = &m_pktBuff;
	}
	else
	{
		m_pPacket = (AVPacket *)pBuff->pBuff;
	}

	return YY_ERR_NONE;
}

int CFFMpegAudioDec::GetBuff (YY_BUFFER * pBuff)
{
	int			nRC = 0;
	int			nGotFrame = 0;
	AVPacket *	pPacket = NULL;

	CAutoLock lockBuff (&m_mtBuffer);
	if (m_pDecCtx == NULL)
		return YY_ERR_STATUS;

	if (m_pPacket == NULL && m_pktData.size == 0)
		return YY_ERR_RETRY;

	if (m_pktData.size == 0)
		pPacket = m_pPacket;
	else
		pPacket = &m_pktData;
	m_pPacket = NULL;

	pBuff->pBuff = NULL;
	pBuff->uSize = 0;

	nRC = avcodec_decode_audio4 (m_pDecCtx, m_pFrmAudio, &nGotFrame, pPacket);
	if (nRC >= 0 && pPacket->size > nRC + 2)
	{
		if (pPacket != &m_pktData)
		{
			memcpy (&m_pktData, pPacket, sizeof (AVPacket));
			m_pktData.buf = NULL;
			if (m_pPktData != NULL)
			{
				if (pPacket->size > m_nPktSize)
				{
					delete []m_pPktData;
					m_pPktData = NULL;
				}
			}
			if (m_pPktData == NULL)
			{
				m_nPktSize = pPacket->size;
				m_pPktData = new unsigned char[m_nPktSize];
			}
			memcpy (m_pPktData, pPacket->data, pPacket->size);
			m_pktData.data = m_pPktData;
			m_pktData.size = pPacket->size;
		}

		m_pktData.data = m_pktData.data + nRC;
		m_pktData.size = m_pktData.size - nRC;

		long long llPcmTime = 0;
		if (nGotFrame)
			llPcmTime = m_pFrmAudio->nb_samples * 1000 / m_fmtAudio.nSampleRate;
		m_pktData.dts += llPcmTime;
		m_pktData.pts += llPcmTime;
	}
	else
	{
		if (pPacket == &m_pktData)
			m_pktData.size = 0;
	}

	if (nRC < 0)
		return YY_ERR_AUDIO;

	if (nGotFrame > 0)
	{
		nRC = YY_ERR_NONE;
		if (m_fmtAudio.nChannels > 2)
			m_fmtAudio.nChannels = 2;

		if (m_pFrmAudio->sample_rate != m_fmtAudio.nSampleRate)
		{
			m_fmtAudio.nSampleRate = m_pFrmAudio->sample_rate;
			nRC = YY_ERR_FORMAT;
		}
		if (m_pFrmAudio->channels <= 2 && m_pFrmAudio->channels != m_fmtAudio.nChannels)
		{
			m_fmtAudio.nChannels = m_pFrmAudio->channels;
			nRC = YY_ERR_FORMAT;
		}

		pBuff->uFlag = YYBUFF_TYPE_AVFrame;
		pBuff->pBuff = (unsigned char *)m_pFrmAudio;
		pBuff->uSize = g_nAVFrameSize;

		if (m_pFrmAudio->pkt_pts >= 0)
			pBuff->llTime = m_pFrmAudio->pkt_pts;
		else if (m_pFrmAudio->pkt_dts >= 0)
			pBuff->llTime = m_pFrmAudio->pkt_dts;
		else if (m_pFrmAudio->pts >= 0)
			pBuff->llTime = m_pFrmAudio->pts;

		if (nRC == YY_ERR_FORMAT)
		{
			pBuff->uFlag |= YYBUFF_NEW_FORMAT;
			pBuff->pFormat = &m_fmtAudio;
		}

		CBaseAudioDec::GetBuff (pBuff);

		return YY_ERR_NONE;
	}

	if (m_pktData.size > 0)
		return YY_ERR_NONE;

	return YY_ERR_FAILED;
}