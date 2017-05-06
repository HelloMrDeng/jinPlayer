/*******************************************************************************
	File:		CPlayCtrl.cpp

	Contains:	The play control implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-09		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CPlayCtrl.h"

#include "USystemFunc.h"
#include "RPlayerDef.h"
#include "yyLog.h"

#pragma warning (disable : 4996)

CPlayCtrl::CPlayCtrl(HINSTANCE hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hWnd (NULL)
{
	SetObjectName ("CPlayCtrl");

	strcpy (m_szName, "CtrlBase");
	strcpy (m_szType, "CtrlBase");

	m_nID = -1;
	SetRectEmpty (&m_rcFile);
	SetRectEmpty (&m_rcItem);
	SetRectEmpty (&m_rcDraw);
	m_nBmpNum = 1;
	m_pBmpFile = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_bShow = true;
	m_bEnable = true;
	m_bUpdate = true;
	m_bBtnDown = false;
	m_bMusOver = false;
}	

CPlayCtrl::~CPlayCtrl(void)
{
	YY_DEL_P (m_pBmpFile);
}

bool CPlayCtrl::Create (HWND hWnd, CBaseConfig * pCfg, char * pItemName)
{
	TCHAR szPath[1024];
	_tcscpy (szPath, pCfg->GetFileName ());
	TCHAR * pPos = _tcsrchr (szPath, _T('\\'));
	if (pPos != NULL)
		*(pPos + 1) = 0;

	m_hWnd = hWnd;
	strcpy (m_szName, pItemName);
	strcpy (m_szType, pCfg->GetItemText (pItemName, "Type", "Base"));
	m_nID = pCfg->GetItemValue (pItemName, "ID", m_nID);
	m_bShow = pCfg->GetItemValue (pItemName, "Show", m_bShow) > 0 ? true : false;
	pCfg->GetItemRect (pItemName, &m_rcFile);
	m_nBmpNum = pCfg->GetItemValue (pItemName, "BmpNum", m_nBmpNum);
	char * pBmpFile = pCfg->GetItemText (pItemName, "BmpFile", "NoFile");
	if (strcmp (pBmpFile, "NoFile"))
		m_pBmpFile = CreateBmpFile (szPath, pBmpFile, m_nBmpNum);
	m_nWidth = pCfg->GetItemValue (pItemName, "Width", m_nWidth);
	m_nHeight = pCfg->GetItemValue (pItemName, "Height", m_nHeight);

	UpdateRect ();

	return true;
}

bool CPlayCtrl::OnDraw (HDC hDC, HBITMAP hBmp, LPBYTE pBuff, RECT * pRect)
{
	return false;
}

bool CPlayCtrl::Show (bool bShow)
{
	if (m_bShow == bShow)
		return true;
	m_bShow = bShow;
	UpdateView (&m_rcDraw, FALSE);
	return true;
}

bool CPlayCtrl::Enable (bool bEnable)
{
	if (m_bEnable == bEnable)
		return true;
	m_bEnable = bEnable;
	UpdateView (&m_rcDraw, FALSE);
	return true;
}

bool CPlayCtrl::NeedUpdate (bool bUpdate)
{
	m_bUpdate = bUpdate;
	UpdateView (&m_rcDraw, FALSE);
	return true;
}

LRESULT	CPlayCtrl::MsgProc (HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ((!m_bShow || !m_bEnable) && uMsg != WM_SIZE)
		return S_FALSE;
	switch (uMsg)
	{
	case WM_COMMAND:
		return OnCommand (uMsg, wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown (uMsg, wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp (uMsg, wParam, lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove (uMsg, wParam, lParam);
	case WM_SIZE:
		return OnSize (uMsg, wParam, lParam);
	case WM_TIMER:
		return OnTimer (uMsg, wParam, lParam);
	default:
		break;
	}
	return S_FALSE;
}

LRESULT	CPlayCtrl::OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_FALSE;
}

LRESULT	CPlayCtrl::OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!InRect (LOWORD (lParam), HIWORD (lParam)))
		return S_FALSE;
	m_bBtnDown = true;
	UpdateView (&m_rcDraw, FALSE);
	return S_OK;
}

LRESULT	CPlayCtrl::OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bBtnDown)
	{
		m_bBtnDown = false;
		UpdateView (&m_rcDraw, FALSE);
		if (InRect (LOWORD (lParam), HIWORD (lParam)))
		{
			SendMessage (m_hWnd, WM_COMMAND, m_nID, 0);
			return S_OK;
		}
	}
	return S_FALSE;
}

LRESULT	CPlayCtrl::OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nX = LOWORD (lParam);
	int nY = HIWORD (lParam);
	if (InRect (nX, nY))
	{
		if (!m_bMusOver)
		{
			m_bMusOver = true;
			UpdateView (&m_rcDraw, FALSE);
		}
		return S_OK;
	}
	else
	{
		if (m_bMusOver)
		{
			m_bMusOver = false;
			UpdateView (&m_rcDraw, FALSE);
		}
	}
	if (wParam == MK_LBUTTON)
	{
		if (m_bBtnDown)
		{
			if (!InRect (nX, nY))
			{
				m_bBtnDown = false;
				UpdateView (&m_rcDraw, FALSE);
			}
			else
			{
				return S_OK;
			}
		}
	}
	return S_FALSE;
}

LRESULT	CPlayCtrl::OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_FALSE;
}

LRESULT	CPlayCtrl::OnSize (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UpdateRect ();
	return S_FALSE;
}

bool CPlayCtrl::UpdateView (RECT * pRect, BOOL bEraseBG)
{
	if (!m_bUpdate)
		return true;
	InvalidateRect (m_hWnd, pRect, bEraseBG);
	return true;
}

bool CPlayCtrl::UpdateRect (void)
{
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);

	int nLeft = m_rcFile.left;
	int nTop = m_rcFile.top;
	if (IsCenter (nLeft))
	{
		if (nLeft == YYPOS_CENTER)
			nLeft = (rcView.right + rcView.left) / 2 - m_nWidth / 2;
		else
			nLeft = (rcView.right + rcView.left) / 2 + (nLeft - YYPOS_CENTER);
	}
	else if (IsRightB (nLeft))
	{
		nLeft = rcView.right - (YYPOS_RIGHTB - nLeft);
	}
	if (IsCenter (nTop))
	{
		if (nTop == YYPOS_CENTER)
			nTop = (rcView.bottom + rcView.top) / 2 - m_nHeight / 2;
		else
			nTop = (rcView.bottom + rcView.top) / 2 + (nTop - YYPOS_CENTER);
	}
	else if (IsRightB (nTop))
	{
		nTop = rcView.bottom - (YYPOS_RIGHTB - nTop);
	}

	if (m_nWidth < 0)
		m_rcItem.right = rcView.right - nLeft;
	else
		m_rcItem.right = nLeft + m_nWidth;
	if (m_nHeight < 0)
		m_rcItem.bottom = rcView.bottom - nTop;
	else
		m_rcItem.bottom = nTop + m_nHeight;

	SetRect (&m_rcItem, nLeft, nTop, m_rcItem.right, m_rcItem.bottom);
	SetRect (&m_rcDraw, nLeft, nTop, m_rcItem.right, m_rcItem.bottom);

	if (rcView.bottom < m_rcItem.bottom)
		m_rcItem.bottom = rcView.bottom;
	if (rcView.right < m_rcItem.right)
		m_rcItem.right = rcView.right;

	return true;
}

bool CPlayCtrl::InRect (int nX, int nY)
{
	POINT pt;
	pt.x = nX;
	pt.y = nY;

	if (PtInRect (&m_rcItem, pt))
		return true;
	else
		return false;
}

bool CPlayCtrl::IsCenter (int nValue)
{
	if (nValue > 10000 && nValue < 30000)
		return true;
	else
		return false;
}

bool CPlayCtrl::IsRightB (int nValue)
{
	if (nValue > 40000 && nValue < 60000)
		return true;
	else
		return false;
}

CBmpFile * CPlayCtrl::CreateBmpFile (TCHAR * pPath, char * pFile, int nNum)
{
	CBmpFile *	pBmpFile = NULL;
	TCHAR		szFile[1024];
	_tcscpy (szFile, pPath);
	MultiByteToWideChar (CP_ACP, 0, pFile, -1, szFile + _tcslen (szFile), sizeof (szFile) - _tcslen (pPath) * sizeof (TCHAR));
	pBmpFile = new CBmpFile ();

	HDC hDC = GetDC (m_hWnd);
	int nRC = pBmpFile->ReadBmpFile (hDC, szFile, nNum);
	ReleaseDC (m_hWnd, hDC);
	if (nRC != YY_ERR_NONE)
	{
		YY_DEL_P (pBmpFile);
		return NULL;
	}

	if (m_rcFile.right <= m_rcFile.left)
		m_rcFile.right = m_rcFile.left + pBmpFile->GetWidth ();
	if (m_rcFile.bottom <= m_rcFile.top)
		m_rcFile.bottom = m_rcFile.top + pBmpFile->GetHeight ();
	m_nWidth = m_rcFile.right - m_rcFile.left;
	m_nHeight = m_rcFile.bottom - m_rcFile.top;

	return pBmpFile;
}

