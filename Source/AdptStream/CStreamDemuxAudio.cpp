/*******************************************************************************
	File:		CStreamDemuxAudio.cpp

	Contains:	stream demux implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "CStreamDemuxAudio.h"

#include "UYYDataFunc.h"
#include "UVV2FF.h"

CStreamDemuxAudio::CStreamDemuxAudio (void)
	: CStreamDemux ()
	, m_pFmtAudio (NULL)
{
	SetObjectName ("CStreamDemuxAudio");
	memset (&m_sBuffer, 0, sizeof (m_sBuffer));
}

CStreamDemuxAudio::~CStreamDemuxAudio(void)
{
	yyDataDeleteAudioFormat (&m_pFmtAudio);
}

int	CStreamDemuxAudio::Demux (int nFlag, VO_PBYTE pBuff, VO_S32 nSize)
{
	int nRC = VO_ERR_NONE;
	if ((nFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
	{
		m_nStreamFlag = YYBUFF_NEW_FORMAT;
		m_bVideoChecked = false;
		m_bAudioChecked = false;
	}
	if ((nFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
	{
		m_nStreamFlag = m_nStreamFlag | YYBUFF_NEW_POS;
		m_bVideoChecked = false;
		m_bAudioChecked = false;
	}


	return nRC;
}


