/*******************************************************************************
	File:		CSubtitleItem.h

	Contains:	the subtitle item header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-17		Fenger			Create file

*******************************************************************************/
#ifndef __CSubtitleItem_H__
#define __CSubtitleItem_H__

#include "CBaseObject.h"
#include "yyMediaPlayer.h"

#define ST_MAX_LINE_NUM	32

class CSubtitleItem : public CBaseObject
{
public:
	CSubtitleItem(void);
	virtual ~CSubtitleItem(void);

	virtual int			AddText (TCHAR * pText, long long llStart = -1, long long llEnd = -1, int nIndex = -1);
	virtual int			Reset (void);

	virtual TCHAR *		GetText (int nLine);
	virtual int			GetTime (long long * pStart, long long * pEnd);
	virtual long long	GetStart (void) {return m_llStart;}
	virtual long long	GetEnd (void) {return m_llEnd;}
	virtual int			GetLineNum (void) {return m_nLineNum;}

//	virtual int			Draw (HDC hdc, RECT * pRect);

public:
	int				m_nIndex;
	long long		m_llPrevEnd;
	long long		m_llStart;
	long long		m_llEnd;
	int				m_nLineNum;
	TCHAR *			m_ppText[ST_MAX_LINE_NUM];

#ifdef _OS_WIN32
	int				m_nTxtColor;
	int				m_nTxtSize;
	HFONT			m_hTxtFont;
#endif // _OS_WIN32
};

#endif // __CSubtitleItem_H__
