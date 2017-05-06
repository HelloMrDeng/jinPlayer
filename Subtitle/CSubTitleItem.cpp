/*******************************************************************************
	File:		CSubtitleItem.cpp

	Contains:	subtitle item implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-03		Fenger			Create file

*******************************************************************************/
#include "CSubtitleItem.h"

#include "stdio.h"
#include "UStringFunc.h"

#include "yyConfig.h"
#include "yyLog.h"

CSubtitleItem::CSubtitleItem(void)
	: CBaseObject ()
	, m_nIndex (-1)
	, m_llPrevEnd (-1)
	, m_llStart (-1)
	, m_llEnd (-1)
	, m_nLineNum (0)
#ifdef _OS_WIN32
	, m_nTxtColor (-1)
	, m_nTxtSize (0)
	, m_hTxtFont (NULL)
#endif // _OS_WIN32
{
	SetObjectName ("CSubtitleItem");
	for (int i = 0; i < ST_MAX_LINE_NUM; i++)
		m_ppText[i] = NULL;
}

CSubtitleItem::~CSubtitleItem(void)
{
	for (int i = 0; i < ST_MAX_LINE_NUM; i++)
	{
		if (m_ppText[i] != NULL)
			delete []m_ppText[i];
	}

#ifdef _OS_WIN32
	if (m_hTxtFont != NULL)
		DeleteObject (m_hTxtFont);
	m_hTxtFont = NULL;
#endif // _OS_WIN32
}

int CSubtitleItem::AddText (TCHAR * pText, long long llStart, long long llEnd, int nIndex)
{
	if (m_nLineNum >= ST_MAX_LINE_NUM)
		return -1;

	if (nIndex >= 0)
		m_nIndex = nIndex;

	if (llStart >= 0)
		m_llStart = llStart;
	if (llEnd >= 0)
		m_llEnd = llEnd;

	if (m_ppText[m_nLineNum] != NULL)
		delete []m_ppText[m_nLineNum];

	m_ppText[m_nLineNum] = new TCHAR[_tcslen (pText) + 1];
	_tcscpy (m_ppText[m_nLineNum], pText);

	m_nLineNum++;

	return 0;
}

int CSubtitleItem::Reset (void)
{
	for (int i = 0; i < ST_MAX_LINE_NUM; i++)
	{
		if (m_ppText[i] != NULL)
			delete []m_ppText[i];
		m_ppText[i] = NULL;
	}
	m_nLineNum = 0;
	m_llStart = -1;
	m_llEnd = -1;
	m_llPrevEnd = -1;
	return 0;
}

TCHAR *	CSubtitleItem::GetText (int nLine)
{
	if (nLine < 0 || nLine >= ST_MAX_LINE_NUM)
		return NULL;

	return m_ppText[nLine];
}

int CSubtitleItem::GetTime (long long * pStart, long long * pEnd)
{
	*pStart = m_llStart;
	*pEnd = m_llEnd;

	return 0;
}

/*
int CSubtitleItem::Draw (HDC hdc, RECT * pRect)
{
	if (m_nLineNum <= 0)
		return 0;

	if (m_nTxtColor > 0)
		SetTextColor (hdc, m_nTxtColor);
	HFONT hFontOld = NULL;
	if (m_hTxtFont != NULL)
		hFontOld = (HFONT)SelectObject (hdc, m_hTxtFont);

	SIZE szTxt;
	GetTextExtentPoint (hdc, m_ppText[0], _tcslen (m_ppText[0]), &szTxt);

	RECT rcDraw;
	for (int i = m_nLineNum; i > 0; i--)
	{
		SetRect (&rcDraw, pRect->left, pRect->bottom - ((szTxt.cy + 8) * i), 
					pRect->right, pRect->bottom - ((szTxt.cy + 8) * (i - 1)));
		DrawText (hdc, m_ppText[m_nLineNum - i], _tcslen (m_ppText[m_nLineNum - i]), &rcDraw, DT_CENTER | DT_BOTTOM);
	}

	if (hFontOld != NULL)
		SelectObject (hdc, hFontOld);

	return (szTxt.cy + 8) * m_nLineNum;
}
*/