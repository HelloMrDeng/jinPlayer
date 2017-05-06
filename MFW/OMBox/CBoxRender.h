/*******************************************************************************
	File:		CBoxRender.h

	Contains:	the base render box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxRender_H__
#define __CBoxRender_H__

#include "CBoxBase.h"
#include "CDataConvert.h"

#include "CThreadWork.h"
#include "yyMediaPlayer.h"

class CSubtitleEngine;

class CBoxRender : public CBoxBase
{
public:
	typedef enum {
		YYRND_INIT = 0,
		YYRND_RUN,
		YYRND_PAUSE,
		YYRND_STOP,
	} YYRND_STATUS;

public:
	CBoxRender(void * hInst);
	virtual ~CBoxRender(void);

	virtual int		RenderFrame (bool bInBox, bool bWait);
	virtual int		Convert (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff, RECT * pZoom);

	virtual int		SetOtherRender (CBoxRender * pRnd);

	virtual int		Start (CThreadWork * pWork);
	virtual int		Pause (void);
	virtual int		Stop (void);
	virtual int		SetPos (int nPos, bool bSeek);
	virtual int		SetDataCB (YY_DATACB * pCB);

	virtual int		GetRndCount (void) {return m_nRndCount;}
	virtual bool	IsEOS (void);

	virtual CBaseClock * GetClock (void);

	virtual int		SetAudioRndType (YY_PLAY_ARType nType) {return YY_ERR_NONE;}
	virtual int		SetSpeed (float fSpeed) {return YY_ERR_NONE;}
	virtual int		SetVolume (int nVolume) {return YY_ERR_NONE;}
	virtual int		GetVolume (void) {return YY_ERR_NONE;}
	virtual int		SetRndType (YYRND_TYPE nRndType) {return YY_ERR_NONE;}
	virtual int		SetDisplay (void * hView, RECT * pRect) {return YY_ERR_NONE;}
	virtual int		UpdateDisp (void) {return YY_ERR_NONE;}
	virtual int		SetAspectRatio (int w, int h) {return YY_ERR_NONE;}
	virtual int		SetDDMode (YY_PLAY_DDMode nMode) {return YY_ERR_NONE;}
	virtual int		SetRotate (int nAngle) {return YY_ERR_NONE;}
	virtual int		DisableVideo (int nFlag) {return YY_ERR_NONE;}
	virtual int		SetSubTTEng (CSubtitleEngine * pSubTTEng) {return YY_ERR_NONE;}
	virtual int		SetExtDDraw (void * pDDExt) {return YY_ERR_NONE;}
	virtual void	SetYYDemoPlayer (bool bYYDemo) {return;}
	virtual int		SetZoom (RECT * pRect) {return YY_ERR_NONE;}
	virtual int		GetVideoData (YY_BUFFER ** ppVideoData) {return YY_ERR_NONE;}
	virtual int		SetVideoExtRnd (YY_DATACB * pDataCB) {return YY_ERR_NONE;}
	virtual int		DisableEraseBG (void) {return YY_ERR_NONE;}
	virtual RECT *		GetRenderRect (void ){return NULL;}
	virtual long long	GetDelayTime (void) {return 0;}

protected:
	virtual int		WaitRenderTime (YY_BUFFER * pBuff);
	static	int		RenderProc (void * pParam);
	virtual int		RenderLoop (void);
	virtual void	WaitForExitRender (unsigned int nMaxWaitTime);

protected:
	CMutexLock			m_mtRnd;
	YYMediaType			m_nMediaType;
	YYRND_STATUS		m_status;
	CBoxRender *		m_pOtherRnd;
	YY_DATACB *			m_pDataCB;

	bool				m_bEOS;
	int					m_nRndCount;

	CThreadWork *		m_pWorkThread;
	yyThreadHandle		m_hRndThread;
	bool				m_bInRender;
	bool				m_bSetThreadPriority;

	CBaseClock *		m_pDataClock;
	CDataConvert *		m_pDataCnvt;
};

#endif // __CBoxRender_H__
