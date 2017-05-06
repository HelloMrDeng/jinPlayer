/*******************************************************************************
	File:		CCtrlBase.cpp

	Contains:	The base control implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-27		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "CCtrlBase.h"

#include "USystemFunc.h"
#include "UBitmapFunc.h"

#include "yyLog.h"

#pragma warning (disable : 4996)

CCtrlBase::CCtrlBase(TCHAR * pPath)
	: CBaseObject ()
	, m_pPath (pPath)
{
	SetObjectName ("CCtrlBase");

	m_hWnd = NULL;
	strcpy (m_szName, "CtrlBase");
	strcpy (m_szType, "CtrlBase");

	m_nID = -1;
	SetRectEmpty (&m_rcPos);
	memset (m_szBmpFile, 0, sizeof (m_szBmpFile));
	m_nBmpNum = 1;
	m_clrTP = RGB (255, 0, 255);

	m_pBmpFile = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_bShow = true;
	m_bEnable = true;
	m_bBtnDown = false;
	m_bMusOver = false;

	m_hMemDC = NULL;
	m_hMemBmp = NULL;
	m_pMemBuf = NULL;
}

CCtrlBase::~CCtrlBase(void)
{
	YY_DEL_P (m_pBmpFile);

	if (m_hMemDC != NULL)
		DeleteDC (m_hMemDC);
	if (m_hMemBmp != NULL)
		DeleteObject (m_hMemBmp);
}

bool CCtrlBase::Create (HWND hWnd, CBaseConfig * pCfg, char * pItemName)
{
	m_hWnd = hWnd;

	strcpy (m_szName, pItemName);
	strcpy (m_szType, pCfg->GetItemText (pItemName, "Type", "Base"));
	m_nID = pCfg->GetItemValue (pItemName, "ID", m_nID);
	pCfg->GetItemRect (pItemName, &m_rcPos);
	char * pBmpFile = pCfg->GetItemText (pItemName, "BmpFile", "NoFile");
	_tcscpy (m_szBmpFile, m_pPath);
	MultiByteToWideChar (CP_ACP, 0, pBmpFile, -1, m_szBmpFile + _tcslen (m_szBmpFile), sizeof (m_szBmpFile) - _tcslen (m_pPath) * sizeof (TCHAR));
	m_nBmpNum = pCfg->GetItemValue (pItemName, "BmpNum", m_nBmpNum);
	m_clrTP = pCfg->GetItemValue (pItemName, "ColorTP", m_clrTP);

	if (m_pBmpFile == NULL)
		m_pBmpFile = new CBmpFile ();

	HDC hDC = GetDC (m_hWnd);
	int nRC = m_pBmpFile->ReadBmpFile (hDC, m_szBmpFile, m_nBmpNum);
	ReleaseDC (m_hWnd, hDC);
	if (nRC != YY_ERR_NONE)
	{
		YY_DEL_P (m_pBmpFile);
		return false;
	}

	if (m_rcPos.right <= m_rcPos.left)
		m_rcPos.right = m_rcPos.left + m_pBmpFile->GetWidth ();
	if (m_rcPos.bottom <= m_rcPos.top)
		m_rcPos.bottom = m_rcPos.top + m_pBmpFile->GetHeight ();

	m_nWidth = m_rcPos.right - m_rcPos.left;
	m_nHeight = m_rcPos.bottom - m_rcPos.top;

	return true;
}

bool CCtrlBase::Show (bool bShow)
{
	if (m_bShow == bShow)
		return true;
	m_bShow = bShow;
	InvalidateRect (m_hWnd, &m_rcPos, FALSE);
	return true;
}

bool CCtrlBase::Enable (bool bEnable)
{
	if (m_bEnable == bEnable)
		return true;
	m_bEnable = bEnable;
	InvalidateRect (m_hWnd, &m_rcPos, FALSE);
	return true;
}

LRESULT CCtrlBase::OnMsg (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		OnLButton (wParam, lParam, true);
		break;

	case WM_LBUTTONUP:	
		OnLButton (wParam, lParam, false);
		break;

	case WM_RBUTTONUP:
		break;

	case WM_MOUSEMOVE:
		OnMouseMove (wParam, lParam);
		break;

	case WM_TIMER:
		break;

	default:
		break;
	}

	return S_FALSE;
}

LRESULT	CCtrlBase::OnLButton (WPARAM wParam, LPARAM lParam, bool bDown)
{
	int nX = LOWORD (lParam);
	int nY = HIWORD (lParam);
	if (bDown)
	{
		if (InRect (nX, nY))
		{
			m_bBtnDown = true;
			InvalidateRect (m_hWnd, &m_rcPos, FALSE);
		}
	}
	else
	{
		if (m_bBtnDown)
		{
			m_bBtnDown = false;
			InvalidateRect (m_hWnd, &m_rcPos, FALSE);
			if (InRect (nX, nY))
				SendMessage (m_hWnd, WM_COMMAND, m_nID, 0);
		}
	}

	return S_OK;
}

LRESULT	CCtrlBase::OnMouseMove (WPARAM wParam, LPARAM lParam)
{
	int nX = LOWORD (lParam);
	int nY = HIWORD (lParam);
	if (InRect (nX, nY))
	{
		if (!m_bMusOver)
		{
			m_bMusOver = true;
			InvalidateRect (m_hWnd, &m_rcPos, FALSE);
		}
	}
	else
	{
		if (m_bMusOver)
		{
			m_bMusOver = false;
			InvalidateRect (m_hWnd, &m_rcPos, FALSE);
		}
	}
	if (wParam == MK_LBUTTON)
	{
		if (m_bBtnDown)
		{
			if (!InRect (nX, nY))
			{
				m_bBtnDown = false;
				InvalidateRect (m_hWnd, &m_rcPos, FALSE);
			}
		}
	}
	return S_OK;
}

void CCtrlBase::OnPaint (HDC hDC)
{
	if (m_pBmpFile == NULL || !m_bShow)
		return;

	if (m_hMemDC == NULL)
		m_hMemDC = CreateCompatibleDC (hDC);
	if (m_hMemBmp == NULL)
		m_hMemBmp = yyBmpCreate (m_hMemDC, m_nWidth, m_nHeight, &m_pMemBuf, 0);

	int nIndex = 0;
	if (!m_bEnable)
		nIndex = 3;
	else if (m_bBtnDown)
		nIndex = 1;
	else if (m_bMusOver)
		nIndex = 2;
	if (nIndex >= m_pBmpFile->GetNum ())
		nIndex = m_pBmpFile->GetNum () - 1;

	LPBYTE	pBuf = NULL;
	HBITMAP	hBmp = m_pBmpFile->GetBmpHandle (nIndex, &pBuf);
	HBITMAP hOld = (HBITMAP)SelectObject (m_hMemDC, m_hMemBmp);
	BitBlt (m_hMemDC, 0, 0, m_nWidth, m_nHeight, hDC, m_rcPos.left, m_rcPos.top, SRCCOPY);
/*
	int * pTar = (int *)m_pMemBuf;
	int * pSrc = (int *)pBuf;
	for (int i = 0; i < m_nHeight; i++)
	{
		for (int j = 0; j < m_nWidth; j++)
		{
			if (*pSrc != m_clrTP)
				*pTar = *pSrc;
			pTar++; pSrc++;
		}
	}
*/
	unsigned char * pTar = m_pMemBuf;
	unsigned char * pSrc = pBuf;
	for (int i = 0; i < m_nHeight; i++)
	{
		for (int j = 0; j < m_nWidth; j++)
		{
			*pTar = (*pTar++  * (256 - *pSrc)) / 256 + *pSrc++;
			*pTar = (*pTar++  * (256 - *pSrc)) / 256 + *pSrc++;
			*pTar = (*pTar++  * (256 - *pSrc)) / 256 + *pSrc++;
			*pTar = (*pTar++  * (256 - *pSrc)) / 256 + *pSrc++;
//			pTar++;
		}
	}
	BitBlt (hDC, m_rcPos.left, m_rcPos.top, m_nWidth, m_nHeight, m_hMemDC, 0, 0, SRCCOPY);
	SelectObject (m_hMemDC, hOld);
}

bool CCtrlBase::InRect (int nX, int nY)
{
	POINT pt;
	pt.x = nX;
	pt.y = nY;

	if (PtInRect (&m_rcPos, pt))
		return true;
	else
		return false;
}