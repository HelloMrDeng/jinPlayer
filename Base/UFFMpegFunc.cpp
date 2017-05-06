/*******************************************************************************
	File:		UFFMpegFunc.cpp

	Contains:	The base utility for ffmpeg implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "UFFMpegFunc.h"

#include "CBaseUtils.h"
#include "yyneonFunc.h"

URLProtocol		g_ioFileExt;
yyNeonFunc		g_yyNeonFunc;
int				g_nPacketSize = 0;
int				g_nAVFrameSize = 0;
int				g_nAVSubTTSize = 0;

void yyInitFFMpeg (void)
{
	CBaseUtils::FillExtIOFunc (&g_ioFileExt);

#ifdef _CPU_NEON
	InitNeonFunc (&g_yyNeonFunc);
#endif // _CPU_NEON

	// register all formats and codecs
#ifdef _CPU_NEON
	av_register_all(&g_yyNeonFunc);
#else
	av_register_all(NULL);
#endif // _OS_WINCE

	avformat_network_init ();

	g_nPacketSize = sizeof (AVPacket);
	g_nAVFrameSize = sizeof (AVFrame);
	g_nAVSubTTSize = sizeof (AVSubtitle);
}

void yyFreeFFMpeg (void)
{
#ifdef _CPU_NEON	
	UninitNeonFunc (&g_yyNeonFunc);
#endif // _CPU_NEON

	CBaseUtils::FreeExtIOFunc (&g_ioFileExt);
}

long long yyBaseToTime (long long llBase, AVStream * pStream)
{
	long long llTime = llBase * 1000 * pStream->time_base.num / pStream->time_base.den;

	return llTime;
}

long long yyTimeToBase (long long llTime, AVStream * pStream)
{
	if (pStream->time_base.num == 0)
		return llTime;

	long long llBase = (llTime * pStream->time_base.den) / (pStream->time_base.num * 1000);

	return llBase;
}

bool yyAdjustVideoSize (int * pW, int * pH, int nW, int nH, int nNum, int nDen)
{
	int nRndW = *pW;
	int nRndH = *pH;
	if ((nNum == 0 || nNum == 1) && (nDen == 1 || nDen == 0))
	{
		if (nW * nRndH >= nH * nRndW)
			nRndH = nRndW * nH / nW;
		else //if (nW * nRndH < nH * nRndW)
			nRndW = nRndH * nW / nH;
	}
	else
	{
		if (nDen == 0)
			nDen = 1;
		nW = nW * nNum / nDen;
		if (nW * nRndH >= nH * nRndW)
			nRndH = nRndW * nH / nW;
		else //if (nW * nRndH < nH * nRndW)
			nRndW = nRndH * nW / nH;
	}
	*pW = nRndW;
	*pH = nRndH;

	return true;
}

