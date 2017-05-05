/*******************************************************************************
	File:		yyThumbnail.h

	Contains:	yy thumbnail info define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __yyThumbnail_h__
#define __yyThumbnail_h__

#include "yyType.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define	YYINFO_Get_Thumbnail	0X00000001
#define	YYINFO_Get_MediaInfo	0X00000002
#define	YYINFO_Get_NoBlack		0X00000010
#define	YYINFO_OPEN_ExtSource	0X00010000

// The thumbnail bitmap is 32 bits format.
struct YYINFO_Thumbnail {
	// indicate the get info type, Thumbnail, media info...
	int				nInfoType;
	// The following is get thumbnail parameters.
	HBITMAP			hThumbnail;
	int				nThumbWidth;
	int				nThumbHeight;
	// Where the video will get thumbnail.
	int				nPos;
	// keep the width and height or not.
	bool			bKeepAspectRatio;
	// if keep, which color will be filled.
	int				nBGColor;
	// how long time will try to get.
	int				nTryTime;

	// the bitmap buffer. The client can check is black or not.
	unsigned char *	pBmpBuff;

	// The media source info.
	int				nDuration;
	int				nBitrate;
	TCHAR			szVideoCodec[8];
	int				nVideoWidth;
	int				nVideoHeight;
	TCHAR			szAudioCodec[8];
	int				nSampleRate;
	int				nChannels;
	int				nVNum;
	int				nVDen;

	YYINFO_Thumbnail () 
	{
		nInfoType			= 3; // Get thumbnail and media info both.
		hThumbnail			= NULL;
		nThumbWidth			= 120;
		nThumbHeight		= 80;
		nPos				= 8000;
		bKeepAspectRatio	= false;
		nBGColor			= 0;
		nTryTime			= 10000;

		pBmpBuff			= NULL;

		nDuration			= 0;
		nBitrate			= 0;
		nVideoWidth			= 0;
		nVideoHeight		= 0;
		nSampleRate			= 0;
		nChannels			= 0;
//		_tcscpy (szVideoCodec, _T("None"));
//		_tcscpy (szAudioCodec, _T("None"));
	}
};



#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif // __yyThumbnail_h__
