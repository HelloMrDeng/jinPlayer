/*******************************************************************************
	File:		UVV2FF.h

	Contains:	The base utility for library header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __UVV2FF_H__
#define __UVV2FF_H__
#ifdef _OS_WIN32
#include <windows.h>
#endif // _OS_WIN32

#include "yyType.h"

int		yyVV2FFAudioCodecID (int nVVID);
int		yyVV2FFVideoCodecID (int nVVID);

#endif // __UVV2FF_H__
