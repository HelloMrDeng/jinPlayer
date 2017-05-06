/*******************************************************************************
	File:		CListStore.h

	Contains:	The favor list header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CListStore_H__
#define __CListStore_H__

#include "CListItem.h"
#include "UFileFunc.h"

class CListStore : public CBaseObject
{
public:
	CListStore(HINSTANCE hInst);
	virtual ~CListStore(void);

	virtual bool	Load (TCHAR * pFile, CListItem * pRoot); 
	virtual bool	Save (TCHAR * pFile, CListItem * pRoot);

	virtual bool	Load (TCHAR * pFile, CObjectList<CListItem> * pList); 
	virtual bool	Save (TCHAR * pFile, CObjectList<CListItem> * pList);

	CListItem *		GetSelItem (void) {return m_pSelItem;}

protected:
	virtual bool	LoadItem (yyFile hFile, CListItem * pItem);
	virtual bool	SaveItem (yyFile hFile, CListItem * pItem);

protected:
	HINSTANCE		m_hInst;
	int				m_nVersion;
	int				m_nVerFile;
	CListItem *		m_pSelItem;
};
#endif //__CListStore_H__