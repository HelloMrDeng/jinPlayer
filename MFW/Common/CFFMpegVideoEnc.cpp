/*******************************************************************************
	File:		CFFMpegVideoEnc.cpp

	Contains:	The ffmpeg audio dec implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CFFMpegVideoEnc.h"

#include "UFFMpegFunc.h"
#include "USystemFunc.h"

#include "yyConfig.h"
#include "yyLog.h"

CFFMpegVideoEnc::CFFMpegVideoEnc(void * hInst)
	: CBaseVideoDec (hInst)
	, m_pEncCtx (NULL)
	, m_pEncVideo (NULL)
{
	SetObjectName ("CFFVideoDec");
}

CFFMpegVideoEnc::~CFFMpegVideoEnc(void)
{
	Uninit ();
}

int CFFMpegVideoEnc::Init (YY_VIDEO_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;
	Uninit ();

	m_pEncVideo = avcodec_find_decoder ((AVCodecID)pFmt->nCodecID);
	if (m_pEncVideo == NULL)
		return YY_ERR_VIDEO;
	m_pEncCtx = avcodec_alloc_context3 (m_pEncVideo);
	if (m_pEncCtx == NULL)
		return YY_ERR_MEMORY;
	// put sample parameters 
	m_pEncCtx->bit_rate = 400000;
	// resolution must be a multiple of two
	m_pEncCtx->width = pFmt->nWidth;
	m_pEncCtx->height = pFmt->nHeight;
	// frames per second 
	m_pEncCtx->time_base.den = 25;
	m_pEncCtx->time_base.num = 1;
	// emit one intra frame every ten frames 
	m_pEncCtx->gop_size = 10; 
	m_pEncCtx->max_b_frames=1;
	m_pEncCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	if(pFmt->nCodecID == AV_CODEC_ID_H264)
		av_opt_set(m_pEncCtx->priv_data, "preset", "slow", 0);
	if (avcodec_open2(m_pEncCtx, m_pEncVideo, NULL) < 0)
		return YY_ERR_VIDEO;

	return YY_ERR_NONE;
}

int CFFMpegVideoEnc::Uninit (void)
{
	if (m_pEncCtx != NULL)
	{
		avcodec_close (m_pEncCtx);
		av_free (m_pEncCtx);
	}
	m_pEncCtx = NULL;

	return YY_ERR_NONE;
}

int CFFMpegVideoEnc::Encode (YY_BUFFER * pIn, YY_BUFFER * pOut)
{
	AVFrame * pFrmVideo = NULL;
	if ((pIn->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
	{
		pFrmVideo = (AVFrame *)pIn->pBuff;
	}
	else if ((pIn->uFlag & YYBUFF_TYPE_VIDEO) == YYBUFF_TYPE_VIDEO)
	{
		YY_VIDEO_BUFF * pBufVideo = (YY_VIDEO_BUFF *)pIn->pBuff;
		m_frmVideo.format = AV_PIX_FMT_YUV420P;
		m_frmVideo.width = pBufVideo->nWidth;
		m_frmVideo.height = pBufVideo->nHeight;
		m_frmVideo.data[0] = pBufVideo->pBuff[0];
		m_frmVideo.data[1] = pBufVideo->pBuff[1];
		m_frmVideo.data[2] = pBufVideo->pBuff[2];
		m_frmVideo.linesize[0] = pBufVideo->nStride[0];
		m_frmVideo.linesize[1] = pBufVideo->nStride[1];
		m_frmVideo.linesize[2] = pBufVideo->nStride[2];
		m_frmVideo.pts = pIn->llTime;
		pFrmVideo = &m_frmVideo;
	}
	int got_output = 0;
	av_init_packet (&m_pktData);
	m_pktData.data = NULL;
	m_pktData.size = 0;

    AVFrame * frame = avcodec_alloc_frame();
    frame->format = m_pEncCtx->pix_fmt;
    frame->width  = m_pEncCtx->width;
    frame->height = m_pEncCtx->height;

    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    int ret = av_image_alloc(frame->data, frame->linesize, m_pEncCtx->width, m_pEncCtx->height,
                         m_pEncCtx->pix_fmt, 32);

	int i = 12;
	int x, y;
    for(y=0;y<m_pEncCtx->height;y++) {
        for(x=0;x<m_pEncCtx->width;x++) {
            frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
        }
    }

    /* Cb and Cr */
    for(y=0;y<m_pEncCtx->height/2;y++) {
        for(x=0;x<m_pEncCtx->width/2;x++) {
            frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
            frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
        }
    }
	int nRC = avcodec_encode_video2 (m_pEncCtx, &m_pktData, frame, &got_output);
	if (nRC < 0)
		return YY_ERR_FAILED;

	if (got_output) 
	{
		if (pOut->uBuffSize < m_pktData.size)
			YY_DEL_A (pOut->pBuff);
		if (pOut->pBuff == NULL)
		{
			pOut->uBuffSize = m_pktData.size * 3 /2;
			pOut->pBuff = new unsigned char[pOut->uBuffSize];
		}
		memcpy (pOut->pBuff, m_pktData.data, m_pktData.size);
		pOut->uSize = m_pktData.size;
		pOut->llTime = m_pktData.pts;

		av_free_packet(&m_pktData);
		return YY_ERR_NONE;
	}

	return YY_ERR_RETRY;
}

int CFFMpegVideoEnc::Flush (void)
{
	if (m_pEncCtx != NULL)
		avcodec_flush_buffers (m_pEncCtx);
	return YY_ERR_NONE;
}

