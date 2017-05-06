/*******************************************************************************
	File:		CVideoDD60Rnd.cpp

	Contains:	The base Video DDraw CD60 render implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CVideoDD60Rnd.h"

#include "UYYDataFunc.h"

#include "yyMonitor.h"
#include "yyConfig.h"
#include "yyLog.h"

CVideoDD60Rnd::CVideoDD60Rnd(void * hInst)
	: CBaseVideoRnd (hInst)
	, m_hWnd (NULL)
	, m_pDDExt (NULL)
	, m_pDD (NULL)
	, m_pPrmSur (NULL)
	, m_pOvlSur (NULL)
	, m_pMemSur (NULL)
	, m_nDDMode (YY_DDM_Memory)
	, m_bOverride (true)
	, m_bOverlayUpdate (false)
	, m_bOverlayShow (false)
	, m_pSubTT (NULL)
{
	SetObjectName ("CVideoDD60Rnd");
	m_nType = YY_VRND_DDCE6;

	memset (&m_ddsd, 0, sizeof (_WINCE_60::DDSURFACEDESC));

	memset(&m_ddBltFX, 0, sizeof(_WINCE_60::DDBLTFX));
	m_ddBltFX.dwSize = sizeof(m_ddBltFX);

	memset(&m_ddOverlayFX,0,sizeof(m_ddOverlayFX));
	m_ddOverlayFX.dwSize = sizeof(m_ddOverlayFX);
	m_ddOverlayFX.dwAlphaConstBitDepth = 8;
	m_ddOverlayFX.dwAlphaConst = 0XFF;
#ifdef _CPU_PRIMA2
	m_ddOverlayFX.dckDestColorkey.dwColorSpaceLowValue	= RGB (8, 8, 8);
	m_ddOverlayFX.dckDestColorkey.dwColorSpaceHighValue	= RGB (8, 8, 8);
#endif // _CPU_PRIMA2
//	m_ddOverlayFX.dckSrcColorkey.dwColorSpaceLowValue	= RGB (8, 8, 8);
//	m_ddOverlayFX.dckSrcColorkey.dwColorSpaceHighValue	= RGB (8, 8, 8);

	SetRectEmpty (&m_rcSubTT);
}

CVideoDD60Rnd::~CVideoDD60Rnd(void)
{
	Uninit ();
}

int CVideoDD60Rnd::SetDisplay (void * hView, RECT * pRect)
{
	CAutoLock lock (&m_mtDraw);
	if (m_hWnd == NULL)
	{
		m_hWnd = (HWND)hView;
		m_hView =hView;
		if (pRect == NULL)
			GetClientRect (m_hWnd, &m_rcView);
		else
			memcpy (&m_rcView, pRect, sizeof (RECT));
		return YY_ERR_NONE;
	}

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
	if (m_pPrmSur == NULL && m_fmtVideo.nWidth > 0 && m_fmtVideo.nHeight > 0)
	{
		if (!CreateDD ())
			return YY_ERR_FAILED;
	}
	return YY_ERR_NONE;
}

int CVideoDD60Rnd::UpdateDisp (void)
{
	CAutoLock lock (&m_mtDraw);
	RECT	rctDest;	
	memcpy (&rctDest, &m_rcRender, sizeof (RECT));
	ClientToScreen(m_hWnd, (LPPOINT)&rctDest.left);
	ClientToScreen(m_hWnd, (LPPOINT)&rctDest.right);
	if (m_nDDMode == YY_DDM_Memory && m_pMemSur != NULL)
		m_pPrmSur->Blt(&rctDest, m_pMemSur, &m_rcVideo, DDBLT_WAITNOTBUSY, &m_ddBltFX);
	return YY_ERR_NONE;
}

int CVideoDD60Rnd::SetDDMode (YY_PLAY_DDMode nMode)
{
	CAutoLock lock (&m_mtDraw);
	if (m_nDDMode == nMode)
		return YY_ERR_NONE;
	m_nDDMode = nMode;
	if (m_pDD == NULL)
		return YY_ERR_NONE;

	ZeroMemory(&m_ddsd, sizeof(m_ddsd));
	m_ddsd.dwSize = sizeof(m_ddsd);
	m_ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	m_ddsd.dwWidth = m_fmtVideo.nWidth;	
	m_ddsd.dwWidth = (m_fmtVideo.nWidth + 15) &~0X0F;	
	m_ddsd.dwHeight = m_fmtVideo.nHeight; 
	m_ddsd.ddpfPixelFormat.dwSize = sizeof(_WINCE_60::DDPIXELFORMAT);
	m_ddsd.ddpfPixelFormat.dwFlags  = DDPF_FOURCC;// | DDPF_YUV ;
	m_ddsd.ddpfPixelFormat.dwFourCC = MAKEFOURCC('Y','V', '1', '2');
	m_ddsd.ddpfPixelFormat.dwYUVBitCount = 8;

	HRESULT hr = S_OK;
	_WINCE_60::IDirectDrawSurface * pDrawSurface = NULL;
	if (m_nDDMode == YY_DDM_Overlay && m_pOvlSur == NULL)
	{
		m_ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY;
		hr = m_pDD->CreateSurface(&m_ddsd, &m_pOvlSur, NULL);
		if (m_pOvlSur == NULL)
			YYLOGE ("Create Overlay surface was failed! err = %08X", hr);
		pDrawSurface = m_pOvlSur;
	}
	else if (m_pMemSur == NULL)
	{
		m_ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
		hr = m_pDD->CreateSurface(&m_ddsd, &m_pMemSur, NULL);
		if (m_pMemSur == NULL)
			YYLOGE ("Create videomem surface was failed! err = %08X", hr);
		pDrawSurface = m_pMemSur;
	}
	if (pDrawSurface != NULL)
	{
		HRESULT ddRval = DDERR_WASSTILLDRAWING;
		while(ddRval == DDERR_WASSTILLDRAWING)
			ddRval = pDrawSurface->Lock(NULL, &m_ddsd, DDLOCK_WAITNOTBUSY | DDLOCK_WRITEONLY,NULL);		
		if (m_ddsd.dwWidth != m_fmtVideo.nWidth)
		{
			LPBYTE	lpSurf = (LPBYTE)m_ddsd.lpSurface;
			memset (lpSurf, 0, m_ddsd.dwHeight * m_ddsd.lPitch);
			lpSurf = (LPBYTE)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight;
			memset (lpSurf, 127, m_ddsd.dwHeight * m_ddsd.lPitch / 4);
			lpSurf = (LPBYTE)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight * 5 / 4;
			memset (lpSurf, 127, m_ddsd.dwHeight * m_ddsd.lPitch / 4);
		}
		pDrawSurface->Unlock(NULL);
	}
	m_bOverlayUpdate = false;
	return YY_ERR_NONE;
}

int CVideoDD60Rnd::SetSubTTEng (void * pSubTTEng)
{
	m_pSubTTEng = pSubTTEng;
	m_pSubTT = (CSubtitleEngine *)m_pSubTTEng;
	return YY_ERR_NONE;
}

int CVideoDD60Rnd::SetExtDDraw (void * pDDExt)
{
	m_pDDExt = (_WINCE_60::IDirectDraw *)pDDExt;
	return YY_ERR_NONE;
}

int CVideoDD60Rnd::Init (YY_VIDEO_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;
	if (m_hWnd == NULL)
	{
		YYLOGW ("The window handle is NULL!");
		return YY_ERR_STATUS;
	}

	yyDataCloneVideoFormat (&m_fmtVideo, pFmt);
	UpdateRenderSize ();
	if (!CreateDD ())
		return YY_ERR_FAILED;

	return YY_ERR_NONE;
}

int CVideoDD60Rnd::Uninit (void)
{
	ReleaseDD ();
	return YY_ERR_NONE;
}

int CVideoDD60Rnd::Render (YY_BUFFER * pBuff)
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

	GetClientRect (m_hWnd, &m_rcWindow);
	if (m_rcWindow.right - m_rcWindow.left < m_rcView.right - m_rcView.left)
	{
		GetClientRect (m_hWnd, &m_rcView);
		UpdateRenderSize ();
	}

	CAutoLock lock (&m_mtDraw);
	if (m_pDD == NULL)
		return YY_ERR_STATUS;
	_WINCE_60::IDirectDrawSurface * pDrawSurface = m_pMemSur;
	if (m_nDDMode == YY_DDM_Overlay && m_pOvlSur != NULL)
		pDrawSurface = m_pOvlSur;
	if (pDrawSurface == NULL)
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

	int nDrawHeight = m_fmtVideo.nHeight;
	HRESULT ddRval = DDERR_WASSTILLDRAWING;
	while(ddRval == DDERR_WASSTILLDRAWING)
		ddRval = pDrawSurface->Lock (NULL, &m_ddsd, DDLOCK_WAITNOTBUSY | DDLOCK_WRITEONLY,NULL);
	if(ddRval != DD_OK)
		return YY_ERR_FAILED;

	if (m_ddsd.lpSurface == NULL || m_ddsd.lPitch < m_fmtVideo.nWidth)
	{
		pDrawSurface->Unlock(NULL);
		return YY_ERR_FAILED;
	}

	int		i = 0;
	LPBYTE	lpSurf = (LPBYTE)m_ddsd.lpSurface;
	LPBYTE	pVideo = pVideoBuff->pBuff[0];

	if (pVideoBuff->nType == YY_VDT_YUV420_P)
	{
		for (i = 0; i < nDrawHeight; i++)
			memcpy (lpSurf + m_ddsd.lPitch * i, pVideo + pVideoBuff->nStride[0] * i, m_fmtVideo.nWidth);

		lpSurf = (LPBYTE)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight;
		pVideo = pVideoBuff->pBuff[2];
		for (i = 0; i < nDrawHeight / 2; i++)
			memcpy (lpSurf + m_ddsd.lPitch * i / 2, pVideo + pVideoBuff->nStride[2] * i, m_fmtVideo.nWidth / 2);
		lpSurf = (LPBYTE)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight * 5 / 4;
		pVideo = pVideoBuff->pBuff[1];
		for (i = 0; i < nDrawHeight / 2; i++)
			memcpy (lpSurf + m_ddsd.lPitch * i / 2, pVideo + pVideoBuff->nStride[1] * i, m_fmtVideo.nWidth / 2);
	}
	else
	{
		pVideoBuff->nType = YY_VDT_YUV420_P;
		pVideoBuff->pBuff[0] = lpSurf;
		pVideoBuff->nStride[0] = m_ddsd.lPitch;
		pVideoBuff->pBuff[2] = lpSurf + m_ddsd.dwHeight * m_ddsd.lPitch;
		pVideoBuff->pBuff[1] = lpSurf + m_ddsd.dwHeight * m_ddsd.lPitch * 5 / 4;
		pVideoBuff->nStride[1] = m_ddsd.lPitch / 2;
		pVideoBuff->nStride[2] = m_ddsd.lPitch / 2;
		pVideoBuff->nWidth = m_fmtVideo.nWidth;
		pVideoBuff->nHeight = m_fmtVideo.nHeight;
		if (m_pVideoRCC == NULL)
		{
			m_pVideoRCC = new CFFMpegVideoRCC (m_hInst);
			if (m_pVideoRCC == NULL)
				return YY_ERR_MEMORY;
		}
		m_pVideoRCC->ConvertBuff (pBuff, &m_bufVideo);
	}

	m_bufLogo.nType = YY_VDT_YUV420_P;
	m_bufLogo.pBuff[0] = (unsigned char *)m_ddsd.lpSurface;
	m_bufLogo.pBuff[1] = (unsigned char *)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight;
	m_bufLogo.pBuff[2] = (unsigned char *)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight * 5 / 4;
	m_bufLogo.nStride[0] = m_ddsd.lPitch;
	m_bufLogo.nStride[1] = m_ddsd.lPitch / 2;
	m_bufLogo.nStride[2] = m_ddsd.lPitch / 2;
	OverLogo ();

	pDrawSurface->Unlock(NULL);
	if (m_nDDMode == YY_DDM_Overlay)
	{
		if (!m_bOverlayUpdate || !m_bOverlayShow)
			ShowOverlay (true);
	}
	else
	{
		if (!m_bOverlayUpdate || m_bOverlayShow)
			ShowOverlay (false);
		m_pPrmSur->Blt(&m_rcDest, m_pMemSur, &m_rcVideo, DDBLT_WAITNOTBUSY, &m_ddBltFX);
	}
	if (m_bUpdateView)
		UpdateBackGround ();

	if (m_pSubTT != NULL)
	{
		HDC hDC = GetDC (m_hWnd);
		m_pSubTT->Draw (hDC, &m_rcSubTT, pBuff->llTime, true);
		ReleaseDC (m_hWnd, hDC);
	}

	m_nRndCount++;
	return YY_ERR_NONE;
}

bool CVideoDD60Rnd::UpdateRenderSize (void)
{
	bool bRC = CBaseVideoRnd::UpdateRenderSize ();

	memcpy (&m_rcDest, &m_rcRender, sizeof (RECT));
	ClientToScreen(m_hWnd, (LPPOINT)&m_rcDest.left);
	ClientToScreen(m_hWnd, (LPPOINT)&m_rcDest.right);

	m_rcDest.left = m_rcDest.left & 0XFFFE;
	m_rcDest.top = m_rcDest.top & 0XFFFE;
	m_rcDest.right = m_rcDest.right & 0XFFFE;
	m_rcDest.bottom = m_rcDest.bottom & 0XFFFE;

	memcpy (&m_rcSubTT, &m_rcRender, sizeof (RECT));
	m_rcSubTT.bottom = m_rcSubTT.bottom - 32;
	m_bOverlayUpdate = false;

	if (m_nDDMode == YY_DDM_Overlay)
	{
		if (!m_bOverlayUpdate || !m_bOverlayShow)
			ShowOverlay (true);
	}

	return bRC;
}

bool CVideoDD60Rnd::CreateDD (void)
{
	if (m_fmtVideo.nWidth <= 0 || m_fmtVideo.nHeight <= 0)
	{
		YYLOGW ("The video size is zero!");
		return false;
	}

	CAutoLock lock (&m_mtDraw);
	if (m_pDD != NULL)
		ReleaseDD ();
	HRESULT hr = S_OK;
	if (m_pDDExt == NULL)
	{
		if (_WINCE_60::DirectDrawCreate(NULL, &m_pDD, NULL) != DD_OK)
		{
			YYLOGW ("Create the direct draw failed!");
			return false;
		}
	}
	else
	{
		m_pDD = m_pDDExt;
	}

	if (m_pDD->SetCooperativeLevel(m_hWnd, DDSCL_NORMAL) != DD_OK)
		return false;
	ZeroMemory(&m_ddsd, sizeof(m_ddsd));
	m_ddsd.dwSize = sizeof(m_ddsd);
	m_ddsd.dwFlags = DDSD_CAPS;
	m_ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if (m_pDD->CreateSurface(&m_ddsd, &m_pPrmSur, NULL) != DD_OK)
	{
		YYLOGE ("Create the PRIMARYSURFACE failed!");
		return false;
	}
	_WINCE_60::LPDIRECTDRAWCLIPPER  pcClipper;   // Cliper
	if( m_pDD->CreateClipper( 0, &pcClipper, NULL ) != DD_OK )
	{
		YYLOGE ("Create Clipper failed!");
		return false;
	}
	if( pcClipper->SetHWnd( 0, m_hWnd ) != DD_OK )
	{
		pcClipper->Release();
		YYLOGE ("Set Clipper window failed!");
		return false;
	}
	if( m_pPrmSur->SetClipper( pcClipper ) != DD_OK )
	{
		pcClipper->Release();
		YYLOGE ("Set Clipper failed!");
		return false;
	}
	// Done with clipper
	pcClipper->Release();

	memset (&m_ddsd, 0, sizeof (_WINCE_60::DDSURFACEDESC));
	m_ddsd.dwSize = sizeof (_WINCE_60::DDSURFACEDESC);
	hr = m_pPrmSur->GetSurfaceDesc (&m_ddsd);
	if (hr != DD_OK)
	{
		YYLOGE ("Get Surface Desc failed!");
		return false;
	}

	// Create YUV surface 
	ZeroMemory(&m_ddsd, sizeof(m_ddsd));
	m_ddsd.dwSize = sizeof(m_ddsd);
	m_ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	m_ddsd.dwWidth = m_fmtVideo.nWidth;	
	m_ddsd.dwWidth = (m_fmtVideo.nWidth + 15) &~0X0F;	
	m_ddsd.dwHeight = m_fmtVideo.nHeight; 
	m_ddsd.ddpfPixelFormat.dwSize = sizeof(_WINCE_60::DDPIXELFORMAT);
	m_ddsd.ddpfPixelFormat.dwFlags  = DDPF_FOURCC;// | DDPF_YUV ;
	m_ddsd.ddpfPixelFormat.dwFourCC = MAKEFOURCC('Y','V', '1', '2');
	m_ddsd.ddpfPixelFormat.dwYUVBitCount = 8;

	if (m_nDDMode == YY_DDM_Overlay)
	{
		m_ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY;
		hr = m_pDD->CreateSurface(&m_ddsd, &m_pOvlSur, NULL);
		if (m_pOvlSur == NULL)
			YYLOGE ("Create overlay surface was failed!");
	}
	else
	{
		m_ddsd.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;
		m_pDD->CreateSurface(&m_ddsd, &m_pMemSur, NULL);
		if (m_pMemSur == NULL)
			YYLOGE ("Create video mem surface was failed!");
	}
	_WINCE_60::IDirectDrawSurface * pDrawSurface = m_pMemSur;
	if (m_nDDMode == YY_DDM_Overlay && m_pOvlSur != NULL)
		pDrawSurface = m_pOvlSur;
	if (pDrawSurface == NULL)
		return false;

//	if (m_ddsd.dwWidth != m_fmtVideo.nWidth)
	{
		HRESULT ddRval = DDERR_WASSTILLDRAWING;
		while(ddRval == DDERR_WASSTILLDRAWING)
			ddRval = pDrawSurface->Lock(NULL, &m_ddsd, DDLOCK_WAITNOTBUSY | DDLOCK_WRITEONLY,NULL);	
		LPBYTE	lpSurf = (LPBYTE)m_ddsd.lpSurface;
		memset (lpSurf, 0, m_ddsd.dwHeight * m_ddsd.lPitch);
		lpSurf = (LPBYTE)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight;
		memset (lpSurf, 127, m_ddsd.dwHeight * m_ddsd.lPitch / 4);
		lpSurf = (LPBYTE)m_ddsd.lpSurface + m_ddsd.lPitch * m_ddsd.dwHeight * 5 / 4;
		memset (lpSurf, 127, m_ddsd.dwHeight * m_ddsd.lPitch / 4);
		pDrawSurface->Unlock(NULL);
	}

	m_bOverlayUpdate = false;

	YYLOGI ("Create the direct draw resource!");
	return true;
}

bool CVideoDD60Rnd::ReleaseDD(void)
{
	CAutoLock lock (&m_mtDraw);
	YY_REL_P (m_pMemSur);
	YY_REL_P (m_pOvlSur);
	YY_REL_P (m_pPrmSur);
	if (m_pDDExt == NULL)
		YY_REL_P (m_pDD);
	YYLOGI ("Release the direct draw resource!");
	return true;
}

int CVideoDD60Rnd::ShowOverlay (bool bShow)
{
	if (m_pOvlSur != NULL)
	{
		if (m_bOverlayUpdate && m_bOverlayShow == bShow)
			return 0;

		m_bOverlayShow = bShow;
		HRESULT hr = S_OK;
		if (m_bOverlayShow)
		{
			InvalidateRect (m_hWnd, NULL, TRUE);
			if (m_bOverride)
				hr = m_pOvlSur->UpdateOverlay(&m_rcVideo, m_pPrmSur, &m_rcDest,
						DDOVER_SHOW | DDOVER_KEYDESTOVERRIDE | DDOVER_ALPHACONSTOVERRIDE, &m_ddOverlayFX);
			else
				hr = m_pOvlSur->UpdateOverlay(&m_rcVideo, m_pPrmSur, &m_rcDest, 
						DDOVER_SHOW | DDOVER_ALPHACONSTOVERRIDE, &m_ddOverlayFX);
		}
		else
		{
			hr = m_pOvlSur->UpdateOverlay(&m_rcVideo, m_pPrmSur, &m_rcDest,
					DDOVER_HIDE | DDOVER_WAITNOTBUSY | DDOVER_WAITVSYNC, &m_ddOverlayFX);
		}

		if(FAILED(hr))
			return -1;

		m_bOverlayUpdate = true;
	}

	return 0;
}
