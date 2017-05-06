/*******************************************************************************
	File:		CFFMpegVideoDec.h

	Contains:	The ffmpeg audio dec header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CFFMpegVideoDec_H__
#define __CFFMpegVideoDec_H__

#include "CBaseVideoDec.h"

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavformat/url.h>
#include <libswresample/swresample.h>

class CFFMpegVideoDec : public CBaseVideoDec
{
public:
	CFFMpegVideoDec(void * hInst);
	virtual ~CFFMpegVideoDec(void);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Flush (void);

	virtual int		SetBuff (YY_BUFFER * pBuff);
	virtual int		GetBuff (YY_BUFFER * pBuff);

protected:
    AVCodecContext *		m_pDecCtx;
    AVCodecContext *		m_pNewCtx;
	AVCodec *				m_pDecVideo;
	AVFrame *				m_pFrmVideo;

	AVPacket *				m_pPacket;
};

#endif // __CFFMpegVideoDec_H__
