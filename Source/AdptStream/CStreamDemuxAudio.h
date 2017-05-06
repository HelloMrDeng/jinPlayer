/*******************************************************************************
	File:		CStreamDemuxAudio.h

	Contains:	The stream data demux header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __CStreamDemuxAudio_H__
#define __CStreamDemuxAudio_H__

#include "voParser.h"
#include "voAdaptiveStreamParser.h"

#include "CStreamDemux.h"

class CStreamDemuxAudio : public CStreamDemux
{
public:
    CStreamDemuxAudio (void);
    virtual ~CStreamDemuxAudio(void);
	
	virtual int		Demux(int nFlag, VO_PBYTE pBuff, VO_S32 nSize);

protected:

protected:
	YY_AUDIO_FORMAT *		m_pFmtAudio;
	YY_BUFFER				m_sBuffer;
};

#endif
