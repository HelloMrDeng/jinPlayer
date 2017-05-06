/*******************************************************************************
	File:		CDemoUI.h

	Contains:	the demo UI header file

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-14		Fenger			Create file

*******************************************************************************/
#ifndef __CDEMOUI_H__
#define __CDEMOUI_H__

#include "COMBoxMng.h"
#include "CExtPlayer.h"
#include "CExtData.h"

#include "CWndView.h"
#include "CWndSlider.h"
#include "CWndPlayList.h"

#include "CDlgFileInfo.h"
#include "CNodeList.h"
#include "CRegMng.h"

class CDemoUI
{
public:
	CDemoUI(void);
	virtual ~CDemoUI(void);

	virtual LRESULT	MsgProc	(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam);
	virtual int		OpenMediaFile (TCHAR * pFile, bool bShow);
	virtual bool	Create (HINSTANCE hInst, HWND hWnd, bool bFillItems); 

	static void		NotifyEvent (void * pUserData, int nID, void * pV1);

	COMBoxMng *		GetPlayer (void) {return m_pMedia;}

protected:
	HWND		CreateButton (TCHAR * pText, int nLeft, int nTop, int nID);
	HWND		CreateSlider (RECT rcPos, int nID, bool bHorizon);
	HWND		CreateText (TCHAR * pText, int nLeft, int nTop, int nW, int nH, int nID);

	int			PlaybackFile (TCHAR * pFile);
	int			PlayNextFile (bool bNext);

	void		CreatePlayList (void);
	void		ReleasePlayList (void);

	void		GetTimeText (long long llTime, TCHAR * szText);

	static int	yyMedeaSubCB (void * pUser, YY_BUFFER * pData);

protected:
	CRegMng *			m_pRegMng;
	HINSTANCE			m_hInst;
	HWND				m_hWnd;
	bool				m_bPrepareClose;
	HMENU				m_hMenuAudio;
	HCURSOR				m_mtWait;
	HCURSOR				m_mtNormal;
	bool				m_bOpening;
	bool				m_bSeeking;
	bool				m_bRunning;
	bool				m_bAutoPlay;
	int					m_hAudioFileTimer;

	COMBoxMng *			m_pMedia;
	CExtPlayer *		m_pExtPlayer;
	CExtData *			m_pExtData;
	int					m_nAudioTrackNum;
	int					m_nAudioTrackMenus;
	int					m_nBitsVideo;
	bool				m_bOverlay;
	TCHAR				m_szFile[1024];
	TCHAR				m_szPath[1024];
	TCHAR				m_szFileExt[1024];
	TCHAR **			m_ppFiles;
	int					m_nFileNum;
	int					m_nFileIdx;

	int					m_nAudioVolume;
	YYRND_TYPE			m_nVideoRendType;
	bool				m_bListView;

	unsigned int		m_uAutoTest;

	CWndView *		m_pWndVideo;
	CWndPlayList *	m_pWndList;

	HMENU			m_hPopup;

	HWND			m_btnLoud;
	HWND			m_sldVoice;
	HWND			m_btnLow;
	HWND			m_btnMute;
	HWND			m_btnSubTitle;
	HWND			m_btnAudio;
	HWND			m_btnAR;
	HWND			m_btnSpeed;

	HWND			m_btnClose;
	HWND			m_btnOpen;
	HWND			m_btnList;
	HWND			m_btnNext;
	HWND			m_btnPrev;
	HWND			m_btnInfo;
	HWND			m_btnPlay;
	HWND			m_btnPause;
	HWND			m_btnStop;

	HWND			m_sldPos;
	CWndSlider *	m_pSldPos;
	HWND			m_txtPos;
	HWND			m_txtDur;

	CWndBase *		m_pWndTitle;
	YY_DATACB		m_cbSubTT;

	int				m_nDbgStartTime;
};
#endif //__CDEMOUI_H__
