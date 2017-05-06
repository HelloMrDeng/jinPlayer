/*******************************************************************************
	File:		CMultiPlayer.h

	Contains:	the mutex lock header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-10-24		Fenger			Create file

*******************************************************************************/
#ifndef __CMultiPlayer_H__
#define __CMultiPlayer_H__

#include "CBaseObject.h"
#include "CMediaEngine.h"
#include "CThreadWork.h"

#ifdef _OS_WIN32
#include "CWaveOutRnd.h"
#else
#define CWaveOutRnd void
#endif // _OS_WIN32

class CLessonInfo;

class CMultiPlayer : public CBaseObject
{
public:
    CMultiPlayer(void);
    virtual ~CMultiPlayer(void);

	virtual int		Open (CLessonInfo * pLesson);
	virtual int		Close (void);
	
	virtual int		Run (void);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		SetPos (int nPos);
	virtual int		GetPos (void);
	virtual int		GetDur (void);

	virtual int		SetVolume (int nVolume);
	virtual int		GetVolume (void);

	static void		NotifyEvent (void * pUserData, int nID, void * pV1);

protected:
	virtual int		CreateEng (CMediaEngine ** ppEng, TCHAR * pFile);
	virtual int		CheckRepeat (int nTime);
	virtual int		HandleEvent (int nID, void * pV1);

protected:
	CMediaEngine **		m_ppEng;
	int					m_nEngs;
	CMediaEngine *		m_pPrevEng;
	CLessonInfo *		m_pLsn;

	bool				m_bPrevPlay;
	int					m_nPrevComp;

	int					m_nOpenStat;
	int					m_nPlayComp;
	int					m_nSeekPos;
	int					m_nSeekComp;


	CThreadWork *		m_pWorkAudio;
	static	int			AudioPlayProc (void * pParam);
	virtual int			RendAudio (void);
	virtual int			WriteAudio (void);	
	virtual bool		ReleaseAudio (void);

	YY_BUFFER 			m_bufBoxAudio;
	YY_AUDIO_FORMAT		m_fmtAudio;
	YY_BUFFER 			m_bufAudioRnd;
	YY_BUFFER_CONVERT	m_dataConvertAudio;

	unsigned char *		m_pPCMBuff;
	int					m_nPCMSize;
	unsigned char *		m_pRndBuff;
	int					m_nRndSize;
	CWaveOutRnd *		m_pRndAudio;
};

#endif //__CMultiPlayer_H__
