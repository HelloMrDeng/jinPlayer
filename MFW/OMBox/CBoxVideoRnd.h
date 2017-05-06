/*******************************************************************************
	File:		CBoxVideoRnd.h

	Contains:	the video render box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxVideoRnd_H__
#define __CBoxVideoRnd_H__

#include "CBoxRender.h"
#include "CBaseVideoRnd.h"
#include "CSubtitleEngine.h"

#include "yyMediaPlayer.h"

class CBoxVideoRnd : public CBoxRender
{
public:
	CBoxVideoRnd(void * hInst);
	virtual ~CBoxVideoRnd(void);

	virtual int		SetRndType (YYRND_TYPE nRndType);
	virtual int		SetDisplay (void * hView, RECT * pRect);
	virtual int		UpdateDisp (void);
	virtual int		SetAspectRatio (int w, int h);
	virtual int		SetDDMode (YY_PLAY_DDMode nMode);
	virtual int		SetRotate (int nAngle);
	virtual int		DisableVideo (int nFlag);
	virtual int		SetSubTTEng (CSubtitleEngine * pSubTTEng);
	virtual int		SetExtDDraw (void * pDDExt);
	virtual void	SetYYDemoPlayer (bool bYYDemo) {m_byyDemoPlayer = bYYDemo;}

	virtual int		SetSource (CBoxBase * pSource);
	virtual int		RenderFrame (bool bInBox, bool bWait);

	virtual int		Start (CThreadWork * pWork);
	virtual int		Pause (void);
	virtual int		Stop (void);
	virtual int		SetPos (int nPos, bool bSeek);
	virtual int		SetZoom (RECT * pRect);
	virtual int		GetVideoData (YY_BUFFER ** ppVideoData);
	virtual int		SetVideoExtRnd (YY_DATACB * pDataCB);
	virtual int		DisableEraseBG (void);

	virtual RECT *			GetRenderRect (void );
	virtual CBaseClock * 	GetClock (void);
	virtual long long	 	GetDelayTime (void) {return m_llDelayTime;}

protected:
	virtual int		WaitRenderTime (YY_BUFFER * pBuff);
	virtual void	ResetMembers (void);
	virtual int		CreateRender (void);

protected:
	YYRND_TYPE			m_nRndType;
	void *				m_hView;
	YY_VIDEO_FORMAT		m_fmtVideo;
	RECT				m_rcView;
	YY_PLAY_DDMode		m_ddMode;
	int					m_nRotateAngle;
	int					m_nARW;
	int					m_nARH;
	YY_BUFFER			m_buffRender;
	CSubtitleEngine *	m_pSubTTEng;
	void *				m_pDDExt;
	YY_BUFFER			m_buffSubTT;
	YY_BUFFER *			m_pGetBuff;

	YY_DATACB *			m_pExtRnd;
	CBaseVideoRnd *		m_pRnd;
	bool				m_bNotifyFirstFrame;
	int					m_nDisableFlag;
	CFFMpegVideoRCC *	m_pRCC;
	
	long long			m_llDelayTime;
	long long			m_llVideoTime;
	long long			m_llSystemTime;
	

	int					m_nDroppedFrames;
	long long			m_llAVOffsetTime;
	long long			m_llLastBuffTime;
	long long			m_llFirstOffsetTime;

	bool				m_bSetThreadPriority;
	bool				m_byyDemoPlayer;
	int					m_nCPUNum;
	int					m_nScreenX;
	int					m_nScreenY;
	
	int					m_nDbgStartTime;
	int					m_nDbgTempTime;
};

#endif // __CBoxVideoRnd_H__
