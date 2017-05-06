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

#include "CRegMng.h"
#include "USystemFunc.h"

#include "yyLog.h"

#pragma warning (disable : 4996)

#define	MEDIA_ITEM_TEXTID		"It is start pos of item!"

bool CWndPlayList::FillFavorItems (MEDIA_Item * pDispItem)
{
	if (m_pFavorItem == NULL)
		LoadMyFavor ();
	if (pDispItem == NULL)
		pDispItem = m_pCurItem;

	ReleaseList ();

	MEDIA_Item * pItem = NULL;
	MEDIA_Item * pNewItem = NULL;
	if (pDispItem->pParent != NULL)
	{
		pNewItem = CreateItem (pDispItem->pName, ITEM_Home);
		pNewItem->nIndex = m_lstItem.GetCount ();
		m_lstItem.AddTail (pNewItem);
	}

	POSITION pos = NULL;
	CObjectList<MEDIA_Item> * pList = pDispItem->pChildList;
	if (pList != NULL)
	{
		pos = pList->GetHeadPosition ();
		while (pos != NULL)
		{
			pItem = pList->GetNext (pos);
			pNewItem = CloneItem (pItem);
			m_lstItem.AddTail (pNewItem);
		}
		SortDispItems ();
	}

	pNewItem = CreateItem (_T("New Folder"), ITEM_NewFolder);
	pNewItem->nIndex = m_lstItem.GetCount ();
	m_lstItem.AddTail (pNewItem);
	pNewItem = CreateItem (_T("New File"), ITEM_NewFile);
	pNewItem->nIndex = m_lstItem.GetCount ();
	m_lstItem.AddTail (pNewItem);

	if (m_nAnimateType == ITEM_Home)
		CopyLastVideo (m_hWnd);

	CreateBitmapBG ();

	ShowBitmapBG ();

	TCHAR szTitle[1024];
	TCHAR szName[1024];
	_tcscpy (szTitle, pDispItem->pName);
	MEDIA_Item * pParant = pDispItem->pParent;
	while (pParant != NULL)
	{
		_tcscpy (szName, szTitle);
		_tcscpy (szTitle, pParant->pName);
		_tcscat (szTitle, _T("\\"));
		_tcscat (szTitle, szName);
		pParant = pParant->pParent;
	}
	SetWindowText (GetParent (m_hWnd), szTitle);

	return true;
}

bool CWndPlayList::FillMediaItems (TCHAR * pFolder)
{
	ReleaseList ();

	MEDIA_Item * pItem = NULL;
	if (_tcslen (pFolder) <= 0)
	{
		ReleaseList ();
		TCHAR szDrives[1024];
		memset (szDrives, 0, sizeof (szDrives));
		DWORD dwRC = GetLogicalDriveStrings (1024, szDrives);
		TCHAR * pDrive = szDrives;
		while (*pDrive != 0)
		{
			int nTextLen = _tcslen (pDrive);
			*(pDrive + 2) = 0;
			pItem = CreateItem (pDrive, ITEM_Folder);
			pItem->nIndex = m_lstItem.GetCount ();
			m_lstItem.AddTail (pItem);
			pDrive = pDrive + nTextLen + 1;
		}
		_tcscpy (m_szFolder, _T(""));
	}
	else
	{
		TCHAR	szFilter[1024];
		_tcscpy (szFilter, pFolder);
		_tcscat (szFilter, _T("\\*.*"));
		WIN32_FIND_DATA  data;
		HANDLE  hFind = FindFirstFile(szFilter,&data);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			if (m_lstItem.GetCount () == 0)
			{
				_tcscpy (m_szRoot, _T(""));
				_tcscpy (m_szFolder, m_szRoot);
				SetTimer (m_hWnd, TIMER_FILL_MEDIA_ITEMS, 10, NULL);
			}
			return false;
		}

		_tcscpy (m_szFolder, pFolder);

		ReleaseList ();
		TCHAR *			pExt = NULL;
		int				nExtLen = 0;
		char *			pExtChar = NULL;

		if (m_lvType == LIST_VIEW_Folder)
		{
			if (_tcslen (m_szFolder) > 0 && _tcscmp (m_szFolder, m_szRoot))
			{
				TCHAR * pName = _tcsrchr (m_szFolder, _T('\\'));
				if (pName != NULL)
					m_lstItem.AddTail (CreateItem (pName + 1, ITEM_Home));
				else
					m_lstItem.AddTail (CreateItem (m_szFolder, ITEM_Home));
			}
		}
		else
		{
			if (_tcslen (m_szFolder) > 0)
			{
				TCHAR * pLastName = _tcsrchr (m_szFolder, _T('\\'));
				if (pLastName != NULL)
				{
					int nNameLen = _tcslen (m_szFolder) - _tcslen (pLastName);
					if (_tcsncmp (m_szFolder, m_szBoxPath, nNameLen))
					{
						pLastName++;
						m_lstItem.AddTail (CreateItem (pLastName, ITEM_Home));
					}
				}
			}
		}
		do
		{
			if (!_tcscmp (data.cFileName, _T(".")) || !_tcscmp (data.cFileName, _T("..")))
				continue;	

			if (m_lvType != LIST_VIEW_MyBox)
			{
				if ((data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN || 
					(data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM)
					continue;
			}
			TCHAR szName[256];
			_tcscpy (szName, data.cFileName);
	
			if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				m_lstItem.AddTail (CreateItem (szName, ITEM_Folder));
				continue;
			}

			pExt = _tcsrchr (szName, _T('.'));
			if (pExt == NULL)
				continue;

			nExtLen = _tcslen (pExt) * sizeof (TCHAR);
			pExtChar = (char *)pExt;
			for (int i = 0; i < nExtLen; i++)
			{
				if (*(pExtChar + i) >= 'A' && *(pExtChar + i) <= 'Z')
					*(pExtChar + i) += 'a' - 'A';
			}

			if (_tcsstr (m_szVideoExt, pExt) != NULL)
				m_lstItem.AddTail (CreateItem (szName, ITEM_Video));
			else if (_tcsstr (m_szAudioExt, pExt) != NULL)
				m_lstItem.AddTail (CreateItem (szName, ITEM_Audio));
		}while(FindNextFile(hFind, &data));
		FindClose (hFind);

		SortDispItems ();
	}
	
	if (m_nAnimateType == ITEM_Home)
		CopyLastVideo (m_hWnd);

	CreateBitmapBG ();

	ShowBitmapBG ();

	if (m_lvType == LIST_VIEW_Folder)
		SetWindowText (GetParent (m_hWnd), m_szFolder);

	return true;
}

bool CWndPlayList::SortDispItems (void)
{
	int				nFolderNum = 0;
	int				nFileNum = 0;
	POSITION		pos = NULL;
	MEDIA_Item *	pItem = NULL;

	if (m_lstItem.GetCount () <= 0)
		return false;

	pos = m_lstItem.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstItem.GetNext (pos);
		if (pItem->nType == ITEM_Folder)
			nFolderNum++;
		else if (pItem->nType == ITEM_Video || pItem->nType == ITEM_Audio)
			nFileNum++;
	}

	MEDIA_Item **	ppFolderItems = NULL;
	MEDIA_Item **	ppFileItems = NULL;
	int				nIdxFile = 0;
	int				nIdxFold = 0;
	if (nFolderNum > 0 || nFileNum > 0)
	{
		if (nFolderNum > 0)
			ppFolderItems = new MEDIA_Item *[nFolderNum];
		if (nFileNum > 0)
			ppFileItems = new MEDIA_Item *[nFileNum];

		pos = m_lstItem.GetHeadPosition ();
		while (pos != NULL)
		{
			pItem = m_lstItem.GetNext (pos);
			if (pItem->nType == ITEM_Folder)
				ppFolderItems[nIdxFold++] = pItem;
			if (pItem->nType == ITEM_Video || pItem->nType == ITEM_Audio)
				ppFileItems[nIdxFile++] = pItem;
		}
		if (ppFolderItems != NULL)
			qsort(ppFolderItems, nFolderNum, sizeof(MEDIA_Item *), compare_filename);
		if (ppFileItems != NULL)
			qsort(ppFileItems, nFileNum, sizeof(MEDIA_Item *), compare_filename);
	}

	MEDIA_Item *	pHomeItem = m_lstItem.GetHead ();
	m_lstItem.RemoveAll ();
	pHomeItem->nIndex = 0;
	if (pHomeItem->nType == ITEM_Home)
		m_lstItem.AddTail (pHomeItem);
	int i = 0;
	for (i = 0; i < nFolderNum; i++)
	{
		ppFolderItems[i]->nIndex = m_lstItem.GetCount ();
		m_lstItem.AddTail (ppFolderItems[i]);
	}
	for (i = 0; i < nFileNum; i++)
	{
		ppFileItems[i]->nIndex = m_lstItem.GetCount ();
		m_lstItem.AddTail (ppFileItems[i]);
	}
	if (ppFolderItems != NULL)
		delete []ppFolderItems;
	if (ppFileItems != NULL)
		delete []ppFileItems;

	return true;
}

MEDIA_Item * CWndPlayList::CreateItem (TCHAR * pPath, TCHAR * pName, ITEM_TYPE nType)
{
	MEDIA_Item * pNewItem = CreateItem (pName, nType);
	if (pNewItem == NULL)
		return NULL;
	if (pPath == NULL)
		return pNewItem;

	int nPathLen = _tcslen (pPath) + 4;
	pNewItem->pPath = new TCHAR[nPathLen];
	memset (pNewItem->pPath, 0, nPathLen * sizeof (TCHAR));
	_tcscpy (pNewItem->pPath, pPath);

	return pNewItem;
}

MEDIA_Item * CWndPlayList::CreateItem (TCHAR * pName, ITEM_TYPE nType)
{
	MEDIA_Item * pItem = new MEDIA_Item ();
	if (pItem == NULL)
		return NULL;

	pItem->nType = nType;
	int nNameLen = _tcslen (pName) + 4;
	pItem->pName = new TCHAR[nNameLen];
	memset (pItem->pName, 0, nNameLen * sizeof (TCHAR));
	_tcscpy (pItem->pName, pName);
	pItem->nIndex = m_lstItem.GetCount ();

	return pItem;
}

MEDIA_Item * CWndPlayList::CloneItem (MEDIA_Item * pOldItem)
{
	if (pOldItem == NULL)
		return NULL;

	MEDIA_Item * pNewItem = CreateItem (pOldItem->pPath, pOldItem->pName, pOldItem->nType);

	return pNewItem;
}

bool CWndPlayList::DeleteItem (MEDIA_Item * pItem)
{
	if (pItem == NULL)
		return false;

	if (pItem->pChildList != NULL)
	{
		MEDIA_Item * pDelItem = pItem->pChildList->RemoveHead ();
		while (pDelItem != NULL)
		{
			DeleteItem (pDelItem); 
			pDelItem = pItem->pChildList->RemoveHead ();
		}
		delete pItem->pChildList;
	}

	if (pItem->pPath != NULL)
		delete []pItem->pPath;
	if (pItem->pName != NULL)
		delete []pItem->pName;
	if (pItem->hThumb != NULL)
		DeleteObject (pItem->hThumb);
	delete pItem;

	return true;
}

bool CWndPlayList::DeleteSelItem (void)
{
	if (m_pSelectedItem == NULL)
		return false;

	if (m_lvType != LIST_VIEW_Favor)
	{
		if (MessageBox (m_hWnd, m_pLangText->GetText (YYTEXT_DeleteFile), m_pLangText->GetText (YYTEXT_DelFile), MB_YESNO) != IDYES)
			return false;
	}
	
	bool			bFound = false;
	MEDIA_Item *	pItem = NULL;
	POSITION pos = m_lstItem.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstItem.GetNext (pos);
		if (pItem == m_pSelectedItem)
			bFound = true;
		if (bFound)
			pItem->nIndex = pItem->nIndex - 1;
	}

	Pause ();

	if (m_lvType == LIST_VIEW_Favor)
	{
		if (m_pCurItem != NULL && m_pCurItem->pChildList != NULL)
		{
			MEDIA_Item * pDelItem = NULL;
			CObjectList<MEDIA_Item>	* pList = m_pCurItem->pChildList;
			POSITION pos = pList->GetHeadPosition ();
			while (pos != NULL)
			{
				pDelItem = pList->GetNext (pos);
				if (pDelItem->nType == m_pSelectedItem->nType)
				{
					if (!_tcscmp (pDelItem->pName, m_pSelectedItem->pName))
					{
						pList->Remove (pDelItem);
						DeleteItem (pDelItem);
						m_bFavorModified = true;
						break;
					}
				}
			}
		}
	}
	else
	{
		TCHAR szDelFile[1024];
		_tcscpy (szDelFile, m_szFolder);
		_tcscat (szDelFile, _T("\\"));
		_tcscat (szDelFile, m_pSelectedItem->pName);
		DeleteFile (szDelFile);
	}

	m_lstItem.Remove (m_pSelectedItem);
	DeleteItem (m_pSelectedItem);
	m_pSelectedItem = NULL;

	int nBmpPos = m_nBmpYPos;
	CreateBitmapBG ();
	m_nBmpYPos = nBmpPos;

	m_nAnimateType = ITEM_Unknown;
	ShowBitmapBG ();

	m_bPause = false;

	return true;
}

void CWndPlayList::ReleaseList (void)
{
	m_bStop = true;
	if (m_pMedia != NULL)
		m_pMedia->SetParam (YYPLAY_PID_Cancel_GetThumb, NULL);
	while (m_hThread != NULL)
		yySleep (10000);

	SelectObject (m_hDCItem, m_hBmpHome);
	MEDIA_Item * pItem = m_lstItem.RemoveHead ();
	while (pItem != NULL)
	{
		DeleteItem (pItem);
		pItem = m_lstItem.RemoveHead ();
	}

	m_pSelectedItem = NULL;
}

bool CWndPlayList::LoadMyFavor (void)
{
	TCHAR szFile[256];
	yyGetAppPath (NULL, szFile, sizeof (szFile));
	_tcscat (szFile, _T("yyMyFavor.dat"));

	if (m_pFavorItem == NULL)
	{
		m_pFavorItem = CreateItem (_T("MyFavor"), ITEM_Folder);
		m_pCurItem = m_pFavorItem;
	}

	yyFile hFile = yyFileOpen (szFile, YYFILE_READ);
	if (hFile == NULL)
		return false;

	bool bRC = LoadItem (hFile, m_pFavorItem);

	yyFileClose (hFile);

	if (!bRC)
		DeleteFile (szFile);

	return bRC;
}

bool CWndPlayList::LoadItem (yyFile hFile, MEDIA_Item * pItem)
{
	if (pItem == NULL)
		return true;

	unsigned char	szBuff[2048];
	int				nSize;

	memset (szBuff, 0, sizeof (szBuff));
	nSize = strlen (MEDIA_ITEM_TEXTID);
	int nRead = yyFileRead (hFile, szBuff, nSize);
	if (nRead != nSize)
		return false;
	if (strcmp ((char *)szBuff, MEDIA_ITEM_TEXTID))
		return false;

	int nItemType = 0;
	nRead = yyFileRead (hFile, (unsigned char *)&pItem->nType, 4);
	if (nRead != 4)
		return false;

	int nTextLen = 0;
	nRead = yyFileRead (hFile, (unsigned char *)&nTextLen, 4);
	if (nRead != 4)
		return false;
	if (nTextLen > 0)
	{
		pItem->pPath = new TCHAR[nTextLen/2 + 1];
		memset (pItem->pPath, 0, nTextLen + 2);
		nRead = yyFileRead (hFile, (unsigned char *)pItem->pPath, nTextLen);
		if (nRead != nTextLen)
			return false;
	}

	nTextLen = 0;
	nRead = yyFileRead (hFile, (unsigned char *)&nTextLen, 4);
	if (nRead != 4)
		return false;
	if (nTextLen > 0)
	{
		pItem->pName = new TCHAR[nTextLen/2 + 1];
		memset (pItem->pName, 0, nTextLen + sizeof (TCHAR));
		nRead = yyFileRead (hFile, (unsigned char *)pItem->pName, nTextLen);
		if (nRead != nTextLen)
			return false;
	}

	int nSelect = 0;
	nRead = yyFileRead (hFile, (unsigned char *)&nSelect, 4);
	if (nRead != 4)
		return false;
	if (nSelect > 0)
		m_pCurItem = pItem;

	int nChild = 0;
	nRead = yyFileRead (hFile, (unsigned char *)&nChild, 4);
	if (nRead != 4)
		return false;
	if (nChild == 0)
		return true;

	MEDIA_Item * pNewItem = NULL;
	for (int i = 0; i < nChild; i++)
	{
		pNewItem = new MEDIA_Item ();
		pNewItem->pParent = pItem;
		if (pItem->pChildList == NULL)
			pItem->pChildList = new CObjectList<MEDIA_Item> ();
		pItem->pChildList->AddTail (pNewItem);

		if (!LoadItem (hFile, pNewItem))
			return false;
	}

	return true;
}

bool CWndPlayList::SaveMyFavor (void)
{
	if (m_pFavorItem == NULL || !m_bFavorModified)
		return false;

	TCHAR szFile[256];
	yyGetAppPath (NULL, szFile, sizeof (szFile));
	_tcscat (szFile, _T("yyMyFavor.dat"));

	yyFile hFile = yyFileOpen (szFile, YYFILE_WRITE);
	if (hFile == NULL)
		return false;

	bool bRC = SaveItem (hFile, m_pFavorItem);

	yyFileClose (hFile);

	if (!bRC)
		DeleteFile (szFile);

	return bRC;
}

bool CWndPlayList::SaveItem (yyFile hFile, MEDIA_Item * pItem)
{
	if (pItem == NULL)
		return false;

	char szItemID[32];
	strcpy (szItemID, MEDIA_ITEM_TEXTID);
	int	nTextLen = strlen (szItemID);

	int nWrite = yyFileWrite (hFile, (unsigned char *)szItemID, nTextLen);
	if (nWrite != nTextLen)
		return false;

	nWrite = yyFileWrite (hFile, (unsigned char *)&pItem->nType, 4);
	if (nWrite != 4)
		return false;

	nTextLen = 0;
	if (pItem->pPath != NULL)
		nTextLen = _tcslen(pItem->pPath) * sizeof (TCHAR);
	nWrite = yyFileWrite (hFile, (unsigned char *)&nTextLen, 4);
	if (nWrite != 4)
		return false;
	if (pItem->pPath != NULL)
	{
		nWrite = yyFileWrite (hFile, (unsigned char *)pItem->pPath, nTextLen);
		if (nWrite != nTextLen)
			return false;
	}

	nTextLen = 0;
	if (pItem->pName != NULL)
		nTextLen = _tcslen(pItem->pName) * sizeof (TCHAR);
	nWrite = yyFileWrite (hFile, (unsigned char *)&nTextLen, 4);
	if (nWrite != 4)
		return false;
	if (pItem->pName != NULL)
	{
		nWrite = yyFileWrite (hFile, (unsigned char *)pItem->pName, nTextLen);
		if (nWrite != nTextLen)
			return false;
	}

	int nSelect = 0;
	if (pItem == m_pCurItem)
		nSelect = 1;
	nWrite = yyFileWrite (hFile, (unsigned char *)&nSelect, 4);
	if (nWrite != 4)
		return false;

	int nCount = 0;
	CObjectList<MEDIA_Item> * pList = pItem->pChildList;
	if (pList == NULL)
	{
		nWrite = yyFileWrite (hFile, (unsigned char *)&nCount, 4);
		if (nWrite != 4)
			return false;
		return true;
	}

	nCount = pList->GetCount ();
	nWrite = yyFileWrite (hFile, (unsigned char *)&nCount, 4);
	if (nWrite != 4)
		return false;

	MEDIA_Item * pChildItem = NULL;
	POSITION pos = pList->GetHeadPosition ();
	while (pos != NULL)
	{
		pChildItem = pList->GetNext (pos);
		if (!SaveItem (hFile, pChildItem))
			return false;
	}

	return true;
}