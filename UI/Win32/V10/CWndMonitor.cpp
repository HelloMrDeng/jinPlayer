/*******************************************************************************
	File:		CWndMonitor.cpp

	Contains:	Window slide pos implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "yyMonitor.h"

#include "CWndMonitor.h"
#include "CBaseUtils.h"

#pragma warning (disable : 4996)

CWndMonitor::CWndMonitor(HINSTANCE hInst)
	: CWndBase (hInst)
	, m_hWndText (NULL)
	, m_pInfo (NULL)
	, m_nSize (1024 * 64)
{
	m_pInfo = new TCHAR[m_nSize];
	memset (m_pInfo, 0, m_nSize * sizeof (TCHAR));
}

CWndMonitor::~CWndMonitor(void)
{
	if (m_pInfo != NULL)
		delete []m_pInfo;
	m_pInfo = NULL;
}

bool CWndMonitor::CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG)
{
	if (!CWndBase::CreateWnd (hParent, rcView, clrBG))
		return false;

	RECT rcText;
	GetClientRect (m_hWnd, &rcText);
	m_hWndText = CreateWindow (_T("edit"), _T(""), WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | ES_READONLY , 
					 rcText.left, rcText.top, rcText.right, rcText.bottom, m_hWnd, (HMENU)1001, m_hInst, NULL);

	return true;
}

LRESULT CWndMonitor::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;		
		HDC hdc = BeginPaint(hwnd, &ps);

		EndPaint(hwnd, &ps);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	default:
		break;
	}

	return	CWndBase::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

int CWndMonitor::ShowMessage (int nMsg, WPARAM wParam, LPARAM lParam)
{
	if (nMsg < WM_YYMNT_BASE || nMsg > WM_YYMNT_MAX)
		return -1;

	SYSTEMTIME tmSys;
	GetSystemTime (&tmSys);
	_stprintf (m_szMsg, _T("[%02d:%02d:%02d]\r\n"), tmSys.wHour, tmSys.wMinute, tmSys.wSecond);
	_tcscat (m_pInfo, m_szMsg);

	switch (nMsg)
	{
	case WM_YYMNT_RTN_INIT:
		_stprintf (m_szMsg, _T("Init %08X \r\n "), (int)wParam);
		break;

	case WM_YYMNT_RTN_OPEN:
		_stprintf (m_szMsg, _T("Open %08X \r\n "), (int)wParam);
		break;

	case WM_YYMNT_RTN_RUN:
		_stprintf (m_szMsg, _T("Run %08X \r\n "), (int)wParam);
		break;

	case WM_YYMNT_RTN_STOP:
		_stprintf (m_szMsg, _T("Stop %08X \r\n "), (int)wParam);
		break;

	case WM_YYMNT_RTN_CLOSE:
		_stprintf (m_szMsg, _T("Close %08X \r\n "), (int)wParam);
		break;

	case WM_YYMNT_RTN_UNINIT:
		_stprintf (m_szMsg, _T("Uninit %08X \r\n "), (int)wParam);
		break;

	case WM_YYMNT_INFO_FILE:
		_stprintf (m_szMsg, _T("File: Streams %d  Bitrate %d\r\n "), (int)wParam, (int)lParam);
		break;

	case WM_YYMNT_INFO_VIDEO:
		_stprintf (m_szMsg, _T("Video: %d X %d  FPS %d\r\n "), HIWORD(wParam), LOWORD(wParam), (int)lParam);
		break;

	case WM_YYMNT_INFO_AUDIO:
		_stprintf (m_szMsg, _T("Audio:  %d  X %d\r\n "), (int)wParam, (int)lParam);
		break;

	case WM_YYMNT_STT_PacketNum:
		_stprintf (m_szMsg, _T("Packets:  %d X %d  ID %d\r\n "), (int)wParam, (int)lParam);
		break;

	case WM_YYMNT_STT_PacketRead:
		_stprintf (m_szMsg, _T("Read:  %d X %d  ID %d\r\n "), (int)wParam, (int)lParam);
		break;

	case WM_YYMNT_STT_VideoDec:
		_stprintf (m_szMsg, _T("VDec:  %d X %d  ID %d\r\n "), (int)wParam, (int)lParam);
		break;

	case WM_YYMNT_STT_VideoRnd:
		_stprintf (m_szMsg, _T("Result: %d  %.2f\r\n\r\n "), (int)wParam, (float)(lParam / 100.0));
		break;

	case WM_YYMNT_STT_AudioDec:
		_stprintf (m_szMsg, _T("ADec: %d %d\r\n "), (int)wParam, (int)lParam);
		break;

	case WM_YYMNT_STT_AudioRnd:
		_stprintf (m_szMsg, _T("ARnd: %d %d\r\n "), (int)wParam, (int)lParam);
		break;


	default:
		break;
	}

	_tcscat (m_pInfo, m_szMsg);
	SetWindowText (m_hWndText, m_pInfo);

	int nLines = SendMessage (m_hWndText, EM_GETLINECOUNT, 0, 0);
	SendMessage (m_hWndText, EM_LINESCROLL, 0, nLines);

	return 0;
}
