/*******************************************************************************
	File:		UFileFormat.cpp

	Contains:	The utility for file operation implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-17		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "UFileFormat.h"
#include "UStringFunc.h"
#include "UFileFunc.h"

#ifdef _OS_WIN32
#pragma warning (disable : 4996)
#endif // _OS_WIN32

TCHAR yyff_szVideoExt[] = (_T(".avi, .divx, .mp4, .m4v, .mov, .mkv, .3gp, .3g2, .rmvb, .rm, .real, .rv, .asf, .wmv, .flv, .ts, .mpeg, .mpg, .vob, .gif, "));
TCHAR yyff_szAudioExt[] = (_T(".mp3, .mp2, .aac, .amr, .ogg, .wav, .ac3, .awb, .ape, .flac, .wma, .ra, .m4a, "));
TCHAR yyff_szImageExt[] = (_T(".jpg, .jpeg, "));//.bmp, .png, "));

YYMediaType	yyffGetType (const TCHAR * pSource, int nFlag)
{
	TCHAR szSource[1024];
	if ((nFlag & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ)
		_tcscpy (szSource, ((YY_READ_EXT_DATA *)pSource)->szName);
	else
		_tcscpy (szSource, pSource);
	TCHAR * pExt = _tcsrchr (szSource, _T('.'));
	if (pExt == NULL)
		return YY_MEDIA_Data;
	int nExtLen = _tcslen (pExt) * sizeof (TCHAR);
	char * pExtChar = (char *)pExt;
	for (int i = 0; i < nExtLen; i++)
	{
		if (*(pExtChar + i) >= 'A' && *(pExtChar + i) <= 'Z')
			*(pExtChar + i) += 'a' - 'A';
	}
	if (_tcsstr (yyff_szVideoExt, pExt) != NULL)
		return YY_MEDIA_Video;
	else if (_tcsstr (yyff_szAudioExt, pExt) != NULL)
		return YY_MEDIA_Audio;
//	else if (_tcsstr (yyff_szImageExt, pExt) != NULL)
//		return YY_MEDIA_Image;

	return YY_MEDIA_Data;
}

bool yyffIsStreaming (const TCHAR * pSource, int nFlag)
{
#ifdef _OS_WINCE
	return false;
#endif // _OS_WINCE

	if ((nFlag & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ || (nFlag & YY_OPEN_SRC_BOX) == YY_OPEN_SRC_BOX )
		return false;

	TCHAR * pM3U8 = NULL;
	TCHAR	szURL[2048];
	memset (szURL, 0, sizeof (szURL));
	_tcscpy (szURL, pSource);
#ifdef _UNICODE
	_tcslwr (szURL);
#else
	yyStringChange (szURL, false);
#endif // _UNICODE
	
	pM3U8 = _tcsstr (szURL, _T(".m3u8"));
	if (pM3U8 == NULL)
		return false;

	int nExtLen = _tcslen (pM3U8);
	if (_tcslen (pM3U8) == 5)
		return true;

	if (pM3U8[5] == _T('?'))
		return true;

	return false;
}
