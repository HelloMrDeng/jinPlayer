/*******************************************************************************
	File:		CPlayBar.h

	Contains:	The play bar header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-09		Fenger			Create file

*******************************************************************************/
#ifndef __CPlayBar_H__
#define __CPlayBar_H__

#include "CMediaEngine.h"
#include "CLangText.h"
#include "CBaseConfig.h"
#include "CPlayCtrl.h"
#include "CNodeList.h"
#include "CMutexLock.h"

#include "CPlaySlider.h"
#include "CPlayButton.h"

#include "CFFMpegVideoRCC.h"

class CWndPlay;

class CPlayBar : public CBaseObject
{
public:
	CPlayBar(HINSTANCE hInst, CWndPlay * pWndPlay);
	virtual ~CPlayBar(void);

	virtual bool	Create (HWND hWnd, CMediaEngine * pMedia); 
	virtual bool	Show (bool bShow);
	virtual bool	IsShow (void) {return m_bShow;}
	virtual void	HandleEvent (int nID, void * pV1);
	virtual LRESULT	MsgProc	(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam);

	static int		VideoExtRender (void * pUserData, YY_BUFFER * pData);
	virtual int		RenderVideo (HDC hDC, YY_BUFFER * pData);
	virtual void	SetPos (bool bForward, int nStep);
	virtual void	SetVolume (int nStep);
	virtual void	SetZoomRect (RECT * pZoom);
	virtual void	SetRotate (int nAngle) {m_nRotate = nAngle;}
	CMutexLock *	GetDrawLock (void) {return &m_mtDraw;}

protected:
	virtual bool	UpdateViewBitmap (void);
	virtual bool	UpdateVideoBitmap (void);
	virtual bool	ResizeVideoBitmap (void);
	virtual bool	OverlayBmpVideo (void);
	virtual bool	ShowZoomArea (HDC hDC);
	virtual bool	ShowZoomSelect (HDC hDC);

	virtual bool	LoadConfig (TCHAR * pCfgFile);
	virtual bool	AddControl (char * pName);
	virtual bool	UpdateControl (bool bUpdate);

	virtual LRESULT	OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnHScroll (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnSize (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnEraseBG (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnPaint (UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HINSTANCE			m_hInst;
	CWndPlay *			m_pWndPlay;
	HWND				m_hWnd;
	CMutexLock			m_mtDraw;
	bool				m_bShow;
	RECT				m_rcView;
	RECT				m_rcBar;
	int					m_nLeft;
	int					m_nTop;
	int					m_nHeight;
	int					m_nArc;
	int					m_nTransParent;

	HDC					m_hViewDC;
	HBITMAP				m_hViewBmp;
	LPBYTE				m_pViewBuff;
	HBITMAP				m_hBmpVideo;
	LPBYTE				m_pBmpVBuff;
	RECT				m_rcBmpVideo;
	HBITMAP				m_hRottBmp;
	LPBYTE				m_pRottBuff;
	bool				m_bRottCopy;
	YY_BUFFER_CONVERT	m_buffConv;
	YY_BUFFER			m_buffData;
	YY_VIDEO_BUFF		m_buffVideo;
	int					m_nVideoW;
	int					m_nVideoH;
	CFFMpegVideoRCC *	m_pVCC;


	HPEN				m_hPenZoom;
	HBRUSH				m_hBrhZoom;
	HPEN				m_hPenSel;

	CMediaEngine *		m_pMedia;
	int					m_nVolume;
	YY_DATACB			m_dcbVideoRnd;
	bool				m_bVideoEOS;
	bool				m_bAudioOnly;
	bool				m_bSeeking;
	int					m_nSeekPos;
	int					m_nRotate;

	CBaseConfig *			m_pConfig;
	CObjectList<CPlayCtrl>	m_lstCtrl;
	CPlaySlider *			m_pSldAudio;
	CPlaySlider *			m_pSldPos;
	CPlayButton *			m_pBtnPlay;
	CPlayButton *			m_pBtnPause;
	CPlayButton *			m_pBtnFull;
	CPlayButton *			m_pBtnNormal;
	TCHAR					m_szTextDur[32];
	TCHAR					m_szTextPos[32];

};
#endif //__CPlayBar_H__