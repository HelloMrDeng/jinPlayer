/*******************************************************************************
	File:		CSubtitleFFMpeg.h

	Contains:	the subtitle srt header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-17		Fenger			Create file

*******************************************************************************/
#ifndef __CSubtitleFFMpeg_H__
#define __CSubtitleFFMpeg_H__

#include "CBaseObject.h"
#include "yyMediaPlayer.h"

#include "CSubtitleBase.h"
#include "CBaseSource.h"
#include "CFFMpegSubTTDec.h"
#include "CMutexLock.h"

class CSubtitleFFMpeg : public CSubtitleBase
{
public:
	CSubtitleFFMpeg(void);
	virtual ~CSubtitleFFMpeg(void);

	virtual int				Parse (CBaseSource * pSource);
	virtual int				SetPos (int nPos);

	virtual CSubtitleItem *	GetItem (long long llTime);

protected:
	virtual int		ParseAss (char * pText);

protected:
	CMutexLock			m_mtSeek;
	CBaseSource *		m_pSrc;
	CFFMpegSubTTDec *	m_pDec;

	YY_BUFFER			m_bufSrc;
	YY_BUFFER			m_bufDec;
	AVSubtitle *		m_pAVSub;
	TCHAR				m_szText[1024];

	long long			m_llStart;
	long long			m_llEnd;
	CSubtitleItem		m_subItem1;
	CSubtitleItem		m_subItem2;
	CSubtitleItem *		m_pSubItem;
};

#endif // __CSubtitleFFMpeg_H__
