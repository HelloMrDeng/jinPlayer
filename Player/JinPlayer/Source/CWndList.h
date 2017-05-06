/*******************************************************************************
	File:		CWndList.h

	Contains:	The list window header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CWndList_H__
#define __CWndList_H__

#include "CListFolder.h"
#include "CListFavor.h"
#include "CListBox.h"
#include "CListRes.h"

class CWndList : public CBaseObject
{
public:
	CWndList(HINSTANCE hInst);
	virtual ~CWndList(void);

	virtual bool		Create (HWND hWnd, CMediaEngine * pMedia, CExtSource * pExtSrc); 
	virtual bool		Show (bool bShow);
	virtual bool		Stop (void);

	virtual bool		SetView (LV_TYPE lvType);
	virtual bool		UpdateLang (void);
	virtual RECT *		GetItemRect (void);
	virtual bool		GetViewBmp (HBITMAP * ppBmp, RECT * pRect, LPBYTE * ppBuff);
	virtual bool		GetSelFile (TCHAR * pSelFile, int nSize);
	virtual CListItem *	GetSelItem (void);
	virtual CListView * GetView (void) {return m_pView;}
	virtual LV_TYPE		GetViewType (void) {return m_lvType;}

	virtual LRESULT		MsgProc	(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HINSTANCE		m_hInst;
	HWND			m_hWnd;
	bool			m_bShow;
	CMediaEngine *	m_pMedia;
	CExtSource *	m_pExtSrc;
	CListRes *		m_pRes;

	CListFolder	*	m_pFolder;
	CListFavor *	m_pFavor;
	CListBox *		m_pBox;
	CListView *		m_pView;
	LV_TYPE			m_lvType;
};
#endif //__CWndList_H__