/*******************************************************************************
	File:		CBaseAudioDec.h

	Contains:	The audio dec header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseAudioDec_H__
#define __CBaseAudioDec_H__

#include "CBaseObject.h"
#include "CMutexLock.h"

#include "yyData.h"

class CBaseAudioDec : public CBaseObject
{
public:
	CBaseAudioDec(void * hInst);
	virtual ~CBaseAudioDec(void);

	virtual int		Init (YY_AUDIO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		SetBuff (YY_BUFFER * pBuff);
	virtual int		GetBuff (YY_BUFFER * pBuff);

	virtual int		Start (void);
	virtual int		Pause (void);
	virtual int		Stop (void);
	virtual int		Flush (void);

	virtual YY_AUDIO_FORMAT *	GetAudioFormat (void) {return &m_fmtAudio;}

protected:
	void *				m_hInst;
	YY_AUDIO_FORMAT		m_fmtAudio;

	CMutexLock			m_mtBuffer;
	unsigned int		m_uBuffFlag;
};

#endif // __CBaseAudioDec_H__
