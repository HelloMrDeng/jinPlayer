/*******************************************************************************
	File:		CWndView.cpp

	Contains:	Window view implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CWndView.h"
#include "resource.h"

#include "CLicenseCheck.h"
#include "CMediaEngine.h"
#include "CWndPanel.h"

#define WM_TIMER_CLICK	1001

#pragma warning (disable : 4996)

typedef int (* yyVerifyLicenseText) (void * pUserData, char * pText, int nSize);
int CheckExtLicense (char * pText, int nSize, yyVerifyLicenseText fVerify, void * pUserData)
{
	if (pText == NULL || nSize <= 0)
		return -2;

	char	szExtLicenseKey[32];
	char *	pExtLicenseText = new char[nSize+1];

	strcpy (pExtLicenseText, pText);
	strcpy (szExtLicenseKey, "yyMediaEngine");
	int nKeyLen = strlen (szExtLicenseKey);

	for (int i = 0; i < nSize; i++)
	{
		for (int j = 0; j < nKeyLen; j++)
		{
			pExtLicenseText[i] = pExtLicenseText[i] ^ szExtLicenseKey[j];
		}
	}

	if (fVerify != NULL)
		fVerify (pUserData, pExtLicenseText, nSize);

	delete []pExtLicenseText;

	return 0;
}

CWndView::CWndView(HINSTANCE hInst)
	: CWndBase (hInst)
	, m_nScreenX (800)
	, m_nScreenY (480)
	, m_hMemDC (NULL)
	, m_hBmpThumb (NULL)
	, m_pMedia (NULL)
	, m_pWndPanel (NULL)
	, m_nVideoRndType (YY_VRND_DDRAW)
	, m_nClickCount (0)
	, m_nClickTimer (0)
{
	m_ptView.x = 0;
	m_ptView.y = 0;
}

CWndView::~CWndView(void)
{
	if (m_hMemDC != NULL)
		DeleteDC (m_hMemDC);
	m_hMemDC = NULL;
}

bool CWndView::CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG)
{
	if (!CWndBase::CreateWnd (hParent, rcView, clrBG))
		return false;

	m_nScreenX = GetSystemMetrics (SM_CXSCREEN);
	m_nScreenY = GetSystemMetrics (SM_CYSCREEN);

	m_ptView.x = m_rcWnd.left;
	m_ptView.y = m_rcWnd.top;

	SetWindowText (m_hWnd, _T("yyDemoPlayerVideoWindow"));
//	SetWindowText (m_hWnd, _T("MP4_OSD"));

	return true;
}

void CWndView::SetMediaEngine (CMediaEngine * pMedia) 
{
	m_pMedia = pMedia;
	if (m_pMedia != NULL)
		m_pMedia->SetView (m_hWnd, m_nVideoRndType);
}

void CWndView::SetFullScreen (void)
{
	if (m_hWnd == NULL)
		return;

	if (!IsFullScreen ())
	{
		GetClientRect (m_hWnd, &m_rcWnd);
		SetParent (m_hWnd, NULL);
		SetWindowPos (m_hWnd, HWND_TOP, 0, 0, m_nScreenX, m_nScreenY, 0);
//		SetWindowPos (m_hWnd, HWND_TOPMOST, 0, 0, m_nScreenX, m_nScreenY, 0);
	}
	else
	{
		SetParent (m_hWnd, m_hParent);
		SetWindowPos (m_hWnd, HWND_BOTTOM, m_ptView.x, m_ptView.y, m_rcWnd.right - m_rcWnd.left, m_rcWnd.bottom - m_rcWnd.top, 0);
	}

//	if (m_pMedia != NULL)
//		m_pMedia->SetView (m_hWnd, m_nVideoRndType);
}

bool CWndView::IsFullScreen (void)
{
	if (m_hWnd == NULL)
		return false;

	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	if (rcView.right == m_nScreenX && rcView.bottom == m_nScreenY)
		return true;
	else
		return false;
}

LRESULT CWndView::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int		wmId, wmEvent;

	switch (uMsg)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDC_BTN_PREV:
		case IDC_BTN_NEXT:
			SendMessage (m_hParent, WM_COMMAND, wParam, lParam);
			break;

		case IDC_BTN_LIST:
		case IDC_BTN_OPEN:
			SendMessage (m_hParent, WM_COMMAND, wParam, lParam);
			break;
		
		default:
			break;
		}

	case WM_YYRR_CHKLCS:
	case WM_YYYF_CHKLCS:
	case WM_YYSM_CHKLCS:
		return (LRESULT)CheckExtLicense;

	case WM_LBUTTONDOWN:
		m_nClickCount++;
		if (m_nClickTimer == 0)
			m_nClickTimer = SetTimer (hwnd, WM_TIMER_CLICK, 250, NULL);
		break;

	case WM_TIMER:
		KillTimer (hwnd, WM_TIMER_CLICK);
		m_nClickTimer = 0;
		if (m_nClickCount == 1 && m_pMedia != NULL)
		{
			if (m_pMedia->GetStatus () == YY_PLAY_Run)
				m_pMedia->Pause ();
			else if (m_pMedia->GetStatus () == YY_PLAY_Pause)
				m_pMedia->Run ();
		}
		else
			SetFullScreen ();
		m_nClickCount = 0;
		break;

	case WM_KEYUP:
		if (wParam == 27 && IsFullScreen ()) // ESC
			SetFullScreen ();
		break;

	case WM_SIZE:
		if (m_pWndPanel != NULL && m_pWndPanel->GetWnd () != NULL)
		{
			int nWidth =  LOWORD (lParam);
			int nHeight = HIWORD (lParam);
			RECT rcPanel;
			GetClientRect (m_pWndPanel->GetWnd (), &rcPanel);
			int nLeft = (nWidth - rcPanel.right) / 2;
			int nTop = nHeight - rcPanel.bottom - 40;
			SetWindowPos (m_pWndPanel->GetWnd (), NULL, nLeft, nTop, 0, 0, SWP_NOSIZE);	
		}
		break;

	case WM_ERASEBKGND:
		if (m_hBmpThumb != NULL)
			return S_OK;
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_PAINT:
	{
		RECT rcView;
		GetClientRect (m_hWnd, &rcView);
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		if (m_hBmpThumb != NULL)
		{
			BITMAP bmpInfo;
			GetObject (m_hBmpThumb, sizeof (BITMAP), &bmpInfo);
			if (m_hMemDC == NULL)
				m_hMemDC = CreateCompatibleDC (hdc);
			HBITMAP hBmpOld = (HBITMAP) SelectObject (m_hMemDC, m_hBmpThumb);
			StretchBlt (hdc, 0, 0, rcView.right, rcView.bottom,
						m_hMemDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);
			SelectObject (m_hMemDC, hBmpOld);
			EndPaint(hwnd, &ps);
			return S_OK;
		}
		EndPaint(hwnd, &ps);

		if (m_pMedia != NULL)
			m_pMedia->UpdateView (NULL);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	default:
		break;
	}

	return	CWndBase::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

