/*******************************************************************************
	File:		CStreamDemux.h

	Contains:	The stream data demux header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __CStreamDemux_H__
#define __CStreamDemux_H__

#include "voParser.h"
#include "voAdaptiveStreamParser.h"

#include "CBaseObject.h"
#include "COutBuffer.h"

class CStreamDemux : public CBaseObject
{
public:
    CStreamDemux (void);
    virtual ~CStreamDemux(void);

	virtual void	SetOutBuffer (COutBuffer * pOutBuff) {m_pOutBuffer = pOutBuff;}
 	virtual void	SetStartTime (long long llStart) {m_llStartTime = llStart;}
	virtual int		SetPos (long long llPos) {m_llSeekPos = llPos; return YY_ERR_NONE;}
	
	virtual int		Demux(int nFlag, VO_PBYTE pBuff, VO_S32 nSize);

protected:
	COutBuffer*  			m_pOutBuffer;
	int                   	m_nStreamFlag;
	long long				m_llStartTime;
	long long				m_llSeekPos;

	bool					m_bVideoChecked;
	VO_U64					m_llLastVideoTime;
	bool					m_bAudioChecked;
	VO_U64					m_llLastAudioTime;
};

#endif
