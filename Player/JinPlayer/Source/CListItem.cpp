/*******************************************************************************
	File:		CListItem.cpp

	Contains:	The list item implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CListItem.h"

#include "USystemFunc.h"
#include "UStringFunc.h"
#include "yyLog.h"

#pragma warning (disable : 4996)

CListItem::CListItem(void)
{
	InitParam ();
}	

CListItem::CListItem(TCHAR * pName, ITEM_TYPE nType)
{
	InitParam ();	
	if (pName != NULL)
	{
		m_pName = new TCHAR[_tcslen (pName) + 1];
		_tcscpy (m_pName, pName);
	}
	m_nType = nType;
}

CListItem::CListItem(TCHAR * pPath, TCHAR * pName, ITEM_TYPE nType)
{
	InitParam ();	
	if (pPath != NULL)
	{
		m_pPath = new TCHAR[_tcslen (pPath) + 1];
		_tcscpy (m_pPath, pPath);
	}
	if (pName != NULL)
	{
		m_pName = new TCHAR[_tcslen (pName) + 1];
		_tcscpy (m_pName, pName);
	}
	m_nType = nType;
}

CListItem::CListItem(CListItem * pItem)
{
	InitParam ();	

	if (pItem != NULL)
	{
		m_nType = pItem->m_nType;
		if (pItem->m_pPath != NULL)
		{
			m_pPath = new TCHAR[_tcslen (pItem->m_pPath) + 1];
			_tcscpy (m_pPath, pItem->m_pPath);
		}
		if (pItem->m_pName != NULL)
		{
			m_pName = new TCHAR[_tcslen (pItem->m_pName) + 1];
			_tcscpy (m_pName, pItem->m_pName);
		}
	}
}

CListItem::~CListItem(void)
{
	Reset ();
}

bool CListItem::MoveTo (CListItem * pTarget)
{
	if (pTarget == NULL)
		return false;

	pTarget->Reset ();
	pTarget->m_nType = m_nType;
	pTarget->m_pPath = m_pPath;
	pTarget->m_pName = m_pName;
	pTarget->m_hThumb = m_hThumb;
	pTarget->m_pBuff = m_pBuff;
	pTarget->m_nPos = m_nPos;
	pTarget->m_nWidth = m_nWidth;
	pTarget->m_nHeight = m_nHeight;
	pTarget->m_nVNum = m_nVNum;
	pTarget->m_nVDen = m_nVDen;
	pTarget->m_nPlayPos = m_nPlayPos;
	pTarget->m_nRotate = m_nRotate;
	pTarget->m_bModified = m_bModified;

	m_pPath = NULL;
	m_pName = NULL;
	m_hThumb = NULL;
	m_pBuff = NULL;
	Reset ();

	return true;
}

void CListItem::Reset (void)
{
	m_nType = ITEM_Unknown;
	m_nSelect = 0;
	YY_DEL_A (m_pPath);
	YY_DEL_A (m_pName);
	if (m_hThumb != NULL)
		DeleteObject (m_hThumb);
	m_hThumb = NULL;
	m_nIndex = -1;

	m_pParent = NULL;
	if (m_pChdList != NULL)
	{
		CListItem * pItem = m_pChdList->RemoveHead ();
		while (pItem != NULL)
		{
			delete pItem;
			pItem = m_pChdList->RemoveHead ();
		}
		delete m_pChdList;
		m_pChdList = NULL;
	}
}

bool CListItem::GetFile (TCHAR * pFile, int nSize)
{
	if (m_nType != ITEM_Video && m_nType != ITEM_Audio && m_nType != ITEM_Image)
		return false;

	if (m_pPath != NULL)
	{
		_tcscpy (pFile, m_pPath);
	}
	else
	{
		_tcscpy (pFile, m_pFolder);
		_tcscat (pFile, _T("\\"));
		_tcscat (pFile, m_pName);
	}

	return true;
}

void CListItem::InitParam (void)
{
	m_nType = ITEM_Unknown;
	m_pPath = NULL;
	m_pName = NULL;
	m_hThumb = NULL;
	m_pBuff = NULL;
	m_nPos = 0;
	m_nVNum = 1;
	m_nVDen = 1;
	m_pFolder = NULL;
	m_nIndex = -1;
	m_nSelect = 0;
	m_nPlayPos = 0;
	m_nRotate = 0;
	m_bModified = false;

	m_pParent = NULL;
	m_pChdList = NULL;
}
