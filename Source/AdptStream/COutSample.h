/*******************************************************************************
	File:		COutSample.h

	Contains:	The out sample header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __COutSample_H__
#define __COutSample_H__

#include "voSource2.h"
#include "voParser.h"

#include "CMutexLock.h"
#include "CNodeList.h"

#include "COutBuffer.h"

class COutSample : public COutBuffer
{
public:
	COutSample (void);
	virtual ~COutSample(void);

	virtual void	Reset (void);

	virtual int		GetSample (YY_BUFFER * pBuff);
	virtual int		AddBuffer (VO_PARSER_OUTPUT_BUFFER * pBuff, int nFlag);

	virtual int		SetPos (long long llPos);
	virtual int		GetBufferInfo (int nTrackType, VO_S64 * llDur, VO_U32 * nCount);
	virtual bool	IsBuffering (void);

protected:
	virtual int	GetVideoSample (YY_BUFFER * pBuff);
	virtual int	GetAudioSample (YY_BUFFER * pBuff);

	virtual int	AddVideo (VO_MTV_FRAME_BUFFER * pBuff);
	virtual int	AddAudio (VO_MTV_FRAME_BUFFER * pBuff);
	virtual int	AddTrackInfo (VO_PARSER_STREAMINFO * pInfo);

	virtual int	SwitchSampleList (void);

	void		FreeSampleList (CObjectList <YY_BUFFER> * pListFull, CObjectList <YY_BUFFER> * pListFree);
	void		DeleteSampleAll (void);

protected:
	CMutexLock					m_mtBuff;
	int							m_nVStreamFlag;
	CObjectList <YY_BUFFER>	 *	m_pLstAddVideo;
	CObjectList <YY_BUFFER>	 *	m_pLstGetVideo;
	CObjectList <YY_BUFFER>		m_lstVideo1;
	CObjectList <YY_BUFFER>		m_lstVideo2;
	CObjectList <YY_BUFFER>		m_lstVideoFree;

	int							m_nAStreamFlag;
	CObjectList <YY_BUFFER>	 *	m_pLstAddAudio;
	CObjectList <YY_BUFFER>	 *	m_pLstGetAudio;
	CObjectList <YY_BUFFER>		m_lstAudio1;
	CObjectList <YY_BUFFER>		m_lstAudio2;
	CObjectList <YY_BUFFER>		m_lstAudioFree;
};

#endif
