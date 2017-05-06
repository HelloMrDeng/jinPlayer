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

#define WM_VIEW_FullScreen		WM_USER+202
#define WM_VIEW_OnPaint			WM_USER+203
#define WM_VIEW_Resize			WM_USER+204
// wParam 1, LButton Down
#define WM_VIEW_EVENT			WM_USER+205

class CWndView : public CWndBase
{
public:
	CWndView(HINSTANCE hInst);
	virtual ~CWndView(void);

	virtual bool	CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG);
	virtual LRESULT	OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void	SetFullScreen (void);
	virtual bool	IsFullScreen (void);

	virtual void	ShowLogo (bool bShow);
	virtual void	ShowOverlay (bool bShow);
	virtual void	SetThumbnail (HBITMAP hBmbThumb);

protected:
	int				m_nScreenX;
	int				m_nScreenY;
	POINT			m_ptView;

	HBITMAP			m_hBmpLogo;
	HDC				m_hLogoDC;
	int				m_nLogoW;
	int				m_nLogoH;

	HBITMAP			m_hBmpOverlay;
	HDC				m_hDCOverlay;
	bool			m_bShowOverlay;

	HBITMAP			m_hBmpThumb;

	TCHAR			m_szInfo[256];
	bool			m_bShowLogo;

	int				m_nClickCount;
	int				m_nClickTimer;

};
#endif //__CWndView_H__