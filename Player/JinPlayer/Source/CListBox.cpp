/*******************************************************************************
	File:		CListBox.cpp

	Contains:	The box list implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CListBox.h"
#include "CDlgNewBox.h"
#include "CDlgExport.h"
#include "CDlgImport.h"

#include "CRegMng.h"

#include "resource.h"
#include "CLangText.h"
#include "RPlayerDef.h"
#include "USystemFunc.h"
#include "UStringFunc.h"
#include "UFileFunc.h"
#include "UFileFormat.h"
#include "yyLog.h"

#pragma warning (disable : 4996)

CListBox::CListBox(HINSTANCE hInst)
	: CListNewFolder (hInst)
	, m_nTimerPW (0)
{
	SetObjectName ("CListBox");
	m_nType = LIST_VIEW_MyBox;

	TCHAR * pBoxPath = CRegMng::g_pRegMng->GetTextValue (_T("BoxRootPath"));
	if (_tcslen (pBoxPath) > 0)
		_tcscpy (m_szRoot, pBoxPath);
	memset (m_szPassWord, 0, sizeof (m_szPassWord));
}	

CListBox::~CListBox(void)
{
}

bool CListBox::Create (HWND hWnd, CListRes * pRes)
{
	CListNewFolder::Create (hWnd, pRes);

	if (_tcslen (m_szRoot) <= 0)
	{
		TCHAR * pBoxPath = CRegMng::g_pRegMng->GetTextValue (_T("BoxRootPath"));
		if (_tcslen (pBoxPath) > 0)
			_tcscpy (m_szRoot, pBoxPath);
	}

	if (_tcslen (m_szRoot) <= 0)
		SetTimer (m_hWnd, WT_MYBOX_CreateBox, 100, NULL);

	return true;
}

bool CListBox::FillItem (TCHAR * pPath)
{
	if (!CBaseKey::g_Key->IsUsed ())
		return true;
	CListItem * pItem = NULL;
	TCHAR	szFilter[1024];
	TCHAR	szFolder[1024];
	if (pPath == NULL)
	{
		_tcscpy (szFolder, m_szRoot);
		_tcscat (szFolder, _T("\\"));
		_tcscat (szFolder, CBaseKey::g_Key->GetWKey ());
	}
	else
	{
		_tcscpy (szFolder, pPath);
	}
	_tcscpy (szFilter, szFolder);
	_tcscat (szFilter, _T("\\*.*"));
	WIN32_FIND_DATA  data;
	HANDLE  hFind = FindFirstFile(szFilter,&data);
	if (hFind == INVALID_HANDLE_VALUE)
		return true;

	ReleaseItems ();

	TCHAR * pPos = _tcsrchr (szFolder, _T('\\'));
	if ((unsigned int)(pPos - szFolder) > _tcslen (m_szRoot))
		m_lstItem.AddTail (new CListItem (yyLangGetText (YYTEXT_BackUpFolder), ITEM_Home));
	else
		m_lstItem.AddTail (new CListItem (yyLangGetText (YYTEXT_ExitPlayer), ITEM_Exit));
	do
	{
		if (!_tcscmp (data.cFileName, _T(".")) || !_tcscmp (data.cFileName, _T("..")))
			continue;	
		
		TCHAR szName[1024];
		_tcscpy (szName, data.cFileName);
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			TCHAR szDir[1024];
			_tcscpy (szDir, szFolder);
			_tcscat (szDir, _T("\\"));
			_tcscat (szDir, szName);
			DWORD dwAttr = GetFileAttributes (szDir);
			if (dwAttr == -1)
				continue;
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

	_tcscpy (m_szFolder, szFolder);

	CListItem * pNewFolder = new CListItem (yyLangGetText (YYTEXT_NewFolder), ITEM_NewFolder);
	pNewFolder->m_nIndex = m_lstItem.GetCount ();
	m_lstItem.AddTail (pNewFolder);

	CListItem * pNewFile = new CListItem (yyLangGetText (YYTEXT_NewFile), ITEM_NewFile);
	pNewFile->m_nIndex = m_lstItem.GetCount ();
	m_lstItem.AddTail (pNewFile);

	m_pSelItem = NULL;
	memset (m_szNewFolder, 0, sizeof (m_szNewFolder));
	SetRectEmpty (&m_rcView);
	SendMessage (m_hWnd, WM_PLAYER_SetWinText, (WPARAM)yyLangGetText (YYTEXT_MyBox), 0);

	return true;
}

bool CListBox::OnNewItemFile (void)
{
	CDlgImport dlgImport(m_hInst, m_hWnd, CBaseKey::g_Key);
	dlgImport.OpenDlg (m_szFolder);
	if (dlgImport.GetFiles () <= 0)
		return false;

	if (FillItem (m_szFolder))
	{
		SetRectEmpty (&m_rcView);
		if (CreateDispBmp ())
			InvalidateRect (m_hWnd, NULL, FALSE);
		Start ();
	}
	return true;
}

bool CListBox::CreateNewFolder (void)
{
	if (_tcslen (m_szNewFolder) <= 0)
		return false;

	// Create the folder
	TCHAR szPath[1024];
	_tcscpy (szPath, m_szFolder);
	_tcscat (szPath, _T("\\"));
	_tcscat (szPath, m_szNewFolder);
	DWORD dwAttr = GetFileAttributes (szPath);
	if (dwAttr != -1)
	{
		MessageBox (m_hWnd, yyLangGetText (YYTEXT_FolderExist), yyLangGetText (YYTEXT_Error), MB_OK);
		return false;
	}

	BOOL bRC = CreateDirectory (szPath, NULL);
	dwAttr = GetFileAttributes (szPath);
	dwAttr |= FILE_ATTRIBUTE_HIDDEN;
	bRC = SetFileAttributes (szPath, dwAttr);

	Pause ();
	CListItem * pItem = new CListItem (m_szNewFolder, ITEM_Folder);
	m_lstItem.AddTail (pItem);
	SortItems ();
	SetRectEmpty (&m_rcView);
	if (CreateDispBmp ())
		InvalidateRect (m_hWnd, NULL, FALSE);
	memset (m_szNewFolder, 0, sizeof (m_szNewFolder));
	ShowEditBox (false);
	Start ();
	return true;
}

bool CListBox::AddNewBox (void)
{
	CDlgNewBox dlgBox (m_hInst, m_hWnd, CBaseKey::g_Key);
	if (dlgBox.OpenDlg (m_szRoot, true) == IDCANCEL)
		return false;

	if (_tcslen (m_szRoot) <= 0)
		_tcscpy (m_szRoot, dlgBox.GetFolder ());
	TCHAR szFolder[1024];
	_tcscat (szFolder, m_szRoot);
	_tcscat (szFolder, _T("\\"));
	_tcscat (szFolder, CBaseKey::g_Key->GetWKey ());
	if (FillItem (szFolder))
	{
		SetRectEmpty (&m_rcView);
		if (CreateDispBmp ())
			InvalidateRect (m_hWnd, NULL, FALSE);
	}
	return true;
}

bool CListBox::OpenBox (TCHAR * pText)
{
	if (_tcslen (m_szRoot) <= 0)
		return false;

	TCHAR szPW[256];
	if (pText == NULL)
	{
		CDlgNewBox dlgBox (m_hInst, m_hWnd, CBaseKey::g_Key);
		if (dlgBox.OpenDlg (m_szRoot, false) == IDCANCEL)
			return false;
		_tcscpy (szPW, dlgBox.GetPW ());
	}
	else
	{
		_tcscpy (szPW, pText);
		CBaseKey::g_Key->CreateKey (szPW);
	}

	TCHAR	szBoxPath[1024];
	_tcscpy (szBoxPath, m_szRoot);
	_tcscat (szBoxPath, _T("\\"));
	_tcscat (szBoxPath, CBaseKey::g_Key->GetWKey ());
	DWORD dwAttr = GetFileAttributes (szBoxPath);
	if (dwAttr == -1)
	{
		if (pText == NULL)
			MessageBox (m_hWnd, yyLangGetText (YYTEXT_FindContent), yyLangGetText (YYTEXT_Error), MB_OK);
		return false;
	}
	if (FillItem (szBoxPath))
	{
		SetRectEmpty (&m_rcView);
		if (CreateDispBmp ())
			InvalidateRect (m_hWnd, NULL, FALSE);
	}
	Start ();
	return true;
}

TCHAR *	CListBox::GetFolder (void)
{
	return yyLangGetText (YYTEXT_MyBox);
}

bool CListBox::ExportFile (void)
{
	if (m_pSelItem == NULL)
		return false;
	if (m_pSelItem->m_nType != ITEM_Video && m_pSelItem->m_nType != ITEM_Audio && m_pSelItem->m_nType != ITEM_Image)
		return false;

	TCHAR szFile[1024];
	if (m_pSelItem->m_pPath != NULL)
		_tcscpy (szFile, m_pSelItem->m_pPath);
	else
	{
		_tcscpy (szFile, m_szFolder);
		_tcscat (szFile, _T("\\"));
		_tcscat (szFile, m_pSelItem->m_pName);
	}

	CDlgExport dlgExport (m_hInst, m_hWnd, CBaseKey::g_Key);
	dlgExport.OpenDlg (szFile);
	return true;
}

bool CListBox::DeleteItem (void)
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

LRESULT	CListBox::OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	if (wmId == ID_MYBOX_NEWBOX)
	{
		AddNewBox ();
		return S_OK;
	}
	else if (wmId == ID_MYBOX_OPENBOX)
	{
		OpenBox (NULL);
		return S_OK;
	}
	else if (wmId == ID_BOXITEM_EXPORT)
	{
		ExportFile ();
		return S_OK;
	}
	else if (wmId == ID_ITEM_DELETE)
	{
		DeleteItem ();
		return S_OK;
	}

	return S_FALSE;
}

LRESULT	CListBox::OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	HMENU hMenu = GetSubMenu (hPopup, 1);
	if (pItem != NULL && (pItem->m_nType == ITEM_Folder || pItem->m_nType == ITEM_Video || 
		pItem->m_nType == ITEM_Audio || pItem->m_nType == ITEM_Image))
	{
		hMenu = GetSubMenu (hPopup, 2);
		if (pItem->m_nType == ITEM_Folder)
		{
			EnableMenuItem (hMenu, ID_FILE_PROPERTIES, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem (hMenu, ID_BOXITEM_EXPORT, MF_BYCOMMAND | MF_GRAYED);
		}
	}
	TrackPopupMenu (hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, m_hWnd, NULL);
	DestroyMenu (hPopup);

	return S_FALSE;
}

LRESULT	CListBox::OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!m_bNewFolder && (wParam == VK_DELETE || wParam == VK_BACK))
	{
		if (m_pSelItem == NULL)
			return S_FALSE;
		if (m_pSelItem->m_nType == ITEM_Home || m_pSelItem->m_nType == ITEM_NewFile || m_pSelItem->m_nType == ITEM_NewFolder)
			return S_FALSE;
		// Delete the select item.
	}

	return CListNewFolder::OnKeyUp (uMsg, wParam, lParam);
}

LRESULT	CListBox::OnChar (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_RETURN || wParam == VK_BACK || wParam == VK_ESCAPE )
		return S_FALSE;
	if (!m_bNewFolder)
	{
		m_szPassWord[_tcslen (m_szPassWord)] = (TCHAR)wParam;
		if (_tcslen (m_szPassWord) >= 6)
		{
			KillTimer (m_hWnd, WT_TYPE_PassWord);
			m_nTimerPW = 0;

			OpenBox (m_szPassWord);
			memset (m_szPassWord, 0, sizeof (m_szPassWord));
		}
		if (m_nTimerPW == 0)
			m_nTimerPW = SetTimer (m_hWnd, WT_TYPE_PassWord, 3000, NULL);
	}
	else
		CListNewFolder::OnChar (uMsg, wParam, lParam);

	return S_FALSE;
}

LRESULT CListBox::OnDropFiles (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_lstItem.GetCount () <= 0)
		return S_OK;

	HDROP	hDrop = (HDROP) wParam;
	int		nFiles = DragQueryFile (hDrop, 0XFFFFFFFF, NULL, 0);
	if (nFiles <= 0)
		return S_OK;
	TCHAR **	ppFiles = new TCHAR*[nFiles];
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

		ppFiles[i] = new TCHAR[1024];
		_tcscpy (ppFiles[i], szFile);

		pName++;
		pName = pName + _tcslen (pName) + 1;
	}

	CDlgImport dlgImport(m_hInst, m_hWnd, CBaseKey::g_Key);
	dlgImport.SetSource (ppFiles, nFiles);
	dlgImport.OpenDlg (m_szFolder);
	if (dlgImport.GetFiles () <= 0)
		return false;

	if (FillItem (m_szFolder))
	{
		SetRectEmpty (&m_rcView);
		if (CreateDispBmp ())
			InvalidateRect (m_hWnd, NULL, FALSE);
		Start ();
	}

	return S_OK;
}

LRESULT	CListBox::OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WT_TYPE_PassWord)
	{
		KillTimer (m_hWnd, WT_TYPE_PassWord);
		m_nTimerPW = 0;
		memset (m_szPassWord, 0, sizeof (m_szPassWord));
		return S_OK;
	}
	else if (wParam == WT_MYBOX_CreateBox)
	{
		KillTimer (m_hWnd, WT_MYBOX_CreateBox);
		AddNewBox ();
	}
	else if (wParam == WT_TYPE_NewFolder)
	{
		m_nTimerCount++;
		InvalidateRect (m_hWnd, &m_rcEdit, FALSE);
	}

	return S_FALSE;
}
