/*******************************************************************************
	File:		CWndBase.cpp

	Contains:	Window base implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CWndBase.h"

#pragma warning (disable : 4996)

CWndBase::CWndBase(HINSTANCE hInst)
	: m_hInst (hInst)
	, m_hParent (NULL)
	, m_hWnd (NULL)
	, m_dwStyle (0)
	, m_hBKBrush (NULL)
	, m_nClrFont (RGB(0,0,0))
{
	_tcscpy (m_szClassName, _T("yyViewWidnow"));
	_tcscpy (m_szWindowName, _T("yyViewWidnow"));

	memset (m_szText, 0, sizeof (m_szText));
}

CWndBase::~CWndBase(void)
{
}

bool CWndBase::CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG, bool bPopup)
{
	m_hParent = hParent;
	m_rcWnd = rcView;

	if (m_hWnd != NULL)
	{
		SetParent (m_hWnd, hParent);
		return true;
	}
#ifdef _OS_WINCE
	HINSTANCE hInst = NULL;
#else
	HINSTANCE hInst = (HINSTANCE )GetWindowLong (hParent, GWL_HINSTANCE);
#endif //_WIN32_WCE

	WNDCLASS wcex;

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)ViewWindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)::CreateSolidBrush (clrBG);
	wcex.lpszMenuName	= (LPCTSTR)NULL;
	wcex.lpszClassName	= m_szClassName;

	RegisterClass(&wcex);

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if (bPopup)
		dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
	if (m_dwStyle != 0)
		dwStyle = m_dwStyle;

	if (m_hParent != NULL)
		m_hWnd = CreateWindow(m_szClassName, m_szWindowName, dwStyle,
								CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hParent, NULL, hInst, NULL);
	else
		m_hWnd = CreateWindow(m_szClassName, m_szWindowName, WS_VISIBLE | WS_BORDER | WS_POPUP,
								rcView.left, rcView.top, rcView.right, rcView.bottom, hParent, NULL, hInst, NULL);

	if (m_hWnd == NULL)
		return false;

	if (rcView.bottom == 0 || rcView.right == 0)
	{
		if (hParent != NULL)
			GetClientRect (hParent, &rcView);
	}

	POINT ptPos;
	ptPos.x = rcView.left;
	ptPos.y = rcView.top;
	if (bPopup)
		ClientToScreen (hParent, &ptPos);
	SetWindowPos (m_hWnd, NULL, ptPos.x, ptPos.y, rcView.right - rcView.left, rcView.bottom - rcView.top, 0);
	LONG lRC = SetWindowLong (m_hWnd, GWL_USERDATA, (LONG)this);

	m_hBKBrush = CreateSolidBrush (clrBG);

	::ShowWindow (m_hWnd, SW_SHOW);
	return true;
}

void CWndBase::Close (void)
{
	SendMessage (m_hWnd, WM_CLOSE, 0, 0);
}

void CWndBase::SetBGColor (COLORREF nColor)
{
	if (m_hBKBrush != NULL)
		DeleteObject (m_hBKBrush);

	m_hBKBrush = CreateSolidBrush (nColor);
	::InvalidateRect (m_hWnd, NULL, TRUE);
}

void CWndBase::SetText (TCHAR * pText)
{
	if (pText == NULL)
		return;

	_tcscpy (m_szText, pText);
	::InvalidateRect (m_hWnd, NULL, TRUE);
}

bool CWndBase::GetText (TCHAR * pText, int nSize)
{
	if (nSize > 0)
	{
		if (_tcslen (m_szText) > nSize)
			return false;
	}
	_tcscpy (pText, m_szText);
	return true;
}

bool CWndBase::ShowWnd (int nShow)
{
	if (m_hWnd == NULL)
		return false;

	ShowWindow (m_hWnd, nShow);
	return true;
}

bool CWndBase::IsShow (void)
{
	if (IsWindowVisible (m_hWnd))
		return true;
	else
		return false;
}

LRESULT CALLBACK CWndBase::ViewWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWndBase * pViewWindow = (CWndBase *)GetWindowLong (hwnd, GWL_USERDATA);
	if (pViewWindow == NULL)
		return(DefWindowProc(hwnd, uMsg, wParam, lParam));
	else
		return pViewWindow->OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

LRESULT CWndBase::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
		break;

	case WM_PAINT:
		{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		if (_tcslen (m_szText) > 0)
		{
			RECT rcView;
			GetClientRect (m_hWnd, &rcView);
			SetTextColor (hdc, m_nClrFont);
			::SetBkMode (hdc, TRANSPARENT);
			DrawText (hdc, m_szText, _tcslen (m_szText), &rcView, DT_CENTER | DT_VCENTER);
		}

		EndPaint(hwnd, &ps);
		}

		break;

	case WM_ERASEBKGND:
		{
		HDC hdc = (HDC)wParam;
		RECT rcView;
		GetClientRect (m_hWnd, &rcView);
		FillRect (hdc, &rcView, m_hBKBrush);
		return S_OK;
		}
		break;

	case WM_DESTROY:
		DeleteObject (m_hBKBrush);
		UnregisterClass (m_szClassName, NULL);
		break;

	default:
		break;
	}

	return	DefWindowProc(hwnd, uMsg, wParam, lParam);
}


