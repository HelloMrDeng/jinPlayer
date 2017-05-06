/*******************************************************************************
	File:		CBaseVideoEnc.h

	Contains:	The Video encoder header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-06-03		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseVideoEnc_H__
#define __CBaseVideoEnc_H__

#include "CBaseObject.h"
#include "CMutexLock.h"

class CBaseVideoEnc : public CBaseObject
{
public:
	CBaseVideoEnc(void * hInst);
	virtual ~CBaseVideoEnc(void);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Encode (YY_BUFFER * pIn, YY_BUFFER * pOut);
	virtual int		Flush (void);

protected:
	void *				m_hInst;
	YY_VIDEO_FORMAT		m_fmtVideo;
};

#endif // __CBaseVideoEnc_H__
