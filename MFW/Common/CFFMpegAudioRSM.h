/*******************************************************************************
	File:		CFFMpegAudioRSM.h

	Contains:	The ffmpeg audio dec header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CFFMpegAudioRSM_H__
#define __CFFMpegAudioRSM_H__

#include "CBaseObject.h"

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

class CFFMpegAudioRSM : public CBaseObject
{
public:
	CFFMpegAudioRSM(void * hInst);
	virtual ~CFFMpegAudioRSM(void);

	virtual int Init (int oLayout, int oFormat, int oSampleRate, 
						int iLayout, int iFormat, int iSampleRate);
	virtual int	Uninit (void);

	virtual int	ConvertData (unsigned char ** ppInBuff, int nInSamples,
								unsigned char ** ppOutBuff, int nOutSize);

protected:
	SwrContext *	m_pSwrCtx;
	int				m_oLayout;
	int				m_oFormat;
	int				m_oSampleRate;
	int				m_iLayout;
	int				m_iFormat;
	int				m_iSampleRate;
};

#endif // __CFFMpegAudioRSM_H__
