/*******************************************************************************
	File:		CPlaySlider.cpp

	Contains:	The play slider implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-09		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CPlaySlider.h"

#include "USystemFunc.h"
#include "RPlayerDef.h"
#include "yyLog.h"

#pragma warning (disable : 4996)

CPlaySlider::CPlaySlider(HINSTANCE hInst)
	: CPlayCtrl (hInst)
	, m_nMin (0)
	, m_nMax (100)
	, m_nPos (0)
	, m_nTmbLeft (0)
	, m_nTmbTop (0)
	, m_nTmbWidth (0)
	, m_nTmbHeight (0)
	, m_pBmpPrev (NULL)
	, m_pBmpNext (NULL)
	, m_pBmpThumb (NULL)
	, m_nThumbNum (1)
{
	SetObjectName ("CPlaySlider");
}	

CPlaySlider::~CPlaySlider(void)
{
	YY_DEL_P (m_pBmpPrev);
	YY_DEL_P (m_pBmpNext);
	YY_DEL_P (m_pBmpThumb);
}

bool CPlaySlider::SetRange (int nMin, int nMax)
{
	if (m_nMin == nMin && m_nMax == nMax)
		return true;
	if (nMin < 0 || nMax < nMin)
		return false;
	m_nMin = nMin;
	m_nMax = nMax;
	UpdateThumbPos ();
	return true;
}

bool CPlaySlider::SetPos (int nPos)
{
	if (m_bBtnDown)
		return false;

	if (m_nPos == nPos)
		return true;
	if (nPos < m_nMin || nPos > m_nMax)
		return false;
	m_nPos = nPos;
	UpdateThumbPos ();
	return true;
}

int CPlaySlider::GetPos (void)
{
	return m_nPos;
}

bool CPlaySlider::UpdateRect (void)
{
	if (!CPlayCtrl::UpdateRect ())
		return false;
	SetRect (&m_rcDraw, m_rcItem.left - m_nTmbWidth / 2, m_rcItem.top, m_rcItem.right + m_nTmbWidth / 2, m_rcItem.bottom);
	return true;
}

void CPlaySlider::UpdateThumbPos (void)
{
	UpdateRect ();
	if (m_nMax == m_nMin)
		return;

	long long llWidth = m_rcItem.right - m_rcItem.left;
	m_nTmbLeft = (int)(m_rcItem.left + llWidth * m_nPos / (m_nMax - m_nMin) - m_nTmbWidth / 2);
	m_nTmbTop = m_rcItem.top + ((m_rcItem.bottom - m_rcItem.top) - m_nTmbHeight) / 2;

	UpdateView (&m_rcDraw, FALSE);
}

bool CPlaySlider::InThumb (int nX, int nY)
{
	UpdateThumbPos ();

	POINT pt;
	pt.x = nX;
	pt.y = nY;

	RECT rcThumb;
	SetRect (&rcThumb, m_nTmbLeft, m_nTmbTop, m_nTmbLeft + m_nTmbWidth, m_nTmbTop + m_nTmbHeight);

	if (PtInRect (&rcThumb, pt))
		return true;
	else
		return false;
}

bool CPlaySlider::Create (HWND hWnd, CBaseConfig * pCfg, char * pItemName)
{
	if (!CPlayCtrl::Create (hWnd, pCfg, pItemName))
		return false;

	TCHAR szPath[1024];
	_tcscpy (szPath, pCfg->GetFileName ());
	TCHAR * pPos = _tcsrchr (szPath, _T('\\'));
	if (pPos != NULL)
		*(pPos + 1) = 0;

	char * pBmpFile = pCfg->GetItemText (pItemName, "BmpPrevFile", "NoFile");
	m_pBmpPrev = CreateBmpFile (szPath, pBmpFile, 1);
	pBmpFile = pCfg->GetItemText (pItemName, "BmpNextFile", "NoFile");
	m_pBmpNext = CreateBmpFile (szPath, pBmpFile, 1);
	pBmpFile = pCfg->GetItemText (pItemName, "BmpThumbFile", "NoFile");
	m_nThumbNum = pCfg->GetItemValue (pItemName, "BmpThumbNum", m_nThumbNum);
	m_pBmpThumb = CreateBmpFile (szPath, pBmpFile, m_nThumbNum);
	if (m_pBmpThumb != NULL)
	{
		BITMAP	bmpInfo;
		GetObject (m_pBmpThumb->GetBmpHandle (0, NULL), sizeof (BITMAP), &bmpInfo);
		m_nTmbWidth = bmpInfo.bmWidth;
		m_nTmbHeight = bmpInfo.bmHeight;
	}
	m_nWidth = pCfg->GetItemValue (pItemName, "Width", m_nWidth);
	m_nHeight = pCfg->GetItemValue (pItemName, "Height", m_nHeight);

	UpdateThumbPos ();

	return true;
}

bool CPlaySlider::OnDraw (HDC hDC, HBITMAP hBmp, LPBYTE pBuff, RECT * pRect)
{
	if (!m_bShow)
		return false;
	UpdateRect ();
	if (m_rcItem.bottom > pRect->bottom || m_rcItem.right > pRect->right)
	{
		YYLOGI ("The rect of slider was large than window! %d X %d   %d X %d", m_rcItem.right, m_rcItem.bottom, pRect->right, pRect->bottom);
		return false;
	}
	int	nW = m_rcItem.right - m_rcItem.left;
	int	nH = m_rcItem.bottom - m_rcItem.top;

	LPBYTE	pBuf = NULL;
	HBITMAP	hBmpItem = m_pBmpNext->GetBmpHandle (0, &pBuf);
	BITMAP	bmpInfo;
	GetObject (hBmpItem, sizeof (BITMAP), &bmpInfo);
	if (m_rcItem.top + bmpInfo.bmHeight > pRect->bottom)
	{
		YYLOGI ("The rect of slider was large than window! %d X %d   %d X %d", m_rcItem.right, m_rcItem.top + bmpInfo.bmHeight, pRect->right, pRect->bottom);
		return false;
	}

	unsigned char	cTPN = 0;
	unsigned char * pSrc = pBuf;
	unsigned char * pTar = pBuff;
	int				h, w, n;
	for (h = 0; h < bmpInfo.bmHeight; h++)
	{
		pTar = pBuff + pRect->right * 4 * (h + m_rcItem.top + (nH - bmpInfo.bmHeight) / 2) + m_rcItem.left * 4;
		for (w = m_rcItem.left; w < m_rcItem.right; w += bmpInfo.bmWidth)
		{
			pSrc = pBuf + bmpInfo.bmWidth * 4 * h;
			for (n = 0; n < bmpInfo.bmWidth; n++)
			{
				cTPN = 255 - pSrc[3];
				*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
				*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
				*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
				pTar++; pSrc++;
			}
		}
	}

	hBmpItem = m_pBmpPrev->GetBmpHandle (0, &pBuf);
	GetObject (hBmpItem, sizeof (BITMAP), &bmpInfo);
	if (m_rcItem.top + bmpInfo.bmHeight > pRect->bottom)
		return false;
	for (h = 0; h < bmpInfo.bmHeight; h++)
	{
		pTar = pBuff + pRect->right * 4 * (h + m_rcItem.top + (nH - bmpInfo.bmHeight) / 2) + m_rcItem.left * 4;
		for (w = m_rcItem.left; w < m_nTmbLeft + m_nTmbWidth / 2; w += bmpInfo.bmWidth)
		{
			pSrc = pBuf + bmpInfo.bmWidth * 4 * h;
			for (n = 0; n < bmpInfo.bmWidth; n++)
			{
				cTPN = 255 - pSrc[3];
				*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
				*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
				*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
				pTar++; pSrc++;
			}
		}
	}

	UpdateThumbPos ();
	if (m_bBtnDown)
		hBmpItem = m_pBmpThumb->GetBmpHandle (1, &pBuf);
	else
		hBmpItem = m_pBmpThumb->GetBmpHandle (0, &pBuf);
	GetObject (hBmpItem, sizeof (BITMAP), &bmpInfo);
	if (m_rcItem.top + bmpInfo.bmHeight > pRect->bottom)
		return false;
	pSrc = pBuf;
	for (h = 0; h < bmpInfo.bmHeight; h++)
	{
		pTar = pBuff + pRect->right * 4 * (h + m_nTmbTop) + m_nTmbLeft * 4;
		for (w = 0; w < m_nTmbWidth; w++)
		{
			cTPN = pSrc[3];
			*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
			*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
			*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
			pTar++; pSrc++;
		}
	}

	return true;
}

LRESULT	CPlaySlider::OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!InThumb (LOWORD (lParam), HIWORD (lParam)))
		return S_FALSE;
	m_bBtnDown = true;
	UpdateView (&m_rcDraw, FALSE);
	return S_OK;
}

LRESULT	CPlaySlider::OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bBtnDown)
	{
		m_bBtnDown = false;
		UpdateView (&m_rcDraw, FALSE);
	}
	if (InRect (LOWORD (lParam), HIWORD (lParam)))
	{
		if (m_nMax == m_nMin)
			return S_OK;
		long long llXPos = LOWORD (lParam);
		m_nPos = (int)((llXPos - m_rcItem.left) * (m_nMax - m_nMin) / (m_rcItem.right - m_rcItem.left));
		if (m_nPos < m_nMin) m_nPos = m_nMin;
		if (m_nPos > m_nMax) m_nPos = m_nMax;
		if ((m_nPos - m_nMin) * 100.0 / (m_nMax - m_nMin) <= 0.5)
			m_nPos = m_nMin;
		UpdateThumbPos ();
		PostMessage (m_hWnd, WT_PLAY_HScroll, m_nPos, (LPARAM)this);
		return S_OK;
	}

	return S_FALSE;
}

LRESULT	CPlaySlider::OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	long long llX = LOWORD (lParam);
	long long llY = HIWORD (lParam);
	if (wParam != MK_LBUTTON || !InThumb ((int)llX, (int)llY))
		return S_FALSE;

	if (m_nMax == m_nMin)
		return S_FALSE;
	m_nPos = (int)((llX - m_rcItem.left) * (m_nMax - m_nMin) / (m_rcItem.right - m_rcItem.left));
	if (m_nPos < m_nMin) m_nPos = m_nMin;
	if (m_nPos > m_nMax) m_nPos = m_nMax;
	UpdateThumbPos ();
	SendMessage (m_hWnd, WM_HSCROLL, m_nPos, (LPARAM)this);

	return S_FALSE;
}
