/*******************************************************************************
	File:		CSubtitleBase.h

	Contains:	the subtitle base header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-03		Fenger			Create file

*******************************************************************************/
#ifndef __CSubtitleBase_H__
#define __CSubtitleBase_H__

#include "CBaseObject.h"
#include "yyMediaPlayer.h"

#include "CNodeList.h"
#include "CBaseFile.h"
#include "CSubtitleItem.h"

class CSubtitleBase : public CBaseObject
{
public:
	CSubtitleBase(void);
	virtual ~CSubtitleBase(void);

	virtual int				Parse (CBaseFile * pFile);
	virtual int				SetPos (int nPos);
	virtual CSubtitleItem *	GetItem (long long llTime);
	virtual int				GetCharset (void) {return m_nCharset;}

	virtual int		GetFontSize (void);
	virtual int		GetFontColor (void);
	virtual HFONT	GetFontHandle (void);

	virtual int		GetTrackNum (void) {return m_nTrackNum;}
	virtual int		SelectTrack (int nTrack) {return 0;}

protected:
	virtual int		CheckTimeStamp (void);
	virtual int		ReleaseItems (void);
	virtual int		CreateTxtFont (void);

	virtual int		ReadLine (TCHAR * pText, int nTextLen, TCHAR * pLine, int nSize);
	virtual int		CharReadLine (char * pText, int nTextLen, char * pLine, int nSize);

protected:
	int								m_nCharset;
	int								m_nTrackNum;

	CObjectList<CSubtitleItem>		m_lstTemp;
	CObjectList<CSubtitleItem>		m_lstText;
	NODEPOS						m_posItem;
	CSubtitleItem *					m_pPosItem;

	int								m_nFileSize;
	TCHAR *							m_pFileBuff;
	int								m_nBuffSize;

	HFONT							m_hTextFont;
	int								m_nFontSize;
	int								m_nFontColor;

public:
#ifdef _OS_WIN32
	static int __cdecl compare_starttime (const void *arg1, const void *arg2);
#else
	static int compare_starttime (const void *arg1, const void *arg2);
#endif // _OS_WIN32	
};

#endif // __CSubtitleBase_H__
