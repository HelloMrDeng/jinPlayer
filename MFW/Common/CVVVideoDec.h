/*******************************************************************************
	File:		CVVVideoDec.h

	Contains:	the vo video dec wrap header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __CVVVideoDec_H__
#define __CVVVideoDec_H__

#include "CBaseVideoDec.h"

#include <libavformat/avformat.h>
#include "voVideo.h"

class CVVVideoDec : public CBaseVideoDec
{
public:
	CVVVideoDec(void * hInst);
	virtual ~CVVVideoDec(void);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Flush (void);

	virtual int		SetBuff (YY_BUFFER * pBuff);
	virtual int		GetBuff (YY_BUFFER * pBuff);

protected:
	virtual int		SetHeadData (YY_VIDEO_FORMAT * pFmt);
	virtual int		SetDecParam (YY_VIDEO_FORMAT * pFmt);

	virtual int		SetInputData (VO_CODECBUFFER * pInData);
	virtual int		GetOutputData (void);

	virtual VO_VIDEO_FRAMETYPE	GetFrameType (VO_CODECBUFFER * pBuffer);

protected:
	VO_VIDEO_DECAPI		m_fAPI;
	VO_HANDLE			m_hDec;
	int					m_nCodecType;
	bool				m_bDecoded;

	VO_CODECBUFFER		m_Input;
	VO_VIDEO_BUFFER		m_Output;
	VO_VIDEO_OUTPUTINFO	m_OutputInfo;

	AVPacket *			m_pPacket;
};

#endif // __CVVVideoDec_H__
