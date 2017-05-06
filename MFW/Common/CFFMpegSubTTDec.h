/*******************************************************************************
	File:		CFFMpegSubTTDec.h

	Contains:	The ffmpeg audio dec header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CFFMpegSubTTDec_H__
#define __CFFMpegSubTTDec_H__

#include "CBaseObject.h"

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavformat/url.h>
#include <libswresample/swresample.h>

class CFFMpegSubTTDec : public CBaseObject
{
public:
	CFFMpegSubTTDec(void * hInst);
	virtual ~CFFMpegSubTTDec(void);

	virtual int		Init (YY_SUBTT_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Flush (void);

	virtual int		SetBuff (YY_BUFFER * pBuff);
	virtual int		GetBuff (YY_BUFFER * pBuff);

protected:
    AVCodecContext *		m_pDecCtx;
	AVCodec *				m_pDecSubTT;
	AVSubtitle *			m_pFrmSubTT;
	AVPacket *				m_pPacket;
};

#endif // __CFFMpegSubTTDec_H__
