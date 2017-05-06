/*******************************************************************************
	File:		CSubtitleSmi.h

	Contains:	the subtitle srt header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-17		Fenger			Create file

*******************************************************************************/
#ifndef __CSubtitleSmi_H__
#define __CSubtitleSmi_H__

#include "CBaseObject.h"
#include "yyMediaPlayer.h"

#include "CSubtitleBase.h"

class CSubtitleSmi : public CSubtitleBase
{
public:
	CSubtitleSmi(void);
	virtual ~CSubtitleSmi(void);

	virtual int		Parse (CBaseFile * pFile);

protected:


};

#endif // __CSubtitleSmi_H__
