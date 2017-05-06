/*******************************************************************************
	File:		CPrima2VideoDec.cpp

	Contains:	The vo video dec wrap implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "CPrima2VideoDec.h"

#include "yyLog.h"

#include "USystemFunc.h"
#include "yyConfig.h"
#include "yyDefine.h"

#include <atlbase.h>
#include <altcecrt.h>
#include <atlconv.h>
#include <atlcoll.h>
#include <atlfile.h>
#include <atlsync.h>
#include <atlstr.h>

#include "OMXDecoder.h"

CPrima2VideoDec::CPrima2VideoDec(void * hInst)
	: CBaseVideoDec (hInst)
	, m_pDec (NULL)
	, m_pPacket (NULL)
{
	SetObjectName ("CPrima2VideoDec");
}

CPrima2VideoDec::~CPrima2VideoDec(void)
{
	Uninit ();
}

int CPrima2VideoDec::Init (YY_VIDEO_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;

	Uninit ();

	int					nCodecID = 0;
    AVCodecContext *	pDecCtx = NULL;
	if (pFmt->pPrivateData != NULL)
		pDecCtx = (AVCodecContext *)pFmt->pPrivateData;
	if (pDecCtx != NULL)
	{
		m_uCodecTag = pDecCtx->codec_tag;
		nCodecID = pDecCtx->codec_id;
	}
	else
	{
		nCodecID = pFmt->nCodecID;
	}

	m_pDec = new COMXDecoder ();
	OMX_ERRORTYPE ret = OMX_ErrorMax;
	if (nCodecID == AV_CODEC_ID_H264)
	{
//		ret = m_pDec->Create ("OMX.IMG.MSVDX.AVC.Decoder", "video_decoder.avc", 
//		ret = m_pDec->Create ("OMX.IMG.MSVDX.MPEG2.Decoder", "video_decoder.mpeg2", 
//		ret = m_pDec->Create ("OMX.IMG.MSVDX.MPEG4.Decoder", "video_decoder.mpeg4", 
		ret = m_pDec->Create ("OMX.IMG.MSVDX.VC1.Decoder", "video_decoder.wmv", 
//		ret = m_pDec->Create ("OMX.IMG.MSVDX.REAL.Decoder", "video_decoder.rv", 
						pFmt->nWidth, pFmt->nHeight, pFmt->nWidth * pFmt->nHeight * 2);
	}
	else if (pDecCtx->codec_id == AV_CODEC_ID_MPEG4)
	{
	}
	else if (pDecCtx->codec_id == AV_CODEC_ID_WMV3)
	{
	}

	memcpy (&m_fmtVideo, pFmt, sizeof (m_fmtVideo));
	m_fmtVideo.nCodecID = AV_CODEC_ID_NONE;
	m_fmtVideo.pHeadData = NULL;
	m_fmtVideo.nHeadSize = 0;
	m_fmtVideo.pPrivateData = NULL;

	m_bDropFrame = true;
	m_nDecCount = 0;
	m_pPacket = NULL;
	m_uBuffFlag = 0;

	return YY_ERR_NONE;
}

int CPrima2VideoDec::Uninit (void)
{
	if (m_pDec != NULL)
	{
		m_pDec->Destroy ();
		delete m_pDec;
		m_pDec = NULL;
	}
	return YY_ERR_NONE;
}

int CPrima2VideoDec::Flush (void)
{
	m_bDropFrame = true;
	m_nDecCount = 0;

	return YY_ERR_NONE;
}

int CPrima2VideoDec::SetBuff (YY_BUFFER * pBuff)
{
	if (pBuff == NULL)
		return YY_ERR_ARG;

//	CAutoLock lock (&m_mtBuffer);
	CBaseVideoDec::SetBuff (pBuff);
/*
	if ((pBuff->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
		Init ((YY_VIDEO_FORMAT *)pBuff->pFormat);
	else if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		Flush ();

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

	m_Input.Buffer = (VO_PBYTE)m_pPacket->data;
	m_Input.Length = m_pPacket->size;
	if (m_pPacket->pts == YY_64_INVALID && m_pPacket->dts != YY_64_INVALID)
		m_Input.Time = m_pPacket->dts;
	else
		m_Input.Time = m_pPacket->pts;

	if (pBuff->llDelay > YYCFG_SKIP_BFRAME_TIME)
	{
		if (GetFrameType (&m_Input) == VO_VIDEO_FRAME_B)
			return YY_ERR_NEEDMORE;
	}
	if ((pBuff->uFlag & YYBUFF_DEC_SKIP_BFRAME) == YYBUFF_DEC_SKIP_BFRAME)
	{
		if (GetFrameType (&m_Input) == VO_VIDEO_FRAME_B)
			return YY_ERR_NEEDMORE;
	}

	int nRC = SetInputData (&m_Input);
	FillInfo (pBuff);

	if (nRC == VO_ERR_INPUT_BUFFER_SMALL)
		return YY_ERR_NEEDMORE;
	else if (nRC == VO_ERR_RETRY)
		return YY_ERR_RETRY;
*/
	return YY_ERR_NONE;
}

int CPrima2VideoDec::GetBuff (YY_BUFFER * pBuff)
{
	if (pBuff == NULL)
		return YY_ERR_ARG;
/*
//	CAutoLock lock (&m_mtBuffer);
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

	int nRC = GetOutputData ();

	if (m_Output.Buffer[0] == NULL)
		return YY_ERR_RETRY;

	m_bufVideo.pBuff[0] = m_Output.Buffer[0];
	m_bufVideo.pBuff[1] = m_Output.Buffer[1];
	m_bufVideo.pBuff[2] = m_Output.Buffer[2];

	m_bufVideo.nStride[0] = m_Output.Stride[0];
	m_bufVideo.nStride[1] = m_Output.Stride[1];
	m_bufVideo.nStride[2] = m_Output.Stride[2];

	m_bufVideo.nWidth = m_OutputInfo.Format.Width;
	m_bufVideo.nHeight = m_OutputInfo.Format.Height;

	m_bufVideo.nType = YY_VDT_YUV420_P;

	pBuff->uFlag = YYBUFF_TYPE_VIDEO;
	pBuff->pBuff = (unsigned char *)&m_bufVideo;
	pBuff->uSize = m_nVdoBuffSize;
	if (m_bufVideo.nWidth != m_fmtVideo.nWidth || m_bufVideo.nHeight != m_fmtVideo.nHeight)
	{
		m_fmtVideo.nWidth = m_bufVideo.nWidth;
		m_fmtVideo.nHeight = m_bufVideo.nHeight;
		pBuff->uFlag |= YYBUFF_NEW_FORMAT;
		pBuff->pFormat = &m_fmtVideo;
	}

	pBuff->llTime = m_Input.Time;

	m_bDecoded = true;
	RestoreInfo (pBuff);

	CBaseVideoDec::GetBuff (pBuff);
*/
	return YY_ERR_NONE;
}

