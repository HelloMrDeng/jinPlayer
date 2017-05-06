/*******************************************************************************
	File:		CListNewFolder.cpp

	Contains:	The list view handle new folder implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CListNewFolder.h"
#include "CLangText.h"
#include "resource.h"
#include "USystemFunc.h"
#include "UStringFunc.h"
#include "RPlayerDef.h"
#include "yyLog.h"

#pragma warning (disable : 4996)

CListNewFolder::CListNewFolder(HINSTANCE hInst)
	: CListView (hInst)
	, m_bNewFolder (false)
	, m_hBrhEdit (NULL)
	, m_nTimerCount (0)
	, m_nTimerEdit (0)
{
	SetObjectName ("CListNewFolder");
	memset (m_szNewFolder, 0, sizeof (m_szNewFolder));
	SetRectEmpty (&m_rcEdit);
}	

CListNewFolder::~CListNewFolder(void)
{
	if (m_hBrhEdit != NULL)
		DeleteObject (m_hBrhEdit);
}

bool CListNewFolder::Create (HWND hWnd, CListRes * pRes)
{
	CListView::Create (hWnd, pRes);
	m_hBrhEdit = CreateSolidBrush (RGB (255, 255, 255));
	return true;
}

bool CListNewFolder::OnNewItemFolder (void)
{
	ShowEditBox (true);
	return true;
}

bool CListNewFolder::OnSelItemChanged (void)
{
	if (m_pSelItem == NULL)
		return true;
	if (m_bNewFolder && m_pSelItem->m_nType != ITEM_NewFolder)
		ShowEditBox (false);
	return true;
}

bool CListNewFolder::UpdateEditRect (void)
{
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	int nItemWidth = rcView.right / m_nBmpCols;
	CListItem * pItem = NULL;
	NODEPOS pos = m_lstItem.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstItem.GetNext (pos);
		if (pItem->m_nType == ITEM_NewFolder)
			break;
	}
	if (pItem == NULL)
		return false;

	int nX = pItem->m_nIndex % m_nBmpCols * nItemWidth;
	int nY = pItem->m_nIndex / m_nBmpCols * ITEM_HEIGHT + ICON_HEIGHT + ICON_OFF_Y + (ITEM_HEIGHT - ICON_HEIGHT) / 6 - m_nBmpYPos;
	SetRect (&m_rcEdit, nX + 8, nY, nX + nItemWidth - 8, nY + (ITEM_HEIGHT - ICON_HEIGHT) / 2);

	return true;
}

bool CListNewFolder::ShowEditBox (bool bShow)
{
	m_bNewFolder = bShow;
	if (m_bNewFolder)
	{
		UpdateEditRect ();
		if (m_nTimerEdit == 0)
			m_nTimerEdit = SetTimer (m_hWnd, WT_TYPE_NewFolder, 400, NULL);
	}
	else
	{
		if (m_nTimerEdit != NULL)
			KillTimer (m_hWnd, WT_TYPE_NewFolder);
		m_nTimerEdit = 0;
		m_nTimerCount = 0;
	}
	InvalidateRect (m_hWnd, &m_rcEdit, FALSE);
	return true;
}

bool CListNewFolder::CreateNewFolder (void)
{
	return false;
}

LRESULT	CListNewFolder::OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bNewFolder && _tcslen (m_szNewFolder) > 0)
	{
		CreateNewFolder ();
		return S_OK;
	}
	return CListView::OnLButtonUp (uMsg, wParam, lParam);
}

LRESULT	CListNewFolder::OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!m_bNewFolder)
		return S_FALSE;

	UpdateEditRect ();
	if (wParam == VK_ESCAPE)
	{
		memset (m_szNewFolder, 0, sizeof (m_szNewFolder));
		ShowEditBox (false);
	}
	else if (wParam == VK_DELETE || wParam == VK_BACK)
	{
		if (_tcslen (m_szNewFolder) > 0)
		{
			m_szNewFolder[_tcslen (m_szNewFolder) - 1] = 0;
			InvalidateRect (m_hWnd, &m_rcEdit, TRUE);
		}
	}
	else if (wParam == VK_RETURN && _tcslen (m_szNewFolder) > 0)
	{
		CreateNewFolder ();
	}

	return S_FALSE;
}

LRESULT	CListNewFolder::OnChar (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!m_bNewFolder)
		return S_FALSE;

	if (wParam == VK_RETURN || wParam == VK_BACK || wParam == VK_ESCAPE )
		return S_FALSE;

	if (_tcslen (m_szNewFolder) >= sizeof (m_szNewFolder) / sizeof (TCHAR) - 1)
		return S_FALSE;

	UpdateEditRect ();
	m_szNewFolder[_tcslen (m_szNewFolder)] = (TCHAR)wParam;			
	InvalidateRect (m_hWnd, &m_rcEdit, FALSE);
	return S_FALSE;
}

LRESULT CListNewFolder::OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WT_TYPE_NewFolder)
	{
		m_nTimerCount++;
		InvalidateRect (m_hWnd, &m_rcEdit, FALSE);
	}
	return S_FALSE;
}

LRESULT	CListNewFolder::OnPaint (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CListView::OnPaint (uMsg, wParam, lParam);
	if (!m_bNewFolder)
		return S_FALSE;

	HDC	hWinDC = GetDC (m_hWnd);
	UpdateEditRect ();
	FillRect (hWinDC, &m_rcEdit, m_hBrhEdit);
	MoveToEx (hWinDC, m_rcEdit.left, m_rcEdit.top, NULL);
	LineTo (hWinDC, m_rcEdit.right, m_rcEdit.top);
	LineTo (hWinDC, m_rcEdit.right, m_rcEdit.bottom);
	LineTo (hWinDC, m_rcEdit.left, m_rcEdit.bottom);
	LineTo (hWinDC, m_rcEdit.left, m_rcEdit.top);
	SetBkMode (hWinDC, TRANSPARENT);
	DrawText (hWinDC, m_szNewFolder, _tcslen (m_szNewFolder), &m_rcEdit, DT_CENTER);
	if (m_nTimerCount % 2 == 0)
	{
		SIZE	szTxt;
		TCHAR	szCursor[4];
		RECT	rcCursor;
		GetTextExtentPoint (hWinDC, m_szNewFolder, _tcslen (m_szNewFolder), &szTxt);
		int		nLeft = m_rcEdit.left + (m_rcEdit.right - m_rcEdit.left + szTxt.cx) / 2;
		if (szTxt.cx > 0)
			nLeft += 2;
		SetRect (&rcCursor, nLeft, m_rcEdit.top, m_rcEdit.right, m_rcEdit.bottom);
		_tcscpy (szCursor, _T("|"));
		DrawText (hWinDC, szCursor, _tcslen (szCursor), &rcCursor, DT_LEFT);
	}
	ReleaseDC (m_hWnd, hWinDC);

	return S_FALSE;
}

