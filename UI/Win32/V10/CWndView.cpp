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
	, m_hBmpLogo (NULL)
	, m_hLogoDC (NULL)
	, m_bShowLogo (true)
	, m_hBmpOverlay (NULL)
	, m_hDCOverlay (NULL)
	, m_bShowOverlay (false)
	, m_hBmpThumb (NULL)
	, m_nClickCount (0)
	, m_nClickTimer (0)
{
	m_ptView.x = 0;
	m_ptView.y = 0;
	
	_tcscpy (m_szInfo, _T("本播放器由上海参烁信息技术有限公司提供技术支持。\r\n\r\n"));
	_tcscat (m_szInfo, _T("Technical support is provided by CanSure Co, in Shanghai.\r\n\r\n"));
	_tcscat (m_szInfo, _T("技术支持热线 (TechnicalSupport Hotline) +86 21 34727188 \r\n\r\n"));
	_tcscat (m_szInfo, _T("邮件联系(E-Mail) cansure69@cansure.cn \r\n"));
}

CWndView::~CWndView(void)
{
	if (m_hBmpLogo != NULL)
		DeleteObject (m_hBmpLogo);
	m_hBmpLogo = NULL;

	if (m_hLogoDC != NULL)
		DeleteDC (m_hLogoDC);
	m_hLogoDC = NULL;

	if (m_hBmpOverlay != NULL)
		DeleteObject (m_hBmpOverlay);
	m_hBmpOverlay = NULL;

	if (m_hDCOverlay != NULL)
		DeleteDC (m_hDCOverlay);
	m_hDCOverlay = NULL;
}

bool CWndView::CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG)
{
	if (!CWndBase::CreateWnd (hParent, rcView, clrBG))
		return false;

#ifndef WINCE
	m_nScreenX = GetSystemMetrics (SM_CXSCREEN);
	m_nScreenY = GetSystemMetrics (SM_CYSCREEN);
#else
	RECT rcWnd;
	GetClientRect (m_hParent, &rcWnd);
	m_nScreenX = rcWnd.right;
	if (m_nScreenX == 1280)
		m_nScreenY = 800;
	else if (m_nScreenX == 1024)
		m_nScreenY = 768;
	else if (m_nScreenX == 960)
		m_nScreenY = 480;
	else if (m_nScreenX == 640)
		m_nScreenY = 480;
	else if (m_nScreenX == 800)
		m_nScreenY = 480;
	else if (m_nScreenX == 640)
		m_nScreenY = 480;
	else if (m_nScreenX == 480)
		m_nScreenY = 320;
	else if (m_nScreenX == 320)
		m_nScreenY = 240;
#endif //WINCE

//	m_hBmpLogo = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_SHOW));
	if (m_hBmpLogo != NULL)
	{
		BITMAP bmpInfo;
		GetObject (m_hBmpLogo, sizeof (BITMAP), &bmpInfo);
		m_nLogoW = bmpInfo.bmWidth;
		m_nLogoH = bmpInfo.bmHeight;
	}

	m_ptView.x = m_rcWnd.left;
	m_ptView.y = m_rcWnd.top;

	SetWindowText (m_hWnd, _T("yyDemoPlayerVideoWindow"));
//	SetWindowText (m_hWnd, _T("MP4_OSD"));

	return true;
}

void CWndView::SetFullScreen (void)
{
	if (m_hWnd == NULL)
		return;

	if (!IsFullScreen ())
	{
		GetClientRect (m_hWnd, &m_rcWnd);
		SetParent (m_hWnd, NULL);
#ifdef _OS_WINCE
		SetWindowPos (m_hWnd, HWND_TOPMOST, 0, 0, m_nScreenX, m_nScreenY, 0);
#else
		SetWindowPos (m_hWnd, HWND_TOP, 0, 0, m_nScreenX, m_nScreenY, 0);
#endif // _OS_WINCE
		PostMessage (m_hParent, WM_VIEW_FullScreen, 1, 0);
	}
	else
	{
		SetParent (m_hWnd, m_hParent);
		SetWindowPos (m_hWnd, HWND_BOTTOM, m_ptView.x, m_ptView.y, m_rcWnd.right - m_rcWnd.left, m_rcWnd.bottom - m_rcWnd.top, 0);
		PostMessage (m_hParent, WM_VIEW_FullScreen, 0, 0);
	}
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

void CWndView::ShowLogo (bool bShow) 
{
	if (m_bShowLogo == bShow)
		return;

	m_bShowLogo = bShow; 
	InvalidateRgn (m_hWnd, NULL, TRUE);
}

void CWndView::ShowOverlay (bool bShow) 
{
	if (m_bShowOverlay == bShow)
		return;

	m_bShowOverlay = bShow; 
//	if (m_bShowOverlay && m_hBmpOverlay == NULL)
//		m_hBmpOverlay = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_OVERLAY));

	InvalidateRgn (m_hWnd, NULL, TRUE);
}

void CWndView::SetThumbnail (HBITMAP hBmbThumb)
{
	m_hBmpThumb = hBmbThumb;
}

LRESULT CWndView::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_YYRR_CHKLCS:
	case WM_YYYF_CHKLCS:
	case WM_YYSM_CHKLCS:
	case WM_YYHS_CHKLCS:
		return (LRESULT)CheckExtLicense;

	case WM_LBUTTONDOWN:
//		SendMessage (m_hParent, WM_VIEW_EVENT, 1, 0);
		m_nClickCount++;
		if (m_nClickTimer == 0)
			m_nClickTimer = SetTimer (hwnd, WM_TIMER_CLICK, 250, NULL);
		break;

	case WM_RBUTTONDOWN:
		SendMessage (m_hParent, WM_VIEW_EVENT, 2, 0);
		break;

	case WM_LBUTTONDBLCLK:
		SetFullScreen ();
		break;

	case WM_TIMER:
		KillTimer (hwnd, WM_TIMER_CLICK);
		m_nClickTimer = 0;
		if (m_nClickCount == 1)
			SendMessage (m_hParent, WM_VIEW_EVENT, 1, 0);
		else
			SendMessage (m_hParent, WM_VIEW_EVENT, 2, 0);
		m_nClickCount = 0;
		break;

	case WM_KEYUP:
		if (wParam == 27 && IsFullScreen ()) // ESC
			SendMessage (m_hParent, WM_VIEW_EVENT, 2, 0);
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
			if (m_hLogoDC == NULL)
				m_hLogoDC = CreateCompatibleDC (hdc);
			HBITMAP hBmpOld = (HBITMAP) SelectObject (m_hLogoDC, m_hBmpThumb);
			StretchBlt (hdc, 0, 0, rcView.right, rcView.bottom,
						m_hLogoDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);
			SelectObject (m_hLogoDC, hBmpOld);
			EndPaint(hwnd, &ps);
			return S_OK;
		}
#ifdef _OS_WINCE
		if (m_bShowLogo)
		{
			if (m_hBmpLogo != NULL)
			{
				if (m_hLogoDC == NULL)
				{
					m_hLogoDC = CreateCompatibleDC (hdc);
					SelectObject (m_hLogoDC, m_hBmpLogo);
				}
			}
			if (m_hLogoDC != NULL)
			{
				SetTextColor (hdc, RGB(255, 255, 255));
				::SetBkMode (hdc, TRANSPARENT);
				rcView.top = rcView.bottom / 2 - 80;
				DrawText (hdc, m_szInfo, _tcslen (m_szInfo), &rcView, DT_CENTER);

				int nHeight = rcView.right * m_nLogoH / m_nLogoW;
				BitBlt (hdc, (rcView.right- m_nLogoW) / 2, rcView.bottom - 155, m_nLogoW, m_nLogoH,
							m_hLogoDC, 0, 0, SRCCOPY);
			}
		}
#endif // WINCE
		if (m_bShowOverlay)
		{
			GetClientRect (m_hWnd, &rcView);
			if (m_hBmpOverlay != NULL)
			{
				if (m_hDCOverlay == NULL)
				{
					m_hDCOverlay = CreateCompatibleDC (hdc);
					SelectObject (m_hDCOverlay, m_hBmpOverlay);
				}
			}
			if (m_hDCOverlay != NULL)
			{
				BITMAP bmpInfo;
				GetObject (m_hBmpOverlay, sizeof (BITMAP), &bmpInfo);

				StretchBlt (hdc, rcView.left, rcView.top, rcView.right, rcView.bottom,
							m_hDCOverlay, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);
			}
		}
		EndPaint(hwnd, &ps);

		SendMessage (m_hParent, WM_VIEW_OnPaint, 0, 0);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	default:
		break;
	}

	return	CWndBase::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

