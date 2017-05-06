/*******************************************************************************
	File:		CWndSubTT.cpp

	Contains:	The control panel implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-13		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"
#include <commctrl.h>
#include <Commdlg.h>

#include "CWndSubTT.h"

#include "CRegMng.h"
#include "resource.h"

#pragma warning (disable : 4996)

CWndSubTT::CWndSubTT(HINSTANCE hInst, HWND hWnd)
	: m_hInst (hInst)
	, m_hParent (hWnd)
	, m_hWnd (NULL)
	, m_bgColor (RGB (10, 10, 10))
	, m_pMedia (NULL)
	, m_nEnable (1)
	, m_hTxtFont (NULL)
{
	_tcscpy (m_szClassName, _T("yySubTTWindow"));
	_tcscpy (m_szWindowName, _T("yySubTTWindow"));
	memset (&m_lfFont, 0, sizeof (m_lfFont));
	memset (&m_cfFont, 0, sizeof (m_cfFont));
	m_nEnable = CRegMng::g_pRegMng->GetIntValue (_T("SubTTEnable"), m_nEnable);
	CreateTextFont ();
}

CWndSubTT::~CWndSubTT(void)
{
	CRegMng::g_pRegMng->SetIntValue (_T("SubTTEnable"), m_nEnable);
	SaveFontParam ();
	if (m_hTxtFont != NULL)
		DeleteObject (m_hTxtFont);
	m_hTxtFont = NULL;

	if (m_hWnd != NULL)
		SendMessage (m_hWnd, WM_CLOSE, 0, 0);
}

void CWndSubTT::SetMediaEngine (CMediaEngine * pMedia)
{
	m_pMedia = pMedia;
	m_pMedia->SetParam (YYPLAY_PID_SubTitle, (void *)m_nEnable);
}

LRESULT	CWndSubTT::OnCommand (int nID)
{
	if (nID == ID_SUBTITLE_ENABLE || nID == ID_SUBTITLE_DISABLE)
	{
		if (nID == ID_SUBTITLE_ENABLE)
			m_nEnable = 1;
		else
			m_nEnable = 0;
		m_pMedia->SetParam (YYPLAY_PID_SubTitle, (void *)m_nEnable);
		UpdateMenuItem ();
	}
	else if (nID == ID_SUBTITLE_FONT)
	{
		if (ChooseFont (&m_cfFont) > 0)
		{
			if (m_hTxtFont != NULL)
				DeleteObject (m_hTxtFont);
			m_hTxtFont = CreateFontIndirect(&m_lfFont); 
			m_pMedia->SetParam (YYPLAY_PID_SubTT_Color, (void *)m_cfFont.rgbColors);
			m_pMedia->SetParam (YYPLAY_PID_SubTT_Font, (void *)m_hTxtFont);
		}
	}
	return S_FALSE;
}

bool CWndSubTT::UpdateLang (void)
{
	UpdateMenuItem ();
	return true;
}

void CWndSubTT::OnSizeMove (void)
{
	RECT	rcView;
	GetWindowRect (m_hParent, &rcView);
	int nLeft = rcView.left;
	int nTop = rcView.top + (rcView.bottom - rcView.top) / 2 - 60;
	SetWindowPos (m_hWnd, NULL, nLeft, nTop, (rcView.right - rcView.left), (rcView.bottom - rcView.top) / 2, 0);
}

bool CWndSubTT::OnMediaStart (void)
{
	int nHasSubTT = 0;
	m_pMedia->GetParam (YYPLAY_PID_SubTitle, &nHasSubTT);
	if (nHasSubTT <= 0)
		return true;

	CreateWnd ();

	m_pMedia->SetParam (YYPLAY_PID_SubTT_View, (void *)m_hWnd);
	m_pMedia->SetParam (YYPLAY_PID_SubTT_Color, (void *)m_cfFont.rgbColors);
	m_pMedia->SetParam (YYPLAY_PID_SubTT_Font, m_hTxtFont);

	return true;
}

bool CWndSubTT::OnMediaClose (void)
{
	if (m_hWnd != NULL)
		SendMessage (m_hWnd, WM_CLOSE, 0, 0);
	m_hWnd = NULL;
	return true;
}

bool CWndSubTT::UpdateMenuItem (void)
{
	HMENU hMenu = GetMenu (m_hParent);
	hMenu = GetSubMenu (hMenu, 1);
	HMENU hMenuVideo = GetSubMenu (hMenu, 4);
	if (m_nEnable > 0)
	{
		CheckMenuItem (hMenuVideo, ID_SUBTITLE_ENABLE, MF_BYCOMMAND | MF_CHECKED);
		CheckMenuItem (hMenuVideo, ID_SUBTITLE_DISABLE, MF_BYCOMMAND | MF_UNCHECKED);
	}
	else
	{
		CheckMenuItem (hMenuVideo, ID_SUBTITLE_ENABLE, MF_BYCOMMAND | MF_UNCHECKED);
		CheckMenuItem (hMenuVideo, ID_SUBTITLE_DISABLE, MF_BYCOMMAND | MF_CHECKED);
	}

	return true;
}

bool CWndSubTT::CreateWnd (void)
{
	if (m_hWnd != NULL)
	{
		SetParent (m_hWnd, m_hParent);
		return true;
	}

	WNDCLASS wcex;
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)ViewWindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)::CreateSolidBrush (m_bgColor);
	wcex.lpszMenuName	= (LPCTSTR)NULL;
	wcex.lpszClassName	= m_szClassName;
	RegisterClass(&wcex);

	DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
	m_hWnd = CreateWindow(m_szClassName, m_szWindowName, dwStyle,
						  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, m_hParent, NULL, m_hInst, NULL);
	if (m_hWnd == NULL)
		return false;

	LONG lRC = SetWindowLong (m_hWnd, GWL_USERDATA, (LONG)this);
	m_hBKBrush = CreateSolidBrush (m_bgColor);

	// Add the transparent effect.
	dwStyle = GetWindowLong (m_hWnd, GWL_EXSTYLE);
	dwStyle |= WS_EX_LAYERED;
	int nRC = SetWindowLong (m_hWnd, GWL_EXSTYLE, dwStyle);
	nRC = SetLayeredWindowAttributes  (m_hWnd, m_bgColor, 150, LWA_COLORKEY);
	if (nRC == 0)
	{
		ShowWindow (m_hWnd, SW_HIDE);
		return false;
	}

	OnSizeMove ();
	ShowWindow (m_hWnd, SW_SHOW);
	return true;
}

LRESULT CWndSubTT::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ERASEBKGND:
		{
		HDC hdc = (HDC)wParam;
		RECT rcView;
		GetClientRect (m_hWnd, &rcView);
		FillRect (hdc, &rcView, m_hBKBrush);
		return S_OK;
		}
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		return S_FALSE;
	}
	case WM_DESTROY:
		DeleteObject (m_hBKBrush);
		UnregisterClass (m_szClassName, NULL);
		break;

	default:
		break;
	}

	return	DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CWndSubTT::ViewWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWndSubTT * pViewWindow = (CWndSubTT *)GetWindowLong (hwnd, GWL_USERDATA);
	if (pViewWindow == NULL)
		return(DefWindowProc(hwnd, uMsg, wParam, lParam));
	else
		return pViewWindow->OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

bool CWndSubTT::CreateTextFont (void)
{
	CRegMng * pReg = CRegMng::g_pRegMng;

	m_lfFont.lfHeight = pReg->GetIntValue (_T("FontHeight"), -36);
	m_lfFont.lfWeight = pReg->GetIntValue (_T("FontWeight"), FW_MEDIUM);
	m_lfFont.lfCharSet = pReg->GetIntValue (_T("FontCharSet"), GB2312_CHARSET);
	m_lfFont.lfOutPrecision = pReg->GetIntValue (_T("FontOutPrec"), 1);
	m_lfFont.lfClipPrecision = pReg->GetIntValue (_T("FontClipPrec"), 2);
	m_lfFont.lfQuality = pReg->GetIntValue (_T("FontQuality"), 1);
	m_lfFont.lfPitchAndFamily	= pReg->GetIntValue (_T("FontPitch"), 49);

	TCHAR * pFontName = pReg->GetTextValue (_T("FontName"));
	if (_tcslen (pFontName) <= 0)
	{
		TCHAR szFontName[64];
		HDC hDC = GetDC (m_hParent);
		GetTextFace (hDC, 64, szFontName);
		_tcscpy (m_lfFont.lfFaceName, szFontName);
	}
	else
	{
		_tcscpy (m_lfFont.lfFaceName, pFontName);
	}

	m_cfFont.lStructSize = sizeof(CHOOSEFONT);
	m_cfFont.hwndOwner = m_hWnd;
	m_cfFont.lpLogFont = &m_lfFont;
	m_cfFont.Flags = CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_SCREENFONTS;
	m_cfFont.rgbColors = pReg->GetIntValue (_T("FontTextColor"), RGB (255, 255, 255));

	if (m_hTxtFont != NULL)
		DeleteObject (m_hTxtFont);
    m_hTxtFont = CreateFontIndirect(&m_lfFont); 

	return true;
}

bool CWndSubTT::SaveFontParam (void)
{
	CRegMng * pReg = CRegMng::g_pRegMng;
	pReg->SetIntValue (_T("SubTTEnable"), m_nEnable);

	pReg->SetIntValue (_T("FontHeight"), m_lfFont.lfHeight);
	pReg->SetIntValue (_T("FontWeight"), m_lfFont.lfWeight);
	pReg->SetIntValue (_T("FontCharSet"), m_lfFont.lfCharSet);
	pReg->SetIntValue (_T("FontOutPrec"), m_lfFont.lfOutPrecision);
	pReg->SetIntValue (_T("FontClipPrec"), m_lfFont.lfClipPrecision);
	pReg->SetIntValue (_T("FontQuality"), m_lfFont.lfQuality );
	pReg->SetIntValue (_T("FontPitch"), m_lfFont.lfPitchAndFamily);
	pReg->SetTextValue (_T("FontName"), m_lfFont.lfFaceName);

	pReg->SetIntValue (_T("FontTextColor"), m_cfFont.rgbColors);

	return true;
}

