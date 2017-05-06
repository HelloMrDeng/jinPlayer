/*******************************************************************************
	File:		CSubtitleEngine.h

	Contains:	the subtitle engine header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-03		Fenger			Create file

*******************************************************************************/
#ifndef __CSubtitleEngine_H__
#define __CSubtitleEngine_H__

#include "CBaseObject.h"
#include "yyMediaPlayer.h"
#include "yySubtitle.h"

#include "CSubtitleBase.h"
#include "CBaseSource.h"

#include "CMutexLock.h"
#include "CBaseClock.h"

#include "UThreadFunc.h"

class CSubtitleEngine : public CBaseObject
{
public:
	CSubtitleEngine(void * hInst);
	virtual ~CSubtitleEngine(void);

	virtual int		SetClock (CBaseClock * pClock);
	virtual void	SetSource (CBaseSource * pSource) {m_pMediaSrc = pSource;}

	virtual int		Open (const TCHAR * pSource);
	virtual int		Close (void);
	virtual int		SetPos (int nPos);

	virtual int		GetCharset (void);
	virtual int		GetItemText (long long llTime, YY_BUFFER * pTextBuff);
	virtual int		Draw (HDC hDC, RECT * pView, long long llTime, bool bOverlay);

	virtual int		Enable (int nEnable);

	virtual int		SetExtRnd (YY_DATACB * pDataCB);
	virtual int		SetExtDraw (YYSUB_ExtDraw * pExtDraw);
	virtual int		SetView (void * hView);
	virtual int		SetFontSize (int nSize);
	virtual int		SetFontColor (int nColor);
	virtual int		SetBackColor (int nColor);
	virtual int		SetFontHandle (void * hFont);
	virtual void *	GetInFont (void) {return m_hTxtFont;}

	virtual int		Start (void);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int 	SetParam (int nID, void * pParam);
	virtual int		GetParam (int nID, void * pParam);

protected:
#ifdef _OS_WIN32	
	virtual int		DrawItem (CSubtitleItem * pItem, HDC hDC, RECT * pRect);
	virtual HBITMAP CreateBMP (HDC hDC, int nW, int nH, LPBYTE * pBmpBuff);
	virtual void	ReleaseBMP (void);
	virtual int		CreateTxtFont (HDC hDC);
#endif // _OS_WIN32

protected:
	static	int		RenderProc (void * pParam);
	virtual int		RenderLoop (void);
	yyThreadHandle	m_hRndThread;
	bool			m_bWorking;

protected:
	void *				m_hInst;
	CBaseSource *		m_pMediaSrc;
	void *				m_hView;
	YYPLAY_STATUS		m_status;
	CBaseClock *		m_pClock;
	int					m_nEnable;
	CMutexLock			m_mtDraw;
	YY_DATACB *			m_pExtRnd;
	YY_BUFFER			m_extBuff;
	YYSUB_ExtDraw *		m_pExtDraw;
	long long			m_llLastTime;

	CSubtitleBase *		m_pSource;
	CSubtitleItem *		m_pDrawItem;

	RECT				m_rcView;
	HDC					m_hMemDC;
	HBITMAP				m_hBmpText;
	unsigned char *		m_pBmpBuff;
	int					m_nBmpWidth;
	int					m_nBmpHeight;
	HBITMAP				m_hBmpOld;
	RECT				m_rcBmp;

	int					m_nTxtSize;
	int					m_nTxtColor;
	HFONT				m_hTxtFont;

	int					m_nExtSize;
	int					m_nExtColor;
	HFONT				m_hExtFont;
	int					m_nBkgColor;
	bool				m_bFontChanged;
};

#endif // __CSubtitleEngine_H__
