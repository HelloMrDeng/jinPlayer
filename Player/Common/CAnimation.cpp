/*******************************************************************************
	File:		CAnimation.cpp

	Contains:	The player ui animation implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-06		Fenger			Create file

*******************************************************************************/
#include "windows.h"

#include "CAnimation.h"
#include "resource.h"

#include "USystemFunc.h"
#include "UBitmapFunc.h"

CAnimation::CAnimation(HINSTANCE hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hBmpBack (NULL)
	, m_pBuffBack (NULL)
	, m_hBmpFore (NULL)
	, m_pBuffFore (NULL)
	, m_nTime (0)
	, m_hBmpNew (NULL)
	, m_hWnd (NULL)
{
	SetObjectName ("CAnimation");
	SetRectEmpty (&m_rcBmpBack);
	SetRectEmpty (&m_rcBmpFore);
	SetRectEmpty (&m_rcRndSrc);
	SetRectEmpty (&m_rcRndTgt);
}	

CAnimation::~CAnimation(void)
{
	if (m_hBmpNew != NULL)
		DeleteObject (m_hBmpNew);
}

bool CAnimation::SetBackBmp (HBITMAP hBmp, LPBYTE pBuff, RECT * pRect)
{
	if (hBmp == NULL)
		return false;

	m_hBmpBack = hBmp;
	m_pBuffBack = pBuff;
	if (pRect != NULL)
		memcpy (&m_rcBmpBack, pRect, sizeof (RECT));
	else
	{
		BITMAP bmpInfo;
		GetObject (hBmp, sizeof (BITMAP), &bmpInfo);
		SetRect (&m_rcBmpBack, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight);
	}
	return true;
}

bool CAnimation::SetForeBmp (HBITMAP hBmp, LPBYTE pBuff, RECT * pRect, int nTime)
{
	m_hBmpFore = hBmp;
	m_pBuffFore = pBuff;
	if (pRect != NULL)
		memcpy (&m_rcBmpFore, pRect, sizeof (RECT));
	else
	{
		BITMAP bmpInfo;
		GetObject (hBmp, sizeof (BITMAP), &bmpInfo);
		SetRect (&m_rcBmpFore, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight);
	}
	m_nTime = nTime;

	if (m_hBmpFore != NULL && m_nTime > 0 && m_pBuffFore != NULL)
	{
		m_hBmpNew = yyBmpClone (m_hBmpFore, &m_rcBmpFore, &m_pBuffFore);
		m_hBmpFore = m_hBmpNew;
	}

	return true;
}

bool CAnimation::SetRndWnd (HWND hWnd, RECT * pRCSrc, RECT * pRCTgt)
{
	if (pRCSrc == NULL || hWnd == NULL)
		return false;

	m_hWnd = hWnd;
	memcpy (&m_rcRndSrc, pRCSrc, sizeof (RECT));
	if (pRCTgt == NULL)
		GetClientRect (m_hWnd, &m_rcRndTgt);
	else
		memcpy (&m_rcRndTgt, pRCTgt, sizeof (RECT));
	return true;
}

bool CAnimation::Show (ANMT_TYPE nType)
{
	if (m_hWnd == NULL)
		return false;
	if (nType == ANMT_Expand)
		return Expand ();
	else if (nType == ANMT_Shrink)
		return Shrink ();
	else if (nType == ANMT_DoorOpen)
		return DoorOpen ();
	else if (nType == ANMT_DoorClose)
		return DoorClose ();
	else if (nType == ANMT_Transition)
		return Transition ();
	else
		return ShowNone ();
}

bool CAnimation::Expand (void)
{
	if (m_hBmpFore == NULL)
		return false;

	int		nSteps = 12;
	int		nSleep = 20;
	int		nStart = 0;
	HDC		hDCWin = GetDC (m_hWnd);
	HDC		hDCFore = CreateCompatibleDC (hDCWin);;
	HBITMAP	hOldFore = (HBITMAP)SelectObject (hDCFore, m_hBmpFore);
	RECT	rcDisp;
	for (int i = 1; i <= nSteps; i++)
	{
		nStart = yyGetSysTime ();
		int nBlackSteps = 5;
		if (i >= nSteps - nBlackSteps && m_nTime > 0 && m_pBuffFore != NULL)
		{
			unsigned char * pRGB = m_pBuffFore;
			for (int h = 0; h < m_rcBmpFore.bottom; h++)
			{
				for (int w = 0; w < m_rcBmpFore.right; w++)
				{
					*pRGB = *pRGB++ * (nSteps - i) / nBlackSteps;
					*pRGB = *pRGB++ * (nSteps - i) / nBlackSteps;
					*pRGB = *pRGB++ * (nSteps - i) / nBlackSteps;
					*pRGB++;
				}
			}
		}			
		rcDisp.left	= m_rcRndSrc.left - (m_rcRndSrc.left -  m_rcRndTgt.left) * i / nSteps;
		rcDisp.top	= m_rcRndSrc.top - (m_rcRndSrc.top -  m_rcRndTgt.top) * i / nSteps;
		rcDisp.right = m_rcRndSrc.right + (m_rcRndTgt.right - m_rcRndSrc.right) * i / nSteps;
		rcDisp.bottom = m_rcRndSrc.bottom + (m_rcRndTgt.bottom - m_rcRndSrc.bottom) * i / nSteps;
		if (rcDisp.left < m_rcRndTgt.left)
			rcDisp.left = m_rcRndTgt.left;
		if (rcDisp.top < m_rcRndTgt.top)
			rcDisp.top = m_rcRndTgt.top;
		if (rcDisp.right > m_rcRndTgt.right)
			rcDisp.right = m_rcRndTgt.right;
		if (rcDisp.bottom > m_rcRndTgt.bottom)
			rcDisp.bottom = m_rcRndTgt.bottom;
		StretchBlt (hDCWin, rcDisp.left, rcDisp.top, rcDisp.right - rcDisp.left, rcDisp.bottom - rcDisp.top,
					hDCFore, m_rcBmpFore.left, m_rcBmpFore.top, m_rcBmpFore.right - m_rcBmpFore.left, m_rcBmpFore.bottom - m_rcBmpFore.top, SRCCOPY);
		int nUsedTime = yyGetSysTime () - nStart;
		if (nUsedTime < nSleep)
			Sleep (nSleep - nUsedTime);
	}
	SelectObject (hDCFore, hOldFore);
	DeleteDC (hDCFore);
	ReleaseDC (m_hWnd, hDCWin);
	return true;
}

bool CAnimation::Shrink (void)
{
	if (m_hBmpBack == NULL)
		return false;

	int		nSteps = 12;
	int		nSleep = 20;
	int		nStart = 0;
	HDC		hDCWin = GetDC (m_hWnd);
	HDC		hDCBack = CreateCompatibleDC (hDCWin);
	HBITMAP	hOldBack = (HBITMAP)SelectObject (hDCBack, m_hBmpBack);
	HDC		hDCFore = CreateCompatibleDC (hDCWin);
	HBITMAP	hOldFore = NULL;
	HBITMAP hBmpFore = NULL;
	if (m_hBmpFore == NULL)
	{
		SetRect (&m_rcBmpFore, 0, 0, m_rcRndTgt.right - m_rcRndTgt.left, m_rcRndTgt.bottom - m_rcRndTgt.top);
		hBmpFore = yyBmpCreate (hDCFore, m_rcRndTgt.right - m_rcRndTgt.left, m_rcRndTgt.bottom - m_rcRndTgt.top, NULL, 0);
		hOldFore = (HBITMAP)SelectObject (hDCFore, hBmpFore);
		BitBlt (hDCFore, 0, 0, m_rcRndTgt.right, m_rcRndTgt.bottom, hDCWin, m_rcRndTgt.left, m_rcRndTgt.top, SRCCOPY);
	}
	else
	{
		hOldFore = (HBITMAP)SelectObject (hDCFore, m_hBmpFore);
	}
	HDC		hDCDisp = CreateCompatibleDC (hDCWin);
	HBITMAP hBmpDisp = yyBmpCreate (hDCDisp, m_rcRndTgt.right - m_rcRndTgt.left, m_rcRndTgt.bottom - m_rcRndTgt.top, NULL, 0);
	HBITMAP hOldDisp = (HBITMAP)SelectObject (hDCDisp, hBmpDisp);

	RECT	rcDisp;
	for (int i = 1; i <= nSteps; i++)
	{
		nStart = yyGetSysTime ();
		BitBlt (hDCDisp, 0, 0, m_rcRndTgt.right, m_rcRndTgt.bottom, hDCBack, m_rcBmpBack.left, m_rcBmpBack.top, SRCCOPY);
		rcDisp.left	= m_rcRndTgt.left + (m_rcRndSrc.left -  m_rcRndTgt.left) * i / nSteps;
		rcDisp.top	= m_rcRndTgt.top + (m_rcRndSrc.top -  m_rcRndTgt.top) * i / nSteps;
		rcDisp.right = m_rcRndTgt.right - (m_rcRndTgt.right - m_rcRndSrc.right) * i / nSteps;
		rcDisp.bottom = m_rcRndTgt.bottom - (m_rcRndTgt.bottom - m_rcRndSrc.bottom) * i / nSteps;
		if (rcDisp.left > m_rcRndSrc.left)
			rcDisp.left = m_rcRndSrc.left;
		if (rcDisp.top > m_rcRndSrc.top)
			rcDisp.top = m_rcRndSrc.top;
		if (rcDisp.right < m_rcRndSrc.right)
			rcDisp.right = m_rcRndSrc.right;
		if (rcDisp.bottom < m_rcRndSrc.bottom)
			rcDisp.bottom = m_rcRndSrc.bottom;
		StretchBlt (hDCDisp, rcDisp.left, rcDisp.top, rcDisp.right - rcDisp.left, rcDisp.bottom - rcDisp.top,
					hDCFore, m_rcBmpFore.left, m_rcBmpFore.top, m_rcBmpFore.right - m_rcBmpFore.left, m_rcBmpFore.bottom - m_rcBmpFore.top, SRCCOPY);
		BitBlt (hDCWin, m_rcRndTgt.left, m_rcRndTgt.top, m_rcRndTgt.right, m_rcRndTgt.bottom, hDCDisp, 0, 0, SRCCOPY);
		int nUsedTime = yyGetSysTime () - nStart;
		if (nUsedTime < nSleep)
			Sleep (nSleep - nUsedTime);
	}
	SelectObject (hDCDisp, hOldDisp);
	DeleteObject (hBmpDisp);
	DeleteDC (hDCDisp);
	if (hBmpFore != NULL)
	{
		SelectObject (hDCFore, hOldFore);
		DeleteObject (hBmpFore);
	}
	BitBlt (hDCWin, m_rcRndTgt.left, m_rcRndTgt.top, m_rcRndTgt.right, m_rcRndTgt.bottom, 
			hDCBack, m_rcBmpBack.left, m_rcBmpBack.top, SRCCOPY);
	SelectObject (hDCBack, hOldBack);
	DeleteDC (hDCBack);
	SelectObject (hDCFore, hOldFore);
	DeleteDC (hDCFore);
	ReleaseDC (m_hWnd, hDCWin);

	return true;
}

bool CAnimation::DoorOpen (void)
{
	if (m_hBmpFore == NULL)
		return false;

	int		nSteps = 12;
	int		nSleep = 20;
	int		nStart = 0;
	HDC		hDCWin = GetDC (m_hWnd);
	HDC		hDCFore = CreateCompatibleDC (hDCWin);
	HBITMAP hOldFore = (HBITMAP)SelectObject (hDCFore, m_hBmpFore);
	HDC		hDCBack = CreateCompatibleDC (hDCWin);
	HBITMAP	hOldBack = NULL;
	HBITMAP hBmpBack = NULL;
	if (m_hBmpBack != NULL)
	{
		hOldBack = (HBITMAP)SelectObject (hDCBack, m_hBmpBack);
	}
	else
	{
		SetRect (&m_rcBmpBack, 0, 0, m_rcRndTgt.right - m_rcRndTgt.left, m_rcRndTgt.bottom - m_rcRndTgt.top);
		hBmpBack = yyBmpCreate (hDCBack, m_rcBmpBack.right, m_rcBmpBack.bottom, NULL, 0);
		hOldBack = (HBITMAP)SelectObject (hDCBack, hBmpBack);
		BitBlt (hDCBack, 0, 0, m_rcBmpBack.right, m_rcBmpBack.bottom, hDCWin, m_rcRndTgt.left, m_rcRndTgt.top, SRCCOPY);
	}
	HDC		hDCDisp = CreateCompatibleDC (hDCWin);
	HBITMAP hBmpDisp = yyBmpCreate (hDCDisp, m_rcRndTgt.right - m_rcRndTgt.left, m_rcRndTgt.bottom - m_rcRndTgt.top, NULL, 0);
	HBITMAP hOldDisp = (HBITMAP)SelectObject (hDCDisp, hBmpDisp);

	int nW = (m_rcRndTgt.right - m_rcRndTgt.left) / 2;
	for (int i = 1; i <= nSteps; i++)
	{
		nStart = yyGetSysTime ();
		int nBlackSteps = 5;
		if (i >= nSteps - nBlackSteps && m_nTime > 0 && m_pBuffFore != NULL)
		{
			unsigned char * pRGB = m_pBuffFore;
			for (int h = 0; h < m_rcBmpFore.bottom; h++)
			{
				for (int w = 0; w < m_rcBmpFore.right; w++)
				{
					*pRGB = *pRGB++ * (nSteps - i) / nBlackSteps;
					*pRGB = *pRGB++ * (nSteps - i) / nBlackSteps;
					*pRGB = *pRGB++ * (nSteps - i) / nBlackSteps;
					*pRGB++;
				}
			}
		}	
		StretchBlt (hDCDisp, 0, 0, m_rcRndTgt.right - m_rcRndTgt.left, m_rcRndTgt.bottom - m_rcRndTgt.top, 
					hDCFore, m_rcBmpFore.left, m_rcBmpFore.top, m_rcBmpFore.right - m_rcBmpFore.left, m_rcBmpFore.bottom - m_rcBmpFore.top, SRCCOPY);
		StretchBlt (hDCDisp, 0, 0, nW - (nW * i / nSteps), m_rcRndTgt.bottom - m_rcRndTgt.top,
					hDCBack, m_rcBmpBack.left, m_rcBmpBack.top, m_rcBmpBack.right / 2 - m_rcBmpBack.left, m_rcBmpBack.bottom, SRCCOPY);		
		StretchBlt (hDCDisp, nW + (nW * i / nSteps), 0, nW - (nW * i / nSteps), m_rcRndTgt.bottom - m_rcRndTgt.top,
					hDCBack, m_rcBmpBack.left, m_rcBmpBack.top, m_rcBmpBack.right / 2 - m_rcBmpBack.left, m_rcBmpBack.bottom, SRCCOPY);
		BitBlt (hDCWin, m_rcRndTgt.left, m_rcRndTgt.top, m_rcRndTgt.right - m_rcRndTgt.left, m_rcRndTgt.bottom - m_rcRndTgt.top, hDCDisp, 0, 0, SRCCOPY);
		int nUsedTime = yyGetSysTime () - nStart;
		if (nUsedTime < nSleep)
			Sleep (nSleep - nUsedTime);
	}

	SelectObject (hDCDisp, hOldDisp);
	DeleteObject (hBmpDisp);
	DeleteDC (hDCDisp);
	SelectObject (hDCFore, hOldFore);
	DeleteDC (hDCFore);
	if (hBmpBack != NULL)
	{
		SelectObject (hDCBack, hOldBack);
		DeleteObject (hBmpBack);
	}
	DeleteDC (hDCBack);
	ReleaseDC (m_hWnd, hDCWin);

	return true;
}

bool CAnimation::DoorClose (void)
{
	if (m_hBmpFore == NULL)
		return false;

	int		nSteps = 12;
	int		nSleep = 20;
	int		nStart = 0;
	HDC		hDCWin = GetDC (m_hWnd);
	HDC		hDCFore = CreateCompatibleDC (hDCWin);
	HBITMAP hOldFore = (HBITMAP)SelectObject (hDCFore, m_hBmpFore);
	HDC		hDCBack = CreateCompatibleDC (hDCWin);
	HBITMAP	hOldBack = NULL;
	HBITMAP hBmpBack = NULL;
	if (m_hBmpBack != NULL)
	{
		hOldBack = (HBITMAP)SelectObject (hDCBack, m_hBmpBack);
	}
	else
	{
		SetRect (&m_rcBmpBack, 0, 0, m_rcRndTgt.right - m_rcRndTgt.left, m_rcRndTgt.bottom - m_rcRndTgt.top);
		hBmpBack = yyBmpCreate (hDCBack, m_rcBmpBack.right, m_rcBmpBack.bottom, NULL, 0);
		hOldBack = (HBITMAP)SelectObject (hDCBack, hBmpBack);
		BitBlt (hDCBack, 0, 0, m_rcBmpBack.right, m_rcBmpBack.bottom, hDCWin, m_rcRndTgt.left, m_rcRndTgt.top, SRCCOPY);
	}
	HDC		hDCDisp = CreateCompatibleDC (hDCWin);
	HBITMAP hBmpDisp = yyBmpCreate (hDCDisp, m_rcRndTgt.right - m_rcRndTgt.left, m_rcRndTgt.bottom - m_rcRndTgt.top, NULL, 0);
	HBITMAP hOldDisp = (HBITMAP)SelectObject (hDCDisp, hBmpDisp);

	int nW = (m_rcRndTgt.right - m_rcRndTgt.left) / 2;
	for (int i = 1; i <= nSteps; i++)
	{
		nStart = yyGetSysTime ();
		StretchBlt (hDCDisp, 0, 0, m_rcRndTgt.right - m_rcRndTgt.left, m_rcRndTgt.bottom - m_rcRndTgt.top, 
					hDCFore, m_rcBmpFore.left, m_rcBmpFore.top, m_rcBmpFore.right - m_rcBmpFore.left, m_rcBmpFore.bottom - m_rcBmpFore.top, SRCCOPY);
		StretchBlt (hDCDisp, 0, 0, (nW * i / nSteps), m_rcRndTgt.bottom - m_rcRndTgt.top,
					hDCBack, m_rcBmpBack.left, m_rcBmpBack.top, m_rcBmpBack.right / 2 - m_rcBmpBack.left, m_rcBmpBack.bottom - m_rcBmpBack.top, SRCCOPY);	
		StretchBlt (hDCDisp, nW * 2 - (nW * i / nSteps), 0, (nW * i / nSteps), m_rcRndTgt.bottom - m_rcRndTgt.top,
					hDCBack, m_rcBmpBack.left, m_rcBmpBack.top, m_rcBmpBack.right / 2 - m_rcBmpBack.left, m_rcBmpBack.bottom - m_rcBmpBack.top, SRCCOPY);
		BitBlt (hDCWin, m_rcRndTgt.left, m_rcRndTgt.top, m_rcRndTgt.right - m_rcRndTgt.left, m_rcRndTgt.bottom - m_rcRndTgt.top, hDCDisp, 0, 0, SRCCOPY);
		int nUsedTime = yyGetSysTime () - nStart;
		if (nUsedTime < nSleep)
			Sleep (nSleep - nUsedTime);
	}

	SelectObject (hDCDisp, hOldDisp);
	DeleteObject (hBmpDisp);
	DeleteDC (hDCDisp);
	SelectObject (hDCFore, hOldFore);
	DeleteDC (hDCFore);
	if (hBmpBack != NULL)
	{
		SelectObject (hDCBack, hOldBack);
		DeleteObject (hBmpBack);
	}
	DeleteDC (hDCBack);
	ReleaseDC (m_hWnd, hDCWin);

	return true;
}

bool CAnimation::Transition (void)
{
	if (m_hBmpBack == NULL)
		return ShowNone ();
	if (m_rcBmpBack.right - m_rcBmpBack.left != m_rcBmpFore.right - m_rcBmpFore.left || m_rcBmpBack.bottom - m_rcBmpBack.top != m_rcBmpFore.bottom - m_rcBmpFore.top)
		return ShowNone ();
	if (m_pBuffFore == NULL || m_pBuffBack == NULL)
		return ShowNone ();

	int		nSteps = 12;
	int		nSleep = 20;
	int		nStart = 0;
	int nW = m_rcBmpBack.right - m_rcBmpBack.left;
	int nH = m_rcBmpBack.bottom - m_rcBmpBack.top;
	HDC		hDCWin = GetDC (m_hWnd);
	HDC		hDCDisp = CreateCompatibleDC (hDCWin);
	LPBYTE	pBufDisp = NULL;
	HBITMAP hBmpDisp = yyBmpCreate (hDCDisp, nW, nH, &pBufDisp, 0);
	HBITMAP hOldDisp = (HBITMAP)SelectObject (hDCDisp, hBmpDisp);

	LPBYTE pDisp = pBufDisp;
	LPBYTE pFore = m_pBuffFore;
	LPBYTE pBack = m_pBuffBack;

	for (int i = 1; i <= nSteps; i++)
	{
		nStart = yyGetSysTime ();
		pDisp = pBufDisp;
		pFore = m_pBuffFore;
		pBack = m_pBuffBack;
		for (int h = 0; h < nH; h++)
		{
			for (int w = 0; w < nW; w++)
			{
				*pDisp++ = *pFore++ * i / nSteps + *pBack++ * (nSteps - i) / nSteps;
				*pDisp++ = *pFore++ * i / nSteps + *pBack++ * (nSteps - i) / nSteps;
				*pDisp++ = *pFore++ * i / nSteps + *pBack++ * (nSteps - i) / nSteps;
				pDisp++; pFore++; pBack++;
			}
		}
		StretchBlt (hDCWin, m_rcRndTgt.left, m_rcRndTgt.top, m_rcRndTgt.right, m_rcRndTgt.bottom,
					hDCDisp, 0, 0, nW, nH, SRCCOPY);
		int nUsedTime = yyGetSysTime () - nStart;
		if (nUsedTime < nSleep)
			Sleep (nSleep - nUsedTime);
	}
	SelectObject (hDCDisp, hOldDisp);
	DeleteObject (hBmpDisp);
	DeleteDC (hDCDisp);
	ReleaseDC (m_hWnd, hDCWin);

	return true;
}

bool CAnimation::ShowNone (void)
{
	if (m_hBmpFore == NULL)
		return false;

	HDC		hDCWin = GetDC (m_hWnd);
	HDC		hDCFore = CreateCompatibleDC (hDCWin);
	HBITMAP hOldFore = (HBITMAP)SelectObject (hDCFore, m_hBmpFore);
	BitBlt (hDCWin, m_rcRndTgt.left, m_rcRndTgt.top, m_rcRndTgt.right, m_rcRndTgt.bottom, hDCFore, m_rcBmpFore.left, m_rcBmpFore.top, SRCCOPY);
	SelectObject (hDCFore, hOldFore);
	DeleteDC (hDCFore);
	ReleaseDC (m_hWnd, hDCWin);

	return true;
}
