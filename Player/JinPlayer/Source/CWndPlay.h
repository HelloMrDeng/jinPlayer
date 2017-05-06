/*******************************************************************************
	File:		CWndPlay.h

	Contains:	The play window header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CWndPlay_H__
#define __CWndPlay_H__

#include "CMediaEngine.h"
#include "CExtSource.h"
#include "CLangText.h"
#include "CListView.h"

#include "CPlayBar.h"
#include "CWndSubTT.h"

class CWndPlay : public CBaseObject
{
public:
	CWndPlay(HINSTANCE hInst);
	virtual ~CWndPlay(void);

	virtual bool	Create (HWND hWnd, CMediaEngine * pMedia, CExtSource * pExtSrc); 
	virtual bool	Show (bool bShow);
	virtual bool	UpdateLang (void);
	virtual LRESULT	MsgProc	(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual int		PlaybackItem (CListItem * pItem, void * pList);
	virtual int		PlaybackFile (TCHAR * pFile, void * pList);
	virtual int		PlayNextFile (bool bNext);
	virtual int		Start (void);
	virtual int		Close (void);

	virtual int		ShowFullScreen (void);
	virtual bool	IsFullScreen (void);
	virtual bool	GetZoomSelect (RECT * pZoomSelect);
	virtual bool	IsZoomSelect (void) {return m_bZoomSelect;}

	CMediaEngine *	GetMedia (void) {return m_pMedia;}
	virtual TCHAR *	GetPlayFile (void) {return m_szFile;}
	CListItem *		GetPlayItem (void) {return m_pPlayItem;}

protected:
	static void		NotifyEvent (void * pUserData, int nID, void * pV1);
	virtual void	HandleEvent (int nID, void * pV1);

	virtual LRESULT	OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnMouseWheel (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnKeyDown (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnSize (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnMove (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnEraseBG (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnPaint (UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool	ShowBar (bool bShow);
	virtual bool	ShowCursor (bool bShow);
	virtual void	OnZoomVideo (float fLevel, bool bFix);
	virtual void	OnZoomMove (int nX, int nY);
	virtual void	OnZoomSelect (WPARAM wParam, LPARAM lParam);
	virtual void	UpdateBackgroud (void);
	virtual void	CaptureVideo (void);

protected:
	HINSTANCE		m_hInst;
	HWND			m_hWnd;
	BOOL			m_bScrActive;   
	bool			m_bShow;
	int				m_nScreenX;
	int				m_nScreenY;
	RECT			m_rcView;
	DWORD			m_dwStyle;
	HMENU			m_hMenu;
	bool			m_bPopupMenu;
	int				m_nClickCount;
	int				m_nClickTimer;
	int				m_nLastClkTime;
	int				m_nMonitorTimer;

	CMediaEngine *	m_pMedia;
	CExtSource *	m_pExtSrc;
	TCHAR			m_szFile[1024];
	TCHAR			m_szPath[1024];
	RECT			m_rcVideo;
	bool			m_bPlayNextFile;

	bool			m_bMediaClose;
	bool			m_bOpening;
	bool			m_bAutoPlay;
	YYRND_TYPE		m_nVRndType;
	int				m_nSeekMode;
	int				m_nRotateAngle;
	int				m_nResizeY;
	int				m_nZoomNum;
	float			m_fZoomLevel;
	RECT			m_rcZoom;
	POINT			m_ptZoom;
	int				m_nLastZoomX;
	int				m_nLastZoomY;
	bool			m_bZoomSelect;
	int				m_nTimerZoomSlt;
	POINT			m_ptZoomSelect;
	int				m_nAudioTrackNum;
	int				m_nPlaySpeed;
	int				m_nViewRatio;

	HBRUSH			m_hBrhBlack;
	CPlayBar *		m_pWndBar;
	int				m_nHideBarTimer;
	int				m_nHideCursorTimer;
	int				m_nLastMoveX;
	int				m_nLastMoveY;
	POINT			m_ptMouseDown;
	HCURSOR			m_hCursor;
	CWndSubTT *		m_pWndSubTT;

	CListItem *					m_pPlayItem;
	CObjectList<CListItem> *	m_pLstItem;

};
#endif //__CWndPlay_H__