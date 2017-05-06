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

#include "COMBoxMng.h"

#include "yyThumbnail.h"

#define	WM_YYLIST_PLAYFILE		WM_APP + 0X3002
#define	WM_YYLIST_PLAYBACK		WM_APP + 0X3001

#define	WM_YYLIST_UPDATE		WM_APP + 0X3012
#define	WM_YYLIST_MOVING		WM_APP + 0X3013
#define	WM_YYLIST_SELECT		WM_APP + 0X3014

class CWndPlayList : public CWndBase
{
public:
typedef enum {
	ITEM_Unknown	= 0,
	ITEM_Home		= 1,
	ITEM_Folder		= 2,
	ITEM_Audio		= 3,
	ITEM_Video		= 4,
	ITEM_MAX		= 0X7FFFFFFF,
} ITEM_TYPE;

struct MEDIA_Item {
	ITEM_TYPE			nType;
	TCHAR *				pName;
	HBITMAP				hThumb;
	int					nIndex;
	YYINFO_Thumbnail	sThumbInfo;
	MEDIA_Item ()
	{
		nType = ITEM_Unknown;
		pName = NULL;
		hThumb = NULL;
		nIndex = -1;
	}
};

public:
	CWndPlayList(HINSTANCE hInst, COMBoxMng * pMedia);
	virtual ~CWndPlayList(void);

	virtual bool	CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG, bool bFillItems);
	virtual LRESULT	OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void	Start (void);
	virtual void	Pause (void);

	virtual int		GetMaxPos (void);
	virtual int		GetPos (void);
	virtual int		SetPos (int nPos);

	virtual int		CopyLastVideo (HWND hWndVideo);
	virtual void	SetPlayingFile (TCHAR * pFile);

	virtual YYINFO_Thumbnail *	GetSelectedItem (void);
	virtual RECT *	GetItemRect (void) {return &m_rcItem;}

protected:
	virtual bool	FillMediaItems (TCHAR * pFolder);
	virtual bool	AddNewItem (TCHAR * pName, ITEM_TYPE nType);
	virtual void	ReleaseList (void);

	virtual bool	CreateBitmapBG (void);
	virtual void	ReleaseDCBmp (void);

	MEDIA_Item *	GetSelectItem (int nX, int nY);
	virtual void	ItemSelected (MEDIA_Item * pItem);
	virtual void	DrawSelectedItem (HDC hdc);

	virtual void	AnimateThumbnail (void);

	virtual HBITMAP	CreateBMP (int nW, int nH, LPBYTE * pBmpBuff);

protected:
	COMBoxMng *		m_pMedia;
	RECT			m_rcClient;
	int				m_nCols;
	int				m_nItemWidth;
	int				m_nItemHeight;
	int				m_nIconWidth;
	int				m_nIconHeight;
	int				m_nIconOffsetY;

	HBITMAP			m_hBmpOld;
	HBITMAP			m_hBmpBG;
	LPBYTE			m_hBmpBGBuff;
	HBITMAP			m_hBmpHome;
	HBITMAP			m_hBmpFolder;
	HBITMAP			m_hBmpVideo;
	HBITMAP			m_hBmpAudio;
	HDC				m_hDCBmp;
	HDC				m_hDCItem;
	HPEN			m_hPenLine;

	int				m_nBmpHeight;
	int				m_nBmpYPos;
	int				m_nMoveYPos;
	bool			m_bMoving;
	bool			m_bMoved;
	POINT			m_ptDown;

	MEDIA_Item *				m_pSelectedItem;

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


	// items
	CObjectList<MEDIA_Item>		m_lstItem;
	TCHAR						m_szAudioExt[256];
	TCHAR						m_szVideoExt[256];
	TCHAR						m_szFolder[1024];
	TCHAR						m_szRoot[256];
	TCHAR						m_szPlayFile[1024];
	bool						m_bFodlerChanged;

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