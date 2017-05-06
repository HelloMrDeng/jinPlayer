/*******************************************************************************
	File:		CBaseVideoRnd.h

	Contains:	The Video render header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseVideoRnd_H__
#define __CBaseVideoRnd_H__

#include "CBaseObject.h"
#include "yyMediaPlayer.h"
#include "CBaseClock.h"
#include "CFFMpegVideoRCC.h"

class CBaseVideoRnd : public CBaseObject
{
public:
	CBaseVideoRnd(void * hInst);
	virtual ~CBaseVideoRnd(void);

	virtual int		SetDisplay (void * hView, RECT * pRect);
	virtual int		UpdateDisp (void);
	virtual int		SetAspectRatio (int w, int h);
	virtual int		SetDDMode (YY_PLAY_DDMode nMode);
	virtual int		SetRotate (int nAngle);
	virtual int		SetSubTTEng (void * pSubTTEng);
	virtual int		SetExtDDraw (void * pDDExt);
	virtual int		DisableEraseBG (void);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Start (void);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		SetZoom (RECT * pRect);
	virtual int		EnableKeyColor (bool bEnable);

	virtual int		Render (YY_BUFFER * pBuff);

	virtual RECT *	GetRenderRect (void ) {return &m_rcRender;}
	virtual int		GetRndCount (void) {return m_nRndCount;}
	virtual CBaseClock * GetClock (void);

protected:
	virtual bool	UpdateRenderSize (void);
	virtual bool	CheckZoomRect (void);

	virtual bool	UpdateBackGround (void);
	int				GetRectW (RECT * pRect) {return pRect->right - pRect->left;}
	int				GetRectH (RECT * pRect) {return pRect->bottom - pRect->top;}

	int				OverLogo (void);

protected:
	void *				m_hInst;
	YYRND_TYPE			m_nType;
	CBaseClock *		m_pClock;
	int					m_nRotate;

	void *				m_pSubTTEng;

	CMutexLock			m_mtDraw;
	void *				m_hView;
	RECT				m_rcVideo;
	RECT				m_rcZoom;
	RECT				m_rcView;
	RECT				m_rcRender;
	RECT				m_rcWindow;
	int					m_nARWidth;
	int					m_nARHeight;
	int					m_nMaxWidth;
	int					m_nMaxHeight;

	YY_VIDEO_FORMAT		m_fmtVideo;
	YY_VIDEO_BUFF		m_bufVideo;

	CFFMpegVideoRCC *	m_pVideoRCC;

	bool				m_bUpdateView;
	bool				m_bDisableEraseBG;
	int					m_nRndCount;

	YY_VIDEO_BUFF		m_bufLogo;

	void *				m_hFile;
};

#endif // __CBaseVideoRnd_H__
