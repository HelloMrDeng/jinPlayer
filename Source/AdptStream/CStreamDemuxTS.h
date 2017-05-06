/*******************************************************************************
	File:		CStreamDemuxTS.h

	Contains:	The stream data demux header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __CStreamDemuxTS_H__
#define __CStreamDemuxTS_H__

#include "voParser.h"
#include "voAdaptiveStreamParser.h"

#include "CStreamDemux.h"

class CStreamDemuxTS : public CStreamDemux
{
public:
    CStreamDemuxTS (void);
    virtual ~CStreamDemuxTS(void);
	
	virtual int		Demux(int nFlag, VO_PBYTE pBuff, VO_S32 nSize);

protected:
    static void		demuxCallback(VO_PARSER_OUTPUT_BUFFER* pData);

	virtual void	CheckBuffer (VO_PARSER_OUTPUT_BUFFER* pData);
	virtual void	SendBuffer (VO_PARSER_OUTPUT_BUFFER* pData);

protected:
	VO_PARSER_API			m_fAPI;
	VO_PTR					m_hDemux;
	VO_PARSER_INIT_INFO		m_initInfo;
	VO_PARSER_INPUT_BUFFER	m_inputData;

	YY_VIDEO_FORMAT *		m_pFmtVideo;
	YY_AUDIO_FORMAT *		m_pFmtAudio;

	YY_BUFFER				m_sBuffer;
};

#endif
