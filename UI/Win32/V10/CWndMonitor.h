/*******************************************************************************
	File:		CWndMonitor.h

	Contains:	the window mem header file

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#ifndef __CWndMonitor_H__
#define __CWndMonitor_H__

#include "CWndBase.h"

class CWndMonitor : public CWndBase
{
public:
	CWndMonitor(HINSTANCE hInst);
	virtual ~CWndMonitor(void);

	virtual bool	CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG);
	virtual LRESULT	OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual int		ShowMessage (int nMsg, WPARAM wParam, LPARAM lParam);

protected:

protected:
	HWND		m_hWndText;
	TCHAR *		m_pInfo;
	int			m_nSize;
	TCHAR		m_szMsg[128];

};
#endif //__CWndMonitor_H__