/*******************************************************************************
	File:		yyLog.h

	Contains:	yyLog define header file

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-29		Fenger			Create file

*******************************************************************************/
#ifndef __yyLog_H__
#define __yyLog_H__

#include <string.h>
#include <stdio.h>

#ifdef _OS_LINUX
#include <pthread.h>
#endif // _OS_LINUX

#if defined _OS_NDK
#include <android/log.h>
#ifndef LOG_TAG
#define  LOG_TAG "@@@YYLOG"
#endif // LOG_TAG
#if !defined LOGW
#define LOGW(...) ((int)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#endif
#if !defined LOGI
#define LOGI(...) ((int)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#endif
#if !defined LOGE
#define LOGE(...) ((int)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#endif
#endif //_OS_NDK

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus 

void	yyDisplayLog (const char * pLogText);
void	yyDisplayMsg (void * hWnd, const char * pLogText);

#ifdef _YYLOG_ERROR
#ifdef _OS_WIN32
#define YYLOGE(fmt, ...) \
{ \
	char		szLogText[1024]; \
	sprintf (szLogText, "@@@YYLOG Err T%08X %s L%d " fmt "\r\n", GetCurrentThreadId(), m_szObjName, __LINE__, __VA_ARGS__); \
	yyDisplayLog (szLogText); \
}
#elif defined _OS_NDK
#define YYLOGE(fmt, args...) \
{ \
	LOGE ("Err  T%08X %s L%d " fmt "\r\n", (int)pthread_self(), m_szObjName, __LINE__, ## args); \
}
#endif // _OS_WIN32
#else
#define YYLOGE(fmt, ...)
#endif // _YYLOG_ERROR

#ifdef _YYLOG_WARNING
#ifdef _OS_WIN32
#define YYLOGW(fmt, ...) \
{ \
	char		szLogText[1024]; \
	sprintf (szLogText, "@@@YYLOG Warn  T%08X %s L%d " fmt "\r\n", GetCurrentThreadId(), m_szObjName, __LINE__, __VA_ARGS__); \
	yyDisplayLog (szLogText); \
}
#elif defined _OS_NDK
#define YYLOGW(fmt, args...) \
{ \
	LOGW ("Warn T%08X %s L%d " fmt "\r\n", (int)pthread_self(), m_szObjName, __LINE__, ## args); \
}
#endif // _OS_WIN32
#else
#define YYLOGW(fmt, ...)
#endif // _YYLOG_WARNING

#ifdef _YYLOG_INFO
#ifdef _OS_WIN32
#define YYLOGI(fmt, ...) \
{ \
	char		szLogText[1024]; \
	sprintf (szLogText, "@@@YYLOG Info T%08X %s L%d " fmt "\r\n", GetCurrentThreadId(), m_szObjName, __LINE__, __VA_ARGS__); \
	yyDisplayLog (szLogText); \
}
#elif defined _OS_NDK
#define YYLOGI(fmt, args...) \
{ \
	LOGI ("Info T%08X %s L%d " fmt "\r\n", (int)pthread_self(), m_szObjName, __LINE__, ## args); \
}
#endif // _OS_WIN32
#else
#define YYLOGI(fmt, ...)
#endif // _YYLOG_INFO

#ifdef _YYLOG_TEXT
#ifdef _OS_WIN32
#define YYLOGT(txt, fmt, ...) \
{ \
	char		szLogText[1024]; \
	sprintf (szLogText, "@@@YYLOG Info T%08X %s L%d " fmt "\r\n", GetCurrentThreadId(), txt, __LINE__, __VA_ARGS__); \
	yyDisplayLog (szLogText); \
}
#elif defined _OS_NDK
#define YYLOGT(txt, fmt, args...) \
{ \
	LOGI ("Info T%08X %s L%d " fmt "\r\n", (int)pthread_self(), txt, __LINE__, ## args); \
}
#endif // _OS_WIN32
#else
#define YYLOGT(txt, fmt, ...)
#endif // _YYLOG_INFO

#ifdef _OS_WIN32
#define YYMSG(win, fmt, ...) \
{ \
	char	szMsgText[1024]; \
	sprintf (szMsgText, "YYMSG T%08X %s L%d " fmt "\r\n", GetCurrentThreadId(), m_szObjName, __LINE__, __VA_ARGS__); \
	yyDisplayMsg ((void *)win,szMsgText); \
}
#else
#define YYMSG(win, fmt, ...)
#endif // _OS_WIN32

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __yyLog_H__
