/*******************************************************************************
	File:		CWndButton.h

	Contains:	the window view header file

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#ifndef __CWndButton_H__
#define __CWndButton_H__

#include "CWndBase.h"

class CWndButton : public CWndBase
{
public:
	CWndButton(HINSTANCE hInst, int nID);
	virtual ~CWndButton(void);

	virtual bool	Create (HWND hParent, RECT rcBtn);
	virtual LRESULT	OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	int			m_nID;
	bool		m_bDown;

	HPEN		m_hPenDown;
	HPEN		m_hPenUp;

/*
	HBRUSH		m_hBrushBG;

	RECT		m_rcThumb;
	HBRUSH		m_hBrushTmb;
	long long	m_nThumbPos;
*/
};
#endif //__CWndButton_H__