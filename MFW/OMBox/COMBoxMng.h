/*******************************************************************************
	File:		COMBoxMng.h

	Contains:	The media engine header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifndef __COMBoxMng_H__
#define __COMBoxMng_H__

#include "CBaseObject.h"
#include "CMutexLock.h"

#include "CNodeList.h"
#include "CBoxVideoRnd.h"
#include "CBoxAudioRnd.h"
#include "CBoxExtRnd.h"
#include "CBoxVideoDec.H"
#include "CBoxSource.h"
#include "CBoxMonitor.h"

#include "CMediaThumb.h"
#include "CSubtitleEngine.h"
#include "CThreadWork.h"
#include "CLicenseCheck.h"

#ifdef _OS_WINCE
#include <ddraw_ce60.h>
#endif // _OS_WINCE

#define	YY_TASK_OPEN	0X70000001
#define	YY_TASK_SEEK	0X70000002

typedef struct YY_EVENT
{
	int		nID;
	void *	pV1;
	void *	pV2;
} YY_EVENT;

class COMBoxMng : public CBaseObject
{
public:
	COMBoxMng(void * hInst, YYM_Player * pMPlayer);
	virtual ~COMBoxMng(void);

	virtual void	SetNotifyFunc (YYMediaNotifyEvent pFunc, void * pUserData);
	virtual void	SetDisplay (void * hView, YYRND_TYPE nRndType);
	virtual int		UpdateView (RECT * rcView);

	virtual int		Open (const TCHAR * pSource, int nFlag);
	virtual int		Close (void);
	
	virtual int		Start (void);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		SetPos (int nPos);
	virtual int		GetPos (void);
	virtual int		GetDuration (void);
	YYPLAY_STATUS	GetStatus (void) {return m_stsPlay;}

	virtual int		SetVolume (int nVolume);
	virtual int		GetVolume (void);

	virtual void *	GetThumb (const TCHAR * pFile, YYINFO_Thumbnail * pThumbInfo);

	virtual int		GetMediaInfo (TCHAR * pInfo, int nSize);

	int				GetBoxCount (void);
	CBoxBase *		GetBox (int nIndex);
	CBaseClock *	GetClock (void) {return m_pClock;}

	virtual int 	SetParam (int nID, void * pParam);
	virtual int		GetParam (int nID, void * pParam);

public:
	static void		NotifyEvent (void * pUserData, int nID, void * pV1);

protected:
	virtual	int		DoOpen (const TCHAR * pSource, int nFlag);
	virtual int		DoSeek (const int nPos);
	virtual int		OpenSubTitle (void);

	virtual int		ReadData (YY_BUFFER * pData);
	virtual int		ConvertData (YY_BUFFER * pSource, YY_BUFFER * pTarget, RECT * pZoom);
	virtual int		WaitAudioRender (int nWaitTime, bool bCheckStatus);
	virtual int		DrawFirstVideoFrame (int nWaitTime);

	virtual void	PrepareClose (void);
	virtual int		CheckOpenStatus (int nWaitTime);

	virtual void	PushTask (int nID, void * pV1);
	virtual void	HandleEvent (int nID, void * pV1);
	virtual void	CloseEvent (void);

protected:
	void *						m_hInst;
	YYM_Player *				m_pMPlayer;
	bool						m_byyDemoPlayer;

	CMutexLock					m_mtFunc;
	CLicenseCheck *				m_pLcsChk;
	YYPLAY_STATUS				m_stsPlay;

	int							m_nOpenFlag;
	TCHAR						m_szSource[1024];
	int							m_nDur;
	bool						m_bOpening;
	bool						m_bOpenCancel;
	bool						m_bForceClosed;
	bool						m_bClosed;

// video render parameters
	void *						m_hView;
	RECT						m_rcView;
	YY_PLAY_VDecMode			m_vdMode;
	TCHAR						m_szWndText[256];
	YYRND_TYPE					m_nRndType;
	bool						m_bForceGDI;
	YY_PLAY_ARType				m_arType;
	YY_PLAY_DDMode				m_ddMode;
	int							m_nDisVideoLevel;
	int							m_nAudioVolume;
	YY_DATACB					m_cbData;

// Seek parameter
	int							m_nSeekPos;
	bool						m_bSeeking;
	int							m_nLastSeekTime;
	YY_PLAY_SeekMode			m_nSeekMode;

// the box and clock parameres
	CObjectList<CBoxBase>		m_lstBox;
	CBoxSource *				m_pBoxSource;
	CBoxRender *				m_pRndAudio;
	CBoxRender *				m_pRndVideo;
	CBoxBase *					m_pDecVideo;
	CBoxMonitor *				m_pBoxMonitor;
	CBaseClock *				m_pClock;
	CBaseClock *				m_pClockMng;


// the thumb parameters
	CMediaThumb	*				m_pThumb;

// define the subtitle parameters
	CSubtitleEngine *			m_pSubTT;
	int							m_nSubTTEnable;
	int							m_nSubTTColor;
	int							m_nSubTTSize;
	void *						m_hSubTTFont;
	void *						m_hSubTTView;
	YY_DATACB *					m_pSubExtRnd;
	YYSUB_ExtDraw *				m_pSubExtDraw;

#ifdef _OS_WINCE
	_WINCE_60::IDirectDraw *	m_pDDExt;
	HMODULE						m_hDDDll;
#endif // _OS_WINCE

// define the event parameters
protected:
	static	int					NotifyProc (void * pParam);
	virtual int					NotifyLoop (void);

	YYMediaNotifyEvent			m_fNotifyEvent;
	void *						m_pUserData;

	CMutexLock					m_mtEvent;
	yyThreadHandle				m_hThreadNotifyEvent;
	bool						m_bStopNotifyEvent;
	int							m_nCurEventID;
	CObjectList<YY_EVENT>		m_lstFreeEvent;
	CObjectList<YY_EVENT>		m_lstFullEvent;

	CThreadWork *				m_pWorkAudio;
	static	int					AudioPlayProc (void * pParam);
	CThreadWork *				m_pWorkVideo;
	static	int					VideoPlayProc (void * pParam);

	int							m_nDbgMemSize;
	int							m_nDbgMemStep;
};

#endif // __COMBoxMng_H__
