/*******************************************************************************
	File:		COutBuffer.h

	Contains:	The out buffer header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __COutBuffer_H__
#define __COutBuffer_H__

#include "voSource2.h"
#include "voParser.h"
#include "voSource2_ProgramInfo.h"

#include "yyData.h"
#include "CBaseObject.h"
#include "CStreamBA.h"

class COutBuffer : public CBaseObject
{
public:
	COutBuffer (void);
	virtual ~COutBuffer(void);

 	virtual void	SetProgInfoFunc (VOS2_ProgramInfo_Func * pProgFunc) {m_pProgFunc = pProgFunc;}
	virtual void	SetStreamBA (CStreamBA * pBA) {m_pBA = pBA;}

	virtual int		GetSample (YY_BUFFER * pBuff);
	virtual int		AddSample (YY_BUFFER * pBuff);
	virtual int		AddBuffer (VO_PARSER_OUTPUT_BUFFER * pBuff, int nFlag);

	virtual int		SetPos (long long llPos) {m_llSeekPos = llPos; return YY_ERR_NONE;}
	virtual void	SetEOS (bool bEOS) {m_bEOS = bEOS;}

	virtual int		GetBufferInfo (int nTrackType, VO_S64 * llDur, VO_U32 * nCount);
	virtual int		GetBuffTime (VO_S64 * pGetBuffTime, VO_S64 * pAddBuffTime);
	virtual bool	IsBuffering (void);

	YY_VIDEO_FORMAT *	GetVideoFormat (void) {return &m_fmtVideo;}
	YY_AUDIO_FORMAT *	GetAudioFormat (void) {return &m_fmtAudio;}

protected:
	virtual int		AddTrackInfo (VO_PARSER_STREAMINFO * pInfo);

protected:
	CStreamBA *					m_pBA;
	VOS2_ProgramInfo_Func *		m_pProgFunc;
	VO_U32						m_uTrackID;

	long long					m_llOffsetTime;
	long long					m_llVAddLastTime;
	long long					m_llVGetLastTime;
	long long					m_llAAddLastTime;
	long long					m_llAGetLastTime;

	YY_VIDEO_FORMAT 			m_fmtVideo;
	YY_AUDIO_FORMAT 			m_fmtAudio;

	long long					m_llSeekPos;
	bool						m_bNeedBuff;
	bool						m_bEOS;
};

#endif
