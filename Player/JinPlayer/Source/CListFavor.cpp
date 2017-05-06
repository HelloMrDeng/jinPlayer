/*******************************************************************************
	File:		CListFavor.cpp

	Contains:	The favor list implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CListFavor.h"
#include "CListStore.h"
#include "CLangText.h"
#include "resource.h"
#include "USystemFunc.h"
#include "UStringFunc.h"
#include "UFileFormat.h"
#include "RPlayerDef.h"
#include "yyLog.h"

#pragma warning (disable : 4996)

CListFavor::CListFavor(HINSTANCE hInst)
	: CListNewFolder (hInst)
	, m_pRootItem (NULL)
	, m_pShowItem (NULL)
	, m_pExitItem (NULL)
	, m_pHomeItem (NULL)
	, m_bModified (false)
{
	SetObjectName ("CListFavor");
	m_nType = LIST_VIEW_Favor;
}	

CListFavor::~CListFavor(void)
{
	SaveFile ();
	ReleaseItems ();
	YY_DEL_P (m_pRootItem);
	YY_DEL_P (m_pExitItem);
	YY_DEL_P (m_pHomeItem);
}

bool CListFavor::Create (HWND hWnd, CListRes * pRes)
{
	CListNewFolder::Create (hWnd, pRes);
	m_pNewMedia = new CListItem (yyLangGetText (YYTEXT_NewFile), ITEM_NewFile);
	m_pNewFolder = new CListItem (yyLangGetText (YYTEXT_NewFolder), ITEM_NewFolder);
	m_pExitItem = new CListItem (yyLangGetText (YYTEXT_ExitPlayer), ITEM_Exit);
	m_pHomeItem = new CListItem (yyLangGetText (YYTEXT_BackUpFolder), ITEM_Home);
	LoadFile ();
	return true;
}

bool CListFavor::FillItem (TCHAR * pPath)
{
	if (m_pSelItem != NULL)
	{
		if (m_pShowItem != NULL)
			m_pShowItem->m_nSelect = 0;
		if (m_pSelItem->m_nType == ITEM_Home)
			m_pShowItem = m_pShowItem->m_pParent;
		else
			m_pShowItem = m_pSelItem;
		m_bModified = true;
	}
	
	if (!FillShowItems ())
		return false;

	m_pSelItem = NULL;
	if (m_pShowItem != NULL)
	{
		TCHAR szName[1024];
		memset (szName, 0, sizeof (szName));
		CListItem * pParent = m_pShowItem;
		while (pParent != NULL)
		{
			_tcscpy (m_szFolder, pParent->m_pName);
			if (pParent != m_pShowItem)
				_tcscat (m_szFolder, _T("\\"));
			_tcscat (m_szFolder, szName);
			_tcscpy (szName, m_szFolder);
			pParent = pParent->m_pParent;
		}
		SendMessage (m_hWnd, WM_PLAYER_SetWinText, (WPARAM)m_szFolder, 0);
	}
	return true;
}

bool CListFavor::FillShowItems (void)
{
	ReleaseItems ();
	if (m_pShowItem == NULL)
		return true;
	if (m_pShowItem != m_pRootItem)
		m_lstItem.AddTail (m_pHomeItem);
	else
		m_lstItem.AddTail (m_pExitItem);

	CObjectList<CListItem> * pList = m_pShowItem->m_pChdList;
	if (pList != NULL)
	{
		CListItem * pItem = NULL;
		NODEPOS pos = pList->GetHeadPosition ();
		while (pos != NULL)
		{
			pItem = pList->GetNext (pos);
			m_lstItem.AddTail (pItem);
		}
	}
	m_lstItem.AddTail (m_pNewFolder);
	m_lstItem.AddTail (m_pNewMedia);

	SortItems ();
	return true;
}

bool CListFavor::ReleaseItems (void)
{
	Pause ();
	CAutoLock lock (&m_mtList);	
	m_lstItem.RemoveAll ();
	memset (m_szNewFolder, 0, sizeof (m_szNewFolder));
	ShowEditBox (false);
	return true;
}

bool CListFavor::DeleteItem (void)
{
	if (m_pSelItem == NULL)
		return false;
	if (m_pSelItem->m_nType == ITEM_Exit || m_pSelItem->m_nType == ITEM_Home ||
		m_pSelItem->m_nType == ITEM_NewFile || m_pSelItem->m_nType == ITEM_NewFolder)
		return false;
	if (m_pShowItem->m_pChdList == NULL)
		return false;
	CListItem * pDelItem = NULL;
	NODEPOS pos = m_pShowItem->m_pChdList->GetHeadPosition ();
	while (pos != NULL)
	{
		pDelItem = m_pShowItem->m_pChdList->GetNext (pos);
		if (pDelItem == m_pSelItem)
		{
			m_pShowItem->m_pChdList->Remove (pDelItem);
			delete pDelItem;
			ShowEditBox (false);
			if (FillShowItems ())
			{
				SetRectEmpty (&m_rcView);
				if (CreateDispBmp ())
					InvalidateRect (m_hWnd, NULL, FALSE);
			}
			return true;
		}
	}
	Start ();
	return false;
}

bool CListFavor::OnNewItemFile (void)
{
	TCHAR	szPath[81920];
	_tcscpy (szPath, _T("*.*"));
	OPENFILENAME	ofn;
	memset( &(ofn), 0, sizeof(ofn));
	ofn.lStructSize	= sizeof(ofn);
	ofn.hInstance = m_hInst;
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = TEXT("Media File (*.*)\0*.*\0");	
	ofn.lpstrFile = szPath;
	ofn.nMaxFile = sizeof (szPath);
	ofn.lpstrTitle = TEXT("Import Media File");
	ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT;		
	if (!GetOpenFileName(&ofn))
		return false;

	Pause ();

	CListItem *	pNewItem = NULL;
	NODEPOS	pos = NULL;
	TCHAR	szFile[1024];
	TCHAR * pPath = szPath;
	TCHAR * pName = szPath + _tcslen (pPath) + 1;
	if (*pName == 0)
	{
		pName = _tcsrchr (szPath, _T('\\'));
		*pName++ = 0;
	}
	while (*pName != 0)
	{
		_tcscpy (szFile, pPath);
		_tcscat (szFile, _T("\\"));
		_tcscat (szFile, pName);
		if (yyffGetType (szFile, 0) == YY_MEDIA_Video)
			pNewItem = new CListItem (szFile, pName, ITEM_Video);
		else if (yyffGetType (szFile, 0) == YY_MEDIA_Audio)
			pNewItem = new CListItem (szFile, pName, ITEM_Audio);
		else if (yyffGetType (szFile, 0) == YY_MEDIA_Image)
			pNewItem = new CListItem (szFile, pName, ITEM_Image);
		else
		{
			pName = pName + _tcslen (pName) + 1;
			continue;
		}
		if (m_pShowItem->m_pChdList == NULL)
			m_pShowItem->m_pChdList = new CObjectList<CListItem> ();
		pNewItem->m_pParent = m_pShowItem;
		m_pShowItem->m_pChdList->AddTail (pNewItem);
		m_bModified = true;
		pName = pName + _tcslen (pName) + 1;
	}
	if (pNewItem == NULL)
		return true;

	if (FillShowItems ())
	{
		SetRectEmpty (&m_rcView);
		if (CreateDispBmp ())
			InvalidateRect (m_hWnd, NULL, FALSE);
	}
	Start ();
	return true;
}

bool CListFavor::CreateNewFolder (void)
{
	if (_tcslen (m_szNewFolder) <= 0)
		return false;

	CObjectList<CListItem> * pList = m_pShowItem->m_pChdList;
	if (pList != NULL)
	{
		CListItem * pItem = NULL;
		NODEPOS pos = pList->GetHeadPosition ();
		while (pos != NULL)
		{
			pItem = pList->GetNext (pos);
			if (pItem->m_nType == ITEM_Folder)
			{
				if (!_tcscmp (m_szNewFolder, pItem->m_pName))
				{
					MessageBox (m_hWnd, yyLangGetText (YYTEXT_FolderExist), yyLangGetText (YYTEXT_Error), MB_OK);
					return false;
				}
			}
		}
	}

	Pause ();
	CListItem * pNewItem = new CListItem (m_szNewFolder, ITEM_Folder);
	if (m_pShowItem->m_pChdList == NULL)
		m_pShowItem->m_pChdList = new CObjectList<CListItem> ();
	pNewItem->m_pParent = m_pShowItem;
	m_pShowItem->m_pChdList->AddTail (pNewItem);
	m_bModified = true;
	memset (m_szNewFolder, 0, sizeof (m_szNewFolder));
	ShowEditBox (false);
	if (FillShowItems ())
	{
		SetRectEmpty (&m_rcView);
		if (CreateDispBmp ())
			InvalidateRect (m_hWnd, NULL, FALSE);
	}
	Start ();
	return true;
}

LRESULT	CListFavor::OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);

	if (wmId == ID_ITEM_DELETE)
	{
		DeleteItem ();
		return S_OK;
	}

	return S_FALSE;
}

LRESULT	CListFavor::OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	pt.x = LOWORD (lParam);
	pt.y = HIWORD (lParam);
	ClientToScreen (m_hWnd, &pt);

	CListItem * pItem = GetFocusItem (LOWORD (lParam), HIWORD (lParam));

	HMENU hPopup = NULL;
	if (CLangText::g_pLang->GetLang () == YYLANG_ENG)
		hPopup = LoadMenu (m_hInst, MAKEINTRESOURCE(IDR_MENU_POPUP));
	else
		hPopup = LoadMenu (m_hInst, MAKEINTRESOURCE(IDR_MENU_POPUP_CHN));
	HMENU hMenu = NULL;
	if (pItem != NULL && (pItem->m_nType == ITEM_Folder || pItem->m_nType == ITEM_Video || 
		pItem->m_nType == ITEM_Audio || pItem->m_nType == ITEM_Image))
		hMenu = GetSubMenu (hPopup, 0);
	if (hMenu != NULL)
	{
		if (pItem->m_nType == ITEM_Folder)
		{
			EnableMenuItem (hMenu, ID_FILE_PROPERTIES, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenu, ID_ITEM_IMPORTBOX, MF_BYCOMMAND | MF_GRAYED);
		}
		TrackPopupMenu (hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, m_hWnd, NULL);
	}
	DestroyMenu (hPopup);

	return S_FALSE;
}

LRESULT	CListFavor::OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!m_bNewFolder && (wParam == VK_DELETE || wParam == VK_BACK))
	{
		DeleteItem ();
		return S_OK;
	}

	return CListNewFolder::OnKeyUp (uMsg, wParam, lParam);
}

LRESULT CListFavor::OnDropFiles (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDROP	hDrop = (HDROP) wParam;
	int		nFiles = DragQueryFile (hDrop, 0XFFFFFFFF, NULL, 0);
	if (nFiles <= 0)
		return S_OK;

	Pause ();

	CListItem *	pNewItem = NULL;
	NODEPOS	pos = NULL;
	TCHAR *		pName = NULL;
	TCHAR		szFile[1024];
	for (int i = 0; i < nFiles; i++)
	{
		memset (szFile, 0, sizeof (szFile));
		int nNameLen = DragQueryFile (hDrop, i, szFile, sizeof (szFile));
		if (nNameLen <= 0)
			continue;
		pName = _tcsrchr (szFile, _T('\\'));
		if (pName == NULL)
			continue;
		pName++;
		if (yyffGetType (szFile, 0) == YY_MEDIA_Video)
			pNewItem = new CListItem (szFile, pName, ITEM_Video);
		else if (yyffGetType (szFile, 0) == YY_MEDIA_Audio)
			pNewItem = new CListItem (szFile, pName, ITEM_Audio);
		else if (yyffGetType (szFile, 0) == YY_MEDIA_Image)
			pNewItem = new CListItem (szFile, pName, ITEM_Image);
		else
		{
			pName = pName + _tcslen (pName) + 1;
			continue;
		}
		if (m_pShowItem->m_pChdList == NULL)
			m_pShowItem->m_pChdList = new CObjectList<CListItem> ();
		pNewItem->m_pParent = m_pShowItem;
		m_pShowItem->m_pChdList->AddTail (pNewItem);
		m_bModified = true;
	}
	if (pNewItem == NULL)
		return S_OK;

	if (FillShowItems ())
	{
		SetRectEmpty (&m_rcView);
		if (CreateDispBmp ())
			InvalidateRect (m_hWnd, NULL, FALSE);
	}
	Start ();
	
	return S_OK;
}

bool CListFavor::LoadFile (void)
{
	TCHAR szFile[256];
	yyGetDataPath (m_hWnd, szFile, sizeof (szFile));
	_tcscat (szFile, _T("yyMyFavor.dat"));

	if (m_pRootItem == NULL)
	{
		m_pRootItem = new CListItem (_T("MyFavor"), ITEM_Folder);
		m_pShowItem = m_pRootItem;
	}

	CListStore store (m_hInst);
	bool bRC = store.Load (szFile, m_pRootItem);
	m_pShowItem = store.GetSelItem ();
	if (m_pShowItem == NULL)
		m_pShowItem = m_pRootItem;
	return bRC;
}

bool CListFavor::SaveFile (void)
{
	if (m_pRootItem == NULL || !m_bModified)
		return false;

	TCHAR szFile[256];
	yyGetDataPath (NULL, szFile, sizeof (szFile));
	_tcscat (szFile, _T("yyMyFavor.dat"));

	m_pShowItem->m_nSelect = 1;
	CListStore store (m_hInst);
	return store.Save (szFile, m_pRootItem);
}

