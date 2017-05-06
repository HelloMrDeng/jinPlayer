/*******************************************************************************
	File:		CBaseVideoEnc.cpp

	Contains:	The base Video encode implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-06-03		Fenger			Create file

*******************************************************************************/
#include "CBaseVideoEnc.h"

#include "yyConfig.h"
#include "yyLog.h"

CBaseVideoEnc::CBaseVideoEnc(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
{
	SetObjectName ("CBaseVideoEnc");
	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
}

CBaseVideoEnc::~CBaseVideoEnc(void)
{
	YY_DEL_A (m_fmtVideo.pHeadData);
}

int CBaseVideoEnc::Init (YY_VIDEO_FORMAT * pFmt)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseVideoEnc::Uninit (void)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseVideoEnc::Encode (YY_BUFFER * pIn, YY_BUFFER * pOut)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseVideoEnc::Flush (void)
{
	return YY_ERR_NONE;
}
