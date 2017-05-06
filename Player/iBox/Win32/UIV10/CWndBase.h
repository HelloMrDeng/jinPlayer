/*******************************************************************************
	File:		CWndBase.h

	Contains:	the base window header file

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#ifndef __CWndBase_H__
#define __CWndBase_H__

class CWndBase
{
public:
	CWndBase(HINSTANCE hInst);
	virtual ~CWndBase(void);

	virtual bool	CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG, bool bPopup = false);
	virtual void	Close (void);

	virtual HWND	GetWnd (void) {return m_hWnd;}

	virtual void	SetFontColor (COLORREF nColor) {m_nClrFont = nColor;}
	virtual void	SetBGColor (COLORREF nColor);

	virtual void	SetText (TCHAR * pText);
	virtual bool	GetText (TCHAR * pText, int nSize);
	virtual TCHAR *	GetText (void) {return m_szText;}
	virtual int 	GetTextSize (void) {return 1024;}

	virtual bool	ShowWnd (int nShow);
	virtual bool	IsShow (void);

protected:
	static LRESULT	CALLBACK ViewWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HINSTANCE		m_hInst;
	HWND			m_hParent;
	HWND			m_hWnd;
	RECT			m_rcWnd;
	DWORD			m_dwStyle;

	TCHAR			m_szClassName[64];
	TCHAR			m_szWindowName[64];

	HBRUSH 			m_hBKBrush;

	TCHAR			m_szText[1024];
	COLORREF		m_nClrFont;
};
#endif //__CWndBase_H__