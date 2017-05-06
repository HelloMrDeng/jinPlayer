/*******************************************************************************
	File:		CBoxAudioRndExt.h

	Contains:	the Audio render box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxAudioRndExt_H__
#define __CBoxAudioRndExt_H__

#include "CBoxAudioRnd.h"
#include "CDataConvert.h"

class CBoxAudioRndExt : public CBoxAudioRnd
{
public:
	CBoxAudioRndExt(void * hInst);
	virtual ~CBoxAudioRndExt(void);

	virtual int		SetAudioRndType (YY_PLAY_ARType nType);
	virtual int		SetSource (CBoxBase * pSource);

	virtual int		ReadBuffer (YY_BUFFER * pBuffer, bool bWait);
	virtual int		Convert (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff, RECT * pZoom);

	virtual int		Start (CThreadWork * pWork);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		SetSpeed (float fSpeed);

	virtual CBaseClock * GetClock (void);

protected:
	long long		m_llDbgPrevTime;
};

#endif // __CBoxAudioRndExt_H__
