/*******************************************************************************
	File:		CLangText.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "CLangText.h"

#include "USystemFunc.h"
#include "UFileFunc.h"
#include "UStringFunc.h"

#include "yyLog.h"

CLangText *	CLangText::g_pLang = NULL;

CLangText::CLangText(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_nLang (YYLANG_CHN)
{
	SetObjectName ("CLangText");
	ReadText ();
	_tcscpy (m_szError, _T("It can't Find text!"));

	g_pLang = this;
}

CLangText::~CLangText(void)
{
	FreeText ();
	g_pLang = NULL;
}

int CLangText::GetLang (void)
{
	return m_nLang;
}

int CLangText::setLang (int nLang)
{
	if (m_nLang == nLang)
		return m_nLang;

	m_nLang = nLang;
	ReadText ();

	return m_nLang;
}

TCHAR *	CLangText::GetText (int nID)
{
	TEXT_Item * pItem = NULL;

	NODEPOS pos = m_lstText.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstText.GetNext (pos);
		if (pItem->nID == nID)
			return pItem->pText;
	}

	return m_szError;
}

bool CLangText::ReadText (void)
{
	yyFile hFile = NULL;
	TCHAR szFile[1024];
	yyGetAppPath (m_hInst, szFile, sizeof (szFile));
	_tcscat (szFile, _T("jpres\\lang_"));
	if (m_nLang == YYLANG_CHN)
		_tcscat (szFile, _T("chn.txt"));
	else
		_tcscat (szFile, _T("eng.txt"));
	hFile = yyFileOpen (szFile, YYFILE_READ);
	if (hFile == NULL)
		return false;
	int nFileSize = (int)yyFileSize (hFile);
	unsigned char * pTextFile = new unsigned char[nFileSize];
	yyFileRead (hFile, pTextFile, nFileSize);
	yyFileClose (hFile);

	FreeText ();

	TEXT_Item * pItem = NULL;
	int nRest = nFileSize / 2;
	TCHAR * pText = (TCHAR *)pTextFile;
	TCHAR szLine[256];
	if (pTextFile[0] == 0XFF && pTextFile[1] == 0XFE)
	{
		pText = (TCHAR *)(pTextFile + 2);
		nRest = nRest -1;
	}

	int		nLine = 0;
	TCHAR * pSpace = NULL;
	while (nRest > 0)
	{
		nLine = yyTextReadLine (pText, nRest, szLine, sizeof (szLine));
		pText += nLine;
		nRest -= nLine;

		if (_tcslen (szLine) <= 2)
			continue;

		pItem = new TEXT_Item ();
		pItem->pText = new TCHAR[nLine];
		_stscanf (szLine, _T("%d %s"), &pItem->nID, pItem->pText);
		m_lstText.AddTail (pItem);
	}

	delete []pTextFile;
	return true;
}

bool CLangText::FreeText (void)
{
	TEXT_Item * pItem = m_lstText.RemoveHead ();
	while (pItem != NULL)
	{
		YY_DEL_A (pItem->pText);
		delete pItem;
		pItem = m_lstText.RemoveHead ();
	}
	return true;
}
