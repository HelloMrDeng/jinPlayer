/*******************************************************************************
	File:		CFFMpegVideoEnc.h

	Contains:	The ffmpeg audio dec header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CFFMpegVideoEnc_H__
#define __CFFMpegVideoEnc_H__

#include "CBaseVideoDec.h"

#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

class CFFMpegVideoEnc : public CBaseVideoDec
{
public:
	CFFMpegVideoEnc(void * hInst);
	virtual ~CFFMpegVideoEnc(void);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Encode (YY_BUFFER * pIn, YY_BUFFER * pOut);
	virtual int		Flush (void);

protected:
    AVCodecContext *		m_pEncCtx;
	AVCodec *				m_pEncVideo;
	AVFrame 				m_frmVideo;
	AVPacket				m_pktData;
};

#endif // __CFFMpegVideoEnc_H__
