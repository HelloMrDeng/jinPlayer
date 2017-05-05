/*******************************************************************************
	File:		yyMediaPlayer.h

	Contains:	yy media engine define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __yyMediaPlayer_h__
#define __yyMediaPlayer_h__

#include "yyType.h"
#include "yyThumbnail.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
	TCHAR			szVideoCodec[8];
	int				nWidth;
	int				nHeight;
	TCHAR			szAudioCodec[8];
	int				nSampleRate;
	int				nChannels;
	YYPLAY_STATUS	sStatus;
} YYINFO_Source;


/**
 * the yy media player interface 
 * The parameter void * hPlayer should be YYM_Player which was created from yyMediaCreate
 */
typedef struct
{
	// Define the version of the player.
	int				nVersion;
	// The player engine handle.
	void *			hPlayer;
	// The Source Info
	YYINFO_Source	sInfo;
	// This function must call before open.
	int				(* SetView)		(void * hPlayer, void * hWnd, YYRND_TYPE nType);
	// Open the source. The format should be 0, the player will auto detect the format.
	int 			(* Open)		(void * hPlayer, const TCHAR * pFile, int nFormat);
	// Close the source to release the resource
	int 			(* Close)		(void * hPlayer);
	int 			(* Run)			(void * hPlayer);
	int 			(* Pause)		(void * hPlayer);
	int 			(* Stop)		(void * hPlayer);
	YYPLAY_STATUS	(* GetStatus)	(void * hPlayer);
	// The time is ms.
	int 			(* GetDur)		(void * hPlayer);
	int 			(* GetPos)		(void * hPlayer);
	int 			(* SetPos)		(void * hPlayer, int nPos);
	// The volume should be 0 - 100. 
	int				(* SetVolume)	(void * hPlayer, int nVolume);
	int				(* GetVolume)	(void * hPlayer);
	// It will recieve the message if you set the notify function.
	int				(* SetNotify)	(void * hPlayer, YYMediaNotifyEvent pFunc, void * pUserData);
	// if the rcView is null, it will redraw the window. or it change the draw area with rcView.
	int				(* UpdateView)	(void * hPlayer, RECT * rcView);
	// return the thumbnail at pos. This is independ function. it will return null if can not get in nTryTime (ms).
	HBITMAP			(* GetThumb)	(void * hPlayer, const TCHAR * pFile, YYINFO_Thumbnail * pThumbInfo);
	// return media file info. The nSize at least 2048.
	int				(* MediaInfo)	(void * hPlayer, TCHAR * pInfo, int nSize);
	// for extend function later.
	int 			(* GetParam)	(void * hPlayer, int nID, void * pParam);
	int 			(* SetParam)	(void * hPlayer, int nID, void * pParam);
} YYM_Player;


// The following message for advance user
// Set the video aspect ratio. 
// The parameter should be YYPLAY_ARInfo *.
#define	YYPLAY_PID_AspectRatio		YY_PLAY_BASE + 1

// Set the play speed.
// The parameter should be float *. it is 0.2 - 32.0
#define	YYPLAY_PID_Speed			YY_PLAY_BASE + 2

// Set the video area should be display. It is not support now.
// The parameter should RECT *. The left and top must be 4 X.
// return YY_ERR_FAILED it doesn't support zoom
#define	YYPLAY_PID_VideoZoomIn		YY_PLAY_BASE + 3

// Set it to disable video. Playback audio only.
// The parameter should int. 1 disable render, 2 decoder, 0, enable
#define	YYPLAY_PID_Disable_Video	YY_PLAY_BASE + 4

// Get Video render area.
// The parameter should be RECT *.
#define	YYPLAY_PID_RenderArea		YY_PLAY_BASE + 5

// Get Audio track number.
// The parameter should be int *.
#define	YYPLAY_PID_AudioTrackNum	YY_PLAY_BASE + 6

// Set Audio track play. The parameter should be int (0, ... Num - 1).
// Get Audio track play. The parameter should be int *.
#define	YYPLAY_PID_AudioTrackPlay	YY_PLAY_BASE + 7

// Set the audio render type.
// The parameter should YY_PLAY_ARType value.
// It must set it before open the file.
#define	YYPLAY_PID_Aduio_RndType	YY_PLAY_BASE + 8

// Set the video render direct draw mode.
// The parameter should YY_PLAY_DDMode value.
#define	YYPLAY_PID_DDMode			YY_PLAY_BASE + 10

// Set the seek mode. Key frame or any position
// The parameter should YY_PLAY_SeekMode value.
#define	YYPLAY_PID_SeekMode			YY_PLAY_BASE + 11

// Set and Get Subtitle enable or disable.
// The parameter should, Get is int * , Set is int. 
// The Value 0 disable, 1 enable. The default is disabled.
#define	YYPLAY_PID_SubTitle			YY_PLAY_BASE + 12

// Set and Get Subtitle font size, the default is 28.
// The parameter should, Get is int * , Set is int. Value 12 - 128.
#define	YYPLAY_PID_SubTT_Size		YY_PLAY_BASE + 13

// Set and Get Subtitle font size, the default is RGB (255, 255, 255) White.
// The parameter should, Get is int * , Set is int. 
// The Value RGB(1, 1, 1, 1) - RGB (255, 255, 255).
#define	YYPLAY_PID_SubTT_Color		YY_PLAY_BASE + 14

// Set Subtitle font handle.
// The parameter should be HFONT. 
// The font handle should be keep before destroy the media engine.
// Set the NULL to destroy the handle.
#define	YYPLAY_PID_SubTT_Font		YY_PLAY_BASE + 15

// Set Subtitle window handle.
// The parameter should be HWND. 
#define	YYPLAY_PID_SubTT_View		YY_PLAY_BASE + 16

// Set video disable erase background.
// The parameter no. 
#define	YYPLAY_PID_DisableEraseBG	YY_PLAY_BASE + 19

// Set it to cancel get thumbnail function immediately.
// no need parameter
#define	YYPLAY_PID_Cancel_GetThumb	YY_PLAY_BASE + 20

// Set the video rotate angle. 
// The parameter should be 90, 180, 270
#define	YYPLAY_PID_Rotate			YY_PLAY_BASE + 21

// Set the video decoder mode.
// The parameter should YY_PLAY_VDMode value.
#define	YYPLAY_PID_VDMode			YY_PLAY_BASE + 30

// Set the pdp file name.
// The parameter should TCHAR * value.
#define	YYPLAY_PID_PDPFile			YY_PLAY_BASE + 40

// prepare to close when opening or seeking. 
// The parameter should be 0.
// It is safe to call this before stop or close
#define	YYPLAY_PID_Prepare_Close	YY_PLAY_BASE + 500

// the video aspect ratio. 
typedef struct {
	int		nWidth;
	int		nHeight;
} YYPLAY_ARInfo;

// the audio render type
typedef enum {
	YY_ART_WAVE_MAPPER				= 1,	// Default type.
	YY_ART_WAVE_FORMAT_QUERY		= 2,	
	YY_ART_WAVE_ALLOWSYNC			= 3,	
	YY_ART_WAVE_MAPPED				= 4,	
	YY_ART_WAVE_FORMAT_DIRECT		= 5,	
	YY_ART_WAVE_FORMAT_DIRECT_QUERY	= 6,	
	YY_ART_MAX						= 0X7FFFFFFF,
} YY_PLAY_ARType;

// The WinCE video render type.
typedef enum {
	YY_DDM_Memory	= 1,	// video memory mode. It can't over window on video.
	YY_DDM_Overlay	= 2,	// Overlay mode. It can over window on video.
	YY_DDM_MAX		= 0X7FFFFFFF,
} YY_PLAY_DDMode;

// The seek mode
typedef enum {
	YY_SEEK_KeyFrame	= 1,	// Seek to the nearest key key frame
	YY_SEEK_AnyPosition	= 2,	// Seek to any position.
	YY_SEEK_MAX		= 0X7FFFFFFF,
} YY_PLAY_SeekMode;

// The WinCE video render type.
typedef enum {
	YY_VDM_Auto			= 0XFF,		// use video decoder auto
	YY_VDM_Soft			= 0X01,		// User software video dec	
	YY_VDM_IOMX			= 0X02,		// User IOMX video dec
	YY_VDM_MediaCodec	= 0X04,		// User MediaCodec video dec
	YY_VDM_MAX		= 0X7FFFFFFF,
} YY_PLAY_VDecMode;

// The video disable type
#define	YY_PLAY_VideoEnable				0
#define	YY_PLAY_VideoDisable_Render		1
#define	YY_PLAY_VideoDisable_Decoder	2

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif // __yyMediaPlayer_h__
