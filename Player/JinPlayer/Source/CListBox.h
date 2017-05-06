/*******************************************************************************
	File:		CListBox.h

	Contains:	The box list header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CListBox_H__
#define __CListBox_H__

#include "CListNewFolder.h"
#include "CBaseKey.h"

class CListBox : public CListNewFolder
{
public:
	CListBox(HINSTANCE hInst);
	virtual ~CListBox(void);

	virtual bool	Create (HWND hWnd, CListRes * pRes); 
	virtual bool	FillItem (TCHAR * pPath);

	virtual bool	AddNewBox (void);
	virtual bool	OpenBox (TCHAR * pText);
	virtual TCHAR *	GetFolder (void);

protected:
	virtual bool	OnNewItemFile (void);
	virtual bool	CreateNewFolder (void);

	virtual bool	ExportFile (void);
	virtual bool	DeleteItem (void);

	virtual LRESULT	OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnChar (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnDropFiles (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	TCHAR			m_szPassWord[32];
	int				m_nTimerPW;
};
#endif //__CListBox_H__