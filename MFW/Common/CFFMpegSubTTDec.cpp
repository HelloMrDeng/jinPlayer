/*******************************************************************************
	File:		CFFMpegSubTTDec.cpp

	Contains:	The ffmpeg audio dec implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CFFMpegSubTTDec.h"

#include "UFFMpegFunc.h"
#include "USystemFunc.h"

#include "yyConfig.h"
#include "yyLog.h"

CFFMpegSubTTDec::CFFMpegSubTTDec(void * hInst)
	: CBaseObject ()
	, m_pDecCtx (NULL)
	, m_pDecSubTT (NULL)
	, m_pFrmSubTT (NULL)
	, m_pPacket (NULL)
{
	SetObjectName ("CFFSubTTDec");
}

CFFMpegSubTTDec::~CFFMpegSubTTDec(void)
{
	Uninit ();
}

int CFFMpegSubTTDec::Init (YY_SUBTT_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;

	Uninit ();

	m_pDecSubTT = avcodec_find_decoder ((AVCodecID)pFmt->nCodecID);
	if (m_pDecSubTT == NULL)
		return YY_ERR_FAILED;
	if (pFmt->pPrivateData != NULL)
		m_pDecCtx = (AVCodecContext *)pFmt->pPrivateData;
	else
		return YY_ERR_FAILED;

	int nRC = avcodec_open2 (m_pDecCtx, m_pDecSubTT, NULL);
	if (nRC < 0)
		return YY_ERR_FAILED;

	m_pFrmSubTT = new AVSubtitle ();
	memset (m_pFrmSubTT, 0, sizeof (AVSubtitle));

	return YY_ERR_NONE;
}

int CFFMpegSubTTDec::Uninit (void)
{
	if (m_pFrmSubTT != NULL)
	{
		avsubtitle_free (m_pFrmSubTT);
		delete m_pFrmSubTT;
	}
	m_pFrmSubTT = NULL;

//	if (m_pDecCtx != NULL)
//		avcodec_close (m_pDecCtx);
	m_pDecCtx = NULL;

	return YY_ERR_NONE;
}

int CFFMpegSubTTDec::Flush (void)
{
	if (m_pDecSubTT != NULL)
		avcodec_flush_buffers (m_pDecCtx);

	return YY_ERR_NONE;
}

int CFFMpegSubTTDec::SetBuff (YY_BUFFER * pBuff)
{
	if (pBuff == NULL)
		return YY_ERR_ARG;

	if ((pBuff->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
	{
		YY_SUBTT_FORMAT * pFmt = (YY_SUBTT_FORMAT *)pBuff->pFormat;
		//int nGotFrame = 0;
		//avcodec_decode_subtitle2 (m_pDecCtx, m_pFrmSubTT, &nGotFrame, &m_pktData);
	}
	else if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
	{
		Flush ();
	}
	if ((pBuff->uFlag & YYBUFF_TYPE_PACKET) != YYBUFF_TYPE_PACKET)
		return YY_ERR_FAILED;
	else
		m_pPacket = (AVPacket *)pBuff->pBuff;

	return YY_ERR_NONE;
}

int CFFMpegSubTTDec::GetBuff (YY_BUFFER * pBuff)
{
	int nRC = YY_ERR_NONE;
	int	nGotFrame = 0;
	if (m_pPacket == NULL || m_pDecCtx == NULL)
		return YY_ERR_STATUS;

	avsubtitle_free (m_pFrmSubTT);

	nRC = avcodec_decode_subtitle2 (m_pDecCtx, m_pFrmSubTT, &nGotFrame, m_pPacket);
	if (nGotFrame > 0)
	{	
		pBuff->uFlag = YYBUFF_TYPE_AVSubTT;
		pBuff->pBuff = (unsigned char *)m_pFrmSubTT;
		pBuff->uSize = g_nAVSubTTSize;
		pBuff->llTime = m_pFrmSubTT->pts;
		return YY_ERR_NONE;
	}

	return YY_ERR_RETRY;
}
