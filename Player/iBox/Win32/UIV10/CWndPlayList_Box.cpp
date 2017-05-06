/*******************************************************************************
	File:		CWndPlayList.cpp

	Contains:	Window play list view code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CWndPlayList.h"
#include "resource.h"
#include "CDlgNewBox.h"
#include "CDlgFileCopy.H"

#include "USystemFunc.h"
#include "UFileFunc.h"

#include "yyLog.h"

#pragma warning (disable : 4996)

bool CWndPlayList::FillMyBoxItems (TCHAR * pBox)
{
	ReleaseList ();

	if (pBox != NULL && _tcslen (pBox) > 0)
	{
		FillMediaItems (pBox);

		MEDIA_Item * pNewItem = CreateItem (_T("New Folder"), ITEM_NewFolder);
		pNewItem->nIndex = m_lstItem.GetCount ();
		m_lstItem.AddTail (pNewItem);
		pNewItem = CreateItem (_T("New File"), ITEM_NewFile);
		pNewItem->nIndex = m_lstItem.GetCount ();
		m_lstItem.AddTail (pNewItem);
	}

	CreateBitmapBG ();
	ShowBitmapBG ();
	SetWindowText (GetParent (m_hWnd), _T("MyBox"));

	return true;
}

bool CWndPlayList::AddNewBox (void)
{
	CDlgNewBox dlgBox (m_hInst, m_hWnd);
	dlgBox.SetLangText (m_pLangText);
	if (dlgBox.OpenDlg (m_szBoxPath) == IDCANCEL)
		return false;
	
	if (_tcslen (m_szBoxPath) <= 0)
		_tcscpy (m_szBoxPath, dlgBox.GetFolder ());
	_tcscpy (m_szBoxName, dlgBox.GetPW ());
	m_keyBase.CreateText (m_szBoxName, sizeof (m_szBoxName));

	TCHAR szBoxFolder[1024];
	_tcscpy (szBoxFolder, m_szBoxPath);
	_tcscat (szBoxFolder, _T("\\"));
	_tcscat (szBoxFolder, m_szBoxName);

	FillMyBoxItems (szBoxFolder);

	m_nBoxNum++;

	return true;
}

bool CWndPlayList::OpenBox (TCHAR * pText)
{
	if (_tcslen (m_szBoxPath) <= 0)
		return false;

	TCHAR szPW[256];
	if (pText == NULL)
	{
		CDlgNewBox dlgBox (m_hInst, m_hWnd);
		dlgBox.SetLangText (m_pLangText);
		if (dlgBox.OpenDlg (m_szBoxPath, false) == IDCANCEL)
			return false;
		_tcscpy (szPW, dlgBox.GetPW ());
	}
	else
	{
		_tcscpy (szPW, pText);
	}

	TCHAR	szBoxPath[1024];
	TCHAR	szFilter[1024];
	_tcscpy (szFilter, m_szBoxPath);
	_tcscat (szFilter, _T("\\*.*"));
	WIN32_FIND_DATA  data;
	HANDLE  hFind = FindFirstFile(szFilter,&data);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		if (pText == NULL)
			MessageBox (m_hWnd, m_pLangText->GetText (YYTEXT_FindContent), m_pLangText->GetText (YYTEXT_Error), MB_OK);
		return false;
	}

	_tcscpy (szBoxPath, _T(""));
	do
	{
		if (_tcslen (data.cFileName) < 3)
			continue;	
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			_tcscpy (szBoxPath, data.cFileName);
			m_keyBase.RestoreText (szBoxPath, sizeof (szBoxPath));
			if(!_tcscmp (szBoxPath, szPW))
			{
				_tcscpy (szBoxPath, m_szBoxPath);
				_tcscat (szBoxPath, _T("\\"));
				_tcscat (szBoxPath, data.cFileName);
				break;
			}
		}
	}while(FindNextFile(hFind, &data));
	FindClose (hFind);

	if (_tcslen (szBoxPath) <= 0)
	{
		if (pText == NULL)
			MessageBox (m_hWnd, m_pLangText->GetText (YYTEXT_FindContent), m_pLangText->GetText (YYTEXT_Error), MB_OK);
		return false;
	}
	m_keyBase.SetPassWord (szPW);

	FillMyBoxItems (szBoxPath);

	return true;
}

bool CWndPlayList::ImportFileInBox (TCHAR * pSource, TCHAR * pTarget)
{
	TCHAR szTgtFile[1024];
	_tcscpy (szTgtFile, m_szFolder);
	TCHAR * pName = _tcsrchr (pSource, _T('\\'));
	_tcscat (szTgtFile, pName);

	CDlgFileCopy dlgCopy (m_hInst, m_hWnd);
	dlgCopy.SetLangText (m_pLangText);
	if (dlgCopy.OpenDlg (pSource, szTgtFile)== IDCANCEL)
		return false;

	_tcscpy (pTarget, szTgtFile);

	return true;
}

bool CWndPlayList::ExportFileInBox (void)
{
	if (m_pSelectedItem == NULL)
		return false;
	if (m_pSelectedItem->nType != ITEM_Video && m_pSelectedItem->nType != ITEM_Audio)
		return false;

	TCHAR szFile[1024];
	if (m_pSelectedItem->pPath != NULL)
		_tcscpy (szFile, m_pSelectedItem->pPath);
	else
	{
		_tcscpy (szFile, m_szFolder);
		_tcscat (szFile, _T("\\"));
		_tcscat (szFile, m_pSelectedItem->pName);
	}

	CDlgFileCopy dlgCopy (m_hInst, m_hWnd);
	dlgCopy.SetLangText (m_pLangText);
	if (dlgCopy.OpenDlg (szFile, NULL)== IDCANCEL)
		return false;

	return true;
}

bool CWndPlayList::NewFolderInBox (TCHAR * pName)
{
	TCHAR szPath[1024];
	_tcscpy (szPath, m_szFolder);
	_tcscat (szPath, _T("\\"));
	_tcscat (szPath, pName);

	DWORD dwAttr = GetFileAttributes (szPath);
	if (dwAttr != -1)
		return false;

	BOOL bRC = CreateDirectory (szPath, NULL);
	dwAttr = GetFileAttributes (szPath);
	dwAttr |= FILE_ATTRIBUTE_HIDDEN;
	bRC = SetFileAttributes (szPath, dwAttr);

	return true;
}
