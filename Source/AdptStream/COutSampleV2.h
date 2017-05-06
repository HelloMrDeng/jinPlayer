/*******************************************************************************
	File:		COutSampleV2.h

	Contains:	The out sample header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __COutSampleV2_H__
#define __COutSampleV2_H__

#include "voSource2.h"
#include "voParser.h"

#include "CMutexLock.h"
#include "CNodeList.h"

#include "COutBuffer.h"

class COutSampleV2 : public COutBuffer
{
public:
	COutSampleV2 (void);
	virtual ~COutSampleV2(void);

	virtual void	Reset (void);

	virtual int		GetSample (YY_BUFFER * pBuff);
	virtual int		AddSample (YY_BUFFER * pBuff);

	virtual int		SetPos (long long llPos);
	virtual int		GetBufferInfo (int nTrackType, VO_S64 * llDur, VO_U32 * nCount);
	virtual bool	IsBuffering (void);

protected:
	virtual int	GetVideoSample (YY_BUFFER * pBuff);
	virtual int	GetAudioSample (YY_BUFFER * pBuff);

	YY_BUFFER *	GetBuffer (CObjectList <YY_BUFFER> * pListFree);
	void		FreeSampleList (CObjectList <YY_BUFFER> * pListFull, CObjectList <YY_BUFFER> * pListFree);
	void		DeleteAllSample (void);

	int			GetListInfo (CObjectList <YY_BUFFER> * pList, long long llStart, VO_S64 * llDur, VO_U32 * nCount);


protected:
	CMutexLock					m_mtBuff;
	CObjectList <YY_BUFFER>		m_lstVideoFull;
	CObjectList <YY_BUFFER>		m_lstVideoBuff;
	CObjectList <YY_BUFFER>		m_lstVideoFree;

	CObjectList <YY_BUFFER>		m_lstAudioFull;
	CObjectList <YY_BUFFER>		m_lstAudioBuff;
	CObjectList <YY_BUFFER>		m_lstAudioFree;

	YY_VIDEO_FORMAT *			m_pFmtVideo;
	YY_AUDIO_FORMAT *			m_pFmtAudio;

};

#endif
