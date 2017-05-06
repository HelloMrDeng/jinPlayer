/*******************************************************************************
	File:		CBaseConfig.h

	Contains:	the config file wrap head file

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-27		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseConfig_H__
#define __CBaseConfig_H__

#include "CBaseObject.h"

class CCfgSect
{
public:
	CCfgSect (void);
	virtual ~CCfgSect (void);

public:
	char *			m_pName;
	void *			m_pData;
	CCfgSect *		m_pNext;
};

class CCfgItem
{
public:
	CCfgItem (void);
	virtual ~CCfgItem (void);

public:
	CCfgSect *		m_pSection;
	char *			m_pName;
	int				m_nValue;
	char *			m_pValue;

	CCfgItem *		m_pNext;
};

class CBaseConfig : public CBaseObject
{
public:
	CBaseConfig(void);
	virtual ~CBaseConfig(void);

	virtual	bool	Open (TCHAR * pFile);
	virtual bool	Write (TCHAR * pFile);

	virtual bool	AddSection (char * pSection);
	virtual bool	RemoveSection (char * pSection);

	virtual bool	AddItem (char * pSection, char * pItemName, int nValue);
	virtual bool	AddItem (char * pSection, char * pItemName, char * pValue);

	virtual bool	RemoveItem (char * pSection, char * pItemName);

	virtual bool	UpdateItem (char * pSection, char * pItemName, int nValue);
	virtual bool	UpdateItem (char * pSection, char * pItemName, char * pValue);

	virtual int		GetItemValue (const char* pSection, const char* pItemName, int nDefault);
	virtual char *	GetItemText (const char* pSection, const char* pItemName, const char* pDefault = NULL);
	virtual bool	GetItemPos (const char* pSection, int & nLeft, int & nTop, int & nWidth, int & nHeight);
	virtual bool	GetItemRect (const char* pSection, RECT * pRect);

	virtual CCfgSect *	FindSect (char * pSection);

	CCfgSect *	GetFirstSect (void) {return m_pFirstSect;}
	CCfgItem *	GetFirstItem (void) {return m_pFirstItem;}
	TCHAR *		GetFileName (void) {return m_pFileName;}

protected:
	CCfgItem *		FindItem (const char* pSection, const char* pItemName);
	CCfgItem *		CreateItem (char * pSection, char * pItemName);

	virtual char *	GetNextLine (char * pBuffer, int nBufSize, char * pLineText, int & nLineSize);
	virtual	void	Release (void);

protected:
	TCHAR *			m_pFileName;
	bool			m_bUpdated;

	CCfgSect *		m_pFirstSect;
	int				m_nSectNum;

	CCfgItem *		m_pFirstItem;
	int				m_nItemNum;

	char			m_szLineText[256];
	char			m_szDefaultValue[256];

};

#endif // __CBaseConfig_H__
