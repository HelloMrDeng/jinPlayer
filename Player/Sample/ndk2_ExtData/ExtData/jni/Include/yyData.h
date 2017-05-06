/*******************************************************************************
	File:		yyData.h

	Contains:	yy player type define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __yyData_H__
#define __yyData_H__

#ifdef _OS_WIN32
#include "windows.h"
#endif // _OS_WIN32

#include "yyType.h"

#define YY_PLAY_DATA_BASE			0X01200000

// Set the clock offset time
// The parameter should int
#define	YYPLAY_PID_Clock_OffTime	YY_PLAY_DATA_BASE + 1

// Set the Data call back function
// The parameter should YY_DATACB *
#define	YYPLAY_PID_DataCB			YY_PLAY_DATA_BASE + 2

// Read the Data from player
// The parameter should YY_BUFFER *
#define	YYPLAY_PID_ReadData			YY_PLAY_DATA_BASE + 11

// convert the Data in player
// The parameter should YY_BUFFER_CONVERT *
#define	YYPLAY_PID_ConvertData		YY_PLAY_DATA_BASE + 12

// Get the audio format. 
// The parameter should be YY_AUDIO_FORMAT **
#define	YYPLAY_PID_Fmt_Audio		YY_PLAY_DATA_BASE + 21

// Get the video format. 
// The parameter should be YY_VIDEO_FORMAT **
#define	YYPLAY_PID_Fmt_Video		YY_PLAY_DATA_BASE + 22

#ifndef _OS_WIN32
#define HDC			void *
#define HANDLE		void *
#define HWND		void *
#define HFONT		void *
#define	_T 
#endif // _OS_WIN32

// The flag of open source
#define	YY_OPEN_SRC_VIDEO		0X00010000		// Open source video only
#define	YY_OPEN_SRC_AUDIO		0X00020000		// Open source audio only
#define	YY_OPEN_SRC_BOX			0X00040000		// Use source box from external
#define	YY_OPEN_SRC_READ		0X00080000		// Read data from external. YY_READ_EXT_DATA *
#define	YY_OPEN_RNDA_EXT		0X00100000		// audio render with ext thread
#define	YY_OPEN_RNDA_CB			0X00200000		// audio render with call back
#define	YY_OPEN_RNDV_EXT		0X00400000		// video render with ext thread
#define	YY_OPEN_RNDV_CB			0X00800000		// video render with call back

// What is the source from.
typedef enum {
	YY_SOURCE_FF		= 1,
	YY_SOURCE_VV		= 2,
	YY_SOURCE_YY		= 3,
	YY_SOURCE_EX		= 4,
	YY_SOURCE_MAX		= 0X7FFFFFFF
}YYSourceType;

// the stream media type
typedef enum {
	YY_MEDIA_Data		= 0,
	YY_MEDIA_Video		= 11,
	YY_MEDIA_Audio		= 12,
	YY_MEDIA_SubTitle	= 13,
	YY_MEDIA_MAX		= 0X7FFFFFFF
}YYMediaType;

// the video raw data format
typedef enum  {
	YY_VDT_YUV420_P		= 0,
	YY_VDT_NV12			= 1,
	YY_VDT_RGB565		= 11,
	YY_VDT_RGB24		= 12,
	YY_VDT_RGBA			= 13,
	YY_VDT_ARGB			= 14,
	YY_VDT_MAX			= 0X7FFFFFFF
} YYVIDEO_TYPE;

/**
 * General audio format
 */
typedef struct
{
	YYSourceType	nSourceType;	/*!< Souce Type*/
	int				nCodecID;		/*!< codec id*/
	int				nSampleRate;	/*!< Sample rate */
	int				nChannels;		/*!< Channel count */
	int				nBits;			/*!< Bits per sample 8, 16 */
	unsigned char *	pHeadData;		/*!< head data*/
	int				nHeadSize;		/*!< head data size*/
	void *			pPrivateData;	/*!< Private data pointer*/
	int				nPrivateFlag;	/*!< Private data flag*/
} YY_AUDIO_FORMAT;

/**
 * General video Size
 */
typedef struct
{
	YYSourceType	nSourceType;	/*!< Souce Type*/
	int				nCodecID;		/*!< codec id*/
	int				nWidth;			/*!< Video Width */
	int				nHeight;		/*!< Video Height */
	int				nNum;			/*!< aspect ratio numerator */
	int				nDen;			/*!< aspect ratio denominator */
	int				nFrameTime;		/*!< frame time*/
	unsigned char *	pHeadData;		/*!< head data*/
	int				nHeadSize;		/*!< head data size*/
	void *			pPrivateData;	/*!< Private data pointer*/
	int				nPrivateFlag;	/*!< Private data flag*/
} YY_VIDEO_FORMAT;

/**
 * General video buffer structure
 */
typedef struct
{
	unsigned char *		pBuff[3];		/*!< Video buffer */
	int					nStride[3];     /*!< Video buffer stride */
	YYVIDEO_TYPE		nType;			/*!< Video buffer type */
	int					nWidth;			/*!< Video width*/
	int					nHeight;		/*!< Video height*/
} YY_VIDEO_BUFF;

// the buffer status
#define	YYBUFF_NEW_POS			0X00000001
#define	YYBUFF_NEW_FORMAT		0X00000002
#define	YYBUFF_EOS				0X00000004
#define	YYBUFF_DROP_FRAME		0X00000008

// for dec
#define	YYBUFF_DEC_DISABLE		0X00000100
#define	YYBUFF_DEC_DISA_DEBLOCK	0X00000200
#define	YYBUFF_DEC_SKIP_BFRAME	0X00000400

// for render
#define	YYBUFF_RND_DISABLE		0X00001000

// The data type of the buffer
#define	YYBUFF_TYPE_DATA		0X00010000
#define	YYBUFF_TYPE_PACKET		0X00020000
#define	YYBUFF_TYPE_AVFrame		0X00040000
#define	YYBUFF_TYPE_VIDEO		0X00080000
#define	YYBUFF_TYPE_PPOINTER	0X000A0000


/**
 * General data buffer.
 */
typedef struct {
	YYMediaType			nType;			/*!< buffer type    */
	unsigned int		uFlag;			/*!< buffer flags    */
	unsigned char *		pBuff;			/*!< Buffer pointer */
	unsigned int		uSize;			/*!< Buffer size in byte */
	long long			llTime;			/*!< The time of the buffer */
	long long			llDelay;		/*!< The time of delay */
	void *				pFormat;		/*!< new format type info   */
	int					nValue;			/*!< the value depend on the flag   */
	void *				pData;			/*!< the reverse data point  */
	int					nDataType;		/*!< the reverse data type  */
} YY_BUFFER;

/**
 * Call back function of player notify event
 */
typedef int	 (YY_API * YYMediaDataCB) (void * pUser, YY_BUFFER * pData);

/**
 * Read the buffer from ext source
 */
typedef int	 (YY_API * YYMediaReadExtData) (void * pUser, YY_BUFFER * pData);

/**
 * call back function structure
 */
typedef struct
{
	YYMediaDataCB	funcCB;			/*!< call function pointer*/
	void *			userData;		/*!< user data*/
} YY_DATACB;

/**
 * call back function structure
 */
typedef struct
{
	YY_BUFFER *		pSource;		/*!< Source buffer pointer*/
	YY_BUFFER *		pTarget;		/*!< Target buffer pointer*/
} YY_BUFFER_CONVERT;

// Read ext data type
typedef enum {
	YY_EXTDATA_Mux			= 1,
	YY_EXTDATA_Raw			= 2,
	YY_EXTDATA_MAX			= 0X7FFFFFFF
}YYExtDataType;

/**
 * Read ext data function structure
 */
typedef struct
{
	void *					pUser;		/*!< User data */
	YYMediaReadExtData 		pRead;		/*!< read ext data function */
	YYExtDataType			nType;		/*!< ext data type */
	long long				llSize;		/*!< ext data total size or duration */
	TCHAR					szName[64]; /*!< file ext name /yyExtData.ts */
	YY_VIDEO_FORMAT *		pFmtVideo;	/*!< the video format info */
	YY_AUDIO_FORMAT *		pFmtAudio;  /*!< the audio format info */
} YY_READ_EXT_DATA;

#endif // __yyData_H__
