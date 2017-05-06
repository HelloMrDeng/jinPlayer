/*******************************************************************************
	File:		CSubtitleSrt.h

	Contains:	the subtitle srt header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-17		Fenger			Create file

*******************************************************************************/
#ifndef __CSubtitleSrt_H__
#define __CSubtitleSrt_H__

#include "CBaseObject.h"
#include "yyMediaPlayer.h"

#include "CSubtitleBase.h"

class CSubtitleSrt : public CSubtitleBase
{
public:
	CSubtitleSrt(void);
	virtual ~CSubtitleSrt(void);

	virtual int		Parse (CBaseFile * pFile);


protected:

};

#endif // __CSubtitleSrt_H__
