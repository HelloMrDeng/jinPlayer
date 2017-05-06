/*******************************************************************************
	File:		CWndSubTT.h

	Contains:	The control panel header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-22		Fenger			Create file

*******************************************************************************/
#ifndef __CWndSubTT_H__
#define __CWndSubTT_H__
#include "commctrl.h"
#include "commDlg.h"

#include "CWndBase.h"
#include "CMediaEngine.h"

class CWndSubTT 
{
public:
	CWndSubTT(HINSTANCE hInst, HWND hWnd);
	virtual ~CWndSubTT(void);

	virtual void	SetMediaEngine (CMediaEngine * pMedia);
	virtual LRESULT	OnCommand (int nID);
	virtual void	OnSizeMove (void);
	virtual bool	UpdateLang (void);

	virtual bool	OnMediaStart (void);
	virtual bool	OnMediaClose (void);

protected:
	static LRESULT	CALLBACK ViewWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool	UpdateMenuItem (void);
	virtual bool	CreateWnd (void);
	virtual bool	CreateTextFont (void);
	virtual bool	SaveFontParam (void);

protected:
	HINSTANCE		m_hInst;
	HWND			m_hParent;
	HWND			m_hWnd;
	TCHAR			m_szClassName[64];
	TCHAR			m_szWindowName[64];
	HBRUSH 			m_hBKBrush;
	DWORD			m_bgColor;

	CMediaEngine *	m_pMedia;
	int				m_nEnable;
	HFONT			m_hTxtFont;
	LOGFONT			m_lfFont;
	CHOOSEFONT		m_cfFont;
};

#endif //__CWndSubTT_H__