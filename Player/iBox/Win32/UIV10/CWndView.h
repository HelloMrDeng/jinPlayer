/*******************************************************************************
	File:		CWndView.h

	Contains:	the window view header file

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#ifndef __CWndView_H__
#define __CWndView_H__

#include "CWndBase.h"
#include "yyMediaPlayer.h"

class CMediaEngine;
class CWndPanel;

class CWndView : public CWndBase
{
public:
	CWndView(HINSTANCE hInst);
	virtual ~CWndView(void);

	virtual bool	CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG);

	virtual void	SetFullScreen (void);
	virtual bool	IsFullScreen (void);

	virtual void	SetThumbnail (HBITMAP hBmbThumb) {m_hBmpThumb = hBmbThumb;}
	virtual void	SetMediaEngine (CMediaEngine * pMedia);
	virtual void	SetWndPanel (CWndPanel * pWndPanel) {m_pWndPanel = pWndPanel;}

protected:
	virtual LRESULT	OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	int				m_nScreenX;
	int				m_nScreenY;
	POINT			m_ptView;

	HDC				m_hMemDC;
	HBITMAP			m_hBmpThumb;
	CMediaEngine *	m_pMedia;
	YYRND_TYPE		m_nVideoRndType;
	CWndPanel *		m_pWndPanel;

	int				m_nClickCount;
	int				m_nClickTimer;

};
#endif //__CWndView_H__