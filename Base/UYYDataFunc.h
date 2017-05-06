/*******************************************************************************
	File:		UYYDataFunc.h

	Contains:	The base utility for library header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __UYYDataFunc_H__
#define __UYYDataFunc_H__
#ifdef _OS_WIN32
#include <windows.h>
#endif // _OS_WIN32

#include "yyData.h"
#include <libavformat/avformat.h>

int	yyDataCloneVideoFormat (YY_VIDEO_FORMAT * pTarget, YY_VIDEO_FORMAT * pSource);
int	yyDataCloneAudioFormat (YY_AUDIO_FORMAT * pTarget, YY_AUDIO_FORMAT * pSource);

void yyDataDeleteVideoFormat (YY_VIDEO_FORMAT ** ppFmt);
void yyDataDeleteAudioFormat (YY_AUDIO_FORMAT ** ppFmt);

bool yyDataCloneBuffer (YY_BUFFER * pTarget, YY_BUFFER * pSource);
bool yyDataResetBuffer (YY_BUFFER * pBuffer, bool bDel);

bool yyDataCloneVideoBuff (YY_BUFFER * pTarget, YY_BUFFER * pSource);
bool yyDataResetVideoBuff (YY_BUFFER * pBuffer, bool bDel);

bool yyDataAVFrameToVideoBuff (AVFrame * pAVFrame, YY_VIDEO_BUFF * pVideoBuff);

#endif // __UYYDataFunc_H__
