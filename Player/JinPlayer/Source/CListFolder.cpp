/*******************************************************************************
	File:		CListFolder.cpp

	Contains:	The folder list implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"
#include "RPlayerDef.h"

#include "CListFolder.h"

#include "CRegMng.h"
#include "CLangText.h"
#include "resource.h"
#include "USystemFunc.h"
#include "UFileFormat.h"
#include "UStringFunc.h"
#include "yyLog.h"

#pragma warning (disable : 4996)

CListFolder::CListFolder(HINSTANCE hInst)
	: CListView (hInst)
{
	SetObjectName ("CListFolder");
	m_nType = LIST_VIEW_Folder;

	memset (m_szFolder, 0, sizeof (m_szFolder));
	_tcscpy (m_szRoot, _T(""));
	TCHAR * pRoot = CRegMng::g_pRegMng->GetTextValue (_T("Root"));
	if (_tcslen (pRoot) > 0)
		_tcscpy (m_szRoot, pRoot);
	TCHAR * pFolder = CRegMng::g_pRegMng->GetTextValue (_T("Folder"));
	if (_tcslen (pFolder) == 0)
		_tcscpy (m_szFolder, m_szRoot);
	else
		_tcscpy (m_szFolder, pFolder);
}	

CListFolder::~CListFolder(void)
{
	CRegMng::g_pRegMng->SetTextValue (_T("Root"), m_szRoot);
	CRegMng::g_pRegMng->SetTextValue (_T("Folder"), m_szFolder);
}

bool CListFolder::Create (HWND hWnd, CListRes * pRes)
{
	CListView::Create (hWnd, pRes);

	return true;
}

bool CListFolder::FillItem (TCHAR * pPath)
{
	TCHAR szFolder[1024];
	if (pPath != NULL)
		_tcscpy (szFolder, pPath);
	else
		_tcscpy (szFolder, m_szFolder);
	DWORD dwAttr = GetFileAttributes (szFolder);
	while (dwAttr == -1)
	{
		TCHAR * pPos = _tcsrchr (szFolder, _T('\\'));
		if (pPos == NULL)
		{
			_tcscpy (szFolder, _T(""));
			break;
		}
		*pPos = 0;
		dwAttr = GetFileAttributes (szFolder);
	}

	CListItem * pItem = NULL;
	if (_tcslen (szFolder) <= 0)
	{
		ReleaseItems ();
		
		m_lstItem.AddTail (new CListItem (yyLangGetText (YYTEXT_ExitPlayer), ITEM_Exit));

		TCHAR szDrives[1024];
		memset (szDrives, 0, sizeof (szDrives));
		DWORD dwRC = GetLogicalDriveStrings (1024, szDrives);
		TCHAR * pDrive = szDrives;
		while (*pDrive != 0)
		{
			int nTextLen = _tcslen (pDrive);
			*(pDrive + 2) = 0;
			if (HasItemInFolder (pDrive))
			{
				pItem = new CListItem (pDrive, ITEM_Folder);
				pItem->m_nIndex = m_lstItem.GetCount ();
				m_lstItem.AddTail (pItem);
			}
			pDrive = pDrive + nTextLen + 1;
		}
	}
	else
	{
		TCHAR	szFilter[1024];
		_tcscpy (szFilter, szFolder);
		_tcscat (szFilter, _T("\\*.*"));
		WIN32_FIND_DATA  data;
		HANDLE  hFind = FindFirstFile(szFilter,&data);
		if (hFind == INVALID_HANDLE_VALUE)
			return false;

		ReleaseItems ();

		if (_tcslen (szFolder) > 0 && _tcscmp (szFolder, m_szRoot))
			m_lstItem.AddTail (new CListItem (yyLangGetText (YYTEXT_BackUpFolder), ITEM_Home));
		else
			m_lstItem.AddTail (new CListItem (yyLangGetText (YYTEXT_ExitPlayer), ITEM_Exit));

		do
		{
			if (!_tcscmp (data.cFileName, _T(".")) || !_tcscmp (data.cFileName, _T("..")))
				continue;	

			if ((data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN || 
				(data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM)
				continue;
			
			TCHAR szName[256];
			_tcscpy (szName, data.cFileName);
	
			if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				TCHAR szSubFolder[1024];
				_tcscpy (szSubFolder, szFolder);
				_tcscat (szSubFolder, _T("\\"));
				_tcscat (szSubFolder, data.cFileName);
				if (HasItemInFolder (szSubFolder))
					m_lstItem.AddTail (new CListItem (szName, ITEM_Folder));
				continue;
			}

			if (yyffGetType (szName, 0) == YY_MEDIA_Video)
				m_lstItem.AddTail (new CListItem (szName, ITEM_Video));
			else if (yyffGetType (szName, 0) == YY_MEDIA_Audio)
				m_lstItem.AddTail (new CListItem (szName, ITEM_Audio));
			else if (yyffGetType (szName, 0) == YY_MEDIA_Image)
				m_lstItem.AddTail (new CListItem (szName, ITEM_Image));
		}while(FindNextFile(hFind, &data));
		FindClose (hFind);

		NODEPOS pos = m_lstItem.GetHeadPosition ();
		while (pos != NULL)
		{
			pItem = m_lstItem.GetNext (pos);
			pItem->m_pFolder = m_szFolder;
		}

		LoadItemThumb (szFolder);

		SortItems ();
	}
	m_pSelItem = NULL;
	_tcscpy (m_szFolder, szFolder);
	SendMessage (m_hWnd, WM_PLAYER_SetWinText, (WPARAM)m_szFolder, 0);
	CRegMng::g_pRegMng->SetTextValue (_T("Folder"), m_szFolder);
	return true;
}

bool CListFolder::HasItemInFolder (TCHAR * pFolder)
{
	TCHAR	szName[256];
	TCHAR	szFilter[1024];
	_tcscpy (szFilter, pFolder);
	_tcscat (szFilter, _T("\\*.*"));
	WIN32_FIND_DATA  data;
	HANDLE  hFind = FindFirstFile(szFilter,&data);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	do
	{
		if (!_tcscmp (data.cFileName, _T(".")) || !_tcscmp (data.cFileName, _T("..")))
			continue;	
		if (m_nType == LIST_VIEW_Folder)
		{
			if ((data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN || 
				(data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM)
				continue;
		}
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			FindClose (hFind);
			return true;
			// continue;
		}
		_tcscpy (szName, data.cFileName);
		if (yyffGetType (szName, 0) != YY_MEDIA_Data)
		{
			FindClose (hFind);
			return true;
		}
	}while(FindNextFile(hFind, &data));
	FindClose (hFind);

	TCHAR szFolder[1024];
	hFind = FindFirstFile(szFilter,&data);
	do
	{
		if (!_tcscmp (data.cFileName, _T(".")) || !_tcscmp (data.cFileName, _T("..")))
			continue;	

		if (m_nType == LIST_VIEW_Folder)
		{
			if ((data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN || 
				(data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM)
				continue;
		}
		
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			_tcscpy (szFolder, pFolder);
			_tcscat (szFolder, _T("\\"));
			_tcscat (szFolder, data.cFileName);
			if (HasItemInFolder (szFolder))
			{
				FindClose (hFind);
				return true;
			}
		}
	}while(FindNextFile(hFind, &data));
	FindClose (hFind);

	return false;
}

bool CListFolder::DeleteItem (void)
{
	if (m_pSelItem == NULL)
		return false;
	if (m_pSelItem->m_nType != ITEM_Folder && m_pSelItem->m_nType != ITEM_Video && 
		m_pSelItem->m_nType != ITEM_Audio && m_pSelItem->m_nType != ITEM_Image)
		return false;

	if (MessageBox (m_hWnd, yyLangGetText (YYTEXT_DeleteFile), yyLangGetText (YYTEXT_DelFile), MB_YESNO) != IDYES)
		return false;

	TCHAR szFile[1024];
	if (m_pSelItem->m_nType == ITEM_Folder)
	{
		_tcscpy (szFile, m_szFolder);
		_tcscat (szFile, _T("\\"));
		_tcscat (szFile, m_pSelItem->m_pName);
		yyDeleteFolder (szFile);
	}
	else
	{
		m_pSelItem->GetFile (szFile, sizeof (szFile));
		DeleteFile (szFile);
	}

	Pause ();
	m_lstItem.Remove (m_pSelItem);
	delete m_pSelItem;
	if (SortItems ())
	{
		SetRectEmpty (&m_rcView);
		if (CreateDispBmp ())
			InvalidateRect (m_hWnd, NULL, FALSE);
	}
	Start ();

	return true;
}

LRESULT	CListFolder::OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);

	if (wmId == ID_ITEM_DELETE)
	{
		DeleteItem ();
		return S_OK;
	}

	return S_FALSE;
}

LRESULT	CListFolder::OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		//EnableMenuItem (hMenu, ID_ITEM_DELETE, MF_BYCOMMAND | MF_GRAYED);
		TrackPopupMenu (hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, m_hWnd, NULL);
	}
	DestroyMenu (hPopup);

	return S_FALSE;
}

LRESULT CListFolder::OnDropFiles (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_FALSE;
}
