/*******************************************************************************
	File:		CBaseAudioRnd.cpp

	Contains:	The base audio render implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CBaseAudioRnd.h"

#include "yyConfig.h"
#include "yyLog.h"

CBaseAudioRnd::CBaseAudioRnd(void * hInst, bool bAudioOnly)
	: CBaseObject ()
	, m_dataConvert (hInst)
	, m_hInst (hInst)
	, m_bAudioOnly (bAudioOnly)
	, m_nType (YY_ART_WAVE_MAPPER)
	, m_pClock (NULL)
	, m_pPCMData (NULL)
	, m_nPCMSize (0)
	, m_pPCMBuff (NULL)
	, m_nPCMLen (0)
	, m_fSpeed (1.0)
	, m_llPrevTime (0)
	, m_nSizeBySec (0)
	, m_nRndCount (0)
{
	SetObjectName ("CBaseAudioRnd");

	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));

	memset (&m_buffPCM, 0, sizeof (m_buffPCM));
	m_buffPCM.nType = YY_MEDIA_Audio;
}

CBaseAudioRnd::~CBaseAudioRnd(void)
{
	YY_DEL_A (m_pPCMData);
	YY_DEL_P (m_pClock);
}

int CBaseAudioRnd::SetType (YY_PLAY_ARType nType)
{
	m_nType = nType;

	return YY_ERR_NONE;
}

int CBaseAudioRnd::Init (YY_AUDIO_FORMAT * pFmt)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseAudioRnd::Uninit (void)
{
	YY_DEL_A (m_fmtAudio.pHeadData);
	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));

	return YY_ERR_IMPLEMENT;
}

int	CBaseAudioRnd::Start (void)
{
	if (m_pClock != NULL)
		m_pClock->Start ();

	return YY_ERR_NONE;
}

int CBaseAudioRnd::Pause (void)
{
	if (m_pClock != NULL)
		m_pClock->Pause ();

	return YY_ERR_NONE;
}

int	CBaseAudioRnd::Stop (void)
{
	return YY_ERR_NONE;
}

int CBaseAudioRnd::Flush (void)
{
	return YY_ERR_NONE;
}

int CBaseAudioRnd::SetSpeed (float fSpeed)
{
	CAutoLock lock (&m_mtRnd);

	if (m_fSpeed == fSpeed)
		return YY_ERR_NONE;

	m_fSpeed = fSpeed;
	m_dataConvert.SetSpeed (m_fSpeed);

	return YY_ERR_NONE;
}

int CBaseAudioRnd::Render (YY_BUFFER * pBuff)
{
	CAutoLock lock (&m_mtRnd);

	if (m_pClock != NULL)
	{
		if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		{
			m_nRndCount = 0;
			m_pClock->SetTime (pBuff->llTime);
		}
		else if (m_nRndCount == 0)
		{
			m_pClock->SetTime (pBuff->llTime);
		}
	}

	return YY_ERR_NONE;
}

int CBaseAudioRnd::SetVolume (int nVolume)
{
	return YY_ERR_FAILED;
}

int CBaseAudioRnd::GetVolume (void)
{
	return 0;
}

CBaseClock * CBaseAudioRnd::GetClock (void)
{
	if (m_pClock == NULL)
		m_pClock = new CBaseClock ();

	return m_pClock;
}

int CBaseAudioRnd::GetRenderBuff (YY_BUFFER * pBuff, unsigned char ** ppOutBuff)
{
	CAutoLock lock (&m_mtRnd);
	if (pBuff == NULL)
		return YY_ERR_ARG;

	unsigned char *	pData = NULL;
	int				nSize = 0;

	AVFrame *		pFrame = NULL;
	bool			bConvert = false;
	int				nInputSize = 0;

	if ((pBuff->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
	{
		pFrame = (AVFrame *)pBuff->pBuff;
		if (pFrame->format != AV_SAMPLE_FMT_S16 || pFrame->channels > 2 || m_fSpeed != 1.0)
		{
			bConvert = true;
		}
		else
		{
			pData = pFrame->data[0];
			nSize = pFrame->nb_samples * m_fmtAudio.nChannels * m_fmtAudio.nBits / 8;
		}
		nInputSize = pFrame->nb_samples * m_fmtAudio.nChannels * m_fmtAudio.nBits / 8;
	}
	else if ((pBuff->uFlag & YYBUFF_TYPE_DATA) == YYBUFF_TYPE_DATA)
	{
		pData = pBuff->pBuff;
		nSize = pBuff->uSize;

		nInputSize = nSize;

		if (m_fSpeed != 1.0)
			bConvert = true;
	}

	if (bConvert)
	{
		if (m_nPCMSize < (m_fmtAudio.nSampleRate * m_fmtAudio.nChannels * m_fmtAudio.nBits / 8) / m_fSpeed)
		{
			if (m_pPCMData != NULL)
			{
				delete []m_pPCMData;
				m_pPCMData = NULL;
			}
		}

		if (m_pPCMData == NULL)
		{
			m_nPCMSize = (m_fmtAudio.nSampleRate * m_fmtAudio.nChannels * m_fmtAudio.nBits / 8) / m_fSpeed;
			m_nPCMSize = (m_nPCMSize + 3) & ~3;
			if (m_nPCMSize < m_fmtAudio.nSampleRate * 2)
				m_nPCMSize = m_fmtAudio.nSampleRate * 2;
			if (m_nPCMSize < nInputSize)
				m_nPCMSize = nInputSize;
			m_pPCMData = new unsigned char[m_nPCMSize];
		}
		m_pPCMBuff = m_pPCMData;

		if (m_bAudioOnly || m_fSpeed <= YYCFG_DISABLE_AUDIO_SPEED)
		{
			m_buffPCM.uFlag = YYBUFF_TYPE_PPOINTER;
			m_buffPCM.pFormat = &m_fmtAudio;
			m_buffPCM.pBuff = (unsigned char *)&m_pPCMData;
			m_buffPCM.uSize = m_nPCMSize;

			m_dataConvert.Convert (pBuff, &m_buffPCM, NULL);

			m_nPCMLen = m_buffPCM.uSize;
		}
		else
		{
			if (m_nSizeBySec == 0)
				m_nSizeBySec = m_fmtAudio.nSampleRate * m_fmtAudio.nChannels * m_fmtAudio.nBits / 8;
			if (pBuff->llTime - m_llPrevTime <= 0)
				m_nPCMLen = 4096;
			else
				m_nPCMLen = (pBuff->llTime - m_llPrevTime) * m_nSizeBySec / (1000 * m_fSpeed);
			
			if (m_nPCMLen >= m_nPCMSize)
				m_nPCMLen = 4096;

			m_nPCMLen = m_nPCMLen & 0XFFFFFFFC;

			memset (m_pPCMData, 0, m_nPCMLen);
		}

		pData = m_pPCMBuff;
		nSize = m_nPCMLen;
	}

	m_llPrevTime = pBuff->llTime;
	*ppOutBuff = pData;

	return nSize;
}
