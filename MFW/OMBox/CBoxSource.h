/*******************************************************************************
	File:		CBoxSource.h

	Contains:	the source box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxSource_H__
#define __CBoxSource_H__

#include "CBoxBase.h"
#include "CBaseSource.h"

class CBoxSource : public CBoxBase
{
public:
	CBoxSource(void * hInst);
	virtual ~CBoxSource(void);

	virtual int		OpenSource (const TCHAR * pSource, int nFlag);
	virtual int		Close (void);
	virtual char *	GetSourceName (void);

	virtual int		GetStreamPlay (YYMediaType nType);
	virtual int		SetStreamPlay (YYMediaType nType, int nIndex);
	virtual int		GetDuration (void);
	virtual int		GetMediaInfo (TCHAR * pInfo, int nSize);

	virtual int		Start (void);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		ReadBuffer (YY_BUFFER * pBuffer, bool bWait);

	virtual int		SetPos (int nPos, bool bSeek);
	virtual int		GetPos (void);

	virtual int 	SetParam (int nID, void * pParam);
	virtual int		GetParam (int nID, void * pParam);

	virtual void	EnableSubTT (bool bEnable) {m_bEnableSubTT = bEnable;}
	CBaseSource *	GetMediaSource (void) {return m_pMediaSource;}

protected:
	CBaseSource *	m_pMediaSource;
	bool			m_bEnableSubTT;
};

#endif // __CBoxSource_H__
