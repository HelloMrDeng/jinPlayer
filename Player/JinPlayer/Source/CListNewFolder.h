/*******************************************************************************
	File:		CListNewFolder.h

	Contains:	The list view handle new folder header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CListNewFolder_H__
#define __CListNewFolder_H__

#include "CListView.h"

class CListNewFolder : public CListView
{
public:
	CListNewFolder(HINSTANCE hInst);
	virtual ~CListNewFolder(void);

	virtual bool	Create (HWND hWnd, CListRes * pRes); 

protected:
	virtual bool	OnNewItemFolder (void);
	virtual bool	OnSelItemChanged (void);
	virtual bool	UpdateEditRect (void);
	virtual bool	ShowEditBox (bool bShow);
	virtual bool	CreateNewFolder (void);

	virtual LRESULT	OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnChar (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnPaint (UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	TCHAR			m_szNewFolder[64];
	bool			m_bNewFolder;
	HBRUSH			m_hBrhEdit;
	RECT			m_rcEdit;
	int				m_nTimerCount;
	int				m_nTimerEdit;
};
#endif //__CListNewFolder_H__