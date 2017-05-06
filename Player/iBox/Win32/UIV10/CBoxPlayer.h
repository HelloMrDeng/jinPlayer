/*******************************************************************************
	File:		CBoxPlayer.h

	Contains:	the box player header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-13		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxPlayer_H__
#define __CBoxPlayer_H__

#include "CRegMng.h"
#include "CMediaEngine.h"
#include "CExtSource.h"

#include "CWndPlayList.h"
#include "CWndPanel.h"
#include "CWndBar.h"
#include "CWndSubTT.h"

#include "CLangText.h"
#include "CBmpFile.h"

typedef int (* yyVerifyLicenseText) (void * pUserData, char * pText, int nSize);
#define	WM_YYRR_CHKLCS	WM_USER + 765
#define	WM_YYYF_CHKLCS	WM_USER + 871
#define	WM_YYSM_CHKLCS	WM_USER + 811

#define YY_VIEW_LIST	1
#define	YY_VIEW_PLAY	2

#define TIMER_PLAY_NEXTFILE		102
#define TIMER_UPDATE_AUDIOFILE	103
#define TIMER_LBUTTON_DOWN		104
#define TIMER_SHOW_PANEL		105

#define	PANEL_SHOW_TIME			4000

class CBoxPlayer
{
public:
	CBoxPlayer(HINSTANCE hInst);
	virtual ~CBoxPlayer(void);

	virtual bool	Create (HWND hWnd, TCHAR * pFile); 
	virtual LRESULT	MsgProc	(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam);

protected:
	static void		NotifyEvent (void * pUserData, int nID, void * pV1);
	virtual void	HandleEvent (int nID, void * pV1);

	virtual int		OpenMediaFile (TCHAR * pFile, bool bShow);
	virtual int		PlaybackFile (TCHAR * pFile);
	virtual int		PlayNextFile (bool bNext);

	virtual int		ShowFullScreen (void);
	virtual bool	IsFullScreen (void);
	virtual void	MovePanel (void);
	virtual void	ShowPanel (bool bShow);
	virtual void	SwitchView (int nView);
	virtual void	UpdatePlayMenu (void);
	virtual void	UpdateMenuLang (void);

	virtual void	OnZoomVideo (void);
	virtual void	OnZoomMove (WPARAM wParam, LPARAM lParam);

	static int		CheckExtLicense (char * pText, int nSize, yyVerifyLicenseText fVerify, void * pUserData);
	static int		VideoExtRender (void * pUserData, YY_BUFFER * pData);
	virtual int		RenderVideo (YY_BUFFER * pData);

protected:
	HINSTANCE			m_hInst;
	HWND				m_hWnd;
	int					m_nScreenX;
	int					m_nScreenY;
	RECT				m_rcView;
	DWORD				m_dwStyle;
	int					m_nClickCount;
	int					m_nClickTimer;

	CRegMng *			m_pRegMng;
	CMediaEngine *		m_pMedia;
	CExtSource *		m_pExtSrc;
	TCHAR				m_szFile[1024];
	TCHAR				m_szPath[1024];
	RECT				m_rcVideo;

	bool				m_bMediaClose;
	bool				m_bOpening;
	bool				m_bAutoPlay;
	int					m_nTimerAudio;
	YYRND_TYPE			m_nVRndType;
	int					m_nZoomNum;
	RECT				m_rcZoom;
	POINT				m_ptZoom;
	int					m_nLastZoomX;
	int					m_nLastZoomY;

	int					m_nViewType;
	CWndPanel *			m_pWndPanel;
	CWndBar *			m_pWndBar;
	DWORD				m_dwTimerPanel;
	int					m_nLastMoveX;
	int					m_nLastMoveY;
	int					m_nLastHideTime;
	CWndSubTT *			m_pWndSubTT;

	CWndPlayList *				m_pWndList;
	CObjectList<MEDIA_Item> *	m_pListFile;
	HDC							m_hMemDC;
	HBITMAP						m_hBmpThumb;
	HBRUSH						m_hBrushBlack;
	HBRUSH						m_hBrushWhite;

	HMENU						m_hMenuList;
	HMENU						m_hMenuPlay;
	HMENU						m_hMenuAudio;
	HMENU						m_hMenuPopup;
	HMENU						m_hMenuZoom;

	int							m_nAudioTrackNum;

	CLangText *					m_pLangText;

	YY_DATACB					m_dcbVideoRnd;
	CBmpFile					m_bmpFile;
};
#endif //__CBoxPlayer_H__