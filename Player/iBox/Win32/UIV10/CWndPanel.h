/*******************************************************************************
	File:		CWndPanel.h

	Contains:	The control panel header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-13		Fenger			Create file

*******************************************************************************/
#ifndef __CWndPanel_H__
#define __CWndPanel_H__

#include "CWndBase.h"
#include "CMediaEngine.h"
#include "CWndSlider.h"

#define	IDC_BTN_MUTE			1101
#define	IDC_SLD_VOLUME			1102

#define	IDC_BTN_PREV			1111
#define	IDC_BTN_SEEKPREV		1112
#define	IDC_BTN_PLAY			1114
#define	IDC_BTN_PAUSE			1115
#define	IDC_BTN_SEEKNEXT		1118
#define	IDC_BTN_NEXT			1119

#define	IDC_BTN_LIST			1120
#define	IDC_BTN_OPEN			1121
#define	IDC_BTN_FULLSRN			1122

#define WM_YYPANEL_COMMAND		WM_USER+303

class CWndPanel : public CWndBase
{
public:
	CWndPanel(HINSTANCE hInst);
	virtual ~CWndPanel(void);

	virtual bool	CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG, bool bPopup = false);
	virtual void	HandleEvent (int nID, void * pV1);

	virtual void	SetMediaEngine (CMediaEngine * pMedia) {m_pMedia = pMedia;}

protected:
	virtual LRESULT	OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND			CreateButton (TCHAR * pText, int nLeft, int nTop, int nID);
	HWND			CreateSlider (RECT rcPos, int nID, bool bHorizon);

protected:
	CMediaEngine *		m_pMedia;
	int					m_nAudioVolume;
	bool				m_bMute;

	CWndBase *			m_pWndBG;
	CWndSlider *		m_pSldPos;
	DWORD				m_dwTimerPos;

	HWND				m_btnMute;
	HWND				m_sldVolume;

	HWND				m_btnPrev;
	HWND				m_btnPlay;
	HWND				m_btnSeekPrev;
	HWND				m_btnPause;
	HWND				m_btnSeekNext;
	HWND				m_btnNext;

	HWND				m_btnList;
	HWND				m_btnOpen;
	HWND				m_btnFullSrn;

};

#endif //__CWndPanel_H__