/*******************************************************************************
	File:		ULibFunc.h

	Contains:	The base utility for library header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __ULibFunc_H__
#define __ULibFunc_H__
#ifdef _OS_WIN32
#include <windows.h>
#endif // _OS_WIN32

#include "yyType.h"

#ifdef _OS_WIN32
#define yyLibHandle HMODULE
#else
#define yyLibHandle void *
#endif // _OS_WIN32

void * 		yyLibLoad (const TCHAR * pLibName, int nFlag);
void *		yyLibGetAddr (void * hLib, const char * pFuncName, int nFlag);
int			yyLibFree (void * hLib, int nFlag);

#endif // __ULibFunc_H__
