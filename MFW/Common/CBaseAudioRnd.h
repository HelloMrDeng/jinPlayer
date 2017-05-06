/*******************************************************************************
	File:		CBaseAudioRnd.h

	Contains:	The audio render header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseAudioRnd_H__
#define __CBaseAudioRnd_H__

#include "CBaseObject.h"
#include "CDataConvert.h"
#include "CBaseClock.h"

#include "yyMediaPlayer.h"

class CBaseAudioRnd : public CBaseObject
{
public:
	CBaseAudioRnd(void * hInst, bool bAudioOnly);
	virtual ~CBaseAudioRnd(void);

	virtual int		SetType (YY_PLAY_ARType nType);
	virtual int		Init (YY_AUDIO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Start (void);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		Flush (void);
	virtual int		SetSpeed (float fSpeed);

	virtual int		Render (YY_BUFFER * pBuff);

	virtual int		SetVolume (int nVolume);
	virtual int		GetVolume (void);

	virtual int		GetRndCount (void) {return m_nRndCount;}
	virtual CBaseClock * GetClock (void);
	virtual YY_AUDIO_FORMAT * GetFormat (void) {return &m_fmtAudio;}

protected:
	virtual int		GetRenderBuff (YY_BUFFER * pBuff, unsigned char ** ppOutBuff);

protected:
	CMutexLock			m_mtRnd;
	void *				m_hInst;
	bool				m_bAudioOnly;
	YY_PLAY_ARType		m_nType;
	YY_AUDIO_FORMAT		m_fmtAudio;
	CBaseClock *		m_pClock;

	CDataConvert		m_dataConvert;
	YY_BUFFER			m_buffPCM;

	unsigned char *		m_pPCMData;
	int					m_nPCMSize;
	unsigned char *		m_pPCMBuff;
	int					m_nPCMLen;

	float				m_fSpeed;
	long long			m_llPrevTime;
	int					m_nSizeBySec;

	int					m_nRndCount;
};

#endif // __CBaseAudioRnd_H__
