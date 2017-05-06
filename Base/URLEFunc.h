/*******************************************************************************
	File:		URLEFunc.h

	Contains:	The RLE func header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-17		Fenger			Create file

*******************************************************************************/
#ifndef __URLEFunc_H__
#define __URLEFunc_H__

#include "yyType.h"

int yyRLEEncodeBmp (char * pBMP, int nW, int nH, char ** ppRLE);
int yyRLEDecodeBmp (char * pRLE, int nW, int nH, char * pBMP);

#endif // __URLEFunc_H__
