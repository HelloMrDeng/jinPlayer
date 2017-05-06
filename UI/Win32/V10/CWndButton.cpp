/*******************************************************************************
	File:		CWndButton.cpp

	Contains:	Window slide pos implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CWndButton.h"

#pragma warning (disable : 4996)

CWndButton::CWndButton(HINSTANCE hInst, int nID)
	: CWndBase (hInst)
	, m_nID (nID)
	, m_bDown (false)
	, m_hPenDown (NULL)
	, m_hPenUp (NULL)
/*
	, m_hBrushBG (NULL)
	, m_nThumbPos (0)
*/
{
}

CWndButton::~CWndButton(void)
{
/*
	if (m_hPenBound != NULL)
		DeleteObject (m_hPenBound);
	m_hPenBound = NULL;
	if (m_hBrushBG != NULL)
		DeleteObject (m_hBrushBG);
	m_hBrushBG = NULL;
*/
}

bool CWndButton::Create (HWND hParent, RECT rcBtn)
{
	if (!CWndBase::CreateWnd (hParent, rcBtn, RGB (120, 120, 120)))
		return false;
/*
	m_hPenBound = ::CreatePen (PS_SOLID, 2, RGB (80, 80, 80));
	m_hBrushBG = ::CreateSolidBrush (RGB (120, 120, 120));

	SetRect (&m_rcThumb, 0, 0, 32, rcView.bottom);
	m_hBrushTmb = ::CreateSolidBrush (RGB (200, 200, 200));
*/
	return true;
}

LRESULT CWndButton::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rcWnd;
	if (hwnd != NULL)
		GetClientRect (hwnd, &rcWnd);

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		SetCapture (hwnd);
		m_bDown = true;
		InvalidateRect (m_hWnd, NULL, TRUE);
		return S_OK;

	case WM_LBUTTONUP:
		ReleaseCapture ();
		SendMessage (m_hParent, WM_COMMAND, m_nID, NULL);
		m_bDown = false;
		InvalidateRect (m_hWnd, NULL, TRUE);
		return S_OK;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

//		SelectObject (hdc, m_hBrushTmb);
//		Rectangle (hdc, m_nThumbPos + m_rcThumb.left, m_rcThumb.top, m_nThumbPos + m_rcThumb.right, m_rcThumb.bottom);

/*
		SelectObject (hdc, m_hPenBound);
		SelectObject (hdc, m_hBrushBG);

		FillRect (hdc, &rcWnd, m_hBrushBG);

		SelectObject (hdc, m_hBrushTmb);
		Rectangle (hdc, m_nThumbPos + m_rcThumb.left, m_rcThumb.top, m_nThumbPos + m_rcThumb.right, m_rcThumb.bottom);
*/
		EndPaint(hwnd, &ps);
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	default:
		break;
	}

	return	CWndBase::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

