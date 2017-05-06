/*******************************************************************************
	File:		CCtrlSlider.cpp

	Contains:	The slider bar control implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-28		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "CCtrlSlider.h"

#include "USystemFunc.h"
#include "UBitmapFunc.h"

#include "yyLog.h"

#pragma warning (disable : 4996)

CCtrlSlider::CCtrlSlider(TCHAR * pPath)
	: CCtrlBase (pPath)
	, m_nMin (0)
	, m_nMax (100)
	, m_nPos (0)
	, m_pThumbBmp (NULL)
	, m_nThumbNum (2)
	, m_hThumbMem (NULL)
	, m_pThumbBuf (NULL)
	, m_nThumbW (0)
	, m_nThumbH (0)
	, m_nOffLeft (0)
	, m_nOffRight (0)
{
	SetObjectName ("CCtrlSlider");
	SetRectEmpty (&m_rcThumb);
}

CCtrlSlider::~CCtrlSlider(void)
{
	YY_DEL_P (m_pThumbBmp);
	if (m_hThumbMem != NULL)
		DeleteObject (m_hThumbMem);
}

bool CCtrlSlider::SetRange (int nMin, int nMax)
{
	if (m_nMin == nMin && m_nMax == nMax)
		return true;
	if (m_nMin < 0 || m_nMax <= m_nMin)
		return false;
	m_nMin = nMin;
	m_nMax = nMax;
	InvalidateRect (m_hWnd, &m_rcPos, FALSE);
	return true;
}

bool CCtrlSlider::SetPos (int nPos)
{
	if (m_nPos == nPos)
		return true;
	if (nPos < m_nMin || nPos > m_nMax)
		return false;
	m_nPos = nPos;
	InvalidateRect (m_hWnd, &m_rcPos, FALSE);
	return true;
}

int CCtrlSlider::GetPos (void)
{
	return m_nPos;
}

bool CCtrlSlider::Create (HWND hWnd, CBaseConfig * pCfg, char * pItemName)
{
	if (!CCtrlBase::Create (hWnd, pCfg, pItemName))
		return false;

	TCHAR szThumbFile[1024];
	char * pBmpFile = pCfg->GetItemText (pItemName, "BmpFile_tn", "NoFile");
	_tcscpy (szThumbFile, m_pPath);
	MultiByteToWideChar (CP_ACP, 0, pBmpFile, -1, szThumbFile + _tcslen (szThumbFile), sizeof (m_szBmpFile) - _tcslen (m_pPath) * sizeof (TCHAR));
	m_nThumbNum = pCfg->GetItemValue (pItemName, "BmpNum", m_nThumbNum);
	m_nOffLeft = pCfg->GetItemValue (pItemName, "OffsetLeft", m_nOffLeft);
	m_nOffRight = pCfg->GetItemValue (pItemName, "OffsetRight", m_nOffRight);

	if (m_pThumbBmp == NULL)
		m_pThumbBmp = new CBmpFile ();
	HDC hDC = GetDC (m_hWnd);
	int nRC = m_pThumbBmp->ReadBmpFile (hDC, szThumbFile, m_nThumbNum);
	ReleaseDC (m_hWnd, hDC);
	if (nRC != YY_ERR_NONE)
	{
		YY_DEL_P (m_pThumbBmp);
		return false;
	}

	m_nThumbW = m_pThumbBmp->GetWidth ();
	m_nThumbH = m_pThumbBmp->GetHeight ();

	UpdateThumbRect ();

	return true;
}

void CCtrlSlider::UpdateThumbRect (void)
{
	int nWidth = m_nWidth - m_nThumbW - m_nOffLeft - m_nOffRight;
	int nLeft = (m_nPos - m_nMin) * nWidth / (m_nMax - m_nMin) - m_nThumbW / 2;
	nLeft = m_rcPos.left + m_nThumbW / 2 + m_nOffLeft + nLeft;
	if (nLeft < m_rcPos.left + m_nOffLeft)
		nLeft = m_rcPos.left + m_nOffLeft;
	if (nLeft + m_nThumbW > m_rcPos.right - m_nOffRight)
		nLeft = m_rcPos.right - m_nOffRight - m_nThumbW;

	SetRect (&m_rcThumb, nLeft, m_rcPos.top + (m_nHeight - m_nThumbH) / 2,
						 nLeft + m_nThumbW, m_rcPos.bottom - (m_nHeight - m_nThumbH) / 2);
}

LRESULT	CCtrlSlider::OnLButton (WPARAM wParam, LPARAM lParam, bool bDown)
{
	int nX = LOWORD (lParam);
	int nY = HIWORD (lParam);
	if (bDown)
	{
		if (InThumb (nX, nY))
		{
			m_bBtnDown = true;
			InvalidateRect (m_hWnd, &m_rcThumb, FALSE);
		}
		if (InRect (nX, nY))
		{
			m_nPos = (nX - m_rcPos.left - m_nThumbW / 2 - m_nOffLeft) * (m_nMax - m_nMin) / (m_nWidth - m_nThumbW - m_nOffLeft - m_nOffRight);
			if (m_nPos < m_nMin) m_nPos = m_nMin;
			if (m_nPos > m_nMax) m_nPos = m_nMax;
			UpdateThumbRect ();
			InvalidateRect (m_hWnd, &m_rcPos, FALSE);
			SendMessage (m_hWnd, WM_HSCROLL, m_nPos, (LPARAM)this);
		}
	}
	else
	{
		if (m_bBtnDown)
		{
			m_bBtnDown = false;
			InvalidateRect (m_hWnd, &m_rcThumb, FALSE);
		}
	}
	return S_OK;
}

LRESULT	CCtrlSlider::OnMouseMove (WPARAM wParam, LPARAM lParam)
{
	int nX = LOWORD (lParam);
	int nY = HIWORD (lParam);
	if (wParam != MK_LBUTTON || !InThumb (nX, nY))
		return S_OK;

	m_nPos = (nX - m_rcPos.left - m_nThumbW / 2 - m_nOffLeft) * (m_nMax - m_nMin) / (m_nWidth - m_nThumbW - m_nOffLeft - m_nOffRight);
	if (m_nPos < m_nMin) m_nPos = m_nMin;
	if (m_nPos > m_nMax) m_nPos = m_nMax;
	UpdateThumbRect ();
	InvalidateRect (m_hWnd, &m_rcPos, FALSE);

	SendMessage (m_hWnd, WM_HSCROLL, m_nPos, (LPARAM)this);

	return S_OK;
}

void CCtrlSlider::OnPaint (HDC hDC)
{
	CCtrlBase::OnPaint (hDC);

	int nIndex = 0;
	if (m_bBtnDown)
		nIndex = 1;
	int nWidth = m_pThumbBmp->GetWidth ();
	int nHeight = m_pThumbBmp->GetHeight ();
	if (m_hThumbMem == NULL)
		m_hThumbMem = yyBmpCreate (m_hMemDC, nWidth, nHeight, &m_pThumbBuf, 0);

	LPBYTE	pBuf = NULL;
	HBITMAP	hBmp = m_pThumbBmp->GetBmpHandle (nIndex, &pBuf);
	HBITMAP hOld = (HBITMAP)SelectObject (m_hMemDC, m_hThumbMem);
	BitBlt (m_hMemDC, 0, 0, nWidth, nHeight, hDC, m_rcThumb.left, m_rcThumb.top, SRCCOPY);

	int * pTar = (int *)m_pThumbBuf;
	int * pSrc = (int *)pBuf;
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			if (*pSrc != m_clrTP)
				*pTar = *pSrc;
			pTar++; pSrc++;
		}
	}

	BitBlt (hDC, m_rcThumb.left, m_rcThumb.top, nWidth, nHeight, m_hMemDC, 0, 0, SRCCOPY);
	SelectObject (m_hMemDC, hOld);
}

bool CCtrlSlider::InThumb (int nX, int nY)
{
	POINT pt;
	pt.x = nX;
	pt.y = nY;

	if (PtInRect (&m_rcThumb, pt))
		return true;
	else
		return false;
}