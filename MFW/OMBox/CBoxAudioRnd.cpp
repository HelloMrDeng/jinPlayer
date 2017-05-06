/*******************************************************************************
	File:		CBoxAudioRnd.cpp

	Contains:	The audio render box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifdef _OS_WIN32
#include "windows.h"
#include "CWaveOutRnd.h"
#endif // _OS_WIN32

#include "CBoxAudioRnd.h"

#include "CBoxMonitor.h"
#include "USystemFunc.h"
#include "UYYDataFunc.h"

#include "yyMediaPlayer.h"
#include "yyLog.h"

CBoxAudioRnd::CBoxAudioRnd(void * hInst)
	: CBoxRender (hInst)
	, m_nRndType (YY_ART_WAVE_MAPPER)
	, m_pRnd (NULL)
	, m_nVolume (-1)
	, m_fSpeed (1.0)
	, m_nNewFmtCount (0)
	, m_pNewFmtBuffer (NULL)
	, m_nNewFmtBuffSize (0)
{
	SetObjectName ("CBoxAudioRnd");
	m_nMediaType = YY_MEDIA_Audio;
	m_nBoxType = OMB_TYPE_RENDER;
	strcpy (m_szBoxName, "Audio Render Box");

	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));
	memset (&m_fmtAudioNew, 0, sizeof (m_fmtAudioNew));
	m_pFmtAudio = &m_fmtAudio;
}

CBoxAudioRnd::~CBoxAudioRnd(void)
{
	Stop ();

	YY_DEL_P (m_pRnd);
	YY_DEL_A (m_fmtAudio.pHeadData);
	YY_DEL_A (m_pNewFmtBuffer);
}

int CBoxAudioRnd::SetAudioRndType (YY_PLAY_ARType nType)
{
	CAutoLock lock (&m_mtRnd);
	if (m_nRndType == nType)
		return YY_ERR_NONE;

	m_nRndType = nType;
	if (m_pBoxSource == NULL)
		return YY_ERR_NONE;
	else
		return YY_ERR_STATUS;
}

int CBoxAudioRnd::SetSource (CBoxBase * pSource)
{
	if (pSource == NULL)
	{
		m_pBoxSource = NULL;
		ResetMembers ();
		return YY_ERR_ARG;
	}

	Stop ();

	CBoxBase::SetSource (pSource);
	YY_AUDIO_FORMAT * pFmt = pSource->GetAudioFormat ();
	if (pFmt == NULL)
		return YY_ERR_AUDIO;
	yyDataCloneAudioFormat (&m_fmtAudio, pFmt);
	m_fmtAudioNew.nSampleRate = pFmt->nSampleRate;
	m_fmtAudioNew.nChannels = pFmt->nChannels;
	m_fmtAudioNew.nBits = pFmt->nBits;
	if (m_pDataCB != NULL)
		return YY_ERR_NONE;

	YY_DEL_P (m_pRnd);
#ifdef _OS_WIN32
	m_pRnd = new CWaveOutRnd (m_hInst, m_pOtherRnd == NULL);
#endif // _OS_WIN32
	if (m_pRnd == NULL)
	{
		if (m_pDataCB == NULL)
			return YY_ERR_MEMORY;
		else
			return YY_ERR_NONE;
	}

	m_pRnd->SetType (m_nRndType);
	int nRC = m_pRnd->Init (pFmt);
	if (nRC != YY_ERR_NONE)
		return nRC;

	if (m_nVolume >= 0)
		m_pRnd->SetVolume (m_nVolume);

	m_pRnd->SetSpeed (m_fSpeed);

	return YY_ERR_NONE;
}

int	CBoxAudioRnd::Start (CThreadWork * pWork)
{
	if (m_pRnd != NULL)
		m_pRnd->Start ();

	return CBoxRender::Start (pWork);
}

int CBoxAudioRnd::Pause (void)
{
	int nRC = CBoxRender::Pause ();

	if (m_pRnd != NULL)
		m_pRnd->Pause ();

	return nRC;
}

int	CBoxAudioRnd::Stop (void)
{
	int nRC = CBoxRender::Stop ();

	if (m_pRnd != NULL)
		m_pRnd->Stop ();

	return nRC;
}

int CBoxAudioRnd::SetPos (int nPos, bool bSeek)
{
	if (m_pRnd != NULL)
		m_pRnd->Flush ();

	return CBoxRender::SetPos (nPos, bSeek);
}

int CBoxAudioRnd::SetSpeed (float fSpeed)
{
	m_fSpeed = fSpeed;

	if (m_pRnd != NULL)
		return m_pRnd->SetSpeed (m_fSpeed);

	return YY_ERR_STATUS;
}

int CBoxAudioRnd::SetVolume (int nVolume)
{
	m_nVolume = nVolume;

	if (m_pRnd != NULL)
		return m_pRnd->SetVolume (nVolume);

	return YY_ERR_STATUS;
}

int CBoxAudioRnd::GetVolume (void)
{
	if (m_pRnd != NULL)
	{
		int nVolume = m_pRnd->GetVolume ();
		if (nVolume < 0)
			return m_nVolume;
		if (abs (m_nVolume - nVolume) <= 1)
			return m_nVolume;
		else
			return nVolume;
	}

	return m_nVolume;
}

CBaseClock * CBoxAudioRnd::GetClock (void)
{
	if (m_pRnd != NULL)
		return m_pRnd->GetClock ();

	return CBoxRender::GetClock ();
}

int CBoxAudioRnd::GetRndCount (void)
{
	if (m_pRnd == NULL)
		return m_nRndCount;

	return m_pRnd->GetRndCount ();
}

int CBoxAudioRnd::RenderFrame (bool bInBox, bool bWait)
{
	CAutoLock lock (&m_mtRnd);

	if (bInBox)
		m_bInRender = true;

	if (m_pBoxSource == NULL)
	{
		yySleep (10000);
		return YY_ERR_STATUS;
	}

	m_pBaseBuffer->nType = YY_MEDIA_Audio;
	m_pBaseBuffer->uFlag = 0;
	m_pBaseBuffer->llTime = 0;
	m_pBaseBuffer->pBuff = NULL;

	int nRC = m_pBoxSource->ReadBuffer (m_pBaseBuffer, false);
	if (nRC == YY_ERR_BUFFERING)
	{
		if (m_pClock != NULL && !m_pClock->IsPaused ())
			m_pClock->Pause ();
		yySleep (2000);
		return YY_ERR_RETRY;
	}

	YY_BUFFER * pBuffer = m_pBaseBuffer;
	BOX_READ_BUFFER_REC_AUDIORND

	if ((m_pBaseBuffer->uFlag & YYBUFF_EOS) == YYBUFF_EOS)
	{
		if (m_bEOS)
		{
			yySleep (10000);
		}
		else
		{
			m_bEOS = true;
			if (m_pDataCB != NULL && m_pDataCB->funcCB != NULL)
				nRC = m_pDataCB->funcCB (m_pDataCB->userData, m_pBaseBuffer);

			if (m_nRndCount == 0)
			{
				if (m_pClock != NULL)
					m_pClock->SetTime (m_llSeekPos);
			}
			if (m_pNotifyFunc != NULL)
				m_pNotifyFunc (m_pUserData, YY_EV_Play_Complete, this);
		}
	}
	if (nRC == YY_ERR_RETRY || nRC == YY_ERR_FINISH)
	{
		if (nRC == YY_ERR_RETRY)
		{
			if (m_nSeekMode == YY_SEEK_AnyPosition)
			{
				if (m_pBaseBuffer->llTime < m_llSeekPos)
					return nRC;
			}
			if (m_nRndCount > 0)
				yySleep (2000);
		}
		return nRC;
	}

	if (m_nSeekMode == YY_SEEK_AnyPosition)
	{
		if (m_pBaseBuffer->llTime < m_llSeekPos)
			return YY_ERR_NONE;
	}

	if (!m_bEOS && nRC == YY_ERR_NONE)
	{
		if (m_status == YYRND_RUN)
		{
			if (m_pClock != NULL && m_pClock->IsPaused ())
				m_pClock->Start ();
		}

		bool	bNewFormat = false;
		if ((m_pBaseBuffer->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
		{
			bNewFormat = true;
			if (m_nRndCount > 0)
			{
				YY_AUDIO_FORMAT *	pFmt = (YY_AUDIO_FORMAT *)m_pBaseBuffer->pFormat;
				if (m_fmtAudioNew.nSampleRate != pFmt->nSampleRate || m_fmtAudioNew.nChannels != pFmt->nChannels || m_fmtAudioNew.nBits != pFmt->nBits)
					m_nNewFmtCount = 0;
				yyDataCloneAudioFormat (&m_fmtAudioNew, pFmt);
				if (m_nNewFmtCount <= 4)
				{
					m_nNewFmtCount++;
					return YY_ERR_NONE;
				}
			}
		}
		if (m_nNewFmtCount > 0)
		{
			bNewFormat = true;
			m_nNewFmtCount = 0;
		}
		if (bNewFormat)
		{
			YY_AUDIO_FORMAT *	pFmt = (YY_AUDIO_FORMAT *)m_pBaseBuffer->pFormat;
			if (pFmt == NULL)
				pFmt = &m_fmtAudioNew;
			yyDataCloneAudioFormat (&m_fmtAudio, pFmt);
			if (m_pNotifyFunc != NULL)
				m_pNotifyFunc (m_pUserData, YY_EV_Audio_Changed, 0);
			if (m_pRnd != NULL)
			{
				nRC = m_pRnd->Init (pFmt);
				m_pRnd->Start ();
			}
		}


		if (m_pRnd != NULL)
		{
			if ((m_pBaseBuffer->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
					m_pRnd->Flush ();

			nRC = m_pRnd->Render (m_pBaseBuffer);
			while (nRC == YY_ERR_RETRY)
			{
				yySleep (5000);
				nRC = m_pRnd->Render (m_pBaseBuffer);
				if (m_status == YYRND_STOP)
					break;
			}
		}
		if ((m_pDataCB != NULL && m_pDataCB->funcCB != NULL) || m_pRnd == NULL)
		{
			if (m_pDataCB != NULL && m_pDataCB->funcCB != NULL)
				nRC = m_pDataCB->funcCB (m_pDataCB->userData, m_pBaseBuffer);
			m_pClock->SetTime (m_pBaseBuffer->llTime);
		}

		m_nRndCount++;
	}

	return nRC;
}

void CBoxAudioRnd::ResetMembers (void)
{
	m_llSeekPos = 0;
	m_bEOS = true;
	m_nRndCount = 0;
}
