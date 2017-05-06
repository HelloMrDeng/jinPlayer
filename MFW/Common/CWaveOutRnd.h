/*******************************************************************************
	File:		CWaveOutRnd.h

	Contains:	The audio dec header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CWaveOutRnd_H__
#define __CWaveOutRnd_H__

#include "windows.h"
#include "mmSystem.h"

#include "CBaseAudioRnd.h"

#include "CNodeList.h"
#include "CMutexLock.h"

#define MAXINPUTBUFFERS		3

class CWaveOutRnd : public CBaseAudioRnd
{
public:
	typedef struct WAVEHDRINFO
	{
		long long	llTime;
		void *		pData;
	} WAVEHDRINFO;

	static bool CALLBACK YYWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, 
													DWORD dwParam1, DWORD dwParam2);
public:
	CWaveOutRnd(void * hInst, bool bAudioOnly);
	virtual ~CWaveOutRnd(void);

	virtual int		SetType (YY_PLAY_ARType nType);
	virtual int		Init (YY_AUDIO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Start (void);
	virtual int		Stop (void);
	virtual int		Flush (void);

	virtual int		Render (YY_BUFFER * pBuff);

	virtual int		SetVolume (int nVolume);
	virtual int		GetVolume (void);

protected:
	virtual bool	InitDevice (YY_AUDIO_FORMAT * pFmt);
	virtual bool	AllocBuffer (void);
	virtual bool	ReleaseBuffer (void);
	virtual int		UpdateFormat (YY_AUDIO_FORMAT * pFmt);
	virtual void	FadeIn (void * pData, int nSize);

	virtual int		WaitAllBufferDone (int nWaitTime);

	virtual bool	AudioDone (WAVEHDR * pWaveHeader);

protected:
	CMutexLock				m_mtWaveOut;
	HWAVEOUT				m_hWaveOut;
	UINT					m_nDeviceID;
	WAVEFORMATEX 			m_wavFormat;
	DWORD					m_dwVolume;
	bool					m_bReseted;
	bool					m_bFirstRnd;
	int						m_nWriteCount;

	CMutexLock				m_mtList;
	CObjectList<WAVEHDR>	m_lstFull;
	CObjectList<WAVEHDR>	m_lstFree;
	unsigned int			m_nBufSize;
	long long				m_llBuffTime;
	long long				m_llRendTime;
};

#endif // __CWaveOutRnd_H__
