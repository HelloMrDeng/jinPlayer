/*******************************************************************************
	File:		CFFMpegVideoRCC.h

	Contains:	The ffmpeg audio dec header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CFFMpegVideoRCC_H__
#define __CFFMpegVideoRCC_H__

#include "CBaseObject.h"

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

#include <libswscale/swscale.h>

#include "voCCRRR.h"

class CFFMpegVideoRCC : public CBaseObject
{
public:
	CFFMpegVideoRCC(void * hInst);
	virtual ~CFFMpegVideoRCC(void);

	virtual int		ConvertBuff (YY_BUFFER * iBuff, YY_VIDEO_BUFF * oBuff, RECT * pZoom = NULL);
protected:
	virtual int		vvConvertVideo (AVFrame * pFrmVideo, YY_VIDEO_BUFF * pOutBuff);

protected:
	SwsContext *		m_pSwsCtx;
	AVFrame				m_frmVideo;

	AVPixelFormat		m_nFormat;
	int					m_nVideoW;
	int					m_nVideoH;

	int					m_nOutputType;
	int					m_nOutputW;
	int					m_nOutputH;

	VO_VIDEO_CCRRRAPI	m_voVCCRR;
	VO_HANDLE			m_hVCCRR;
	VO_VIDEO_BUFFER		m_inBuff;
	VO_VIDEO_BUFFER		m_outBuff;

	RECT				m_rcZoom;

};

#endif // __CFFMpegVideoRCC_H__
