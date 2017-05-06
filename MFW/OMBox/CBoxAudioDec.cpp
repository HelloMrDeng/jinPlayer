/*******************************************************************************
	File:		CBoxAudioDec.cpp

	Contains:	The audio dec box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#include "CBoxAudioDec.h"
#include "CFFMpegAudioDec.h"
#ifdef _EXT_VO
#include "CVVAudioDec.h"
#endif // _EXT_VO

#include "CBoxMonitor.h"
#include "UYYDataFunc.h"

#include "yyLog.h"

CBoxAudioDec::CBoxAudioDec(void * hInst)
	: CBoxBase (hInst)
	, m_pDec (NULL)
{
	SetObjectName ("CBoxAudioDec");
	m_nBoxType = OMB_TYPE_FILTER;
	strcpy (m_szBoxName, "Audio Dec Box");

	m_nAudioStreamNum = 1;
	m_nAudioStreamPlay = 0;

	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));
	m_pFmtAudio = &m_fmtAudio;
}

CBoxAudioDec::~CBoxAudioDec(void)
{
	YY_DEL_P (m_pDec);
	YY_DEL_A (m_fmtAudio.pHeadData);
}

int CBoxAudioDec::SetSource (CBoxBase * pSource)
{
	if (pSource == NULL)
		return YY_ERR_ARG;

	Stop ();

	YY_DEL_P (m_pDec);

	CBoxBase::SetSource (pSource);

	YY_AUDIO_FORMAT * pFmt = pSource->GetAudioFormat ();
	if (pFmt == NULL)
		return YY_ERR_AUDIO;
	yyDataCloneAudioFormat (&m_fmtAudio, pFmt);

	bool bFF = true;
#ifdef _EXT_VO
#ifdef _OS_WINCE
	if (pFmt->nCodecID == AV_CODEC_ID_AAC || pFmt->nCodecID == AV_CODEC_ID_AC3)	
	{
		bFF = false;
		m_pDec = new CVVAudioDec (m_hInst);
	}
	else
#endif // _OS_WIN32
#endif // _EXT_VO
		m_pDec = new CFFMpegAudioDec (m_hInst);
	if (m_pDec == NULL)
		return YY_ERR_MEMORY;

	int nRC = m_pDec->Init (pFmt);
	if (nRC != YY_ERR_NONE)
	{
		if (!bFF)
		{
			delete m_pDec;
			m_pDec = new CFFMpegAudioDec (m_hInst);
			nRC = m_pDec->Init (pFmt);
		}

		if (nRC != YY_ERR_NONE)
			return nRC;
	}

	m_pFmtAudio = m_pDec->GetAudioFormat ();

	return YY_ERR_NONE;
}

int CBoxAudioDec::SetPos (int nPos, bool bSeek)
{
	if (m_pDec != NULL)
		m_pDec->Flush ();
	return CBoxBase::SetPos (nPos, bSeek);
}

int CBoxAudioDec::ReadBuffer (YY_BUFFER * pBuffer, bool bWait)
{
	if (m_pDec == NULL)
		return YY_ERR_FAILED;
	
	if (m_pDec->GetBuff (pBuffer) == YY_ERR_NONE)
		return YY_ERR_NONE;

	int nRC = YY_ERR_NEEDMORE;
	if (m_pCurrBuffer != NULL)
	{
		nRC = m_pDec->SetBuff (m_pCurrBuffer);
		if (nRC == YY_ERR_RETRY || nRC == YY_ERR_NONE)
		{
			if (nRC == YY_ERR_NONE)
				m_pCurrBuffer = NULL;	
			return 	m_pDec->GetBuff (pBuffer);
		}
		m_pCurrBuffer = NULL;
		if (nRC < 0)
			return nRC;
	}

	m_pBaseBuffer->nType = YY_MEDIA_Audio;
	m_pBaseBuffer->uFlag = 0;
	m_pBaseBuffer->llTime = 0;
	while (m_nStatus == OMB_STATUS_RUN || m_nStatus == OMB_STATUS_PAUSE)
	{
		nRC = m_pBoxSource->ReadBuffer (m_pBaseBuffer, false);
		if (nRC != YY_ERR_NONE)
		{
			if ((m_pBaseBuffer->uFlag & YYBUFF_EOS) == YYBUFF_EOS)
				pBuffer->uFlag |= YYBUFF_EOS;
			return nRC;
		}

		BOX_READ_BUFFER_REC_AUDIODEC

		if ((m_pBaseBuffer->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
		{
			YY_AUDIO_FORMAT * pFmt = (YY_AUDIO_FORMAT*) m_pBaseBuffer->pFormat;
			yyDataCloneAudioFormat (&m_fmtAudio, pFmt);
			nRC = m_pDec->Init (pFmt);
			if (nRC != YY_ERR_NONE)
				return nRC;
			m_pFmtAudio = m_pDec->GetAudioFormat ();
		}

		nRC = m_pDec->SetBuff (m_pBaseBuffer);
		if (nRC == YY_ERR_NONE)
			break;
		else if (nRC == YY_ERR_RETRY)
		{
			m_pCurrBuffer = m_pBaseBuffer;
			break;
		}
		else if (nRC == YY_ERR_NEEDMORE)
			continue;
		else
			return nRC;
	}

	nRC = m_pDec->GetBuff (pBuffer);
	if (pBuffer->uSize > 0)
		return YY_ERR_NONE;

	return nRC;
}
