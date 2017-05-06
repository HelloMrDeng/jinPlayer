/*******************************************************************************
	File:		CBoxAudioDec.h

	Contains:	the audio dec box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxAudioDec_H__
#define __CBoxAudioDec_H__

#include "CBoxBase.h"
#include "CBaseAudioDec.h"

class CBoxAudioDec : public CBoxBase
{
public:
	CBoxAudioDec(void * hInst);
	virtual ~CBoxAudioDec(void);

	virtual int		SetSource (CBoxBase * pSource);
	virtual int		SetPos (int nPos, bool bSeek);
	virtual int		ReadBuffer (YY_BUFFER * pBuffer, bool bWait);

protected:
	YY_AUDIO_FORMAT		m_fmtAudio;
	CBaseAudioDec *		m_pDec;
};

#endif // __CBoxAudioDec_H__
