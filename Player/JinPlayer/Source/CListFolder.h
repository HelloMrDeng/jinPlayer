/*******************************************************************************
	File:		CListFolder.h

	Contains:	The folder list header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CListFolder_H__
#define __CListFolder_H__

#include "CListView.h"

class CListFolder : public CListView
{
public:
	CListFolder(HINSTANCE hInst);
	virtual ~CListFolder(void);

	virtual bool	Create (HWND hWnd, CListRes * pRes); 
	virtual bool	FillItem (TCHAR * pPath);

protected:
	virtual bool	HasItemInFolder (TCHAR * pFolder);
	virtual bool	DeleteItem (void);

	virtual LRESULT	OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnDropFiles (UINT uMsg, WPARAM wParam, LPARAM lParam);
	
};
#endif //__CListFolder_H__