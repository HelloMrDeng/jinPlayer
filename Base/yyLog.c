/*******************************************************************************
	File:		yyLog.c

	Contains:	Log printf implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-05		Fenger			Create file

*******************************************************************************/
#ifdef _OS_WIN32
#include "windows.h"
#include "tchar.h"
#endif // _OS_WIN32

#include "yyLog.h"

int		g_nInitLogTime = 0;

void yyDisplayLog (const char * pLogText)
{
#ifdef _OS_WIN32
#ifdef _UNICODE
	TCHAR wzLogText[1024];
	int	nLogTime =  0;
	if (g_nInitLogTime == 0)
		g_nInitLogTime = GetTickCount ();
	nLogTime = GetTickCount () - g_nInitLogTime;
	_stprintf (wzLogText, _T("%02d:%02d:%03d   "), ((nLogTime / 1000) % 3600) / 60, (nLogTime / 1000) % 60, nLogTime % 1000);
	MultiByteToWideChar (CP_ACP, 0, pLogText, -1, wzLogText + 10, sizeof (wzLogText));
#ifdef _CPU_MSB2531
	RETAILMSG (1, (wzLogText));
#else
	OutputDebugString (wzLogText); 
#endif // _CPU_MSB2531
#else
	OutputDebugString (pLogText); 
#endif // _UNIDCOE
#endif // _OS_WIN32
}


void yyDisplayMsg (void * hWnd, const char * pLogText)
{
#ifdef _OS_WIN32
#ifdef _UNICODE
	TCHAR wzLogText[1024];
	MultiByteToWideChar (CP_ACP, 0, pLogText, -1, wzLogText, sizeof (wzLogText));
	MessageBox ((HWND)hWnd, wzLogText, _T("YYMSG"), MB_OK); 
#else
	MessageBox ((HWND)hWnd, pLogText, _T("YYMSG"), MB_OK); 
#endif // _UNIDCOE
#endif // _OS_WIN32
}
