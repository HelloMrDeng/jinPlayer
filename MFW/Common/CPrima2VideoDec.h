/*******************************************************************************
	File:		CPrima2VideoDec.h

	Contains:	the vo video dec wrap header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __CPrima2VideoDec_H__
#define __CPrima2VideoDec_H__
#include "CBaseVideoDec.h"
#include <libavformat/avformat.h>

class COMXDecoder;

class CPrima2VideoDec : public CBaseVideoDec
{
public:
	CPrima2VideoDec(void * hInst);
	virtual ~CPrima2VideoDec(void);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Flush (void);

	virtual int		SetBuff (YY_BUFFER * pBuff);
	virtual int		GetBuff (YY_BUFFER * pBuff);

protected:


protected:
	COMXDecoder *		m_pDec;
	AVPacket *			m_pPacket;

};

#endif // __CPrima2VideoDec_H__
