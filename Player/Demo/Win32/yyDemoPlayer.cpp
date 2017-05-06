// yyPlayer.cpp : Defines the entry point for the application.
//
#include <windows.h>
#include <commctrl.h>
#include <Commdlg.h>
#include <winuser.h>
#include <shellapi.h>
#include <tchar.h>

#include "Resource.h"

#include "UFFMpegFunc.h"

#include "CDemoUI.h"

#pragma warning (disable : 4996)

#define MAX_LOADSTRING			100

// Global Variables:
HINSTANCE			g_hInst;				// current instance
HWND				g_hWndCommandBar;	// command bar handle

TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


CDemoUI	*			g_demoUI = NULL;
TCHAR				g_szCmdLine[1024];

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
	_tcscpy (szTitle, _T("yyDemoPlayer"));
	_tcscpy (szWindowClass, _T("yyDemoPlayerWindow"));
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

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_YYPLAYER));

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
#ifndef WINCE
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(LTGRAY_BRUSH);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDR_MENU_MAIN);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON));

	return RegisterClassEx(&wcex);
#else //WINCE
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
#endif // WINCE
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	g_hInst = hInstance; // Store instance handle in our global variable

#ifdef _OS_WINCE	
    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
						CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
#else
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
						CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
#endif // WINCE

	if (!hWnd)
	  return FALSE;

#ifdef _OS_WINCE
    if (g_hWndCommandBar)
        CommandBar_Show(g_hWndCommandBar, TRUE);
#endif // WINCE

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	yyInitFFMpeg ();

	g_demoUI = new CDemoUI ();
	if (_tcslen (g_szCmdLine) > 0)
		g_demoUI->Create (g_hInst, hWnd, false);
	else
		g_demoUI->Create (g_hInst, hWnd, true);

	if (_tcslen (g_szCmdLine) > 0)
		g_demoUI->OpenMediaFile (g_szCmdLine, true);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	if (g_demoUI != NULL && g_demoUI->MsgProc (hWnd, message, wParam, lParam) == S_OK)
		return S_OK;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_HELP_ABOUT:
		{
			COMBoxMng * pMedia = NULL;
			YYPLAY_STATUS status = YY_PLAY_Stop;
			if (g_demoUI != NULL)
			{
				pMedia = g_demoUI->GetPlayer ();
				if (pMedia != NULL)
				{
					status = pMedia->GetStatus ();
					if (status == YY_PLAY_Run)
					{
						pMedia->Pause ();
						Sleep (100);
					}
				}
			}

			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);

			if (status == YY_PLAY_Run)
				pMedia->Start ();

		}
			break;

		case ID_FILE_EXIT:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

#ifdef _OS_WINCE
    case WM_CREATE:
        g_hWndCommandBar = CommandBar_Create(g_hInst, hWnd, 1);
        CommandBar_InsertMenubar(g_hWndCommandBar, g_hInst, IDR_MENU_MAIN, 0);
        CommandBar_AddAdornments(g_hWndCommandBar, 0, 0);
        break;
#endif // WINCE

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;

	case WM_CLOSE:
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_DESTROY:
		if (g_demoUI != NULL)
			delete g_demoUI;
		g_demoUI = NULL;
		yyFreeFFMpeg ();
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rcWnd;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		GetClientRect (hDlg, &rcWnd);
		SetWindowPos (hDlg, HWND_TOPMOST, (GetSystemMetrics (SM_CXSCREEN) - rcWnd.right) / 2, (GetSystemMetrics (SM_CYSCREEN) - rcWnd.bottom ) / 2, 0, 0, SWP_NOSIZE);
		//SetTimer (hDlg, 1001, 1000, NULL);
		return (INT_PTR)TRUE;

	case WM_TIMER:
	{
		KillTimer (hDlg, 1001);
		PostMessage (GetParent (hDlg), WM_COMMAND, ID_FILE_DISABLEVIDEO, 0);
		break;
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
