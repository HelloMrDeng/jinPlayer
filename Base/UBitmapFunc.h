/*******************************************************************************
	File:		UBitmapFunc.h

	Contains:	The bitmap utility header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-27		Fenger			Create file

*******************************************************************************/
#ifndef __UBitmapFunc_H__
#define __UBitmapFunc_H__
#ifdef _OS_WIN32
#include <windows.h>
#endif // _OS_WIN32

#include "yyType.h"

HBITMAP yyBmpCreate (HDC hDC, int nW, int nH, LPBYTE * ppBmpBuff, int nColor);
HBITMAP yyBmpClone (HBITMAP hBmpSrc, RECT * pRect, LPBYTE * ppBmpBuff);
void	yyBmpCheck (HWND hWnd, HBITMAP hBmp);
bool	yyBmpSave (HBITMAP hBmp, LPBYTE pBuff, TCHAR * pFile);

#endif // __UBitmapFunc_H__
