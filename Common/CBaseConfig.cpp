/*******************************************************************************
	File:		CBaseConfig.h

	Contains:	the config file wrap class file

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-27		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "CBaseConfig.h"

#include "UStringFunc.h"
#include "UFileFunc.h"

#ifdef _OS_WIN32
#pragma warning (disable : 4996)
#endif

CCfgSect::CCfgSect (void)
	: m_pName (NULL)
	, m_pData (NULL)
	, m_pNext (NULL)
{
}

CCfgSect::~CCfgSect (void)
{
	YY_DEL_A (m_pName);
//	YY_DEL_A (m_pData);
}

CCfgItem::CCfgItem (void)
	: m_pSection (NULL)
	, m_pName (NULL)
	, m_nValue (0)
	, m_pValue (NULL)
	, m_pNext (NULL)
{
}

CCfgItem::~CCfgItem (void)
{
	YY_DEL_A (m_pName);
	YY_DEL_A (m_pValue);
}

CBaseConfig::CBaseConfig(void)
	: CBaseObject ()
	, m_pFileName (NULL)
	, m_bUpdated (false)
	, m_pFirstSect (NULL)
	, m_nSectNum (0)
	, m_pFirstItem (NULL)
	, m_nItemNum (0)
{
	SetObjectName ("CBaseConfig");
}

CBaseConfig::~CBaseConfig(void)
{
	Release ();
	YY_DEL_A (m_pFileName);
}

int CBaseConfig::GetItemValue (const char* pSection, const char* pItemName, int nDefault)
{
	CCfgItem * pItem = FindItem (pSection, pItemName);
	if (pItem == NULL)
		return nDefault;

	return pItem->m_nValue;
}

char * CBaseConfig::GetItemText (const char* pSection, const char* pItemName, const char* pDefault)
{
	CCfgItem * pItem = FindItem (pSection, pItemName);
	if (pItem == NULL)
	{
		if (pDefault != NULL)
		{
			strcpy (m_szDefaultValue, pDefault);
			return m_szDefaultValue;
		}
		else
		{
			return NULL;
		}
	}

	return pItem->m_pValue;
}

bool CBaseConfig::GetItemPos (const char* pSection, int & nLeft, int & nTop, int & nWidth, int & nHeight)
{
	CCfgSect * pSect = FindSect ((char *)pSection);
	if (pSect == NULL)
		return false;

	nLeft = GetItemValue (pSection, "Left", -1);
	nTop = GetItemValue (pSection, "Top", -1);
	nWidth = GetItemValue (pSection, "Width", -1);
	nHeight = GetItemValue (pSection, "Height", -1);

	return true;
}

bool CBaseConfig::GetItemRect (const char* pSection, RECT * pRect)
{
	CCfgSect * pSect = FindSect ((char *)pSection);
	if (pSect == NULL)
		return false;

	int nLeft = GetItemValue (pSection, "Left", -1);
	int nTop = GetItemValue (pSection, "Top", -1);
	int nWidth = GetItemValue (pSection, "Width", -1);
	int nHeight = GetItemValue (pSection, "Height", -1);

#ifdef _OS_WIN32
	SetRect (pRect, nLeft, nTop, nLeft + nWidth, nTop + nHeight);
#endif // _OS_WIN32

	return true;
}

bool CBaseConfig::AddSection (char * pSection)
{
	CCfgSect * pNewSect = new CCfgSect ();
	pNewSect->m_pName = new char[strlen (pSection) + 1];
	strcpy (pNewSect->m_pName, pSection);

	if (m_pFirstSect == NULL)
		m_pFirstSect = pNewSect;
	else
	{
		CCfgSect * pSect = m_pFirstSect;
		while (pSect != NULL)
		{
			if (pSect->m_pNext == NULL)
			{
				pSect->m_pNext = pNewSect;
				break;
			}
			pSect = pSect->m_pNext;
		}
	}

	m_nSectNum++;

	m_bUpdated = true;

	return true;
}

bool CBaseConfig::RemoveSection (char * pSection)
{
	if (pSection == NULL)
		return false;

	CCfgSect * pSect = m_pFirstSect;
	CCfgSect * pPrev = m_pFirstSect;
	while (pSect != NULL)
	{
		if (!strcmp (pSect->m_pName, pSection))
		{
			if (pSect == m_pFirstSect)
				m_pFirstSect = m_pFirstSect->m_pNext;
			else
			{
				pPrev->m_pNext = pSect->m_pNext;
			}

			delete pSect;
			return true;
		}

		pPrev = pSect;
		pSect = pSect->m_pNext;
	}

	m_bUpdated = true;

	return false;
}


bool CBaseConfig::AddItem (char * pSection, char * pItemName, int nValue)
{
	CCfgItem * pNewItem = CreateItem (pSection, pItemName);
	if (pNewItem == NULL)
		return false;

	pNewItem->m_pName = new char[strlen (pItemName) + 1];
	strcpy (pNewItem->m_pName, pItemName);
	pNewItem->m_nValue = nValue;

	CCfgItem * pItem = m_pFirstItem;
	while (pItem != NULL)
	{
		if (pItem->m_pNext == NULL)
		{
			pItem->m_pNext = pNewItem;
			break;
		}

		pItem = pItem->m_pNext;
	}

	m_bUpdated = true;

	return true;
}

bool CBaseConfig::AddItem (char * pSection, char * pItemName, char * pValue)
{
	CCfgItem * pNewItem = CreateItem (pSection, pItemName);
	if (pNewItem == NULL)
		return false;

	pNewItem->m_pName = new char[strlen (pItemName) + 1];
	strcpy (pNewItem->m_pName, pItemName);

	pNewItem->m_pName = new char[strlen (pValue) + 1];
	strcpy (pNewItem->m_pValue, pValue);

	CCfgItem * pItem = m_pFirstItem;
	while (pItem != NULL)
	{
		if (pItem->m_pNext == NULL)
		{
			pItem->m_pNext = pNewItem;
			break;
		}

		pItem = pItem->m_pNext;
	}

	m_bUpdated = true;

	return true;
}

bool CBaseConfig::RemoveItem (char * pSection, char * pItemName)
{
	CCfgItem * pFound = FindItem (pSection, pItemName);
	if (pFound == NULL)
		return false;

	CCfgItem * pItem = m_pFirstItem;
	CCfgItem * pPrev = m_pFirstItem;
	while (pItem != NULL)
	{
		if (pFound == pItem)
		{
			if (pItem == m_pFirstItem)
				m_pFirstItem = m_pFirstItem->m_pNext;
			else
			{
				pPrev->m_pNext = pItem->m_pNext;
			}

			delete pItem;
			return true;
		}

		pPrev = pItem;
		pItem = pItem->m_pNext;
	}

	m_bUpdated = true;

	return true;
}

bool CBaseConfig::UpdateItem (char * pSection, char * pItemName, int nValue)
{
	CCfgItem * pFound = FindItem (pSection, pItemName);
	if (pFound == NULL)
		return false;

	if (pFound->m_nValue == nValue)
		return true;

	pFound->m_nValue = nValue;

	m_bUpdated = true;

	return true;
}

bool CBaseConfig::UpdateItem (char * pSection, char * pItemName, char * pValue)
{
	CCfgItem * pFound = FindItem (pSection, pItemName);
	if (pFound == NULL)
		return false;

	if (pFound->m_pValue != NULL)
	{
		if (!strcmp (pFound->m_pValue, pValue))
			return false;
		delete []pFound->m_pValue;
	}

	pFound->m_pValue = new char[strlen (pValue) + 1];
	strcpy (pFound->m_pValue, pValue);

	m_bUpdated = true;

	return true;
}

bool CBaseConfig::Open (TCHAR * pFile)
{
	YY_DEL_A (m_pFileName);
	m_pFileName = new TCHAR[_tcslen (pFile) + 1];
	_tcscpy (m_pFileName, pFile);

	yyFile hFile = yyFileOpen (pFile, YYFILE_READ);
	if (hFile == 0)
		return false;

	int	nFileSize = (int)yyFileSize (hFile);
	if (nFileSize <= 0)
	{
		yyFileClose (hFile);
		return false;
	}

	Release ();

	char *	pFileBuffer = new char[nFileSize];
	int		dwRead = 0;
	memset (pFileBuffer, 0, nFileSize);
	dwRead = yyFileRead (hFile, (unsigned char *)pFileBuffer, nFileSize);
	yyFileClose (hFile);
	if(dwRead == 0)
		return false;

	CCfgSect *	pCurSect = NULL;
	CCfgSect *	pNewSect = NULL;

	CCfgItem *	pCurItem = NULL;
	CCfgItem *	pNewItem = NULL;

	char *	pName = NULL;
	char *	pPos = 0;
	char *	pValue = NULL;

	int		nLineSize = 256;
	char *	pBuffer = pFileBuffer;
	int		nBufSize = nFileSize;
	char *	pNextLine = GetNextLine (pBuffer, nBufSize, (char*)m_szLineText, nLineSize);

	while (pNextLine != NULL)
	{
		if (m_szLineText[0] != ('/') || m_szLineText[1] != ('/'))
		{
			if (m_szLineText[0] == ('['))
			{
				pNewSect = new CCfgSect ();
				pNewSect->m_pName = new char[nLineSize];
				memset (pNewSect->m_pName, 0, nLineSize);

				int nEnd = 0;
				for (nEnd = 2; nEnd < nLineSize; nEnd++)
				{
					if (m_szLineText[nEnd] == ']')
						break;
				}
				strncpy (pNewSect->m_pName, (const char *)m_szLineText + 1, nEnd - 1);

				if (m_pFirstSect == NULL)
					m_pFirstSect = pNewSect;
				if (pCurSect != NULL)
					pCurSect->m_pNext = pNewSect;
				pCurSect = pNewSect;
				m_nSectNum++;
			}
			else
			{
				pPos = (char *) strstr ((const char *)m_szLineText, "=");
				if (pPos != NULL)
				{
					pNewItem = new CCfgItem ();
					pValue = pPos + 1;
					*pPos = 0;
					pName = (char *)m_szLineText;
					pNewItem->m_pName = new char[strlen (pName) + 1];
					if (pNewItem->m_pName == NULL)
						break;
					strcpy (pNewItem->m_pName, pName);

					if (pValue[0] == '\"')
					{
						pValue = pValue + 1;
						char * pEnd = strstr (pValue, "\"");
						if (pEnd != NULL)
							*pEnd = 0;

						pNewItem->m_pValue = new char[strlen (pValue) + 1];
						if (pNewItem->m_pValue == NULL)
							break;
						strcpy (pNewItem->m_pValue, pValue);
					}
					else
					{
						if ((* (pValue + 1)) == 'X' || (* (pValue + 1)) == 'x')
							sscanf (pValue, "%xd", (unsigned int*)&pNewItem->m_nValue);
						else
							pNewItem->m_nValue = atoi (pValue);
					}

					pNewItem->m_pSection = pCurSect;

					if (m_pFirstItem == NULL)
						m_pFirstItem = pNewItem;
					if (pCurItem != NULL)
						pCurItem->m_pNext = pNewItem;
					pCurItem = pNewItem;
					m_nItemNum++;
				}
			}
		}

		nLineSize = 256;
		nBufSize = nFileSize - (pNextLine - pFileBuffer);
		pBuffer = pNextLine;
		pNextLine = GetNextLine (pBuffer, nBufSize, (char*)m_szLineText, nLineSize);
	}

	delete []pFileBuffer;

	return true;
}


bool CBaseConfig::Write (TCHAR * pFile)
{
	if (!m_bUpdated)
		return true;

	yyFile hFile = yyFileOpen (pFile, YYFILE_WRITE);
	if (hFile == 0)
		return false;

	int			dwWrite = 0;
	CCfgSect *	pSect = m_pFirstSect;
	CCfgItem *	pItem = m_pFirstItem;

	char	szLine[256];
	char *	pLine = (char *)&szLine[0];
	while (pSect != NULL)
	{
		memset (szLine, 0, 256);

		strcpy (pLine, "[");
		strcat (pLine, pSect->m_pName);
		strcat (pLine, "]\r\n");
		dwWrite = yyFileWrite (hFile, (unsigned char *)pLine, strlen (pLine));
		if(dwWrite != strlen (pLine))
			return false;

		pItem = m_pFirstItem;
		while (pItem != NULL)
		{
			if (pItem->m_pSection == pSect)
			{
				memset (szLine, 0, 256);
				if (pItem->m_pValue == NULL)
					sprintf (pLine, "%s=%d\r\n", pItem->m_pName, (int)pItem->m_nValue);
				else
					sprintf (pLine, "%s=\"%s\"\r\n", pItem->m_pName, pItem->m_pValue);
				dwWrite = yyFileWrite (hFile, (unsigned char *)pLine, strlen (pLine));
				if(dwWrite == strlen (pLine))
					return false;
			}

			pItem = pItem->m_pNext;
		}

		strcpy (pLine, "\r\n\r\n");
		dwWrite = yyFileWrite (hFile, (unsigned char *)pLine, strlen (pLine));
		if(dwWrite == strlen (pLine))
			return false;

		pSect = pSect->m_pNext;
	}

	yyFileClose (hFile);
	return true;
}

void CBaseConfig::Release (void)
{
	CCfgItem * pItem = m_pFirstItem;
	CCfgItem * pTempItem = pItem;
	while (pItem != NULL)
	{
		pTempItem = pItem->m_pNext;
		delete pItem;
		pItem = pTempItem;
	}

	m_pFirstItem = NULL;
	m_nItemNum = 0;

	CCfgSect * pSect = m_pFirstSect;
	CCfgSect * pTempSect = pSect;
	while (pSect != NULL)
	{
		pTempSect = pSect->m_pNext;
		delete pSect;
		pSect = pTempSect;
	}

	m_pFirstSect = NULL;
	m_nSectNum = 0;
}

CCfgSect * CBaseConfig::FindSect (char * pSection)
{
	if (pSection == NULL)
		return NULL;

	CCfgSect * pSect = m_pFirstSect;
	CCfgSect * pFound = NULL;

	while (pSect != NULL)
	{
		if (!strcmp (pSect->m_pName, pSection))
		{
			pFound = pSect;
			break;
		}

		pSect = pSect->m_pNext;
	}

	return pFound;
}

CCfgItem * CBaseConfig::FindItem (const char* pSection, const char* pItemName)
{
	if (pSection == NULL || pItemName == NULL)
		return NULL;

	CCfgItem * pItem = m_pFirstItem;
	CCfgItem * pFound = NULL;

	while (pItem != NULL)
	{
		if (pItem->m_pSection != NULL && !strcmp (pItem->m_pSection->m_pName, pSection))
		{
			if (!strcmp (pItem->m_pName, pItemName))
			{
				pFound = pItem;
				break;
			}
		}

		pItem = pItem->m_pNext;
	}

	return pFound;
}

CCfgItem * CBaseConfig::CreateItem (char * pSection, char * pItemName)
{
	if (pSection == NULL || pItemName == NULL)
		return NULL;

	bool	bFound = false;
	CCfgSect *	pSect = m_pFirstSect;
	while (pSect != NULL)
	{
		if (!strcmp (pSect->m_pName, pSection))
		{
			bFound = true;
			break;
		}
		pSect = pSect->m_pNext;
	}
	if (!bFound)
		return NULL;

	bFound = false;
	CCfgItem *	pItem = m_pFirstItem;
	while (pItem != NULL)
	{
		if (!strcmp (pItem->m_pName, pItemName))
		{
			bFound = true;
			break;
		}
		pItem = pItem->m_pNext;
	}
	if (!bFound)
		return NULL;

	CCfgItem * pNewItem  = new CCfgItem ();
	return pNewItem;
}


char * CBaseConfig::GetNextLine (char * pBuffer, int nBufSize, char * pLineText, int & nLineSize)
{
	bool	bFound = false;
	char *	pCurPos = (char *) pBuffer;
	char *	pNextPos = pCurPos;

	while ((char *)pNextPos - pBuffer < nBufSize)
	{
		if (!strncmp (pNextPos, ("\n"), 1))
		{
			if (pNextPos - pCurPos > 4)
			{
				if (pCurPos[0] == (';') || pCurPos[0] == ('\\'))
				{
					pCurPos = pNextPos + 1;
				}
				else
				{
					bFound = true;
					break;
				}
			}
			else
			{
				pCurPos = pNextPos + 1;
			}
		}

		pNextPos++;
	}

	if (bFound)
	{
		if (pNextPos - pCurPos > nLineSize)
			return NULL;

		memset (pLineText, 0, nLineSize);
		strncpy ((char *)pLineText, pCurPos, pNextPos - pCurPos);
		nLineSize = strlen ((char *)pLineText);

		return (char *)pNextPos + 1;
	}

	return NULL;
}
