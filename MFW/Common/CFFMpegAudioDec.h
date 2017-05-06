/*******************************************************************************
	File:		CFFMpegAudioDec.h

	Contains:	The ffmpeg audio dec header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CFFMpegAudioDec_H__
#define __CFFMpegAudioDec_H__

#include "CBaseAudioDec.h"

#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavformat/url.h>
#include <libswresample/swresample.h>

class CFFMpegAudioDec : public CBaseAudioDec
{
public:
	CFFMpegAudioDec(void * hInst);
	virtual ~CFFMpegAudioDec(void);

	virtual int		Init (YY_AUDIO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Flush (void);

	virtual int		SetBuff (YY_BUFFER * pBuff);
	virtual int		GetBuff (YY_BUFFER * pBuff);

protected:
    AVCodecContext *		m_pDecCtx;
    AVCodecContext *		m_pNewCtx;
	AVCodec *				m_pDecAudio;
	AVFrame *				m_pFrmAudio;

	AVPacket *				m_pPacket;
	AVPacket				m_pktData;
	unsigned char *			m_pPktData;
	int						m_nPktSize;

	AVPacket				m_pktBuff;

};

#endif // __CFFMpegAudioDec_H__
