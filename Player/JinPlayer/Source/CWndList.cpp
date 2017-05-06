/*******************************************************************************
	File:		CWndList.cpp

	Contains:	The list window implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CWndList.h"

#include "CRegMng.h"
#include "CBaseKey.h"
#include "resource.h"
#include "USystemFunc.h"
#include "UStringFunc.h"
#include "RPlayerDef.h"
#include "yyLog.h"

#pragma warning (disable : 4996)

CWndList::CWndList(HINSTANCE hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hWnd (NULL)
	, m_pMedia (NULL)
	, m_bShow (false)
	, m_pExtSrc (NULL)
	, m_pRes (NULL)
	, m_pFolder (NULL)
	, m_pFavor (NULL)
	, m_pBox (NULL)
	, m_pView (NULL)
	, m_lvType (LIST_VIEW_MAX)
{
	SetObjectName ("CWndList");
}	

CWndList::~CWndList(void)
{
	CRegMng::g_pRegMng->SetIntValue (_T("ListView"), m_lvType);

	YY_DEL_P (m_pFolder);
	YY_DEL_P (m_pFavor);
	YY_DEL_P (m_pBox);
	YY_DEL_P (m_pRes);
}

bool CWndList::Create (HWND hWnd, CMediaEngine * pMedia, CExtSource * pExtSrc)
{
	m_hWnd = hWnd;
	m_pMedia = pMedia;
	m_pExtSrc = pExtSrc;

	m_pRes = new CListRes (m_hInst);

	LV_TYPE lvType = (LV_TYPE)CRegMng::g_pRegMng->GetIntValue (_T("ListView"), LIST_VIEW_Folder);
	if (lvType == LIST_VIEW_MyBox)
		lvType = LIST_VIEW_Folder;

	SetView (lvType);

	return true;
}

bool CWndList::Show (bool bShow)
{
	m_bShow = bShow;
	if (m_bShow)
		m_pView->Start ();
	return true;
}

bool CWndList::Stop (void)
{
	if (m_pFolder != NULL)
		m_pFolder->Stop ();
	if (m_pFavor != NULL)
		m_pFavor->Stop ();
	if (m_pBox != NULL)
		m_pBox->Stop ();
	return true;
}

bool CWndList::SetView (LV_TYPE lvType)
{
	if (lvType == m_lvType)
		return true;
	CListView * pCurView = m_pView;
	if (m_pView != NULL)
		m_pView->Stop ();
	int  nID = ID_FILE_FOLDERVIEW;
	bool bCreate = false;
	m_lvType = lvType;
	if (m_lvType == LIST_VIEW_Folder)
	{
		if (m_pFolder == NULL)
		{
			m_pFolder = new CListFolder (m_hInst);
			bCreate = true;
		}
		nID = ID_FILE_FOLDERVIEW;
		m_pView = m_pFolder;
	}
	else if (m_lvType == LIST_VIEW_Favor)
	{
		if (m_pFavor == NULL)
		{
			m_pFavor = new CListFavor (m_hInst);
			bCreate = true;
		}
		nID = ID_FILE_PLAYLISTVIEW;
		m_pView = m_pFavor;
	}
	else if (m_lvType == LIST_VIEW_MyBox)
	{
		if (m_pBox == NULL)
		{
			m_pBox = new CListBox (m_hInst);
			bCreate = true;
		}
		else
		{
			if (m_pBox->FillItem (NULL))
				m_pBox->CreateDispBmp ();
		}
		nID = ID_FILE_MYBOXVIEW;
		m_pView = m_pBox;
	}
	else
	{
		return false;
	}

	if (bCreate)
	{
		m_pView->SetMediaSource (m_pMedia, m_pExtSrc);
		m_pView->Create (m_hWnd, m_pRes);
		if (m_pView->FillItem (NULL))
			m_pView->CreateDispBmp ();
	}
	
	m_pView->ShowDispBmp (pCurView);
	m_pView->Start ();

	UpdateLang ();

	TCHAR	szWinText[1024];
	GetSelFile (szWinText, sizeof (szWinText));
	SendMessage (m_hWnd, WM_PLAYER_SetWinText, (WPARAM)szWinText, 0);

	if (m_lvType == LIST_VIEW_MyBox)
		InvalidateRect (m_hWnd, NULL, FALSE);

	return true;
}

bool CWndList::UpdateLang (void)
{
	int nID = ID_FILE_FOLDERVIEW;
	if (m_lvType == LIST_VIEW_Folder)
		nID = ID_FILE_FOLDERVIEW;
	else if (m_lvType == LIST_VIEW_Favor)
		nID = ID_FILE_PLAYLISTVIEW;
	else if (m_lvType == LIST_VIEW_MyBox)
		nID = ID_FILE_MYBOXVIEW;
	
	HMENU hWinMenu = GetMenu (m_hWnd);
	HMENU hViewMenu = GetSubMenu (hWinMenu, 0);
	CheckMenuRadioItem (hViewMenu, ID_FILE_FOLDERVIEW, ID_FILE_MYBOXVIEW, nID, MF_BYCOMMAND | MF_CHECKED);

	return true;
}

RECT * CWndList::GetItemRect (void) 
{
	if (m_pView == NULL)
		return NULL;

	return m_pView->GetItemRect ();
}

bool CWndList::GetViewBmp (HBITMAP * ppBmp, RECT * pRect, LPBYTE * ppBuff)
{
	if (m_pView == NULL)
		return false;
	return m_pView->GetViewBmp (ppBmp, pRect, ppBuff);
}

bool CWndList::GetSelFile (TCHAR * pSelFile, int nSize)
{
	bool bFind = false;
	CListItem * pItem = m_pView->GetSelItem ();
	if (pItem != NULL)
		bFind = pItem->GetFile (pSelFile, nSize);
	if (!bFind)
		_tcscpy (pSelFile, m_pView->GetFolder ());
	return bFind;
}

CListItem *	CWndList::GetSelItem (void)
{
	return 	m_pView->GetSelItem ();
}

LRESULT	CWndList::MsgProc (HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pView == NULL || !m_bShow)
		return S_FALSE;

	return m_pView->MsgProc (hWnd, uMsg, wParam, lParam);
}
