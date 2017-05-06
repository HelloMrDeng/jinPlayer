/*******************************************************************************
	File:		CSubtitleAss.h

	Contains:	the subtitle srt header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-17		Fenger			Create file

*******************************************************************************/
#ifndef __CSubtitleAss_H__
#define __CSubtitleAss_H__

#include "CBaseObject.h"
#include "yyMediaPlayer.h"

#include "CSubtitleBase.h"

class CSubtitleAss : public CSubtitleBase
{
public:
	CSubtitleAss(void);
	virtual ~CSubtitleAss(void);

	virtual int		Parse (CBaseFile * pFile);

protected:


};

#endif // __CSubtitleAss_H__
