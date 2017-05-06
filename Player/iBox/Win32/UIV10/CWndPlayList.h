/*******************************************************************************
	File:		CWndPlayList.h

	Contains:	the window view header file

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#ifndef __CWndPlayList_H__
#define __CWndPlayList_H__

#include "CWndBase.h"
#include "CNodeList.h"

#include "CMediaEngine.h"
#include "CExtSource.h"
#include "CBaseKey.h"
#include "CLangText.h"

#include "yyThumbnail.h"
#include "UFileFunc.h"

#define	WM_YYLIST_PLAYFILE		WM_APP + 0X3002
#define	WM_YYLIST_PLAYBACK		WM_APP + 0X3001

#define	WM_YYLIST_UPDATE		WM_APP + 0X3012
#define	WM_YYLIST_MOVING		WM_APP + 0X3013
#define	WM_YYLIST_SELECT		WM_APP + 0X3014

#define	TIMER_FILL_MEDIA_ITEMS		1001
#define	TIMER_SHOW_ITEM_INFO		1002
#define TIMER_DRAW_THUMBNAIL		1003
#define TIMER_HIDE_LASTVIDEO		1004
#define TIMER_CHAR_LASTTEXT			1005

typedef enum {
	LIST_VIEW_Folder	= 0,
	LIST_VIEW_Favor		= 1,
	LIST_VIEW_MyBox		= 2,
	LIST_VIEW_MAX		= 0X7FFFFFFF,
} LV_TYPE;

typedef enum {
	ITEM_Unknown	= 0,
	ITEM_Home		= 1,
	ITEM_Folder		= 2,
	ITEM_Audio		= 3,
	ITEM_Video		= 4,
	ITEM_NewFile	= 5,
	ITEM_NewFolder	= 6,
	ITEM_MAX		= 0X7FFFFFFF,
} ITEM_TYPE;

struct MEDIA_Item {
	ITEM_TYPE			nType;
	int					nSelect;
	TCHAR *				pPath;
	TCHAR *				pName;
	HBITMAP				hThumb;
	int					nIndex;
	YYINFO_Thumbnail	sThumbInfo;
	MEDIA_Item *		pParent;
	CObjectList<MEDIA_Item> * pChildList; 
	MEDIA_Item ()
	{
		nType = ITEM_Unknown;
		nSelect = 0;
		pPath = NULL;
		pName = NULL;
		hThumb = NULL;
		nIndex = -1;
		pParent = NULL;
		pChildList = NULL;
	}
};

class CWndPlayList : public CWndBase
{
public:
	CWndPlayList(HINSTANCE hInst, CMediaEngine * pMedia);
	virtual ~CWndPlayList(void);

	virtual bool	CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG, bool bFillItems);
	virtual void	SetLangText (CLangText * pLangText) {m_pLangText = pLangText;}
	virtual bool	SetViewType (LV_TYPE nType);
	virtual LV_TYPE	GetViewType (void) {return m_lvType;}
	virtual void	UpdateMenuLang (void);
	virtual void	OnDropFiles (WPARAM wParam);
	virtual void	OnSize (void);

	virtual void	Start (void);
	virtual void	Pause (void);

	virtual int		CopyLastVideo (HWND hWndVideo);
	virtual void	SetPlayingFile (TCHAR * pFile);

	virtual RECT *	GetItemRect (void) {return &m_rcItem;}
	virtual TCHAR *	GetFolder (void) {return m_szFolder;}
	virtual MEDIA_Item *		GetSelectedItem (void);
	CObjectList<MEDIA_Item> *	GetFileList (void) {return &m_lstItem;}

protected:
	virtual LRESULT	OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool			FillMediaItems (TCHAR * pFolder);
	virtual bool			FillFavorItems (MEDIA_Item * pDispItem);
	virtual bool			FillMyBoxItems (TCHAR * pBox);
	virtual bool			SortDispItems (void);
	virtual MEDIA_Item *	CreateItem (TCHAR * pName, ITEM_TYPE nType);
	virtual MEDIA_Item *	CreateItem (TCHAR * pPath, TCHAR * pName, ITEM_TYPE nType);
	virtual MEDIA_Item *	CloneItem (MEDIA_Item * pOldItem);
	virtual bool			DeleteItem (MEDIA_Item * pItem);
	virtual bool			DeleteSelItem (void);
	virtual void			ReleaseList (void);

	virtual LRESULT			OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			OnChar (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			OnMouseWheel (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			OnVScroll (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT			OnPaint (UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool			LoadMyFavor (void);
	virtual bool			LoadItem (yyFile hFile, MEDIA_Item * pItem);
	virtual bool			SaveMyFavor (void);
	virtual bool			SaveItem (yyFile hFile, MEDIA_Item * pItem);

	virtual	bool			AddNewBox (void);
	virtual	bool			OpenBox (TCHAR * pText);
	virtual	bool			ImportFileInBox (TCHAR * pSource, TCHAR * pTarget);
	virtual	bool			NewFolderInBox (TCHAR * pName);
	virtual	bool			ExportFileInBox (void);

	virtual bool	CreateBitmapBG (void);
	virtual void	ReleaseDCBmp (void);
	virtual void	ShowBitmapBG (void);

	MEDIA_Item *	GetSelectItem (int nX, int nY);
	virtual void	ItemSelected (MEDIA_Item * pItem);
	virtual void	DrawSelectedItem (HDC hdc);

	virtual void	AnimateThumbnail (void);

	virtual HBITMAP	CreateBMP (int nW, int nH, LPBYTE * pBmpBuff);

protected:
	CMediaEngine *	m_pMedia;
	CExtSource *	m_pExtSrc;
	CLangText *		m_pLangText;
	LV_TYPE			m_lvType;
	HMENU			m_hMenuPopup;
	HMENU			m_hMenuItem;
	HMENU			m_hMenuBox;
	HMENU			m_hMenuBoxItem;
	RECT			m_rcClient;
	int				m_nCols;
	int				m_nItemWidth;
	int				m_nItemHeight;
	int				m_nIconWidth;
	int				m_nIconHeight;
	int				m_nIconOffsetY;
	SCROLLINFO		m_sbInfo;

	HBITMAP			m_hBmpOld;
	HBITMAP			m_hBmpBG;
	LPBYTE			m_hBmpBGBuff;
	HBITMAP			m_hBmpHome;
	HBITMAP			m_hBmpFolder;
	HBITMAP			m_hBmpVideo;
	HBITMAP			m_hBmpAudio;
	HBITMAP			m_hBmpNewFile;
	HBITMAP			m_hBmpNewFolder;
	HDC				m_hDCBmp;
	HDC				m_hDCItem;
	HPEN			m_hPenLine;

	int				m_nBmpHeight;
	int				m_nBmpYPos;
	int				m_nMoveYPos;
	bool			m_bMoving;
	bool			m_bMoved;
	POINT			m_ptDown;

	HBITMAP						m_hThumbBmp;
	LPBYTE						m_pThumbBuff;
	int							m_nSteps;
	int							m_nDrawStep;
	RECT						m_rcThumb;
	RECT						m_rcItem;
	DWORD						m_dwThumbTimer;
	ITEM_TYPE					m_nAnimateType;	

	HBITMAP						m_hVideoBmp;
	LPBYTE						m_pVideoBmpBuff;

	CWndBase *					m_pWndEdit;
	CBaseKey					m_keyBase;
	TCHAR						m_szPWText[64];
	int							m_nTimerOnChar;

	// items
	MEDIA_Item *				m_pFavorItem;
	MEDIA_Item *				m_pCurItem;
	CObjectList<MEDIA_Item>		m_lstItem;
	MEDIA_Item *				m_pSelectedItem;
	TCHAR						m_szAudioExt[256];
	TCHAR						m_szVideoExt[256];
	TCHAR						m_szFolder[1024];
	TCHAR						m_szRoot[256];
	TCHAR						m_szPlayFile[1024];
	bool						m_bFodlerChanged;
	TCHAR						m_szBoxPath[1024];
	TCHAR						m_szBoxName[256];
	int							m_nBoxNum;
	TCHAR						m_szFolderBox[1024];
	TCHAR						m_szFolderFold[1024];
	bool						m_bFavorModified;

	static	int			GetThumbProc (void * pParam);
	virtual int			GetThumbLoop (MEDIA_Item * pItem);
	virtual bool		IsBlackThumb (YYINFO_Thumbnail * pThumb);

	HANDLE				m_hThread;
	bool				m_bStop;
	bool				m_bPause;
	bool				m_bWorking;
	DWORD				m_dwTimerShow;

	int					m_nDrawStepTime;

public:
	static int __cdecl compare_filename(const void *arg1, const void *arg2);
};
#endif //__CWndPlayList_H__