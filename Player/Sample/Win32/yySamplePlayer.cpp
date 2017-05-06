// yyPlayer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <commctrl.h>
#include <Commdlg.h>
#include <winuser.h>
#include <shellapi.h>

#include "yySamplePlayer.h"
#include "CMediaPlayer.h"
#include "CWndVideo.h"

#define MAX_LOADSTRING			100

// Global Variables:
HINSTANCE			hInst;				// current instance
HWND				g_hWndCommandBar;	// command bar handle

// Add for yySDK
HWND				g_hSldPos = NULL;
CWndVideo *			g_wndVideo = NULL;

CMediaPlayer		gEngine;
YYM_Player *		gPlayer = NULL;
HBITMAP				g_hThumb = NULL;
YYINFO_Thumbnail	g_thumbInfo;
bool				g_bSeeking = false;

TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

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
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_YYPLAYER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YYPLAYER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(LTGRAY_BRUSH);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_YYPLAYER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
#else //WINCE
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YYPLAYER));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	return RegisterClass(&wc);
#endif // WINCE
}

void NotifyEvent (void * pUserData, int nID, void * pV1)
{
	HWND hWnd = (HWND)pUserData;
	if (nID == YY_EV_Play_Complete)
	{
		gPlayer->Stop (gPlayer);
	}
	else if (nID == YY_EV_Open_Complete)
	{
		ShowWindow (g_wndVideo->GetWnd (), SW_SHOW);
		int nDuration = gPlayer->GetDur (gPlayer);
		SendMessage (g_hSldPos, TBM_SETRANGE, TRUE, MAKELONG (0, nDuration / 1000));
		gPlayer->Run (gPlayer);
	}
	else if (nID == YY_EV_Open_Failed)
	{
		MessageBox (hWnd, _T("Open file failed!"), _T("Error"), MB_OK);
	}
	else if (nID == YY_EV_Seek_Complete)
	{
		g_bSeeking = false;
	}
	else if (nID == YY_EV_Seek_Failed)
	{
		MessageBox (hWnd, _T("Sep Pos failed!"), _T("Error"), MB_OK);
	}
}

int OpenMediaFile (HWND hWnd, int nType)
{
	DWORD				dwID = 0;
	OPENFILENAME		ofn;
	TCHAR				szFile[256];

	memset (szFile, 0, sizeof (szFile));
	memset( &(ofn), 0, sizeof(ofn));
	ofn.lStructSize	= sizeof(ofn);
	ofn.hwndOwner = hWnd;

	ofn.lpstrFilter = TEXT("Media File (*.*)\0*.*\0");	
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;

	ofn.lpstrTitle = TEXT("Open Media File");
	ofn.Flags = OFN_EXPLORER;
			
	if (!GetOpenFileName(&ofn))
		return -1;

	int nRC = 0;
	if (nType == 1)
	{
		if (g_hThumb != NULL)
			DeleteObject (g_hThumb);

		g_thumbInfo.bKeepAspectRatio = true;
		g_thumbInfo.nBGColor = RGB (0, 255, 0);

		g_hThumb = gPlayer->GetThumb (gPlayer, szFile, &g_thumbInfo);
		if (g_hThumb != NULL)
		{
			ShowWindow (g_wndVideo->GetWnd (), SW_HIDE);
			::InvalidateRect (hWnd, NULL, TRUE);
		}
	}
	else
	{
		gPlayer->Close (gPlayer);
		InvalidateRect (g_wndVideo->GetWnd (), NULL, TRUE);
		gPlayer->Open (gPlayer, szFile, 0);
	}

	return 0;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

#ifdef WINCE	
    hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
						CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
#else
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
						CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
#endif // WINCE

	if (!hWnd)
	  return FALSE;

#ifdef WINCE
    if (g_hWndCommandBar)
        CommandBar_Show(g_hWndCommandBar, TRUE);
#endif // WINCE

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

#ifndef WINCE
	SetWindowLong (hWnd, GWL_EXSTYLE, WS_EX_ACCEPTFILES);
	SetWindowPos (hWnd, NULL, (GetSystemMetrics (SM_CXSCREEN) - 818) / 2, (GetSystemMetrics (SM_CYSCREEN) - 498 - 100 ) / 2, 818, 498, 0);
#endif // WINCE

	RECT	rcWnd;
	GetClientRect (hWnd, &rcWnd);
	rcWnd.bottom = rcWnd.bottom - 24;

	g_wndVideo = new CWndVideo (hInst);
	rcWnd.top += 24;
	g_wndVideo->CreateWnd (hWnd, rcWnd, RGB (0, 0, 0));

	rcWnd.top = rcWnd.bottom;
	rcWnd.bottom = rcWnd.bottom + 24;
	g_hSldPos = CreateWindow (_T("msctls_trackbar32"), _T(""), WS_VISIBLE | WS_CHILD | TBS_BOTH | TBS_NOTICKS, 
							rcWnd.left, rcWnd.top, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top,
							hWnd, (HMENU)1001, hInst, NULL);

	gPlayer = gEngine.GetPlayer ();
	if (gPlayer != NULL)
	{
#ifdef WINCE
		gPlayer->SetView (gPlayer, g_wndVideo->GetWnd (), YY_VRND_DDCE6);
#else
		gPlayer->SetView (gPlayer, g_wndVideo->GetWnd (), YY_VRND_DDRAW);
#endif // WICE
		gPlayer->SetNotify (gPlayer, NotifyEvent, hWnd);
	}

	SetTimer (hWnd, 2002, 500, NULL);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int				wmId, wmEvent;
	PAINTSTRUCT		ps;
	HDC				hdc;
	float			fSpeed = 1.0;
	YYPLAY_ARInfo	arInfo;
	RECT			rcVideo;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_FILE_OPENFILE:
			if (gPlayer != NULL)
				gPlayer->Pause (gPlayer);
			OpenMediaFile (hWnd, 0);
			break;

		case ID_FILE_DDMEMORY:
			if (gPlayer != NULL)
				gPlayer->SetParam (gPlayer, YYPLAY_PID_DDMode, (void *)YY_DDM_Memory);
			return S_OK;

		case ID_FILE_DDOVERLAY:
			if (gPlayer != NULL)
				gPlayer->SetParam (gPlayer, YYPLAY_PID_DDMode, (void *)YY_DDM_Overlay);
			MessageBox (hWnd, _T("Test overlay mode!"), _T("Overlay"), MB_OK);
			return S_OK;

		case ID_FILE_DISABLEVIDEO:
			if (gPlayer != NULL)
				gPlayer->SetParam (gPlayer, YYPLAY_PID_Disable_Video, (void *)YY_PLAY_VideoDisable_Render);
			return S_OK;
		case ID_FILE_DISABLEDECODER:
			if (gPlayer != NULL)
				gPlayer->SetParam (gPlayer, YYPLAY_PID_Disable_Video, (void *)YY_PLAY_VideoDisable_Decoder);
			return S_OK;
		case ID_FILE_ENABLEVIDEO:
			if (gPlayer != NULL)
				gPlayer->SetParam (gPlayer, YYPLAY_PID_Disable_Video, (void *)YY_PLAY_VideoEnable);
			return S_OK;

		case ID_PLAY_PLAY:
			if (gPlayer != NULL)
				gPlayer->Run (gPlayer);
			break;

		case ID_PLAY_PAUSE:
			if (gPlayer != NULL)
				gPlayer->Pause (gPlayer);
			break;

		case ID_PLAY_STOP:
			if (gPlayer != NULL)
				gPlayer->Stop (gPlayer);
			break;

		case ID_TOOLS_FILEINFO:
			{
				TCHAR szInfo[1024];
				if (gPlayer != NULL)
				{
					YYPLAY_STATUS status = gPlayer->GetStatus (gPlayer);
					if (status == YY_PLAY_Run)
						gPlayer->Pause (gPlayer);

					gPlayer->MediaInfo (gPlayer, szInfo, sizeof (szInfo));
					MessageBox (hWnd, szInfo, _T("Info"),  MB_OK);

					if (status == YY_PLAY_Run)
						gPlayer->Run (gPlayer);
				}
			}
			break;

		case ID_TOOLS_THUMBNAIL:
			OpenMediaFile (hWnd, 1);
			break;

		case ID_TOOLS_FULLSCREEN:
			break;

		case ID_SPEED_0:
			fSpeed = 0.2;
			gPlayer->SetParam (gPlayer, YYPLAY_PID_Speed, &fSpeed);
			break;

		case ID_SPEED_1:
			fSpeed = 0.5;
			gPlayer->SetParam (gPlayer, YYPLAY_PID_Speed, &fSpeed);
			break;

		case ID_SPEED_2:
			fSpeed = 1.5;
			gPlayer->SetParam (gPlayer, YYPLAY_PID_Speed, &fSpeed);
			break;

		case ID_SPEED_3:
			fSpeed = 1.0;
			gPlayer->SetParam (gPlayer, YYPLAY_PID_Speed, &fSpeed);
			break;

		case ID_SPEED_2X:
			fSpeed = 2.0;
			gPlayer->SetParam (gPlayer, YYPLAY_PID_Speed, &fSpeed);
			break;

		case ID_SPEED_4X:
			fSpeed = 4.0;
			gPlayer->SetParam (gPlayer, YYPLAY_PID_Speed, &fSpeed);
			break;

		case ID_SPEED_8X:
			fSpeed = 8.0;
			gPlayer->SetParam (gPlayer, YYPLAY_PID_Speed, &fSpeed);
			break;

		case ID_ASPECTRATIO_4X3:
			arInfo.nWidth = 4;
			arInfo.nHeight = 3;
			gPlayer->SetParam (gPlayer, YYPLAY_PID_AspectRatio, &arInfo);
			InvalidateRect (g_wndVideo->GetWnd (), NULL, TRUE);
			break;

		case ID_ASPECTRATIO_16X9:
			arInfo.nWidth = 16;
			arInfo.nHeight = 9;
			gPlayer->SetParam (gPlayer, YYPLAY_PID_AspectRatio, &arInfo);
			InvalidateRect (g_wndVideo->GetWnd (), NULL, TRUE);
			break;

		case ID_ASPECTRATIO_FITWINDOW:
		{
			RECT rcView;
			GetClientRect (g_wndVideo->GetWnd (), &rcView);
			arInfo.nWidth = rcView.right;
			arInfo.nHeight = rcView.bottom;
			gPlayer->SetParam (gPlayer, YYPLAY_PID_AspectRatio, &arInfo);
			InvalidateRect (g_wndVideo->GetWnd (), NULL, TRUE);
		}
			break;

		case ID_ASPECTRATIO_ORIGINAL:
			arInfo.nWidth = 1;
			arInfo.nHeight = 1;
			gPlayer->SetParam (gPlayer, YYPLAY_PID_AspectRatio, &arInfo);
			InvalidateRect (g_wndVideo->GetWnd (), NULL, TRUE);
			break;

		case ID_VIDEOZOOM_1:
			SetRect (&rcVideo, 0, 0, gPlayer->sInfo.nWidth * 2 / 3, gPlayer->sInfo.nHeight * 2 / 3);
			gPlayer->SetParam (gPlayer, YYPLAY_PID_VideoZoomIn, &rcVideo);
			break;

		case ID_VIDEOZOOM_2X:
			SetRect (&rcVideo, 0, 0, gPlayer->sInfo.nWidth / 2, gPlayer->sInfo.nHeight / 2);
			gPlayer->SetParam (gPlayer, YYPLAY_PID_VideoZoomIn, &rcVideo);
			break;

		case ID_VIDEOZOOM_4X:
			SetRect (&rcVideo, 24, 36, 24 + gPlayer->sInfo.nWidth / 4, 36 + gPlayer->sInfo.nHeight / 4);
			gPlayer->SetParam (gPlayer, YYPLAY_PID_VideoZoomIn, &rcVideo);
			break;

		case ID_FILE_SEEKKEYFRAME:
			gPlayer->SetParam (gPlayer, YYPLAY_PID_SeekMode, (void *)YY_SEEK_KeyFrame);
			break;

		case ID_FILE_SEEKANYPOS:
			gPlayer->SetParam (gPlayer, YYPLAY_PID_SeekMode, (void *)YY_SEEK_AnyPosition);
			break;

		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_HSCROLL:
		if (gPlayer != NULL)
		{
			int nPos = SendMessage (g_hSldPos, TBM_GETPOS, 0, 0);
			g_bSeeking = true;
			gPlayer->SetPos (gPlayer, nPos * 1000);
		}
		break;

	case WM_TIMER:
		if (gPlayer != NULL)
			SendMessage (g_hSldPos, TBM_SETPOS, TRUE, gPlayer->GetPos (gPlayer) / 1000);
		break;

	case WM_VIEW_FullScreen:
		if (gPlayer != NULL)
		{
			RECT rcView;
			GetClientRect (g_wndVideo->GetWnd(), &rcView);
			gPlayer->UpdateView (gPlayer, &rcView);
		}
		break;

	case WM_VIEW_OnPaint:
		if (gPlayer != NULL)
			gPlayer->UpdateView (gPlayer, NULL);
		break;

#ifdef WINCE
    case WM_CREATE:
        g_hWndCommandBar = CommandBar_Create(hInst, hWnd, 1);
        CommandBar_InsertMenubar(g_hWndCommandBar, hInst, IDC_YYPLAYER, 0);
        CommandBar_AddAdornments(g_hWndCommandBar, 0, 0);
        break;
#endif // WINCE

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (g_hThumb != NULL)
		{
			HDC hMemDC = ::CreateCompatibleDC (hdc);
			SelectObject (hMemDC, g_hThumb);
			BitBlt (hdc, 0, 0, 320, 240, hMemDC, 0, 0, SRCCOPY);
			DeleteDC (hMemDC);
		}
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;

	case WM_CLOSE:
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_DESTROY:
		if (g_hThumb != NULL)
			DeleteObject (g_hThumb);
		g_hThumb = NULL;
		gEngine.Uninit ();
		if (g_wndVideo != NULL)
		{
			SendMessage (g_wndVideo->GetWnd (), WM_CLOSE, 0, 0);
			delete g_wndVideo;
			g_wndVideo = NULL;
		}
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
		SetWindowPos (hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcWnd.right) / 2, (GetSystemMetrics (SM_CYSCREEN) - rcWnd.bottom ) / 2, 0, 0, SWP_NOSIZE);
		return (INT_PTR)TRUE;

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
