/*******************************************************************************
	File:		CFFMpegVideoRCC.cpp

	Contains:	The ffmpeg audio dec implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CFFMpegVideoRCC.h"

#include "yyLog.h"

CFFMpegVideoRCC::CFFMpegVideoRCC(void * hInst)
	: CBaseObject ()
	, m_pSwsCtx (NULL)
	, m_nFormat (AV_PIX_FMT_NONE)
	, m_nVideoW (0)
	, m_nVideoH (0)
	, m_nOutputType (-1)
	, m_nOutputW (0)
	, m_nOutputH (0)
	, m_hVCCRR (NULL)
{	
	SetObjectName ("CFFVideoRCC");

	memset (&m_voVCCRR, 0, sizeof (m_voVCCRR));
#ifdef _EXT_VO
	yyGetColorConvertFunc (&m_voVCCRR, 0);
#endif // _EXT_VO
}

CFFMpegVideoRCC::~CFFMpegVideoRCC(void)
{	
	if (m_pSwsCtx != NULL)
		sws_freeContext (m_pSwsCtx);
	m_pSwsCtx = NULL;

	if (m_hVCCRR != NULL)
	{
		m_voVCCRR.Uninit (m_hVCCRR);
		m_hVCCRR = NULL;
	}
}

int CFFMpegVideoRCC::ConvertBuff (YY_BUFFER * iBuff, YY_VIDEO_BUFF * oBuff, RECT * pZoom)
{	
	if (iBuff == NULL || oBuff == NULL)
		return YY_ERR_ARG;
	
	AVFrame *		pFrmVideo = NULL;
	YY_VIDEO_BUFF * pBufVideo = NULL;
	if ((iBuff->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
	{
		//pFrmVideo = (AVFrame *)iBuff->pBuff;
		memcpy (&m_frmVideo, iBuff->pBuff, iBuff->uSize);
		pFrmVideo = &m_frmVideo;
	}
	else if ((iBuff->uFlag & YYBUFF_TYPE_VIDEO) == YYBUFF_TYPE_VIDEO)
		pBufVideo = (YY_VIDEO_BUFF *)iBuff->pBuff;
	if (pFrmVideo == NULL && pBufVideo == NULL)
		return YY_ERR_IMPLEMENT;	
	if (pFrmVideo == NULL)
	{
		m_frmVideo.format = AV_PIX_FMT_YUV420P;
		m_frmVideo.width = pBufVideo->nWidth;
		m_frmVideo.height = pBufVideo->nHeight;
		m_frmVideo.data[0] = pBufVideo->pBuff[0];
		m_frmVideo.data[1] = pBufVideo->pBuff[1];
		m_frmVideo.data[2] = pBufVideo->pBuff[2];
		m_frmVideo.linesize[0] = pBufVideo->nStride[0];
		m_frmVideo.linesize[1] = pBufVideo->nStride[1];
		m_frmVideo.linesize[2] = pBufVideo->nStride[2];
		pFrmVideo = &m_frmVideo;
	}
	
	if (oBuff->nType == YY_VDT_YUV420_P && pFrmVideo->format == AV_PIX_FMT_YUV420P && pFrmVideo->width == oBuff->nWidth)
	{
		int h = 0;
		for (h = 0; h < pFrmVideo->height; h++)
			memcpy (oBuff->pBuff[0] + oBuff->nStride[0] * h, pFrmVideo->data[0] + h * pFrmVideo->linesize[0], oBuff->nWidth);
		for (h = 0; h < pFrmVideo->height / 2; h++)
			memcpy (oBuff->pBuff[2] + oBuff->nStride[2] * h, pFrmVideo->data[1] + h * pFrmVideo->linesize[1], oBuff->nWidth / 2);
		for (h = 0; h < pFrmVideo->height / 2; h++)
			memcpy (oBuff->pBuff[1] + oBuff->nStride[1] * h, pFrmVideo->data[2] + h * pFrmVideo->linesize[2], oBuff->nWidth / 2);
		return YY_ERR_NONE;
	}
	if (pFrmVideo->format == AV_PIX_FMT_YUV420P && pZoom != NULL)
	{
		m_frmVideo.width = pZoom->right - pZoom->left;
		m_frmVideo.height = pZoom->bottom - pZoom->top;
		m_frmVideo.data[0] = m_frmVideo.data[0] + pZoom->top * m_frmVideo.linesize[0] + pZoom->left;
		m_frmVideo.data[1] = m_frmVideo.data[1] + (pZoom->top * m_frmVideo.linesize[1] + pZoom->left) / 2;
		m_frmVideo.data[2] = m_frmVideo.data[2] + (pZoom->top * m_frmVideo.linesize[2] + pZoom->left) / 2;
	}

#ifdef _EXT_VO
#ifdef _OS_NDK
	if (pFrmVideo->format == AV_PIX_FMT_YUV420P)
	{
		return vvConvertVideo (pFrmVideo, oBuff);
	}
#endif // _OS_NDK
#endif // _EXT_VO

	int nRC = 0;
	if (pFrmVideo->width != m_nVideoW || pFrmVideo->height != m_nVideoH ||
		oBuff->nWidth != m_nOutputW || oBuff->nHeight != m_nOutputH)
	{
		if (m_pSwsCtx != NULL)
			sws_freeContext (m_pSwsCtx);
		m_pSwsCtx = NULL;

		m_nVideoW = pFrmVideo->width;
		m_nVideoH = pFrmVideo->height;
		m_nOutputW = oBuff->nWidth;
		m_nOutputH = oBuff->nHeight;
	}
	
	if (m_pSwsCtx == NULL)
	{
		AVPixelFormat fmtPixel = AV_PIX_FMT_BGRA;
		if (oBuff->nType == YY_VDT_RGB565)
			fmtPixel = AV_PIX_FMT_RGB565LE;
		else if (oBuff->nType == YY_VDT_RGB24)
			fmtPixel = AV_PIX_FMT_BGR24;
		else if (oBuff->nType == YY_VDT_RGBA)
			fmtPixel = AV_PIX_FMT_BGRA;
		else if (oBuff->nType == YY_VDT_ARGB)
			fmtPixel = AV_PIX_FMT_RGBA;
		else if (oBuff->nType == YY_VDT_YUV420_P)
			fmtPixel = AV_PIX_FMT_YUV420P;
		else if (oBuff->nType == YY_VDT_NV12)
			fmtPixel = AV_PIX_FMT_NV12;
//		YYLOGI ("Param %d, %d, %d, %d, %d, %d ", m_nVideoW, m_nVideoH, pFrmVideo->format,
//						m_nOutputW, m_nOutputH, fmtPixel);	
		m_pSwsCtx = sws_getContext(m_nVideoW, m_nVideoH, 
						(AVPixelFormat)pFrmVideo->format,
						m_nOutputW, m_nOutputH, fmtPixel,
						SWS_FAST_BILINEAR, NULL, NULL, NULL);
	}			
	nRC = sws_scale(m_pSwsCtx, pFrmVideo->data, pFrmVideo->linesize, 
					0, pFrmVideo->height, oBuff->pBuff, oBuff->nStride);
	return nRC > 0 ? YY_ERR_NONE : YY_ERR_FAILED;
}

int CFFMpegVideoRCC::vvConvertVideo (AVFrame * pFrmVideo, YY_VIDEO_BUFF * pOutBuff)
{
	int nRC = 0;
	if (pFrmVideo->width != m_nVideoW || pFrmVideo->height != m_nVideoH ||
		pOutBuff->nWidth != m_nOutputW || pOutBuff->nHeight != m_nOutputH)
	{
		if (m_hVCCRR != NULL)
		{
			m_voVCCRR.Uninit (m_hVCCRR);
			m_hVCCRR = NULL;
		}

		m_nVideoW = pFrmVideo->width;
		m_nVideoH = pFrmVideo->height;
		m_nOutputW = pOutBuff->nWidth;
		m_nOutputH = pOutBuff->nHeight;
	}

	if (m_hVCCRR == NULL)
	{
		m_voVCCRR.Init (&m_hVCCRR, NULL, NULL, 0);
		if (m_hVCCRR == NULL)
			return YY_ERR_FAILED;

		VO_IV_COLORTYPE outColorType = VO_COLOR_RGB32_PACKED;
		if (pOutBuff->nType == YY_VDT_RGB565)
			outColorType = VO_COLOR_RGB565_PACKED;
		else if (pOutBuff->nType == YY_VDT_RGB24)
			outColorType = VO_COLOR_RGB888_PACKED;
		else if (pOutBuff->nType == YY_VDT_RGBA)
			outColorType = VO_COLOR_RGB32_PACKED;
		else if (pOutBuff->nType == YY_VDT_ARGB)
			outColorType = VO_COLOR_ARGB32_PACKED;
		else if (pOutBuff->nType == YY_VDT_YUV420_P)
			outColorType = VO_COLOR_YUV_PLANAR420;
		else if (pOutBuff->nType == YY_VDT_NV12)
			outColorType = VO_COLOR_YUV_420_PACK;

		m_voVCCRR.SetColorType (m_hVCCRR, VO_COLOR_YUV_PLANAR420, outColorType);

		int nIW = m_nVideoW & 0XFFFFFFFC;
		int nIH = m_nVideoH & 0XFFFFFFFE;
		int nOW = m_nOutputW & 0XFFFFFFFC;
		int nOH = m_nOutputH & 0XFFFFFFFE;

		m_voVCCRR.SetCCRRSize (m_hVCCRR, (VO_U32 *)&nIW, (VO_U32 *)&nIH, 
								(VO_U32 *)&nOW, (VO_U32 *)&nOH, VO_RT_DISABLE);
	}

	m_inBuff.ColorType = VO_COLOR_YUV_PLANAR420;
	m_inBuff.Buffer[0] = pFrmVideo->data[0];
	m_inBuff.Buffer[1] = pFrmVideo->data[1];
	m_inBuff.Buffer[2] = pFrmVideo->data[2];
	m_inBuff.Stride[0] = pFrmVideo->linesize[0];
	m_inBuff.Stride[1] = pFrmVideo->linesize[1];
	m_inBuff.Stride[2] = pFrmVideo->linesize[2];

//	m_outBuff.ColorType = VO_COLOR_YUV_PLANAR420;
	m_outBuff.Buffer[0] = pOutBuff->pBuff[0];
	m_outBuff.Buffer[1] = pOutBuff->pBuff[1];
	m_outBuff.Buffer[2] = pOutBuff->pBuff[2];
	m_outBuff.Stride[0] = pOutBuff->nStride[0];
	m_outBuff.Stride[1] = pOutBuff->nStride[1];
	m_outBuff.Stride[2] = pOutBuff->nStride[2];

	m_voVCCRR.Process (m_hVCCRR, &m_inBuff, &m_outBuff, 0, VO_FALSE);

	return YY_ERR_NONE;
}
