/*******************************************************************************
	File:		CBoxAudioRnd.h

	Contains:	the Audio render box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxAudioRnd_H__
#define __CBoxAudioRnd_H__

#include "CBoxRender.h"

#include "CBaseAudioRnd.h"

class CBoxAudioRnd : public CBoxRender
{
public:
	CBoxAudioRnd(void * hInst);
	virtual ~CBoxAudioRnd(void);

	virtual int		SetAudioRndType (YY_PLAY_ARType nType);
	virtual int		SetSource (CBoxBase * pSource);
	virtual int		RenderFrame (bool bInBox, bool bWait);

	virtual int		Start (CThreadWork * pWork);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		SetPos (int nPos, bool bSeek);
	virtual int		SetSpeed (float fSpeed);

	virtual int		SetVolume (int nVolume);
	virtual int		GetVolume (void);

	virtual CBaseClock *	GetClock (void);
	virtual int				GetRndCount (void);

protected:
	virtual void	ResetMembers (void);

protected:
	YY_PLAY_ARType		m_nRndType;
	YY_AUDIO_FORMAT		m_fmtAudio;
	CBaseAudioRnd *		m_pRnd;
	int					m_nVolume;
	float				m_fSpeed;

	int					m_nNewFmtCount;
	unsigned char *		m_pNewFmtBuffer;
	int					m_nNewFmtBuffSize;
	YY_AUDIO_FORMAT		m_fmtAudioNew;
};

#endif // __CBoxAudioRnd_H__
