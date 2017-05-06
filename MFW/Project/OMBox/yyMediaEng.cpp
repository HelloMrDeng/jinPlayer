/*******************************************************************************
	File:		yyMediaEng.cpp

	Contains:	yy media engine  implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-23		Fenger			Create file

*******************************************************************************/
#ifdef _OS_WIN32
#include <windows.h>
#else
#include "UJNIFunc.h"
#endif // _OS_WIN32

#include "yyMediaEng.h"

#include "CMediaManager.h"
#include "UFFMpegFunc.h"
#include "yyLog.h"


#ifdef _OS_WIN32
HINSTANCE	g_hInst = NULL;
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	g_hInst = (HINSTANCE) hModule;
    return TRUE;
}
#else
void *	g_hInst = NULL;
#endif // _OS_WIN32


int YY_API yyMediaInit (void ** phMedia, int nFlag, void * pParam)
{	
	yyInitFFMpeg ();

	CMediaManager * pMM = new CMediaManager (g_hInst);
	if (pMM == NULL)
		return YY_ERR_MEMORY;

	*phMedia = pMM;
	
//	yyGetOutputLatency (44100, 2);
	
	return YY_ERR_NONE;
}

int YY_API yyMediaCreate (void * hMeda, int nType, void ** ppMedia)
{	
	CMediaManager * pMM = (CMediaManager *)hMeda;
	if (pMM == NULL)
		return YY_ERR_ARG;

	return pMM->CreateMedia (nType, ppMedia);

//	return YY_ERR_NONE;
}

int YY_API yyMediaDestroy (void * hMeda, int nType, void * pMedia)
{
	CMediaManager * pMM = (CMediaManager *)hMeda;
	if (pMM == NULL)
		return YY_ERR_ARG;

	return pMM->DestroyMedia (nType, pMedia);

//	return YY_ERR_NONE;	
}

int YY_API yyMediaUninit (void * hMeda)
{
	CMediaManager * pMM = (CMediaManager *)hMeda;
	if (pMM == NULL)
		return YY_ERR_ARG;

	delete pMM;

	yyFreeFFMpeg ();

	return YY_ERR_NONE;
}
