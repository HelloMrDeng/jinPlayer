/*******************************************************************************
	File:		yyConfig.h

	Contains:	yy player type define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-12		Fenger			Create file

*******************************************************************************/
#ifndef __yyConfig_H__
#define __yyConfig_H__

// Disable all drop frame method
#define YYCFG_CHECK_PERFORMANCE			0

// If the playback speed greater than 1.6, 
// the audio whill be disable.
#ifdef _OS_WINPC
#define	YYCFG_DISABLE_AUDIO_SPEED		8.0
#else
#define	YYCFG_DISABLE_AUDIO_SPEED		1.6
#endif // _OS_WINPC

// The number of first list number 
#define YYCFG_VIDEO_LIST_FIRST			30

// The max number of video packet list 
#define YYCFG_SUBTT_LIST_MAX			10000

// The max number of video packet list 
#define YYCFG_VIDEO_LIST_MAX			30000

// The max number of audio packet list 
#define YYCFG_AUDIO_LIST_MAX			50000

// Read the next key frame time (ms)
// pPacket = GetPacket (YY_MEDIA_Video, pBuff->llTime + YYCFG_READ_NEXTKEY_TIME);
// if (pPacket->pts < llPlayTime)
//     AVPacket * pKeyPacket = GetNextKeyFrame (llPlayTime);
#define YYCFG_READ_NEXTKEY_TIME			50

// Disable deblock delay time
#define YYCFG_DDEBLOCK_TIME				20

// Skip B frame delay time
#define YYCFG_SKIP_BFRAME_TIME			30

// Detect the performance audio dec, video dec and rnd
#define YYCFG_DETECT_PERFORMANCE		0


// If delay 200 ms, drop the 1 / 2 video frame
#define YYCFG_RENDER_DROP_TIME			200

// Video render start after audio render number
#define YYCFG_WAIT_AUDIO_RNDNUM			3

// Video render will sleep 30ms if larger than error time
#define YYCFG_ERROR_AV_TIME				10000

// wait audio offset time
// while (m_pAudioRnd->GetPlayTime (false) + YYCFG_WAIT_AUDIO_OFFSET_TIME < llTime)
#define YYCFG_WAIT_AUDIO_OFFSET_TIME	100

#endif // __yyConfig_H__
