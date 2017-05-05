/*******************************************************************************
	File:		yyMonitor.h

	Contains:	yy monitor define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __yyMonitor_h__
#define __yyMonitor_h__

#include "tchar.h"

#include "yyType.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define	YYMNT_WINDOW_NAME			_T("yyMonitorWindowView")
#define	YYMNT_WINDOW_CLASS			_T("yyMonitorWindowClass")

#define	WM_YYMNT_BASE				WM_APP + 0X100
#define	WM_YYMNT_MAX				WM_APP + 0X800

#define	WM_YYMNT_RTN_BASE			WM_APP + 0X100
#define	WM_YYMNT_RTN_INIT			WM_YYMNT_RTN_BASE + 0X0001
#define	WM_YYMNT_RTN_OPEN			WM_YYMNT_RTN_BASE + 0X0002
#define	WM_YYMNT_RTN_RUN			WM_YYMNT_RTN_BASE + 0X0003
#define	WM_YYMNT_RTN_STOP			WM_YYMNT_RTN_BASE + 0X0004
#define	WM_YYMNT_RTN_SEEK			WM_YYMNT_RTN_BASE + 0X0005
#define	WM_YYMNT_RTN_CLOSE			WM_YYMNT_RTN_BASE + 0X0006
#define	WM_YYMNT_RTN_UNINIT			WM_YYMNT_RTN_BASE + 0X0007

#define	WM_YYMNT_INFO_BASE			WM_APP + 0X200
// wParam stream number, lParam bitrate
#define	WM_YYMNT_INFO_FILE			WM_YYMNT_INFO_BASE + 0X0001
// wParam video size, lParam frame rate, codec ID
#define	WM_YYMNT_INFO_VIDEO			WM_YYMNT_INFO_BASE + 0X0002
// wParam audio format, lParam codec ID.
#define	WM_YYMNT_INFO_AUDIO			WM_YYMNT_INFO_BASE + 0X0003

#define	WM_YYMNT_STT_BASE			WM_APP + 0X300
// wParam, Video, lParam Audio pcaket num, 
#define	WM_YYMNT_STT_PacketNum		WM_YYMNT_STT_BASE + 0X0001
// wParam system time, lParam thread time.
#define	WM_YYMNT_STT_PacketRead		WM_YYMNT_STT_BASE + 0X0002
// wParam system time, lParam thread time.
#define	WM_YYMNT_STT_VideoDec		WM_YYMNT_STT_BASE + 0X0003
// wParam frame number, lParam frame per second (100).
#define	WM_YYMNT_STT_VideoRnd		WM_YYMNT_STT_BASE + 0X0004
// wParam system time, lParam thread time.
#define	WM_YYMNT_STT_AudioDec		WM_YYMNT_STT_BASE + 0X0005
// wParam sample number, lParam thread time
#define	WM_YYMNT_STT_AudioRnd		WM_YYMNT_STT_BASE + 0X0006

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif // __yyMonitor_h__
