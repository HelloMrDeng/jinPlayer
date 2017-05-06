/*******************************************************************************
	File:		UFFMpegFunc.h

	Contains:	The base utility for ffmpeg header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __UFFMpegFunc_H__
#define __UFFMpegFunc_H__
#ifdef _OS_WIN32
#include "tchar.h"
#endif // _OS_WIN32

#include "string.h"
#include "stdio.h"

#pragma warning (disable : 4005)

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavformat/url.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>

extern  URLProtocol	g_ioFileExt;
extern	int			g_nPacketSize;
extern	int			g_nAVFrameSize;
extern	int			g_nAVSubTTSize;

void		yyInitFFMpeg (void);
void		yyFreeFFMpeg (void);

long long	yyBaseToTime (long long llBase, AVStream * pStream);
long long	yyTimeToBase (long long llTime, AVStream * pStream);

bool		yyAdjustVideoSize (int * pW, int * pH, int nW, int nH, int nNum, int nDen);

#endif // __UFFMpegFunc_H__
