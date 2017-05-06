/*******************************************************************************
	File:		CVVVideoH265Dec.h

	Contains:	the vo video dec wrap header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __CVVVideoH265Dec_H__
#define __CVVVideoH265Dec_H__

#include "CVVVideoDec.h"
#include "voH265.h"

class CVVVideoH265Dec : public CVVVideoDec
{
public:
	CVVVideoH265Dec(void * hInst);
	virtual ~CVVVideoH265Dec(void);

	virtual int		Uninit (void);
	virtual int		Flush (void);
	virtual int		GetBuff (YY_BUFFER * pBuff);

protected:
	virtual int		SetHeadData (YY_VIDEO_FORMAT * pFmt);
	virtual int		SetDecParam (YY_VIDEO_FORMAT * pFmt);

	virtual int		SetInputData (VO_CODECBUFFER * pInData);
	virtual int		GetOutputData (void);

	virtual VO_VIDEO_FRAMETYPE	GetFrameType (VO_CODECBUFFER * pBuffer) {return VO_VIDEO_FRAME_NULL;}

protected:
	VO_CODECBUFFER	m_dataInput;
	bool			m_bEOS;

	int				m_nDgbIndex;
	long long		m_llDbgLastTime;
};

#endif // __CVVVideoH265Dec_H__
