/*******************************************************************************
	File:		yySubtitle.h

	Contains:	yy subtitle info define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-03		Fenger			Create file

*******************************************************************************/
#ifndef __yySubtitle_h__
#define __yySubtitle_h__

#include "yyType.h"
#include "string.h"
#ifdef _OS_WIN32
#include "tchar.h"
#endif // _OS_WIN32

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// The Subtitle bitmap is 32 bits format.
struct YYSUB_ExtDraw {
	HDC			hDC;
	RECT *		pRect;
	long long	llTime;
	bool		bOverLay;
};

#define YY_PLAY_SUB_BASE			0X01400000

// Set call back the subtitle text. 
// The parameter should be YY_DATACB *
#define	YYPLAY_PID_Sub_CallBack		YY_PLAY_SUB_BASE + 1

// Set ext subtitle draw info. 
// The parameter should be YYSUB_ExtDraw *
#define	YYPLAY_PID_Sub_ExtDraw		YY_PLAY_SUB_BASE + 2

// Get subtitle char set
// The parameter should be int *. 1 UTF-8, 2 GB2312
#define	YYPLAY_PID_Sub_Charset		YY_PLAY_SUB_BASE + 10

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif // __yySubtitle_h__
