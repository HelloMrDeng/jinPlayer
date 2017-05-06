/*******************************************************************************
	File:		CBoxExtData.h

	Contains:	the source box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-12-23		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxExtData_H__
#define __CBoxExtData_H__

#include "CBoxSource.h"

class CBoxExtData : public CBoxSource
{
public:
	CBoxExtData(void * hInst);
	virtual ~CBoxExtData(void);

	virtual int		OpenSource (const TCHAR * pSource, int nFlag);

	virtual int		GetDuration (void);

	virtual int		ReadBuffer (YY_BUFFER * pBuffer, bool bWait);

protected:
	YY_READ_EXT_DATA	m_extData;

	YY_AUDIO_FORMAT 	m_fmtAudio;
	YY_VIDEO_FORMAT 	m_fmtVideo;

};

#endif // __CBoxExtData_H__
