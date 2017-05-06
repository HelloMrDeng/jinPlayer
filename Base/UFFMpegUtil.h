/*******************************************************************************
	File:		UFFMpegUtil.h

	Contains:	The base utility for ffmpeg header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __UFFMpegUtil_H__
#define __UFFMpegUtil_H__
#ifdef _OS_WIN32
#include "tchar.h"
#endif // _OS_WIN32

#include "stdint.h"
#include "stddef.h"

#include <libavcodec/avcodec.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus 

void video_encode_example(const char *filename, int codec_id);

#ifdef __cplusplus
}
#endif // __cplusplus 
#endif // __UFFMpegUtil_H__
