/*******************************************************************************
	File:		UFileFunc.h

	Contains:	The base utility for file operation header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-17		Fenger			Create file

*******************************************************************************/
#ifndef __UFileFunc_H__
#define __UFileFunc_H__
#ifdef _OS_WIN32
#include <windows.h>
#endif // _OS_WIN32

#include "yyType.h"

#ifdef _OS_WIN32
#define yyFile		HANDLE
#elif defined _OS_LINUX
#define	yyFile		int
#endif // _OS_WIN32

#define	YYFILE_READ		1
#define	YYFILE_WRITE	2

#define	YYFILE_BEGIN	1
#define	YYFILE_CUR		2
#define	YYFILE_END		3

yyFile		yyFileOpen (TCHAR * pFile, int nFlag);
int			yyFileRead (yyFile hFile, unsigned char * pBuff, int nSize);
int			yyFileWrite (yyFile hFile, unsigned char * pBuff, int nSize);
long long	yyFileSeek (yyFile hFile, long long llPos, int nFlag);
long long	yyFileSize (yyFile hFile);
int			yyFileClose (yyFile hFile);

#endif // __UFileFunc_H__
