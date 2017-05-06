/*******************************************************************************
	File:		CStreamDemux.cpp

	Contains:	stream demux implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "CStreamDemux.h"

CStreamDemux::CStreamDemux (void)
	: CBaseObject ()
	, m_pOutBuffer (NULL)
	, m_nStreamFlag (0)
	, m_llStartTime (0)
	, m_llSeekPos (0)
	, m_bVideoChecked (true)
	, m_llLastVideoTime (0)
	, m_bAudioChecked (true)
	, m_llLastAudioTime (0)
{
	SetObjectName ("CStreamDemux");
}

CStreamDemux::~CStreamDemux(void)
{
}

int	CStreamDemux::Demux (int nFlag, VO_PBYTE pBuff, VO_S32 nSize)
{
	return YY_ERR_FAILED;
}
