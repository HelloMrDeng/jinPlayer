/*******************************************************************************
	File:		UFileFormat.h

	Contains:	The base utility for file operation header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-17		Fenger			Create file

*******************************************************************************/
#ifndef __UFileFormat_H__
#define __UFileFormat_H__
#ifdef _OS_WIN32
#include <windows.h>
#endif // _OS_WIN32

#include "yyType.h"
#include "yyData.h"

YYMediaType	yyffGetType (const TCHAR * pSource, int nFlag);
bool yyffIsStreaming (const TCHAR * pSource, int nFlag);

#endif // __UFileFormat_H__
