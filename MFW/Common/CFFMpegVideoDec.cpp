/*******************************************************************************
	File:		CFFMpegVideoDec.cpp

	Contains:	The ffmpeg audio dec implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CFFMpegVideoDec.h"

#include "UFFMpegFunc.h"
#include "USystemFunc.h"

#include "yyConfig.h"
#include "yyLog.h"

CFFMpegVideoDec::CFFMpegVideoDec(void * hInst)
	: CBaseVideoDec (hInst)
	, m_pDecCtx (NULL)
	, m_pNewCtx (NULL)
	, m_pDecVideo (NULL)
	, m_pFrmVideo (NULL)
	, m_pPacket (NULL)
{
	SetObjectName ("CFFVideoDec");
	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
}

CFFMpegVideoDec::~CFFMpegVideoDec(void)
{
	Uninit ();
}

int CFFMpegVideoDec::Init (YY_VIDEO_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;

	Uninit ();

	m_pDecVideo = avcodec_find_decoder ((AVCodecID)pFmt->nCodecID);
	if (m_pDecVideo == NULL)
		return YY_ERR_VIDEO;

	if (pFmt->pPrivateData != NULL)
		m_pDecCtx = (AVCodecContext *)pFmt->pPrivateData;
	else
	{
		m_pNewCtx = avcodec_alloc_context3 (m_pDecVideo);
		m_pDecCtx = m_pNewCtx;
	}
	if (m_pDecCtx == NULL)
		return YY_ERR_MEMORY;

#ifdef _OS_WIN32
	m_pDecCtx->thread_count = yyGetCPUNum ();
	m_pDecCtx->thread_type = FF_THREAD_FRAME;
#else
	int nCPUNum = yyGetCPUNum ();
	if (nCPUNum > 2)
	{
		m_pDecCtx->thread_count = nCPUNum;
		m_pDecCtx->thread_type = FF_THREAD_FRAME;		
	}
#endif // _OS_WIN32
//#define FF_THREAD_FRAME   1 ///< Decode more than one frame at once
//#define FF_THREAD_SLICE   2 ///< Decode more than one part of a single frame at once

	int nRC = avcodec_open2 (m_pDecCtx, m_pDecVideo, NULL);
	if (nRC < 0)
		return YY_ERR_VIDEO;

	m_pFrmVideo = avcodec_alloc_frame ();

	if (pFmt->pPrivateData == NULL && pFmt->pHeadData != NULL)
	{
		int nGotFrame = 0;
		m_pktData.data = pFmt->pHeadData;
		m_pktData.size = pFmt->nHeadSize;
		m_pktData.pts = 0;
		nRC = avcodec_decode_video2(m_pDecCtx, m_pFrmVideo, &nGotFrame, &m_pktData);
	}

	memcpy (&m_fmtVideo, pFmt, sizeof (m_fmtVideo));
	m_fmtVideo.nCodecID = AV_CODEC_ID_NONE;
	m_fmtVideo.pHeadData = NULL;
	m_fmtVideo.nHeadSize = 0;
	m_fmtVideo.pPrivateData = NULL;

	m_bDropFrame = false;
	m_nDecCount = 0;

	m_uBuffFlag = 0;

	m_llLastTime = 0;
	m_llFirstTime = 0;
	m_llFrameTime = 0;


	return YY_ERR_NONE;
}

int CFFMpegVideoDec::Uninit (void)
{
	if (m_pFrmVideo != NULL)
		avcodec_free_frame (&m_pFrmVideo);
	m_pFrmVideo = NULL;

	if (m_pDecCtx != NULL)
		avcodec_close (m_pDecCtx);
	m_pDecCtx = NULL;
	if (m_pNewCtx != NULL)
		av_free (m_pNewCtx);
	m_pNewCtx = NULL;

	return YY_ERR_NONE;
}

int CFFMpegVideoDec::Flush (void)
{
	CAutoLock lock (&m_mtBuffer);

	ResetInfoList ();

	if (m_pDecVideo != NULL)
		avcodec_flush_buffers (m_pDecCtx);

	m_bDropFrame = true;
	m_nDecCount = 0;
	m_llLastTime = 0;
	m_llFirstTime = 0;

	return YY_ERR_NONE;
}

int CFFMpegVideoDec::SetBuff (YY_BUFFER * pBuff)
{
	if (pBuff == NULL)
		return YY_ERR_ARG;

	CAutoLock lock (&m_mtBuffer);
	CBaseVideoDec::SetBuff (pBuff);

	if ((pBuff->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
	{
//		if (m_lstFull.GetCount () > 0)
//			Flush ();

		YY_VIDEO_FORMAT * pFmt = (YY_VIDEO_FORMAT *)pBuff->pFormat;
		int nGotFrame = 0;
		m_pktData.data = pFmt->pHeadData;
		m_pktData.size = pFmt->nHeadSize;
		m_pktData.pts = 0;
		avcodec_decode_video2(m_pDecCtx, m_pFrmVideo, &nGotFrame, &m_pktData);
	}
	else if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
	{
		Flush ();
		m_llLastTime = pBuff->llTime;
		m_llFirstTime = pBuff->llTime;
	}

	if ((pBuff->uFlag & YYBUFF_DROP_FRAME) == YYBUFF_DROP_FRAME)
	{
		m_bDropFrame = true;
		m_nDecCount = 0;
	}

	if ((pBuff->uFlag & YYBUFF_TYPE_PACKET) != YYBUFF_TYPE_PACKET)
	{
		m_pktData.data = pBuff->pBuff;
		m_pktData.size = pBuff->uSize;
		m_pktData.pts = pBuff->llTime;

		m_pPacket = &m_pktData;
	}
	else
	{
		m_pPacket = (AVPacket *)pBuff->pBuff;
	}

	m_pPacket->dts = abs ((int)pBuff->llDelay);

	FillInfo (pBuff);

	return YY_ERR_NONE;
}

int CFFMpegVideoDec::GetBuff (YY_BUFFER * pBuff)
{
	int nRC = YY_ERR_NONE;
	int	nGotFrame = 0;

	if (m_pDecCtx == NULL)
		return YY_ERR_STATUS;
	if (m_pPacket == NULL)
		return YY_ERR_NEEDMORE;

	CAutoLock lock (&m_mtBuffer);

	if ((pBuff->uFlag & YYBUFF_DEC_DISABLE) == YYBUFF_DEC_DISABLE)
	{
		m_bWaitKeyFrame = true;

		if (m_pPacket->pts >= 0)
			pBuff->llTime = m_pPacket->pts;
		else if (m_pPacket->dts >= 0)
			pBuff->llTime = m_pPacket->dts;

		return YY_ERR_NONE;
	}

	if (m_bWaitKeyFrame && m_pPacket != &m_pktData)
	{
		if ((m_pPacket->flags & AV_PKT_FLAG_KEY) != AV_PKT_FLAG_KEY)
			return YY_ERR_RETRY;
		else
			m_bWaitKeyFrame = false;
	}

	m_pDecCtx->skip_frame = AVDISCARD_DEFAULT;
	m_pDecCtx->skip_loop_filter = AVDISCARD_DEFAULT;

#ifdef _OS_WINPC
	if (pBuff->llDelay > YYCFG_DDEBLOCK_TIME * 5)
		m_pDecCtx->skip_loop_filter = AVDISCARD_ALL;
	if (pBuff->llDelay > YYCFG_SKIP_BFRAME_TIME * 5)
		m_pDecCtx->skip_frame = AVDISCARD_NONREF;
#else
	if (pBuff->llDelay > YYCFG_DDEBLOCK_TIME)
		m_pDecCtx->skip_loop_filter = AVDISCARD_ALL;
	if (pBuff->llDelay > YYCFG_SKIP_BFRAME_TIME)
		m_pDecCtx->skip_frame = AVDISCARD_NONREF;
#endif // _OS_WINPC

	if ((pBuff->uFlag & YYBUFF_DEC_DISA_DEBLOCK) == YYBUFF_DEC_DISA_DEBLOCK)
		m_pDecCtx->skip_loop_filter = AVDISCARD_ALL;
	if ((pBuff->uFlag & YYBUFF_DEC_SKIP_BFRAME) == YYBUFF_DEC_SKIP_BFRAME)
		m_pDecCtx->skip_frame = AVDISCARD_NONREF;
	
	if (m_bDropFrame)
		m_pDecCtx->skip_frame = AVDISCARD_NONREF;
		
//	m_pDecCtx->skip_loop_filter = AVDISCARD_ALL;		
//	m_pDecCtx->skip_frame = AVDISCARD_NONREF;
//	m_pDecCtx->skip_loop_filter = AVDISCARD_ALL;

	if (m_pDecVideo->id == AV_CODEC_ID_H264)
	{
#ifdef _OS_NDK
		if (m_fmtVideo.nWidth > 1280 || m_fmtVideo.nHeight > 720)
			m_pDecCtx->skip_loop_filter = AVDISCARD_ALL;	
#endif // _OS_NDK	

#ifdef _OS_WINCE
		if (m_fmtVideo.nWidth >= 640 || m_fmtVideo.nHeight >= 480)
			m_pDecCtx->skip_loop_filter = AVDISCARD_ALL;
#endif // _OS_WINCE
	}

	nRC = avcodec_decode_video2(m_pDecCtx, m_pFrmVideo, &nGotFrame, m_pPacket);
	m_pPacket = NULL;
	if (nGotFrame > 0)
	{
		if (m_bDropFrame)
		{
			if (m_pFrmVideo->pict_type == AV_PICTURE_TYPE_B)
			{
				pBuff->llDelay = m_pFrmVideo->pkt_dts;
				RestoreInfo (pBuff);
				return YY_ERR_RETRY;
			}

			if (m_nDecCount >= 2)
				m_bDropFrame = false;
		}
		m_nDecCount++;
		
		pBuff->uFlag = YYBUFF_TYPE_AVFrame;
		if (m_pFrmVideo->width != m_fmtVideo.nWidth || m_pFrmVideo->height != m_fmtVideo.nHeight)
		{
			m_fmtVideo.nWidth = m_pFrmVideo->width;
			m_fmtVideo.nHeight = m_pFrmVideo->height;
			pBuff->uFlag |= YYBUFF_NEW_FORMAT;
			pBuff->pFormat = &m_fmtVideo;
		}

		pBuff->pBuff = (unsigned char *)m_pFrmVideo;
		pBuff->uSize = g_nAVFrameSize;

		if (m_pFrmVideo->pkt_pts == YY_64_INVALID)
		{
			if (m_llFrameTime > 0)
				m_pFrmVideo->pkt_pts = m_llLastTime + m_llFrameTime;
			else
				m_pFrmVideo->pkt_pts = m_llLastTime + m_pFrmVideo->pkt_duration;
		}

		if (m_pFrmVideo->pkt_pts >= 0)
			pBuff->llTime = m_pFrmVideo->pkt_pts;
		else if (m_pFrmVideo->pkt_dts >= 0)
			pBuff->llTime = m_pFrmVideo->pkt_dts;
		else if (m_pFrmVideo->pts >= 0)
			pBuff->llTime = m_pFrmVideo->pts;
		pBuff->llDelay = m_pFrmVideo->pkt_dts;

		if (m_llLastTime > pBuff->llTime)
		{
			if (m_nDecCount > 1)
				m_llFrameTime = (pBuff->llTime - m_llFirstTime) / (m_nDecCount - 1);
			else
				m_llFrameTime = pBuff->llTime - m_llFirstTime;
		}
		m_llLastTime = pBuff->llTime;
		if (m_llFirstTime == 0)
			m_llFirstTime = pBuff->llTime;

		RestoreInfo (pBuff);

		CBaseVideoDec::GetBuff (pBuff);

		return YY_ERR_NONE;
	}

	return YY_ERR_RETRY;
}
