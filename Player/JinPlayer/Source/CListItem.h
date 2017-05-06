/*******************************************************************************
	File:		CListItem.h

	Contains:	The list item header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CListItem_H__
#define __CListItem_H__

#include "CBaseObject.h"
#include "CNodeList.h"

typedef enum {
	ITEM_Unknown		= 0,
	ITEM_View_Folder	= 1,
	ITEM_View_Favor		= 2,
	ITEM_View_Box		= 3,
	ITEM_View_LastPlay	= 47,
	ITEM_Exit			= 10,
	ITEM_Home			= 11,
	ITEM_Folder			= 21,
	ITEM_Audio			= 22,
	ITEM_Video			= 23,
	ITEM_Image			= 24,
	ITEM_NewFile		= 91,
	ITEM_NewFolder		= 92,
	ITEM_MAX			= 0X7FFFFFFF,
} ITEM_TYPE;

class CListItem
{
public:
	CListItem(void);
	CListItem(TCHAR * pName, ITEM_TYPE nType);
	CListItem(TCHAR * pPath, TCHAR * pName, ITEM_TYPE nType);
	CListItem(CListItem * pItem);
	virtual ~CListItem(void);

	virtual bool	MoveTo (CListItem * pTarget);
	virtual void	Reset (void);
	virtual bool	GetFile (TCHAR * pFile, int nSize);

protected:
	virtual	void	InitParam (void);

public:
	ITEM_TYPE		m_nType;
	TCHAR *			m_pPath;
	TCHAR *			m_pName;
	HBITMAP			m_hThumb;
	LPBYTE			m_pBuff;
	int				m_nPos;
	int				m_nWidth;
	int				m_nHeight;
	int				m_nVNum;
	int				m_nVDen;
	TCHAR *			m_pFolder;
	int				m_nIndex;
	int				m_nSelect;
	int				m_nPlayPos;
	int				m_nRotate;
	bool			m_bModified;

	CListItem *					m_pParent;
	CObjectList<CListItem> *	m_pChdList; 
};
#endif //__CListItem_H__