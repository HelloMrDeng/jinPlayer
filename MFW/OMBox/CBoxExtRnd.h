/*******************************************************************************
	File:		CBoxExtRnd.h

	Contains:	the video render box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxExtRnd_H__
#define __CBoxExtRnd_H__

#include "CBoxVideoRnd.h"
#include "CNodeList.h"
#include "CBaseVideoDec.h"

#include "yyMediaPlayer.h"

class CBoxExtRnd : public CBoxRender
{
public:
	CBoxExtRnd(void * hInst);
	virtual ~CBoxExtRnd(void);

	virtual int		SetSource (CBoxBase * pSource);
	virtual int		RenderFrame (bool bInBox, bool bWait);
	virtual int		ReadBuffer (YY_BUFFER * pBuffer, bool bWait);

#ifdef _OS_NDK
	virtual int		Start (CThreadWork * pWork);
	virtual int		Pause (void);
	virtual int		Stop (void);
#endif // _OS_NDK

protected:
	YY_BUFFER		m_bufRnd;

	unsigned int	m_uCodecTag;
	CBaseVideoDec *	m_pVDec;

	unsigned char *	m_pVHead;
	int				m_nVSize;
};

#endif // __CBoxExtRnd_H__
