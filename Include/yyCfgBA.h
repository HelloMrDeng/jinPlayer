/*******************************************************************************
	File:		yyCfgBA.h

	Contains:	yy player type define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-12		Fenger			Create file

*******************************************************************************/
#ifndef __yyCfgBA_H__
#define __yyCfgBA_H__

// The max buffer in out buffer down new chunk data (ms)
#define YYCFG_BA_BUFFER_TIME			120000

// The buffer size read from IO one time.
#define YYCFG_BA_READONE_SIZE			18800

// The time when start to check bitrate after playback
#define YYCFG_BA_STARTCHECK_TIME		2000

// The last duration time to calculate the download speed.
#define YYCFG_BA_LASTTIME_DOWNLOAD		10000

// The max number of download info in list
#define YYCFG_BA_MAXNUM_DLINFO			4096

// The step time will adjust buffer timestamp
#define	YYCFG_BA_BUFFERSTEP_TIME		120000

#endif // __yyCfgBA_H__
