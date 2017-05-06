// JinPlayer.cpp : Defines the entry point for the application.
#include "stdafx.h"
#include "shellapi.h"

#include "JinPlayer.h"
#include "CJinPlayer.h"
#include "CLangText.h"
#include "CRegMng.h"

#include "UFileFunc.h"
#include "USystemFunc.h"

#define MAX_LOADSTRING	100

#define WND_WIDTH		848
#define	WND_HEIGHT		550

// Global Variables:
HINSTANCE		g_hInst;								// current instance
TCHAR			g_szTitle[MAX_LOADSTRING];			// The title bar text
TCHAR			g_szWindowClass[MAX_LOADSTRING];		// the main window class name
CJinPlayer *	g_Player = NULL;
TCHAR			g_szCmdLine[1024];
HBRUSH			g_hBrhBG = NULL;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_JINPLAYER, g_szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	memset (g_szCmdLine, 0, sizeof (g_szCmdLine));
	if (_tcslen (lpCmdLine) > 0)
	{
		if (lpCmdLine[0] == _T('"'))
			lpCmdLine++;
		if (lpCmdLine[_tcslen (lpCmdLine)-1] == _T('"'))
			lpCmdLine[_tcslen (lpCmdLine)-1] = 0;
		_tcscpy (g_szCmdLine, lpCmdLine);
	}

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JINPLAYER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	g_hBrhBG = ::CreateSolidBrush (RGB (239, 239, 239));

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JINPLAYER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)g_hBrhBG;
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_JINPLAYER);
	wcex.lpszClassName	= g_szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	g_hInst = hInstance; // Store instance handle in our global variable
	hWnd = CreateWindow(g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW | WS_VSCROLL,
						CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return FALSE;

	SetWindowLong (hWnd, GWL_EXSTYLE, WS_EX_ACCEPTFILES);

	int nX = GetSystemMetrics (SM_CXSCREEN);
	int nY = GetSystemMetrics (SM_CYSCREEN);
	if (nX > WND_WIDTH)
		nX = WND_WIDTH;
	if (nY > WND_HEIGHT)
		nY = WND_HEIGHT;

	SetWindowPos (hWnd, NULL, (GetSystemMetrics (SM_CXSCREEN) - nX) / 2, (GetSystemMetrics (SM_CYSCREEN) - nY) / 2, nX, nY, 0);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	g_Player = new CJinPlayer (g_hInst);
	if (_tcslen (g_szCmdLine) > 0)
		g_Player->Create (hWnd, g_szCmdLine);
	else
		g_Player->Create (hWnd, NULL);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	if (g_Player != NULL)
	{
		int nRC = g_Player->MsgProc (hWnd, message, wParam, lParam);
		if (nRC != S_FALSE)
			return nRC;
	}

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_SYSCOMMAND:
//		if (wParam == SC_MONITORPOWER)
//			return S_FALSE;
		break;

	case WM_POWERBROADCAST:
//		if (wParam == PBT_APMQUERYSUSPEND)
//			return BROADCAST_QUERY_DENY;
		break;

	case WM_DESTROY:
		YY_DEL_P (g_Player);
		DeleteObject (g_hBrhBG);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		yyFile hFile = NULL;
		TCHAR szFile[1024];
		yyGetAppPath (g_hInst, szFile, sizeof (szFile));
		_tcscat (szFile, _T("jpres\\about_"));
		if (CLangText::g_pLang->GetLang () == YYLANG_CHN)
			_tcscat (szFile, _T("chn.txt"));
		else
			_tcscat (szFile, _T("eng.txt"));
		hFile = yyFileOpen (szFile, YYFILE_READ);
		if (hFile == NULL)
			return false;
		int nFileSize = (int)yyFileSize (hFile) + 8;
		unsigned char * pTextFile = new unsigned char[nFileSize];
		memset (pTextFile, 0, nFileSize);
		yyFileRead (hFile, pTextFile, nFileSize - 8);
		yyFileClose (hFile);

		SetWindowText (hDlg, yyLangGetText (YYTEXT_AboutPlayer));
		SetWindowText (GetDlgItem (hDlg, IDC_EDIT_ABOUT), (TCHAR *)(pTextFile + 2));

		TCHAR szVer[64];
		_tcscpy (szVer, _T("jinPlayer Version: "));
		TCHAR * pVer = CRegMng::g_pRegMng->GetTextValue (_T("CurVersion"));
		_tcscat (szVer, pVer);
		SetDlgItemText (hDlg, IDC_STATIC_VER, szVer);

		RECT			rcDlg;
		GetClientRect (hDlg, &rcDlg);
		SetWindowPos (hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcDlg.right) / 2, 
						(GetSystemMetrics (SM_CYSCREEN) - rcDlg.bottom ) / 2, 0, 0, SWP_NOSIZE);
		return (INT_PTR)TRUE;
	}

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
