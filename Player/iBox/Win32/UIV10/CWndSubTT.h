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

class CWndSubTT : public CWndBase
{
public:
	CWndSubTT(HINSTANCE hInst);
	virtual ~CWndSubTT(void);

	virtual bool	CreateWnd (HWND hParent);

	virtual void	SetSubTTMenu (HMENU hMenu);
	virtual void	SetMediaEngine (CMediaEngine * pMedia);
	virtual void	OnSizeMove (void);

protected:
	virtual LRESULT	OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool	CreateTextFont (void);
	virtual bool	SaveFontParam (void);

protected:
	CMediaEngine *		m_pMedia;
	HMENU				m_hMenuSubTT;
	DWORD				m_bgColor;

	int					m_nEnable;
	HFONT				m_hTxtFont;
	LOGFONT				m_lfFont;
	CHOOSEFONT			m_cfFont;
};

#endif //__CWndSubTT_H__