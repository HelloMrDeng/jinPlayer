/*******************************************************************************
	File:		CBoxVideoRndExt.h

	Contains:	the video render box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxVideoRndExt_H__
#define __CBoxVideoRndExt_H__

#include "CBoxVideoRnd.h"
	
#include "CNodeList.h"

#include "yyMediaPlayer.h"

class CBoxVideoRndExt : public CBoxVideoRnd
{
public:
	CBoxVideoRndExt(void * hInst);
	virtual ~CBoxVideoRndExt(void);

	virtual int		SetRndType (YYRND_TYPE nRndType);
	virtual int		SetDisplay (void * hView, RECT * pRect);
	virtual int		UpdateDisp (void);
	virtual int		SetAspectRatio (int w, int h);
	virtual int		SetDDMode (YY_PLAY_DDMode nMode);
	virtual int		DisableVideo (int nFlag);

	virtual int		SetSource (CBoxBase * pSource);
	virtual int		RenderFrame (bool bInBox, bool bWait);

	virtual int		ReadBuffer (YY_BUFFER * pBuffer, bool bWait);

	virtual int		Start (CThreadWork * pWork);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual CBaseClock * GetClock (void);

protected:
	CMutexLock					m_mtList;
	CObjectList<YY_BUFFER>		m_lstFull;
	CObjectList<YY_BUFFER>		m_lstFree;
};

#endif // __CBoxVideoRndExt_H__
