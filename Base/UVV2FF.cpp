/*******************************************************************************
	File:		UVV2FF.cpp

	Contains:	The utility for library implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "UVV2FF.h"

#include <libavformat/avformat.h>
#include "voVideo.h"
#include "voAudio.h"

int	yyVV2FFAudioCodecID (int nVVID)
{
	int nFFID = 0;
	switch (nVVID)
	{
	case VO_AUDIO_CodingAAC:
		nFFID = AV_CODEC_ID_AAC;
		break;

	case VO_AUDIO_CodingAC3:
		nFFID = AV_CODEC_ID_AC3;
		break;

	case VO_AUDIO_CodingMP3:
		nFFID = AV_CODEC_ID_MP3;
		break;

	default:
		break;
	}

	return nFFID;
}

int	yyVV2FFVideoCodecID (int nVVID)
{
	int nFFID = 0;
	switch (nVVID)
	{
	case VO_VIDEO_CodingH264:
		nFFID = AV_CODEC_ID_H264;
		break;

	case VO_VIDEO_CodingH265:
		nFFID = AV_CODEC_ID_H265;
		break;

	case VO_VIDEO_CodingMPEG4:
		nFFID = AV_CODEC_ID_MPEG4;
		break;

	case VO_VIDEO_CodingMPEG2:
		nFFID = AV_CODEC_ID_MPEG2VIDEO;
		break;

	default:
		break;
	}

	return nFFID;
}
