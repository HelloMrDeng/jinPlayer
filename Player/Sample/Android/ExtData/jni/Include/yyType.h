/*******************************************************************************
	File:		yyType.h

	Contains:	yy player type define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __yyType_H__
#define __yyType_H__

#ifdef _OS_WIN32
#define YY_API __cdecl
#define YY_CBI __stdcall
#else
#define YY_API 
#define YY_CBI 
#endif // _OS_WIN32

/* define the error ID */
#define YY_ERR_NONE						0x00000000
#define YY_ERR_FINISH					0x00000001
#define YY_ERR_RETRY					0x00000002
#define YY_ERR_FORMAT					0x00000003
#define YY_ERR_FAILED					0x80000001
#define YY_ERR_MEMORY					0x80000002
#define YY_ERR_IMPLEMENT				0x80000003
#define YY_ERR_ARG						0x80000004
#define YY_ERR_SOURCE					0x80000005
#define YY_ERR_AUDIO					0x80000006
#define YY_ERR_VIDEO					0x80000007
#define YY_ERR_STATUS					0x80000008
#define YY_ERR_PARAMID					0x80000009
#define YY_ERR_LICENSE					0x8000000a
#define YY_ERR_UNSUPPORT				0x8000000b

/* define the notify event ID */
#define YY_EV_Open_Complete				0x00000001
#define YY_EV_Open_Failed				0x00000002
#define YY_EV_Seek_Complete				0x00000003
#define YY_EV_Seek_Failed				0x00000004
#define YY_EV_Play_Complete				0x00000005
#define YY_EV_Draw_FirstFrame			0x00000010

#define YY_EV_Err_Source				0x00000101
#define YY_EV_Err_AudioDec				0x00000102
#define YY_EV_Err_VideoDec				0x00000103
#define YY_EV_Err_AudioRnd				0x00000104
#define YY_EV_Err_VideoRnd				0x00000105
#define YY_EV_War_TimeStamp				0x00000201

#define YY_PLAY_BASE					0X01000000
#define YY_MTV_BASE						0X02000000
#define YY_REC_BASE						0X03000000

typedef enum  {
	YY_VRND_GDI			= 0,
	YY_VRND_DDRAW		= 1,
	YY_VRND_DDCE6		= 2,
	YY_VRND_MAX			= 0X7FFFFFFF
} YYRND_TYPE;

typedef enum {
	YY_PLAY_Init	= 0,
	YY_PLAY_Open	= 1,
	YY_PLAY_Run		= 2,
	YY_PLAY_Pause	= 3,
	YY_PLAY_Stop	= 4,
	YY_PLAY_MAX		= 0X7FFFFFFF
} YYPLAY_STATUS;


/**
 * General rect structure
 */
typedef struct
{
	int				left;	
	int				top;	
	int				right;	
	int				bottom;	
} YYRECT;

#define TCHAR		char
#define HBITMAP		void *
#define RECT		YYRECT

/**
 * Call back function of player notify event
 */
typedef void (YY_API * YYMediaNotifyEvent) (void * pUserData, int nID, void * pValue1);

#endif // __yyType_H__
