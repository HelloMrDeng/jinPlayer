/*******************************************************************************
	File:		CListView.h

	Contains:	The base list view header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CListView_H__
#define __CListView_H__

#include "CBaseObject.h"
#include "CMutexLock.h"
#include "CListRes.h"
#include "CListItem.h"
#include "CNodeList.h"

#include "CMediaEngine.h"
#include "CExtSource.h"

#include "UThreadFunc.h"

#define ITEM_WIDTH		200
#define	ITEM_HEIGHT		160
#define	ICON_WIDTH		160
#define	ICON_HEIGHT		100
#define	ICON_OFF_Y		8

typedef enum {
	LIST_VIEW_Folder	= 0,
	LIST_VIEW_Favor		= 1,
	LIST_VIEW_MyBox		= 2,
	LIST_VIEW_MAX		= 0X7FFFFFFF,
} LV_TYPE;

class CListView : public CBaseObject
{
public:
	CListView(HINSTANCE hInst);
	virtual ~CListView(void);

	virtual void	SetMediaSource (CMediaEngine * pMedia, CExtSource * pSource);
	virtual bool	Create (HWND hWnd, CListRes * pRes); 
	virtual bool	FillItem (TCHAR * pPath);
	virtual bool	CreateDispBmp (void);
	virtual bool	ShowDispBmp (CListView * pPrev);
	virtual bool	GetViewBmp (HBITMAP * ppBmp, RECT * pRect, LPBYTE * ppBuff);

	virtual CListItem * GetSelItem (void) {return m_pSelItem;}
	virtual RECT *		GetItemRect (void) {return &m_rcLastItem;}
	virtual TCHAR *		GetFolder (void) {return m_szFolder;}

	virtual LRESULT	MsgProc	(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam);

	static int __cdecl compare_filename(const void *arg1, const void *arg2);

protected:
	virtual bool		SortItems (void);
	virtual bool		ReleaseItems (void);
	virtual bool		FindItem (TCHAR * pName, ITEM_TYPE nType);
	virtual bool		OnNewItemFile (void) {return false;}
	virtual bool		OnNewItemFolder (void) {return false;}
	virtual bool		OnSelItemChanged (void) {return true;}
	virtual CListItem *	GetFocusItem (int nX, int nY);
	virtual bool		DrawItemRect (HDC hDC, CListItem * pItem);
	virtual bool		DrawItemThumb (CListItem * pItem);

	virtual LRESULT		OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnChar (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnMouseWheel (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnVScroll (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnDropFiles (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnSize (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnEraseBG (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnPaint (UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool		CreateTextFont (void);
	virtual bool		ReleaseBmpDC (void);
	virtual bool		SaveItemThumb (TCHAR * pFolder);
	virtual bool		LoadItemThumb (TCHAR * pFolder);

protected:
	HINSTANCE		m_hInst;
	HWND			m_hWnd;
	LV_TYPE			m_nType;
	RECT			m_rcView;
	bool			m_bLBtnDown;
	CMediaEngine *	m_pMedia;
	CExtSource *	m_pExtSrc;
	CListRes *		m_pRes;

	CMutexLock		m_mtBmp;
	HDC				m_hMemDC;
	HDC				m_hBmpDC;
	HBITMAP			m_hBmpOld;
	HBITMAP			m_hBmpList;
	LPBYTE			m_hBmpBuff;
	int				m_nBmpHeight;
	int				m_nBmpYPos;
	int				m_nBmpCols;
	HFONT			m_hTxtFont;
	SCROLLINFO		m_sbInfo;
	int				m_nMoveYPos;
	RECT			m_rcLastItem;
	int				m_nActvType;

	CMutexLock				m_mtList;
	CObjectList<CListItem>	m_lstItem;
	CListItem *				m_pSelItem;
	CListItem *				m_pNewFolder;
	CListItem *				m_pNewMedia;
	TCHAR					m_szFolder[1024];
	TCHAR					m_szRoot[256];
	TCHAR					m_szPlayFile[1024];
	TCHAR					m_szThumb[1024];
	
	int						m_nFolderLevel;
	int						m_aBmpPos[1024];

public:
	virtual	bool		Start (void);
	virtual	bool		Pause (void);
	virtual	bool		Stop (void);

protected:
	static	int			GetThumbProc (void * pParam);
	virtual int			GetThumbLoop (void);
	virtual int			GetThumbItem (TCHAR * pFile, HBITMAP * ppBmp);
	yyThreadHandle		m_hThread;
	YYTHRD_Status		m_nStatus;
	bool				m_bWorking;
	YYINFO_Thumbnail	m_sThumbInfo;
};
#endif //__CListView_H__
