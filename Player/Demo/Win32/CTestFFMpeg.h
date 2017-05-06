/*******************************************************************************
	File:		CTestFFMpeg.h

	Contains:	The ext render player header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-21		Fenger			Create file

*******************************************************************************/
#ifndef __CTestFFMpeg_H__
#define __CTestFFMpeg_H__

#include "yyData.h"

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavformat/url.h>

class CTestFFMpeg
{
public:
	CTestFFMpeg(void * hInst);
	virtual ~CTestFFMpeg(void);
	
	void	Test (void);

	void	TestReadOnly (void);
	void	TestFFMpegSource (void);

protected:
	void *					m_hInst;
	AVFormatContext *		m_pFmtCtx;
	URLProtocol				m_ioFileExt;
	TCHAR					m_szFileName[1024];

};

#endif // __CTestFFMpeg_H__
