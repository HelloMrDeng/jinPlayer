/*******************************************************************************
	File:		CWndBar.cpp

	Contains:	The control panel implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-13		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"
#include <commctrl.h>
#include <Commdlg.h>

#include "CWndBar.h"
#include "CCtrlButton.h"
#include "CCtrlSlider.h"

#include "CRegMng.h"
#include "USystemFunc.h"

#define ID_BUTTON_PLAY		1001
#define ID_BUTTON_PAUSE		1002

#pragma warning (disable : 4996)

CWndBar::CWndBar(HINSTANCE hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hParent (NULL)
	, m_hWnd (NULL)
	, m_pMedia (NULL)
	, m_pConfig (NULL)
{
	SetObjectName ("CWndBar");
	_tcscpy (m_szClass, _T("yyWndBarClass"));
	_tcscpy (m_szPath, _T(""));
}

CWndBar::~CWndBar(void)
{
	CCtrlBase * pCtrl = m_lstCtrl.RemoveHead ();
	while (pCtrl != NULL)
	{
		delete pCtrl;
		pCtrl = m_lstCtrl.RemoveHead ();
	}
	YY_DEL_P (m_pConfig);
}

bool CWndBar::CreateWnd (HWND hParent, TCHAR * pCfgFile)
{
	if (m_hWnd != NULL)
		return true;
	m_hParent = hParent;

	COLORREF	clrBG = RGB (25, 10, 15);

	WNDCLASS	wcex;
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndBarProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)::CreateSolidBrush (clrBG);
	wcex.lpszMenuName	= (LPCTSTR)NULL;
	wcex.lpszClassName	= m_szClass;
	RegisterClass(&wcex);

	DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
	m_hWnd = CreateWindow(m_szClass, _T(""), dwStyle,
						  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, m_hParent, NULL, m_hInst, NULL);
	if (m_hWnd == NULL)
		return false;

	LONG lRC = SetWindowLong (m_hWnd, GWL_USERDATA, (LONG)this);

	// Add the transparent effect.
	dwStyle = GetWindowLong (m_hWnd, GWL_EXSTYLE);
	dwStyle |= WS_EX_LAYERED;
//	int nRC = SetWindowLong (m_hWnd, GWL_EXSTYLE, dwStyle);
//	nRC = SetLayeredWindowAttributes  (m_hWnd, clrBG, 150, LWA_COLORKEY);


	if (pCfgFile == NULL)
	{
		TCHAR szPath[1024];
		yyGetAppPath (m_hInst, szPath, sizeof (szPath));
		_tcscat (szPath, _T("sureres\\WndBar.skn"));
		LoadConfig (szPath);
	}
	else
	{
		LoadConfig (pCfgFile);
	}

	ShowWindow (m_hWnd, SW_SHOW);

	return true;
}

bool CWndBar::LoadConfig (TCHAR * pCfgFile)
{
	if (m_pConfig == NULL)
		m_pConfig = new CBaseConfig ();
	if (!m_pConfig->Open (pCfgFile))
		return false;

	_tcscpy (m_szPath, pCfgFile);
	TCHAR * pPos = _tcsrchr (m_szPath, _T('\\'));
	if (pPos != NULL)
		*(pPos + 1) = 0;

	RECT rcBar;
	m_pConfig->GetItemRect ("Bar", &rcBar);
	POINT ptPos;
	ptPos.x = rcBar.left;
	ptPos.y = rcBar.top;
	ClientToScreen (m_hParent, &ptPos);
	SetWindowPos (m_hWnd, NULL, ptPos.x, ptPos.y, rcBar.right - rcBar.left, rcBar.bottom- rcBar.top, 0);

	char * pSectType = NULL;
	CCfgSect * pSect = m_pConfig->GetFirstSect ();
	while (pSect != NULL)
	{
		pSectType = m_pConfig->GetItemText (pSect->m_pName, "Type", NULL);
		if (pSectType != NULL)
			AddControl (pSect->m_pName);
		pSect = pSect->m_pNext;
	}

	return true;
}

bool CWndBar::AddControl (char * pName)
{
	char * pType = m_pConfig->GetItemText (pName, "Type", NULL);
	if (pType == NULL)
		return false;

	if (strcmp (pType, "Button") && strcmp (pType, "Slider"))
		return false;

	CCtrlBase * pCtrl = NULL;
	if (!strcmp (pType, "Button"))
		 pCtrl = new CCtrlButton (m_szPath);
	else if (!strcmp (pType, "Slider"))
		 pCtrl = new CCtrlSlider (m_szPath);
	else
		return false;

	pCtrl->Create (m_hWnd, m_pConfig, pName);

	m_lstCtrl.AddTail (pCtrl);

	return true;
}

LRESULT CWndBar::WndBarProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWndBar * pWndBar = (CWndBar *)GetWindowLong (hWnd, GWL_USERDATA);
	if (pWndBar == NULL)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	else
		return pWndBar->OnMsg (hWnd, uMsg, wParam, lParam);
}

LRESULT CWndBar::OnMsg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT		nRC = S_OK;
	CCtrlBase * pCtrl = NULL;
	POSITION	pos = m_lstCtrl.GetHeadPosition ();
	while (pos != NULL)
	{
		pCtrl = m_lstCtrl.GetNext (pos);
		nRC = pCtrl->OnMsg (uMsg, wParam, lParam);
		if (nRC == S_OK)
			return S_OK;
	}

	switch (uMsg)
	{
	case WM_COMMAND:
	{
		switch (wParam)
		{
		case ID_BUTTON_PLAY:
			m_pMedia->Run ();
			break;

		case ID_BUTTON_PAUSE:
			m_pMedia->Pause ();
			break;

		default:
			break;
		}
		return S_OK;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		pos = m_lstCtrl.GetHeadPosition ();
		while (pos != NULL)
		{
			pCtrl = m_lstCtrl.GetNext (pos);
			pCtrl->OnPaint (hdc);
		}
		EndPaint(hwnd, &ps);
		return S_OK;
	}

	case WM_ERASEBKGND:
		break;

	case WM_DESTROY:
		UnregisterClass (m_szClass, NULL);
		break;

	default:
		break;
	}

	return	DefWindowProc(hwnd, uMsg, wParam, lParam);
}

