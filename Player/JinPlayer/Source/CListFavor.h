/*******************************************************************************
	File:		CListFavor.h

	Contains:	The favor list header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CListFavor_H__
#define __CListFavor_H__

#include "CListNewFolder.h"
#include "UFileFunc.h"

class CListFavor : public CListNewFolder
{
public:
	CListFavor(HINSTANCE hInst);
	virtual ~CListFavor(void);

	virtual bool	Create (HWND hWnd, CListRes * pRes); 
	virtual bool	FillItem (TCHAR * pPath);

protected:
	virtual bool	FillShowItems (void);
	virtual bool	ReleaseItems (void);
	virtual bool	DeleteItem (void);

	virtual bool	OnNewItemFile (void);
	virtual bool	CreateNewFolder (void);

	virtual LRESULT	OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnDropFiles (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool	LoadFile (void);
	virtual bool	SaveFile (void);

protected:
	CListItem *		m_pRootItem;
	CListItem *		m_pShowItem;
	CListItem *		m_pHomeItem;
	CListItem *		m_pExitItem;
	bool			m_bModified;
};
#endif //__CListFavor_H__