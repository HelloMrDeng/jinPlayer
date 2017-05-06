/*******************************************************************************
	File:		CFFMpegTest.h

	Contains:	the ffmpeg source header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CFFMpegTest_H__
#define __CFFMpegTest_H__
#include "stdio.h"
#include "CBaseSource.h"

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavformat/id3v1.h>
#include <libavformat/id3v2.h>
#include <libavformat/url.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>

#include "CMutexLock.h"
#include "CNodeList.h"

class CFFMpegTest : public CBaseSource
{
public:
	CFFMpegTest(void * hInst);
	virtual ~CFFMpegTest(void);

	virtual int		Open (const TCHAR * pSource, int nType);
	virtual int		Close (void);
	virtual int		ForceClose (void);

	virtual int		Start (void);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		SetStreamPlay (YYMediaType nType, int nStream);

	virtual int		ReadData (YY_BUFFER * pBuff);
	virtual int		SetPos (long long llPos);

	virtual int		GetMediaInfo (TCHAR * pInfo, int nSize);
	virtual int		FillMediaInfo (YYINFO_Thumbnail * pInfo);

	virtual int 	SetParam (int nID, void * pParam);
	virtual int		GetParam (int nID, void * pParam);

protected:
	AVPacket *		GetPacket (YYMediaType nType, long long llPlayTime);
	int				ReadPacket (void);
	void			UpdatePacketTime (AVPacket * pPacket);
	long long		GetVideoBuffTime (void);
	AVPacket *		GetNextKeyFrame (long long llTime);
	bool			ReleasePacket (bool bAudio, bool bVideo);

	bool			ForceCheckDuration (void);

	char *			GetLineText (char * pLine);
	virtual void	ResetParam (int nLevel);

protected:
	CMutexLock					m_mtFile;
	AVFormatContext *			m_pFmtCtx;
	AVFormatContext *			m_pTmpCtx;
	long long					m_llFileStartTime;

	// Define audio stream parameters
	int							m_nIdxAudio;
	AVStream *					m_pStmAudio;

	// Define video stream parameters
	int							m_nIdxVideo;
	AVStream *					m_pStmVideo;

	// Data and Buffer parameters
	CMutexLock					m_mtPacket;
	CObjectList<AVPacket>		m_lstMedia;
	CObjectList<AVPacket>		m_lstAudio;
	CObjectList<AVPacket>		m_lstVideo;
	AVPacket *					m_pPktAudio;
	AVPacket *					m_pPktVideo;

	// video 
	int							m_nVideoReadCount;
	long long					m_llVideoTime;
	int							m_nKeyFrameNum;
	int							m_nVideoDropFrames;
	long long					m_nVideoFrameTime;
	long long					m_llVideoLastTime;

	// Audio
	int							m_nAudioReadCount;
	long long					m_llAudioTime;
	bool						m_bNewAudioStream;

	// for switch audio track
	long long					m_llAudioTrackPos;
	long long					m_llVideoTrackPos;

	char						m_szLineText[256];

	// for debug parameter
	long long					m_llDbgPrevTime;

protected:
	static	int			ReadProc (void * pParam);
	virtual int			ReadLoop (void);
	
	CMutexLock			m_mtRead;
	void *				m_hReadThread;
	bool				m_bReadStop;
	bool				m_bReadPaused;
};

#endif // __CFFMpegTest_H__
