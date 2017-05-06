/*******************************************************************************
	File:		CVideoGDIRnd.cpp

	Contains:	The base Video GDI render implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CVideoGDIRnd.h"

#include "UYYDataFunc.h"
#include "URGB32Rotate.h"
#include "yyMonitor.h"
#include "yyConfig.h"
#include "yyLog.h"

#define iMASK_COLORS 3
#define SIZE_MASKS (iMASK_COLORS * sizeof(DWORD))
const DWORD bits565[3] = {0X0000F800, 0X000007E0, 0X0000001F,};

CVideoGDIRnd::CVideoGDIRnd(void * hInst)
	: CBaseVideoRnd (hInst)
	, m_hWnd (NULL)
	, m_hWinDC(NULL)
	, m_hMemDC(NULL)
	, m_hBmpVideo (NULL)
	, m_pBmpBuff (NULL)
	, m_pBmpInfo (NULL)
#ifdef _OS_WINCE
	, m_nPixelBits (32)
#else
	, m_nPixelBits (32)
#endif // WINCE
	, m_nRndStride (0)
	, m_hBmpOld (NULL)
	, m_pRGBBuff (NULL)
	, m_pSubTT (NULL)
{
	SetObjectName ("CVideoGDIRnd");
	m_nType = YY_VRND_GDI;
	memset (&m_rcBmp, 0, sizeof (m_rcBmp));
}

CVideoGDIRnd::~CVideoGDIRnd(void)
{
	Uninit ();
}

int CVideoGDIRnd::SetDisplay (void * hView, RECT * pRect)
{
	CAutoLock lock (&m_mtDraw);

	if (m_hWnd == (HWND)hView)
	{
		if (pRect != NULL && !memcmp (pRect, &m_rcView, sizeof (m_rcView)))
			return YY_ERR_NONE;

		ReleaseResBMP ();
	}
	else
	{
		ReleaseResDC ();
	}

	m_hWnd = (HWND)hView;
	m_hView =hView;

	if (pRect == NULL)
		GetClientRect (m_hWnd, &m_rcView);
	else
		memcpy (&m_rcView, pRect, sizeof (RECT));

	if (m_hWinDC == NULL)
	{
		m_hWinDC = GetDC (m_hWnd);
		m_hMemDC = ::CreateCompatibleDC (m_hWinDC);
	}

	UpdateRenderSize ();	

	return YY_ERR_NONE;
}

int CVideoGDIRnd::UpdateDisp (void)
{
	CAutoLock lock (&m_mtDraw);
	if (m_hWinDC != NULL)
	{
		BitBlt(m_hWinDC, m_rcRender.left, m_rcRender.top, 
				GetRectW (&m_rcRender), GetRectH (&m_rcRender), m_hMemDC, 0, 0, SRCCOPY);
		return YY_ERR_NONE;
	}

	return YY_ERR_IMPLEMENT;
}

int CVideoGDIRnd::SetSubTTEng (void * pSubTTEng)
{
	m_pSubTTEng = pSubTTEng;

	m_pSubTT = (CSubtitleEngine *)m_pSubTTEng;

	return YY_ERR_NONE;
}

int CVideoGDIRnd::Init (YY_VIDEO_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;
	if (m_hWnd == NULL)
		return YY_ERR_STATUS;

	ReleaseResDC ();
	yyDataCloneVideoFormat (&m_fmtVideo, pFmt);
	YY_DEL_A (m_pRGBBuff);
	if (m_nRotate == 90 || m_nRotate == 270)
	{
		m_fmtVideo.nWidth = pFmt->nHeight;
		m_fmtVideo.nHeight = pFmt->nWidth;
	}
	else if (m_nRotate == 180)
	{
		m_fmtVideo.nWidth = pFmt->nWidth;
		m_fmtVideo.nHeight = pFmt->nHeight;
	}
	m_hWinDC = GetDC (m_hWnd);
	m_hMemDC = ::CreateCompatibleDC (m_hWinDC);

	UpdateRenderSize ();

	if (CreateResBMP ())
	{
		return YY_ERR_NONE;
	}

	return YY_ERR_FAILED;
}

int CVideoGDIRnd::Uninit (void)
{
	ReleaseResDC ();
	YY_DEL_A (m_pRGBBuff);
	return YY_ERR_NONE;
}

int CVideoGDIRnd::SetZoom (RECT * pRect)
{
	int nRC = CBaseVideoRnd::SetZoom (pRect);
	YY_DEL_A (m_pRGBBuff);
	if (!CreateResBMP ())
		return YY_ERR_FAILED;
	return nRC;
}

int CVideoGDIRnd::Render (YY_BUFFER * pBuff)
{
	if (!IsWindowVisible (m_hWnd))
		return YY_ERR_NONE;
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	if (rcView.bottom <= 16 || rcView.right <= 16)
		return YY_ERR_NONE;

	CAutoLock lock (&m_mtDraw);
	if (m_hBmpVideo == NULL)
	{
		CreateResBMP ();
		if (m_hBmpVideo == NULL)
			return YY_ERR_MEMORY;
	}
	if (m_pVideoRCC == NULL)
	{
		m_pVideoRCC = new CFFMpegVideoRCC (m_hInst);
		if (m_pVideoRCC == NULL)
			return YY_ERR_MEMORY;
	}
	int nRC = 0;
	if (m_nRotate == 0)
	{
		m_buffRnd.pBuff[0] = m_pBmpBuff;
		m_buffRnd.nStride[0] = m_nRndStride;
		if (m_nPixelBits == 32)
			m_buffRnd.nType = YY_VDT_RGBA;
		else if (m_nPixelBits == 24)
			m_buffRnd.nType = YY_VDT_RGB24;
		else
			m_buffRnd.nType = YY_VDT_RGB565;
		m_buffRnd.nWidth = GetRectW (&m_rcRender);
		m_buffRnd.nHeight = GetRectH (&m_rcRender);
		nRC = m_pVideoRCC->ConvertBuff (pBuff, &m_buffRnd, &m_rcZoom);
	}
	else
	{
		if (m_nRotate == 180)
		{
			m_buffRnd.nWidth = GetRectW (&m_rcRender);
			m_buffRnd.nHeight = GetRectH (&m_rcRender);
		}
		else
		{
			m_buffRnd.nWidth = GetRectH (&m_rcRender);
			m_buffRnd.nHeight = GetRectW (&m_rcRender);
		}
		if (m_pRGBBuff == NULL)
			m_pRGBBuff = new BYTE[m_buffRnd.nWidth * m_buffRnd.nHeight * 4];
		m_buffRnd.nType = YY_VDT_RGBA;
		m_buffRnd.pBuff[0] = m_pRGBBuff;
		m_buffRnd.nStride[0] = m_buffRnd.nWidth * 4;
		nRC = m_pVideoRCC->ConvertBuff (pBuff, &m_buffRnd, &m_rcZoom);
		if (m_nRotate == 90)
			yyRGB32Rotate90 (m_pRGBBuff, m_buffRnd.nWidth, m_buffRnd.nHeight, m_buffRnd.nWidth * 4, m_pBmpBuff, m_nRndStride);	
		else if (m_nRotate == 180)
			yyRGB32Rotate180 (m_pRGBBuff, m_buffRnd.nWidth, m_buffRnd.nHeight, m_buffRnd.nWidth * 4, m_pBmpBuff, m_nRndStride);	
		else if (m_nRotate == 270)
			yyRGB32Rotate270 (m_pRGBBuff, m_buffRnd.nWidth, m_buffRnd.nHeight, m_buffRnd.nWidth * 4, m_pBmpBuff, m_nRndStride);	
	}


	if (nRC == YY_ERR_NONE)
	{
		m_bufLogo.pBuff[0] = m_pBmpBuff;
		m_bufLogo.nStride[0] = m_nRndStride;
		if (m_nPixelBits == 16)
			m_bufLogo.nType = YY_VDT_RGB565;
		else
			m_bufLogo.nType = YY_VDT_RGBA;
		OverLogo ();

		if (m_pSubTT != NULL)
			m_pSubTT->Draw (m_hMemDC, &m_rcBmp, pBuff->llTime, false);

		BitBlt(m_hWinDC, m_rcRender.left, m_rcRender.top, 
			GetRectW (&m_rcRender), GetRectH (&m_rcRender), m_hMemDC, 0, 0, SRCCOPY);
	}

	return nRC;
}

bool CVideoGDIRnd::UpdateRenderSize (void)
{
	bool bRC = CBaseVideoRnd::UpdateRenderSize ();

	if ((m_rcBmp.right != m_rcRender.right - m_rcRender.left) ||
		(m_rcBmp.bottom != m_rcRender.bottom - m_rcRender.top))
	{
		m_rcBmp.right = m_rcRender.right - m_rcRender.left;
		m_rcBmp.bottom = m_rcRender.bottom - m_rcRender.top;

		ReleaseResBMP ();
		CreateResBMP ();
	}

	return bRC;
}

bool CVideoGDIRnd::CreateResBMP (void )
{
	if (m_pBmpInfo != NULL)
		delete[]m_pBmpInfo;

	int nBmpSize = sizeof(BITMAPINFOHEADER);
	if (m_nPixelBits == 16)
		nBmpSize += SIZE_MASKS; // for RGB bitMask;

	m_pBmpInfo = new BYTE[nBmpSize];
	memset (m_pBmpInfo, 0, nBmpSize);

	BITMAPINFO * pBmpInfo = (BITMAPINFO *)m_pBmpInfo;
	pBmpInfo->bmiHeader.biSize			= nBmpSize;
	pBmpInfo->bmiHeader.biWidth			= GetRectW (&m_rcRender);
	pBmpInfo->bmiHeader.biHeight		= 0 - GetRectH (&m_rcRender);
	pBmpInfo->bmiHeader.biBitCount		= (WORD)m_nPixelBits;
	if (m_nPixelBits == 16)
		pBmpInfo->bmiHeader.biCompression	= BI_BITFIELDS;
	else
		pBmpInfo->bmiHeader.biCompression	= BI_RGB;
	pBmpInfo->bmiHeader.biXPelsPerMeter	= 0;
	pBmpInfo->bmiHeader.biYPelsPerMeter	= 0;
	pBmpInfo->bmiHeader.biPlanes			= 1;

	if (m_nPixelBits == 16)
	{
		DWORD *	pBmiColors = (DWORD *)((LPBYTE)pBmpInfo + sizeof(BITMAPINFOHEADER));
		for (int i = 0; i < 3; i++)
    		*(pBmiColors + i) = bits565[i];
	}

	m_nRndStride = ((pBmpInfo->bmiHeader.biWidth * pBmpInfo->bmiHeader.biBitCount / 8) + 3) & ~3;
	pBmpInfo->bmiHeader.biSizeImage	= m_nRndStride * (m_rcRender.bottom - m_rcRender.top);

	if (m_hWinDC == NULL)
		return false;

	m_hBmpVideo = CreateDIBSection(m_hWinDC , (BITMAPINFO *)m_pBmpInfo , DIB_RGB_COLORS , (void **)&m_pBmpBuff, NULL , 0);
	if (m_pBmpBuff != NULL)
		memset (m_pBmpBuff, 0, ((BITMAPINFO *)m_pBmpInfo)->bmiHeader.biSizeImage);

	m_hBmpOld = (HBITMAP)SelectObject (m_hMemDC, m_hBmpVideo);

	return true;
}

bool CVideoGDIRnd::ReleaseResBMP (void)
{
	YY_DEL_A (m_pBmpInfo);

	if (m_hBmpOld != NULL && m_hMemDC != NULL)
		SelectObject (m_hMemDC, m_hBmpOld);

	if (m_hBmpVideo != NULL)
		DeleteObject (m_hBmpVideo);
	m_hBmpVideo = NULL;

	return true;
}

bool CVideoGDIRnd::ReleaseResDC (void)
{
	ReleaseResBMP ();

	if (m_hWinDC != NULL)
	{
		DeleteDC (m_hMemDC);
		ReleaseDC (m_hWnd, m_hWinDC);
	}

	m_hMemDC = NULL;
	m_hWinDC = NULL;

	return true;
}
