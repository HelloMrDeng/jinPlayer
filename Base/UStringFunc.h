/*******************************************************************************
	File:		UStringFunc.h

	Contains:	The base utility for string header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __UStringFunc_H__
#define __UStringFunc_H__
#include "yyType.h"

#include "string.h"
#ifdef _OS_WIN32
#include "tchar.h"
#endif // _OS_WIN32

#ifndef _OS_WIN32
#define _tcscpy		strcpy
#define _tcslen		strlen
#define	_tcscat		strcat
#define	_tcsstr		strstr
#define	_tcsrchr	strrchr
#define	_tcscmp		strcmp
#define _stscanf	sscanf
#define _tcsncmp	strncmp
#define	_tcschr		strchr
#define	_tcsncat	strncat
#define _tcsncpy	strncpy
#define _tcslwr		strlwr
#define _T
#endif // _OS_WIN32

typedef enum {
	YY_PROT_FILE	= 0,
	YY_PROT_HTTP	= 1,
	YY_PROT_RTSP	= 2,
	YY_PROT_MMS		= 3,
	YY_PROT_FTP		= 4,
	YY_PROT_TCP		= 5,
	YY_PROT_UDP		= 6,
	YY_PROT_PDP		= 7,
	YY_PROT_MAX		= 0X7FFFFFFF
} YY_PROT_TYPE;

bool			yyChangeFileExtName (char * pFileName, bool bUP);
bool			yyStringChange (char * pString, bool bUP);
bool			yyCheckTextUTF8 (char * pText);
YY_PROT_TYPE	yyGetProtType (const TCHAR * pSource);
void			yyURLSplit (char *proto, int proto_size,
							  char *authorization, int authorization_size,
							  char *hostname, int hostname_size,
							  int *port_ptr,
							  char *path, int path_size,
							  const char *url);
int				yyFindInfoTag (char *arg, int arg_size, const char *tag1, const char *info);
int				yyTextReadLine (TCHAR * pText, int nTextLen, TCHAR * pLine, int nSize);

#endif // __UStringFunc_H__
