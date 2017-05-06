/*******************************************************************************
	File:		CListStore.cpp

	Contains:	The favor list implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file
	2014-05-21      Fenger          Add the play position for VER1001
	2014-05-21      Fenger          Add the rotate angle for VER1002
	2014-05-30      Fenger          Change the item type for VER1003
*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CListStore.h"
#include "USystemFunc.h"
#include "UBitmapFunc.H"
#include "UStringFunc.h"
#include "URLEFunc.h"

#include "yyLog.h"

#define	LIST_ITEM_TEXTID		"yyPlayListStorageFile"

#pragma warning (disable : 4996)

CListStore::CListStore(HINSTANCE hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_nVersion (1003)
	, m_nVerFile (1000)
	, m_pSelItem (NULL)
{
	SetObjectName ("CListStore");
}	

CListStore::~CListStore(void)
{
}

bool CListStore::Load (TCHAR * pFile, CListItem * pRoot)
{
	if (pFile == NULL || pRoot == NULL)
		return false;

	yyFile hFile = yyFileOpen (pFile, YYFILE_READ);
	if (hFile == NULL)
		return false;

	char szID[64];
	memset (szID, 0, sizeof (szID));
	int nIDLen = strlen (LIST_ITEM_TEXTID);
	int nWrite = yyFileRead (hFile, (unsigned char *)szID, nIDLen);
	if (strcmp (szID, LIST_ITEM_TEXTID))
	{
		yyFileClose (hFile);
		return false;
	}
	yyFileRead (hFile, (unsigned char *)&m_nVerFile, 4);
	// The item type value was changed from 1003
	if (m_nVerFile <= 1002 && m_nVersion >= 1003) 
	{
		yyFileClose (hFile);
		DeleteFile (pFile);
		return YY_ERR_FAILED;
	}

	bool bRC = LoadItem (hFile, pRoot);
	yyFileClose (hFile);
	return bRC;
}

bool CListStore::Save (TCHAR * pFile, CListItem * pRoot)
{
	if (pFile == NULL || pRoot == NULL)
		return false;

	yyFile hFile = yyFileOpen (pFile, YYFILE_WRITE);
	if (hFile == NULL)
		return false;

	int nIDLen = strlen (LIST_ITEM_TEXTID);
	int nWrite = yyFileWrite (hFile, (unsigned char *)LIST_ITEM_TEXTID, nIDLen);
	if (nWrite != nIDLen)
	{
		yyFileClose (hFile);
		return false;
	}
	yyFileWrite (hFile, (unsigned char *)&m_nVersion, 4);

	bool bRC = SaveItem (hFile, pRoot);
	yyFileClose (hFile);
	return bRC;
}

bool CListStore::Load (TCHAR * pFile, CObjectList<CListItem> * pList)
{
	CListItem * pItem = new CListItem ();
	pItem->m_pChdList = pList;
	bool bRC = Load (pFile, pItem);
	pItem->m_pChdList = NULL;
	delete pItem;
	return bRC;
}

bool CListStore::Save (TCHAR * pFile, CObjectList<CListItem> * pList)
{
	CListItem * pItem = new CListItem ();
	pItem->m_pChdList = pList;
	bool bRC = Save (pFile, pItem);
	pItem->m_pChdList = NULL;
	delete pItem;
	return bRC;
}

bool CListStore::LoadItem (yyFile hFile, CListItem * pItem)
{
	int		nRead = 0;
	int		nTextLen = 0;
	if (m_nVerFile >= 1000)
	{
		nRead = yyFileRead (hFile, (unsigned char *)&pItem->m_nType, 4);
		if (nRead != 4)
			return false;

		// Read the path
		nTextLen = 0;
		nRead = yyFileRead (hFile, (unsigned char *)&nTextLen, 4);
		if (nRead != 4)
			return false;
		if (nTextLen > 0)
		{
			pItem->m_pPath = new TCHAR[nTextLen/2 + 1];
			memset (pItem->m_pPath, 0, nTextLen + 2);
			nRead = yyFileRead (hFile, (unsigned char *)pItem->m_pPath, nTextLen);
			if (nRead != nTextLen)
				return false;
		}

		// read the name
		nTextLen = 0;
		nRead = yyFileRead (hFile, (unsigned char *)&nTextLen, 4);
		if (nRead != 4)
			return false;
		if (nTextLen > 0)
		{
			pItem->m_pName = new TCHAR[nTextLen/2 + 1];
			memset (pItem->m_pName, 0, nTextLen + sizeof (TCHAR));
			nRead = yyFileRead (hFile, (unsigned char *)pItem->m_pName, nTextLen);
			if (nRead != nTextLen)
				return false;
		}

		// read the thumbnail info
		nRead = yyFileRead (hFile, (unsigned char *)&pItem->m_nPos, 4);
		if (nRead != 4)
			return false;
		nRead = yyFileRead (hFile, (unsigned char *)&pItem->m_nWidth, 4);
		if (nRead != 4)
			return false;
		nRead = yyFileRead (hFile, (unsigned char *)&pItem->m_nHeight, 4);
		if (nRead != 4)
			return false;
		nRead = yyFileRead (hFile, (unsigned char *)&pItem->m_nVNum, 4);
		if (nRead != 4)
			return false;
		nRead = yyFileRead (hFile, (unsigned char *)&pItem->m_nVDen, 4);
		if (nRead != 4)
			return false;
		int nBmpW = 0, nBmpH = 0;
		nRead = yyFileRead (hFile, (unsigned char *)&nBmpW, 4);
		if (nRead != 4)
			return false;
		nRead = yyFileRead (hFile, (unsigned char *)&nBmpH, 4);
		if (nRead != 4)
			return false;
		if (nBmpW > 0 && nBmpH > 0)
		{
			pItem->m_hThumb = yyBmpCreate (NULL, nBmpW, nBmpH, &pItem->m_pBuff, 0);
			if (pItem->m_hThumb == NULL)
				return false;
			int nSize = 0;
			nRead = yyFileRead (hFile, (unsigned char *)&nSize, 4);
			char * pRLEBuff = new char[nSize];
			nRead = yyFileRead (hFile, (unsigned char *)pRLEBuff, nSize);
			if (nRead != nSize)
				return false;
			nSize = yyRLEDecodeBmp (pRLEBuff, nBmpW, nBmpH, (char *)pItem->m_pBuff);
			//nRead = yyFileRead (hFile, pItem->m_pBuff, nBmpW * nBmpH * 4);
		}
 
		// Read the selection
		nRead = yyFileRead (hFile, (unsigned char *)&pItem->m_nSelect, 4);
		if (nRead != 4)
			return false;
		if (pItem->m_nSelect > 0)
			m_pSelItem = pItem;
	}
	if (m_nVerFile >= 1001)
	{
		// write the play pos info
		nRead = yyFileRead (hFile, (unsigned char *)&pItem->m_nPlayPos, 4);
		if (nRead != 4)
			return false;
	}
	if (m_nVerFile >= 1002)
	{
		// write the play pos info
		nRead = yyFileRead (hFile, (unsigned char *)&pItem->m_nRotate, 4);
		if (nRead != 4)
			return false;
	}

	int nChild = 0;
	nRead = yyFileRead (hFile, (unsigned char *)&nChild, 4);
	if (nRead != 4)
		return false;
	if (nChild == 0)
		return true;
	CListItem * pNewItem = NULL;
	for (int i = 0; i < nChild; i++)
	{
		pNewItem = new CListItem ();
		pNewItem->m_pParent = pItem;
		if (pItem->m_pChdList == NULL)
			pItem->m_pChdList = new CObjectList<CListItem> ();
		pItem->m_pChdList->AddTail (pNewItem);
		if (!LoadItem (hFile, pNewItem))
			return false;
	}
	return true;
}

bool CListStore::SaveItem (yyFile hFile, CListItem * pItem)
{
	int		nWrite = 0;
	int		nTextLen = 0;
	if (m_nVersion >= 1000)
	{
		nWrite = yyFileWrite (hFile, (unsigned char *)&pItem->m_nType, 4);
		if (nWrite != 4)
			return false;

		// Read the path
		nTextLen = 0;
		if (pItem->m_pPath != NULL)
			nTextLen = _tcslen (pItem->m_pPath) * sizeof (TCHAR);
		nWrite = yyFileWrite (hFile, (unsigned char *)&nTextLen, 4);
		if (nWrite != 4)
			return false;
		if (nTextLen > 0)
		{
			nWrite = yyFileWrite (hFile, (unsigned char *)pItem->m_pPath, nTextLen);
			if (nWrite != nTextLen)
				return false;
		}

		// read the name
		nTextLen = 0;
		if (pItem->m_pName != NULL)
			nTextLen = _tcslen (pItem->m_pName) * sizeof (TCHAR);
		nWrite = yyFileWrite (hFile, (unsigned char *)&nTextLen, 4);
		if (nWrite != 4)
			return false;
		if (nTextLen > 0)
		{
			nWrite = yyFileWrite (hFile, (unsigned char *)pItem->m_pName, nTextLen);
			if (nWrite != nTextLen)
				return false;
		}

		// write the thumbnail info
		nWrite = yyFileWrite (hFile, (unsigned char *)&pItem->m_nPos, 4);
		if (nWrite != 4)
			return false;
		nWrite = yyFileWrite (hFile, (unsigned char *)&pItem->m_nWidth, 4);
		if (nWrite != 4)
			return false;
		nWrite = yyFileWrite (hFile, (unsigned char *)&pItem->m_nHeight, 4);
		if (nWrite != 4)
			return false;
		nWrite = yyFileWrite (hFile, (unsigned char *)&pItem->m_nVNum, 4);
		if (nWrite != 4)
			return false;
		nWrite = yyFileWrite (hFile, (unsigned char *)&pItem->m_nVDen, 4);
		if (nWrite != 4)
			return false;
		int nBmpW = 0, nBmpH = 0;
		if (pItem->m_hThumb != NULL && pItem->m_pBuff != NULL)
		{
			BITMAP bmpInfo;
			GetObject (pItem->m_hThumb, sizeof (BITMAP), &bmpInfo);
			nBmpW = bmpInfo.bmWidth;
			nBmpH = bmpInfo.bmHeight;
		}
		nWrite = yyFileWrite (hFile, (unsigned char *)&nBmpW, 4);
		if (nWrite != 4)
			return false;
		nWrite = yyFileWrite (hFile, (unsigned char *)&nBmpH, 4);
		if (nWrite != 4)
			return false;
		if (nBmpW > 0 && nBmpH > 0)
		{
			char * pRLEBuff = NULL;
			int nSize = yyRLEEncodeBmp ((char *)pItem->m_pBuff, nBmpW, nBmpH, &pRLEBuff);
			nWrite = yyFileWrite (hFile, (unsigned char *)&nSize, 4);
			nWrite = yyFileWrite (hFile, (unsigned char *)pRLEBuff, nSize);
			delete []pRLEBuff;
			//nWrite = yyFileWrite (hFile, pItem->m_pBuff, nBmpW * nBmpH * 4);
			if (nWrite != nSize)
				return false;
		}
 
		// Read the selection
		nWrite = yyFileWrite (hFile, (unsigned char *)&pItem->m_nSelect, 4);
		if (nWrite != 4)
			return false;
	}
	if (m_nVersion >= 1001)
	{
		// write the play pos info
		nWrite = yyFileWrite (hFile, (unsigned char *)&pItem->m_nPlayPos, 4);
		if (nWrite != 4)
			return false;
	}
	if (m_nVersion >= 1002)
	{
		// write the play pos info
		nWrite = yyFileWrite (hFile, (unsigned char *)&pItem->m_nRotate, 4);
		if (nWrite != 4)
			return false;
	}

	int nChild = 0;
	if (pItem->m_pChdList != NULL)
		nChild = pItem->m_pChdList->GetCount ();
	nWrite = yyFileWrite (hFile, (unsigned char *)&nChild, 4);
	if (nWrite != 4)
		return false;
	if (nChild == 0)
		return true;
	CListItem * pChildItem = NULL;
	NODEPOS pos = pItem->m_pChdList->GetHeadPosition ();
	while (pos != NULL)
	{
		pChildItem = pItem->m_pChdList->GetNext (pos);
		if (!SaveItem (hFile, pChildItem))
			return false;
	}
	return true;
}

