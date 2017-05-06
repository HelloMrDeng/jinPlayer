/*******************************************************************************
	File:		CVideoDDrawRnd.cpp

	Contains:	The base Video DDraw render implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CVideoDDrawRnd.h"

#include "UYYDataFunc.h"
#include "UYUV420Rotate.h"
#include "UNV12Rotate.h"

#include "yyMonitor.h"
#include "yyConfig.h"
#include "yyLog.h"

CVideoDDrawRnd::CVideoDDrawRnd(void * hInst)
	: CBaseVideoRnd (hInst)
	, m_hWnd (NULL)
	, m_pDD (NULL)
	, m_pDDSPrimary (NULL)
	, m_pDDSOffScr (NULL)
	, m_pFourCC (NULL)
	, m_dwFourCC (0)
	, m_pYUVBuff (NULL)
	, m_pSubTT (NULL)
{
	SetObjectName ("CVideoDDrawRnd");
	m_nType = YY_VRND_DDRAW;

	memset (&m_ddsd, 0, sizeof (DDSURFACEDESC));

	memset(&m_ddBltFX, 0, sizeof(DDBLTFX));
	m_ddBltFX.dwSize = sizeof(m_ddBltFX);

	memset (&m_rcDraw, 0, sizeof (m_rcDraw));
}

CVideoDDrawRnd::~CVideoDDrawRnd(void)
{
	Uninit ();
}

int CVideoDDrawRnd::SetDisplay (void * hView, RECT * pRect)
{
	CAutoLock lock (&m_mtDraw);

	if (m_hWnd == (HWND)hView)
	{
		if (pRect != NULL && !memcmp (pRect, &m_rcView, sizeof (m_rcView)))
			return YY_ERR_NONE;
	}
	else
	{
		ReleaseDD ();
	}

	m_hWnd = (HWND)hView;
	m_hView =hView;

	if (pRect == NULL)
		GetClientRect (m_hWnd, &m_rcView);
	else
		memcpy (&m_rcView, pRect, sizeof (RECT));

	UpdateRenderSize ();

	if (m_pDDSPrimary == NULL)
		CreateDD ();

	return YY_ERR_NONE;
}

int CVideoDDrawRnd::UpdateDisp (void)
{
	CAutoLock lock (&m_mtDraw);

	RECT	rctDest;	
	memcpy (&rctDest, &m_rcRender, sizeof (RECT));
	ClientToScreen(m_hWnd, (LPPOINT)&rctDest.left);
	ClientToScreen(m_hWnd, (LPPOINT)&rctDest.right);

	if (m_pDDSOffScr != NULL)
		m_pDDSPrimary->Blt(&rctDest, m_pDDSOffScr, &m_rcZoom, DDBLT_WAIT, &m_ddBltFX);

	return YY_ERR_NONE;
}

int CVideoDDrawRnd::SetSubTTEng (void * pSubTTEng)
{
	m_pSubTTEng = pSubTTEng;

	m_pSubTT = (CSubtitleEngine *)m_pSubTTEng;

	return YY_ERR_NONE;
}

int CVideoDDrawRnd::Init (YY_VIDEO_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;
	if (m_hWnd == NULL)
		return YY_ERR_STATUS;

	SetRectEmpty (&m_rcZoom);
	yyDataCloneVideoFormat (&m_fmtVideo, pFmt);
	if (m_nRotate == 90 || m_nRotate == 270)
	{
		m_fmtVideo.nWidth = pFmt->nHeight & ~7;
		m_fmtVideo.nHeight = pFmt->nWidth & ~7;
	}
	else if (m_nRotate == 180)
	{
		m_fmtVideo.nWidth = pFmt->nWidth & ~3;
		m_fmtVideo.nHeight = pFmt->nHeight & ~3;
	}
	YY_DEL_A (m_pYUVBuff);

	UpdateRenderSize ();

	if (m_fmtVideo.nWidth > m_nMaxWidth || m_fmtVideo.nHeight > m_nMaxHeight)
	{
		if (m_fmtVideo.nWidth > m_nMaxWidth)
			m_nMaxWidth = m_fmtVideo.nWidth;
		if (m_fmtVideo.nHeight > m_nMaxHeight)
			m_nMaxHeight = m_fmtVideo.nHeight;

		ReleaseDD ();
	}

	if (m_pDD == NULL)
	{
		if (!CreateDD ())
			return YY_ERR_FAILED;
	}

	return YY_ERR_NONE;
}

int CVideoDDrawRnd::Uninit (void)
{
	ReleaseDD ();
	YY_DEL_A (m_pYUVBuff);
	return YY_ERR_NONE;
}

int CVideoDDrawRnd::Render (YY_BUFFER * pBuff)
{
	CBaseVideoRnd::Render (pBuff);
	if (pBuff->pBuff == NULL)
		return YY_ERR_ARG;
	if (!IsWindowVisible (m_hWnd))
		return YY_ERR_NONE;
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	if (rcView.bottom <= 16 || rcView.right <= 16)
		return YY_ERR_NONE;

	CAutoLock lock (&m_mtDraw);
	if (m_pDDSOffScr == NULL)
		return YY_ERR_STATUS;
	YY_VIDEO_BUFF * pVideoBuff = NULL;
	if ((pBuff->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
	{
		AVFrame * pFrmVideo = (AVFrame *)pBuff->pBuff;
		m_bufVideo.pBuff[0] = pFrmVideo->data[0];
		m_bufVideo.pBuff[1] = pFrmVideo->data[1];
		m_bufVideo.pBuff[2] = pFrmVideo->data[2];
		m_bufVideo.nStride[0] = pFrmVideo->linesize[0];
		m_bufVideo.nStride[1] = pFrmVideo->linesize[1];
		m_bufVideo.nStride[2] = pFrmVideo->linesize[2];
		if (pFrmVideo->format == AV_PIX_FMT_YUV420P)
			m_bufVideo.nType = YY_VDT_YUV420_P;
		else if (pFrmVideo->format == AV_PIX_FMT_NV12)
			m_bufVideo.nType = YY_VDT_NV12;
		else if (pFrmVideo->format == AV_PIX_FMT_RGB24)
			m_bufVideo.nType = YY_VDT_RGB24;
		else
			m_bufVideo.nType = YY_VDT_MAX;
		pVideoBuff = &m_bufVideo;
	}
	else if ((pBuff->uFlag & YYBUFF_TYPE_VIDEO) == YYBUFF_TYPE_VIDEO)
	{
		pVideoBuff = (YY_VIDEO_BUFF *)pBuff->pBuff;
	}
	if (pVideoBuff == NULL)
		return YY_ERR_RETRY;

	HRESULT ddRval = DDERR_WASSTILLDRAWING;
	while(ddRval == DDERR_WASSTILLDRAWING)
		ddRval = m_pDDSOffScr->Lock (NULL, &m_ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL);
	if(ddRval != DD_OK)
		return YY_ERR_FAILED;

	if (pVideoBuff->nType == YY_VDT_YUV420_P)
	{
		RendVideo (pVideoBuff);
	}
	else
	{
		if (m_nRotate == 0)
		{
			pVideoBuff->pBuff[0] = (LPBYTE)m_ddsd.lpSurface;
			pVideoBuff->nStride[0] = m_ddsd.lPitch;
			if (m_dwFourCC == MAKEFOURCC('Y','V', '1', '2'))
			{
				pVideoBuff->nType = YY_VDT_YUV420_P;
				pVideoBuff->pBuff[2] = (LPBYTE)m_ddsd.lpSurface + m_ddsd.dwHeight * m_ddsd.lPitch;
				pVideoBuff->pBuff[1] = (LPBYTE)m_ddsd.lpSurface + m_ddsd.dwHeight * m_ddsd.lPitch * 5 / 4;
				pVideoBuff->nStride[1] = m_ddsd.lPitch / 2;
				pVideoBuff->nStride[2] = m_ddsd.lPitch / 2;
			}
			else
			{
				pVideoBuff->nType = YY_VDT_NV12;
				pVideoBuff->pBuff[1] = (LPBYTE)m_ddsd.lpSurface + m_ddsd.dwHeight * m_ddsd.lPitch;
				pVideoBuff->nStride[1] = m_ddsd.lPitch;
			}
			pVideoBuff->nWidth = m_fmtVideo.nWidth;
			pVideoBuff->nHeight = m_fmtVideo.nHeight;
		}
		else
		{
			if (m_pYUVBuff == NULL)
				m_pYUVBuff = new unsigned char[m_fmtVideo.nWidth * m_fmtVideo.nHeight * 2];
			pVideoBuff->nType = YY_VDT_YUV420_P;
			if (m_nRotate == 90 || m_nRotate == 270)
			{
				pVideoBuff->nWidth = m_fmtVideo.nHeight;
				pVideoBuff->nHeight = m_fmtVideo.nWidth;	
			}
			else
			{
				pVideoBuff->nWidth = m_fmtVideo.nWidth;
				pVideoBuff->nHeight = m_fmtVideo.nHeight;	
			}
			pVideoBuff->pBuff[0] = m_pYUVBuff;
			pVideoBuff->pBuff[2] = m_pYUVBuff + m_fmtVideo.nWidth * m_fmtVideo.nHeight;
			pVideoBuff->pBuff[1] = m_pYUVBuff + m_fmtVideo.nWidth * m_fmtVideo.nHeight * 5 / 4;
			pVideoBuff->nStride[0] = pVideoBuff->nWidth;
			pVideoBuff->nStride[1] = pVideoBuff->nStride[0] / 2;
			pVideoBuff->nStride[2] = pVideoBuff->nStride[0] / 2;
		}
		if (m_pVideoRCC == NULL)
		{
			m_pVideoRCC = new CFFMpegVideoRCC (m_hInst);
			if (m_pVideoRCC == NULL)
				return YY_ERR_MEMORY;
		}
		m_pVideoRCC->ConvertBuff (pBuff, pVideoBuff);
		if (m_nRotate != 0)
			RendVideo (pVideoBuff);
	}

	// start to over the logo 
	if (m_dwFourCC == MAKEFOURCC('Y','V', '1', '2'))
	{
		m_bufLogo.nType = YY_VDT_YUV420_P;
		m_bufLogo.pBuff[0] = (unsigned char *)m_ddsd.lpSurface;
		m_bufLogo.pBuff[1] = (unsigned char *)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight;
		m_bufLogo.pBuff[2] = (unsigned char *)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight * 5 / 4;
		m_bufLogo.nStride[0] = m_ddsd.lPitch;
		m_bufLogo.nStride[1] = m_ddsd.lPitch / 2;
		m_bufLogo.nStride[2] = m_ddsd.lPitch / 2;
	}
	else
	{
		m_bufLogo.nType = YY_VDT_NV12;
		m_bufLogo.pBuff[0] = (unsigned char *)m_ddsd.lpSurface;
		m_bufLogo.pBuff[1] = (unsigned char *)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight;
		m_bufLogo.nStride[0] = m_ddsd.lPitch;
		m_bufLogo.nStride[1] = m_ddsd.lPitch;
	}
	OverLogo ();

	m_pDDSOffScr->Unlock(NULL);

	if (m_pSubTT != NULL)
	{
		HDC hDC = NULL;
		m_pDDSOffScr->GetDC (&hDC);
		if (hDC != NULL)
		{
			m_pSubTT->Draw (hDC, &m_rcDraw, pBuff->llTime, false);
			m_pDDSOffScr->ReleaseDC (hDC);
		}
	}

	RECT	rctDest;	
	memcpy (&rctDest, &m_rcRender, sizeof (RECT));
	ClientToScreen(m_hWnd, (LPPOINT)&rctDest.left);
	ClientToScreen(m_hWnd, (LPPOINT)&rctDest.right);

	RECT rcZoom;
	memcpy (&rcZoom, &m_rcZoom, sizeof (RECT));
	if (m_nRotate == 90)
		SetRect (&rcZoom, m_fmtVideo.nWidth - m_rcZoom.bottom, m_rcZoom.left, m_fmtVideo.nWidth - m_rcZoom.top, m_rcZoom.right);
	else if (m_nRotate == 270)
		SetRect (&rcZoom, m_rcZoom.top, m_fmtVideo.nHeight - m_rcZoom.right, m_rcZoom.bottom, m_fmtVideo.nHeight - m_rcZoom.left);
	else if (m_nRotate == 180)
		SetRect (&rcZoom, m_fmtVideo.nWidth - m_rcZoom.right, m_fmtVideo.nHeight - m_rcZoom.bottom, m_fmtVideo.nWidth - m_rcZoom.left, m_fmtVideo.nHeight - m_rcZoom.top);

	ddRval = m_pDDSPrimary->Blt(&rctDest, m_pDDSOffScr, &rcZoom, DDBLT_WAIT, &m_ddBltFX);
	if(ddRval != DD_OK)
		return YY_ERR_FAILED;

	if (m_bUpdateView)
		UpdateBackGround ();

	m_nRndCount++;

	return YY_ERR_NONE;
}

bool CVideoDDrawRnd::RendVideo (YY_VIDEO_BUFF * pVideoBuff)
{
	if (pVideoBuff->nType != YY_VDT_YUV420_P || m_ddsd.lpSurface == NULL)
		return false;

	int		i = 0;
	LPBYTE	lpSurfY = (LPBYTE)m_ddsd.lpSurface;
	LPBYTE	lpSurfU = (LPBYTE)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight;
	LPBYTE	lpSurfV = (LPBYTE)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight * 5 / 4;
	if (m_nRotate == 0)
		yyYUVRotate00 (pVideoBuff->pBuff[0], m_fmtVideo.nWidth, m_fmtVideo.nHeight, pVideoBuff->nStride[0], lpSurfY, m_ddsd.lPitch);
	else if (m_nRotate == 90)
		yyYUVRotate90 (pVideoBuff->pBuff[0], m_fmtVideo.nHeight, m_fmtVideo.nWidth, pVideoBuff->nStride[0], lpSurfY, m_ddsd.lPitch);
	else if (m_nRotate == 180)
		yyYUVRotate180 (pVideoBuff->pBuff[0], m_fmtVideo.nWidth, m_fmtVideo.nHeight, pVideoBuff->nStride[0], lpSurfY, m_ddsd.lPitch);
	else if (m_nRotate == 270)
		yyYUVRotate270 (pVideoBuff->pBuff[0], m_fmtVideo.nHeight, m_fmtVideo.nWidth, pVideoBuff->nStride[0], lpSurfY, m_ddsd.lPitch);
	if (m_dwFourCC == MAKEFOURCC('Y','V', '1', '2'))
	{
		if (m_nRotate == 0)
		{
			yyYUVRotate00 (pVideoBuff->pBuff[2], m_fmtVideo.nWidth / 2, m_fmtVideo.nHeight / 2, pVideoBuff->nStride[2], lpSurfU, m_ddsd.lPitch / 2);
			yyYUVRotate00 (pVideoBuff->pBuff[1], m_fmtVideo.nWidth / 2, m_fmtVideo.nHeight / 2, pVideoBuff->nStride[1], lpSurfV, m_ddsd.lPitch / 2);
		}
		else
		{
			if (m_nRotate == 90)
			{
				yyYUVRotate90 (pVideoBuff->pBuff[2], m_fmtVideo.nHeight / 2, m_fmtVideo.nWidth / 2, pVideoBuff->nStride[2], lpSurfU, m_ddsd.lPitch / 2);
				yyYUVRotate90 (pVideoBuff->pBuff[1], m_fmtVideo.nHeight / 2, m_fmtVideo.nWidth / 2, pVideoBuff->nStride[1], lpSurfV, m_ddsd.lPitch / 2);
			}
			else if (m_nRotate == 180)
			{
				yyYUVRotate180 (pVideoBuff->pBuff[2], m_fmtVideo.nWidth / 2, m_fmtVideo.nHeight / 2, pVideoBuff->nStride[2], lpSurfU, m_ddsd.lPitch / 2);
				yyYUVRotate180 (pVideoBuff->pBuff[1], m_fmtVideo.nWidth / 2, m_fmtVideo.nHeight / 2, pVideoBuff->nStride[1], lpSurfV, m_ddsd.lPitch / 2);
			}
			else if (m_nRotate == 270)
			{
				yyYUVRotate270 (pVideoBuff->pBuff[2], m_fmtVideo.nHeight / 2, m_fmtVideo.nWidth / 2, pVideoBuff->nStride[2], lpSurfU, m_ddsd.lPitch / 2);
				yyYUVRotate270 (pVideoBuff->pBuff[1], m_fmtVideo.nHeight / 2, m_fmtVideo.nWidth / 2, pVideoBuff->nStride[1], lpSurfV, m_ddsd.lPitch / 2);
			}
		}
	}
	else
	{
		if (m_nRotate == 0)
			yyNV12Rotate00 (pVideoBuff->pBuff[1], pVideoBuff->pBuff[2], m_fmtVideo.nWidth / 2, m_fmtVideo.nHeight / 2, pVideoBuff->nStride[1], lpSurfU, m_ddsd.lPitch);
		else if (m_nRotate == 90)
			yyNV12Rotate90 (pVideoBuff->pBuff[1], pVideoBuff->pBuff[2], m_fmtVideo.nHeight / 2, m_fmtVideo.nWidth / 2, pVideoBuff->nStride[1], lpSurfU, m_ddsd.lPitch);
		else if (m_nRotate == 180)
			yyNV12Rotate180 (pVideoBuff->pBuff[1], pVideoBuff->pBuff[2], m_fmtVideo.nWidth / 2, m_fmtVideo.nHeight / 2, pVideoBuff->nStride[1], lpSurfU, m_ddsd.lPitch);
		else if (m_nRotate == 270)
			yyNV12Rotate270 (pVideoBuff->pBuff[1], pVideoBuff->pBuff[2], m_fmtVideo.nHeight / 2, m_fmtVideo.nWidth / 2, pVideoBuff->nStride[1], lpSurfU, m_ddsd.lPitch);
	}

	return true;
}

bool CVideoDDrawRnd::UpdateRenderSize (void)
{
	bool bRC = CBaseVideoRnd::UpdateRenderSize ();

	memcpy (&m_rcDest, &m_rcRender, sizeof (RECT));

	ClientToScreen(m_hWnd, (LPPOINT)&m_rcDest.left);
	ClientToScreen(m_hWnd, (LPPOINT)&m_rcDest.right);

	m_rcDraw.right = m_rcRender.right - m_rcRender.left;
	m_rcDraw.bottom = m_rcRender.bottom - m_rcRender.top;

	return bRC;
}

bool CVideoDDrawRnd::CreateDD (void)
{
	HRESULT hr = S_OK;
	if (m_fmtVideo.nWidth <= 0 || m_fmtVideo.nHeight <= 0)
		return false;

	ReleaseDD ();
	if (DirectDrawCreateEx(NULL, (VOID**)&m_pDD, IID_IDirectDraw7, NULL) != DD_OK)
		return false;
	memset (&m_DDCaps, 0, sizeof (DDCAPS));
	m_DDCaps.dwSize = sizeof (DDCAPS);
	hr = m_pDD->GetCaps (&m_DDCaps, NULL);
	if (hr != DD_OK)
		return false;

	DWORD	nFourCC = 20;
	hr = m_pDD->GetFourCCCodes (&nFourCC, NULL);
	if (nFourCC > 0)
	{
		m_pFourCC = new DWORD[nFourCC];
		hr = m_pDD->GetFourCCCodes (&nFourCC, m_pFourCC);
	}
	else
	{
		return false;
	}

	if (m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL) != DD_OK)
		return false;

	ZeroMemory(&m_ddsd, sizeof(m_ddsd));
	m_ddsd.dwSize = sizeof(m_ddsd);
	m_ddsd.dwFlags = DDSD_CAPS;
	m_ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (m_pDD->CreateSurface(&m_ddsd, &m_pDDSPrimary, NULL) != DD_OK)
		return false;

	LPDIRECTDRAWCLIPPER  pcClipper;   // Cliper
	if( m_pDD->CreateClipper( 0, &pcClipper, NULL ) != DD_OK )
		return false;

	if( pcClipper->SetHWnd( 0, m_hWnd ) != DD_OK )
	{
		pcClipper->Release();
		return false;
	}

	if( m_pDDSPrimary->SetClipper( pcClipper ) != DD_OK )
	{
		pcClipper->Release();
		return false;
	}

	// Done with clipper
	pcClipper->Release();

	// Create YUV surface 
	ZeroMemory(&m_ddsd, sizeof(m_ddsd));
	m_ddsd.dwSize = sizeof(m_ddsd);
	m_ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	m_ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	m_ddsd.dwWidth = m_nMaxWidth; //m_fmtVideo.nWidth;	//m_nShowWidth;
	m_ddsd.dwHeight = m_nMaxHeight; //m_fmtVideo.nHeight; //m_nShowHeight;
	m_ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	m_ddsd.ddpfPixelFormat.dwFlags  = DDPF_FOURCC | DDPF_YUV ;
	if (m_pFourCC != NULL)
	{
		for (int i = 0; i < nFourCC; i++)
		{
			if (m_pFourCC[i] == MAKEFOURCC('Y','V', '1', '2'))
			{
				m_dwFourCC = MAKEFOURCC('Y','V', '1', '2');
				break;
			}
		}
	}
	if (m_dwFourCC == 0)
		m_dwFourCC = MAKEFOURCC('N','V', '1', '2');
	m_ddsd.ddpfPixelFormat.dwFourCC = m_dwFourCC;
	m_ddsd.ddpfPixelFormat.dwYUVBitCount = 8;
	m_pDD->CreateSurface(&m_ddsd, &m_pDDSOffScr, NULL);

	return m_pDDSOffScr == NULL ? false : true;
}

bool CVideoDDrawRnd::ReleaseDD(void)
{
	if(m_pDD != NULL)
	{
		if(m_pDDSPrimary != NULL)
		{
			m_pDDSPrimary->Release();
			m_pDDSPrimary = NULL;
		}

		if (m_pDDSOffScr != NULL)
			m_pDDSOffScr->Release ();
		m_pDDSOffScr = NULL;

		m_pDD->Release();
		m_pDD = NULL;
	}

	YY_DEL_A (m_pFourCC);
	m_dwFourCC = 0;

	return TRUE;
}
