/*******************************************************************************
	File:		CBaseVideoRnd.cpp

	Contains:	The base Video render implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CBaseVideoRnd.h"

#include "UThreadFunc.h"

#include "CLicenseCheck.h"

#include "yyConfig.h"
#include "yyLog.h"
#include "yyLogoData.h"

CBaseVideoRnd::CBaseVideoRnd(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_nType (YY_VRND_MAX)
	, m_nRotate (0)
	, m_pClock (NULL)
	, m_pSubTTEng (NULL)
	, m_hView (NULL)
	, m_nARWidth (1)
	, m_nARHeight (1)
	, m_nMaxWidth (1920)
	, m_nMaxHeight (1080)
	, m_pVideoRCC (NULL)
	, m_bUpdateView (false)
	, m_bDisableEraseBG (false)
	, m_nRndCount (0)
	, m_hFile (NULL)
{
	SetObjectName ("CBaseVideoRnd");

	memset (&m_rcVideo, 0, sizeof (RECT));
	memset (&m_rcZoom, 0, sizeof (RECT));
	memset (&m_rcView, 0, sizeof (RECT));
	memset (&m_rcRender, 0, sizeof (RECT));
	memset (&m_rcWindow, 0, sizeof (RECT));

	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
	memset (&m_bufVideo, 0, sizeof (m_bufVideo));

//	m_hFile = CreateFile(_T("C:\\Temp\\yyVideo.yuv"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, (DWORD) 0, NULL);
//	m_hFile = CreateFile(_T("C:\\Temp\\yyVideo.yuv"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
}

CBaseVideoRnd::~CBaseVideoRnd(void)
{
	Uninit ();

	YY_DEL_P (m_pClock);

#ifdef _OS_WIN32
	if (m_hFile != NULL)
		CloseHandle (m_hFile);
#endif // _OS_WIN32
}

int CBaseVideoRnd::SetDisplay (void * hView, RECT * pRect)
{
	CAutoLock lock (&m_mtDraw);

	m_hView = hView;
	if (pRect != NULL)
		memcpy (&m_rcView, pRect, sizeof (RECT));

	return YY_ERR_NONE;
}

int CBaseVideoRnd::UpdateDisp (void)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseVideoRnd::SetAspectRatio (int w, int h)
{
	CAutoLock lock (&m_mtDraw);
	if (m_nARWidth == w && m_nARHeight == h)
		return YY_ERR_NONE;
	m_nARWidth = w;
	m_nARHeight = h;
	UpdateRenderSize ();
	return YY_ERR_NONE;
}

int CBaseVideoRnd::SetDDMode (YY_PLAY_DDMode nMode)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseVideoRnd::SetRotate (int nAngle)
{
	m_nRotate = nAngle;
	return YY_ERR_NONE;
}

int CBaseVideoRnd::SetSubTTEng (void * pSubTTEng)
{
	m_pSubTTEng = pSubTTEng;

	return YY_ERR_NONE;
}

int CBaseVideoRnd::SetExtDDraw (void * pDDExt)
{
	return YY_ERR_NONE;
}

int CBaseVideoRnd::DisableEraseBG (void)
{
	m_bDisableEraseBG = true;
	return YY_ERR_NONE;
}

int CBaseVideoRnd::Init (YY_VIDEO_FORMAT * pFmt)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseVideoRnd::Uninit (void)
{
	YY_DEL_P (m_pVideoRCC);

	YY_DEL_A (m_fmtVideo.pHeadData);
	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));

	return YY_ERR_NONE;
}

int	CBaseVideoRnd::Start (void)
{
	if (m_pClock != NULL)
		m_pClock->Start ();

	return YY_ERR_NONE;
}

int CBaseVideoRnd::Pause (void)
{
	if (m_pClock != NULL)
		m_pClock->Pause ();

	return YY_ERR_NONE;
}

int	CBaseVideoRnd::Stop (void)
{
	return YY_ERR_NONE;
}

int CBaseVideoRnd::SetZoom (RECT * pRect)
{
	if (pRect == NULL)
		return YY_ERR_ARG;

	CAutoLock lock (&m_mtDraw);
	if (pRect->right - pRect->left == m_rcZoom.right - m_rcZoom.left && pRect->bottom - pRect->top == m_rcZoom.bottom - m_rcZoom.top)
	{
		memcpy (&m_rcZoom, pRect, sizeof (RECT));
		CheckZoomRect ();
	}
	else
	{
		memcpy (&m_rcZoom, pRect, sizeof (RECT));
		UpdateRenderSize ();
	}
	memcpy (pRect, &m_rcZoom, sizeof (RECT));
	return YY_ERR_NONE;
}

int CBaseVideoRnd::EnableKeyColor (bool bEnable)
{
	return YY_ERR_NONE;
}

int CBaseVideoRnd::Render (YY_BUFFER * pBuff)
{
	if (m_pClock != NULL)
	{
		if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		{
			m_nRndCount = 0;
			m_pClock->SetTime (pBuff->llTime);
		}
		else if (m_nRndCount == 0)
		{
			m_pClock->SetTime (pBuff->llTime);
		}
	}

#ifdef _OS_WINPC
	if (m_hFile == NULL)
		return YY_ERR_NONE;

//	if (m_nRndCount < 300 || m_nRndCount > 310)
	if (m_nRndCount > 10)
		return YY_ERR_NONE;

	if ((pBuff->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
	{
		AVFrame * pFrmVideo = (AVFrame *)pBuff->pBuff;

		DWORD	dwWrite = 0;
		int		i = 0;

#if 0
		for (i = 0; i < m_fmtVideo.nHeight; i++)
			WriteFile (m_hFile, pFrmVideo->data[0] + pFrmVideo->linesize[0] * i, m_fmtVideo.nWidth, &dwWrite, NULL);
		for (i = 0; i < m_fmtVideo.nHeight / 2; i++)
			WriteFile (m_hFile, pFrmVideo->data[1] + pFrmVideo->linesize[1] * i, m_fmtVideo.nWidth / 2, &dwWrite, NULL);
		for (i = 0; i < m_fmtVideo.nHeight / 2; i++)
			WriteFile (m_hFile, pFrmVideo->data[2] + pFrmVideo->linesize[2] * i, m_fmtVideo.nWidth / 2, &dwWrite, NULL);
#else
		for (i = 0; i < m_fmtVideo.nHeight; i++)
			ReadFile (m_hFile, pFrmVideo->data[0] + pFrmVideo->linesize[0] * i, m_fmtVideo.nWidth, &dwWrite, NULL);
		for (i = 0; i < m_fmtVideo.nHeight / 2; i++)
			ReadFile (m_hFile, pFrmVideo->data[1] + pFrmVideo->linesize[1] * i, m_fmtVideo.nWidth / 2, &dwWrite, NULL);
		for (i = 0; i < m_fmtVideo.nHeight / 2; i++)
			ReadFile (m_hFile, pFrmVideo->data[2] + pFrmVideo->linesize[2] * i, m_fmtVideo.nWidth / 2, &dwWrite, NULL);
#endif // 1
	}
#endif // _OS_WINPC

	return YY_ERR_NONE;
}

CBaseClock * CBaseVideoRnd::GetClock (void)
{
	if (m_pClock == NULL)
		m_pClock = new CBaseClock ();

	return m_pClock;
}

bool CBaseVideoRnd::UpdateRenderSize (void)
{
	if (m_fmtVideo.nWidth == 0 || m_fmtVideo.nHeight == 0)
		return false;

	m_rcVideo.top = 0;
	m_rcVideo.left = 0;
	m_rcVideo.right = m_fmtVideo.nWidth;
	m_rcVideo.bottom = m_fmtVideo.nHeight;
	CheckZoomRect ();

	int nRndW = m_rcView.right - m_rcView.left;
	int nRndH = m_rcView.bottom - m_rcView.top;

	if (m_nARWidth != 1 || m_nARHeight != 1)
	{
		if (nRndH * m_nARWidth >= m_nARHeight * nRndW)
			nRndH = nRndW * m_nARHeight / m_nARWidth;
		else 
			nRndW = nRndH * m_nARWidth / m_nARHeight;
	}
	else
	{
		int nWidth = m_rcZoom.right - m_rcZoom.left;
		int nHeight = m_rcZoom.bottom - m_rcZoom.top;
		if (m_nRotate == 90 || m_nRotate == 270)
		{
			nWidth = m_rcZoom.bottom - m_rcZoom.top;
			nHeight = m_rcZoom.right - m_rcZoom.left;
		}

		if ((m_fmtVideo.nNum == 0 || m_fmtVideo.nNum == 1) &&
			(m_fmtVideo.nDen == 1 || m_fmtVideo.nDen == 0))
		{
			if (nWidth * nRndH >= nHeight * nRndW)
				nRndH = nRndW * nHeight / nWidth;
			else 
				nRndW = nRndH * nWidth / nHeight;
		}
		else
		{
			if (m_fmtVideo.nDen == 0)
				m_fmtVideo.nDen = 1;
			nWidth = nWidth * m_fmtVideo.nNum / m_fmtVideo.nDen;
			if (nWidth * nRndH >= nHeight * nRndW)
				nRndH = nRndW * nHeight / nWidth;
			else 
				nRndW = nRndH * nWidth / nHeight;
		}
	}

	m_rcRender.left = m_rcView.left + (GetRectW (&m_rcView) - nRndW) / 2;
	m_rcRender.top = m_rcView.top + (GetRectH (&m_rcView) - nRndH) / 2;
	m_rcRender.right = m_rcRender.left + nRndW;
	m_rcRender.bottom = m_rcRender.top + nRndH;
	m_rcRender.left = m_rcRender.left & ~3;
	m_rcRender.top = m_rcRender.top & ~1;
	m_rcRender.right = m_rcRender.right & ~3;
	m_rcRender.bottom = m_rcRender.bottom & ~1;

	m_bUpdateView = true;

	return true;
}

bool CBaseVideoRnd::CheckZoomRect (void)
{
	if (m_rcZoom.right == 0 || m_rcZoom.right == 0)
	{
		memcpy (&m_rcZoom, &m_rcVideo, sizeof (m_rcVideo));
		if (m_nRotate == 90 || m_nRotate == 270)
		{
			m_rcZoom.left = m_rcVideo.left;
			m_rcZoom.top = m_rcVideo.top;
			m_rcZoom.right = m_rcVideo.bottom;
			m_rcZoom.bottom = m_rcVideo.right;
		}
	}

	if (m_rcZoom.left < 0) m_rcZoom.left= 0;
	if (m_rcZoom.top < 0) m_rcZoom.top= 0;
	if (m_nRotate == 90 || m_nRotate == 270)
	{
		if (m_rcZoom.right > m_rcVideo.bottom) m_rcZoom.bottom = m_rcVideo.bottom;
		if (m_rcZoom.bottom > m_rcVideo.right) m_rcZoom.right = m_rcVideo.right;
	}
	else
	{
		if (m_rcZoom.right > m_rcVideo.right) m_rcZoom.right = m_rcVideo.right;
		if (m_rcZoom.bottom > m_rcVideo.bottom) m_rcZoom.bottom = m_rcVideo.bottom;
	}

	m_rcZoom.left = m_rcZoom.left & ~3;
	m_rcZoom.top = m_rcZoom.top & ~1;
	m_rcZoom.right = m_rcZoom.right & ~3;
	m_rcZoom.bottom = m_rcZoom.bottom & ~1;

	return true;
}

bool CBaseVideoRnd::UpdateBackGround (void)
{
	if (!m_bUpdateView || m_hView == NULL || m_bDisableEraseBG)
		return false;

	m_bUpdateView = false;
#ifdef _OS_WIN32
	HWND hWnd = (HWND)m_hView;
	RECT rcDraw;
	if (m_rcRender.left > m_rcView.left)
	{
		SetRect (&rcDraw, m_rcView.left, m_rcView.top, m_rcRender.left + 4, m_rcView.bottom);
		InvalidateRect (hWnd, &rcDraw, TRUE);
	}
	if (m_rcRender.right < m_rcView.right)
	{
		SetRect (&rcDraw, m_rcRender.right - 2, m_rcView.top, m_rcView.right, m_rcView.bottom);
		InvalidateRect (hWnd, &rcDraw, TRUE);
	}
	if (m_rcRender.top > m_rcView.top)
	{
		SetRect (&rcDraw, m_rcView.left, m_rcView.top, m_rcView.right, m_rcRender.top + 2);
		InvalidateRect (hWnd, &rcDraw, TRUE);
	}
	if (m_rcRender.bottom < m_rcView.bottom)
	{
		SetRect (&rcDraw, m_rcView.left, m_rcRender.bottom - 2, m_rcView.right, m_rcView.bottom);
		InvalidateRect (hWnd, &rcDraw, TRUE);
	}
#endif // _OS_WIN32

	return true;
}

int CBaseVideoRnd::OverLogo (void)
{
	if (CLicenseCheck::m_pLcsChk != NULL)
	{
		if (CLicenseCheck::m_pLcsChk->m_nLcsStatus1 == YY_LCS_V1 &&
			CLicenseCheck::m_pLcsChk->m_nLcsStatus2 == YY_LCS_V2 &&
			CLicenseCheck::m_pLcsChk->m_nLcsStatus3 == YY_LCS_V3)
		{
			return YY_ERR_NONE;
		}
	}

	int				h = 0;
	unsigned char *	pLOGOBuff = (unsigned char *)yyLogoBuffY;

	if (m_bufLogo.nType == YY_VDT_YUV420_P)
	{
		for (h = 0; h < YYLOGO_HEIGHT; h++)
			memcpy (m_bufLogo.pBuff[0] + h * m_bufLogo.nStride[0], pLOGOBuff + h * YYLOGO_WIDTH, YYLOGO_WIDTH);
		pLOGOBuff = (unsigned char *)yyLogoBuffY + YYLOGO_WIDTH * YYLOGO_HEIGHT;
		for (h = 0; h < YYLOGO_HEIGHT / 2; h++)
			memcpy (m_bufLogo.pBuff[1] + h * m_bufLogo.nStride[1], pLOGOBuff + h * YYLOGO_WIDTH / 2, YYLOGO_WIDTH / 2);
		pLOGOBuff = (unsigned char *)yyLogoBuffY + (YYLOGO_WIDTH * YYLOGO_HEIGHT) * 5 / 4;
		for (h = 0; h < YYLOGO_HEIGHT / 2; h++)
			memcpy (m_bufLogo.pBuff[2] + h * m_bufLogo.nStride[2], pLOGOBuff + h * YYLOGO_WIDTH / 2, YYLOGO_WIDTH / 2);
	}
	else if (m_bufLogo.nType == YY_VDT_NV12)
	{
		for (h = 0; h < YYLOGO_HEIGHT; h++)
			memcpy (m_bufLogo.pBuff[0] + h * m_bufLogo.nStride[0], pLOGOBuff + h * YYLOGO_WIDTH, YYLOGO_WIDTH);
	
		unsigned char * pUV = m_bufLogo.pBuff[1];
		unsigned char * pU = (unsigned char *)yyLogoBuffY + YYLOGO_WIDTH * YYLOGO_HEIGHT;
		unsigned char * pV = (unsigned char *)yyLogoBuffY + (YYLOGO_WIDTH * YYLOGO_HEIGHT) * 5 / 4;
		for (h = 0; h < YYLOGO_HEIGHT / 2; h++)
		{
			pUV = m_bufLogo.pBuff[1] + h * m_bufLogo.nStride[1];
			for (int j = 0; j < YYLOGO_WIDTH / 2; j++)
			{
				*pUV++ = *pU++;
				*pUV++ = *pV++;
			}
		}
	}
	else if (m_bufLogo.nType == YY_VDT_RGBA)
	{
		pLOGOBuff = (unsigned char *)yyLogoBuffRGBA;
		for (h = 0; h < YYLOGO_HEIGHT; h++)
			memcpy (m_bufLogo.pBuff[0] + h * m_bufLogo.nStride[0], pLOGOBuff + (YYLOGO_HEIGHT - h -1) * YYLOGO_WIDTH * 4, YYLOGO_WIDTH * 4);
	}
	else if (m_bufLogo.nType == YY_VDT_ARGB)
	{
		pLOGOBuff = (unsigned char *)yyLogoBuffARGB;
		for (h = 0; h < YYLOGO_HEIGHT; h++)
			memcpy (m_bufLogo.pBuff[0] + h * m_bufLogo.nStride[0], pLOGOBuff + (YYLOGO_HEIGHT - h -1) * YYLOGO_WIDTH * 4, YYLOGO_WIDTH * 4);
	}
	else if (m_bufLogo.nType == YY_VDT_RGB565)
	{
	}
	else if (m_bufLogo.nType == YY_VDT_RGB24)
	{
		pLOGOBuff = (unsigned char *)yyLogoBuffRGB24;
		for (h = 0; h < YYLOGO_HEIGHT; h++)
			memcpy (m_bufLogo.pBuff[0] + h * m_bufLogo.nStride[0], pLOGOBuff + h * YYLOGO_WIDTH * 3, YYLOGO_WIDTH * 3);
	}

	return YY_ERR_NONE;
}
