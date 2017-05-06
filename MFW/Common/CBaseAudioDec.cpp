/*******************************************************************************
	File:		CBaseAudioDec.cpp

	Contains:	The base audio dec implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CBaseAudioDec.h"

#include "yyConfig.h"
#include "yyLog.h"

CBaseAudioDec::CBaseAudioDec(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_uBuffFlag (0)
{
	SetObjectName ("CBaseAudioDec");

	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));
}

CBaseAudioDec::~CBaseAudioDec(void)
{
	YY_DEL_A (m_fmtAudio.pHeadData);
}

int CBaseAudioDec::Init (YY_AUDIO_FORMAT * pFmt)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseAudioDec::Uninit (void)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseAudioDec::SetBuff (YY_BUFFER * pBuff)
{
	if ((pBuff->uFlag & YYBUFF_EOS) == YYBUFF_EOS)
		m_uBuffFlag |= YYBUFF_EOS;

	if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		m_uBuffFlag |= YYBUFF_NEW_POS;

	return YY_ERR_NONE;
}

int CBaseAudioDec::GetBuff (YY_BUFFER * pBuff)
{
	if ((m_uBuffFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		pBuff->uFlag |= YYBUFF_NEW_POS;
	if ((m_uBuffFlag & YYBUFF_EOS) == YYBUFF_EOS)
		pBuff->uFlag |= YYBUFF_EOS;
	m_uBuffFlag = 0;

	return YY_ERR_NONE;
}

int	CBaseAudioDec::Start (void)
{
	return YY_ERR_NONE;
}

int CBaseAudioDec::Pause (void)
{
	return YY_ERR_NONE;
}

int	CBaseAudioDec::Stop (void)
{
	return YY_ERR_NONE;
}

int CBaseAudioDec::Flush (void)
{
	return YY_ERR_NONE;
}
