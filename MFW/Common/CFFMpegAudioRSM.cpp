/*******************************************************************************
	File:		CFFMpegAudioRSM.cpp

	Contains:	The ffmpeg audio dec implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CFFMpegAudioRSM.h"

#include "yyConfig.h"
#include "yyLog.h"

CFFMpegAudioRSM::CFFMpegAudioRSM(void * hInst)
	: CBaseObject ()
	, m_pSwrCtx (NULL)
	, m_oLayout (0)
	, m_oFormat (0)
	, m_oSampleRate (0)
	, m_iLayout (0)
	, m_iFormat (0)
	, m_iSampleRate (0)
{
	SetObjectName ("CFFAudioRSM");
}

CFFMpegAudioRSM::~CFFMpegAudioRSM(void)
{
	Uninit ();
}

int CFFMpegAudioRSM::Init (int oLayout, int oFormat, int oSampleRate, 
						   int iLayout, int iFormat, int iSampleRate)
{
	if (m_oLayout == oLayout && m_oFormat == oFormat && m_oSampleRate == oSampleRate &&
		m_iLayout == iLayout && m_iFormat == iFormat && m_iSampleRate == iSampleRate)
		return YY_ERR_NONE;

	Uninit ();

	m_pSwrCtx = swr_alloc_set_opts (NULL, 
					oLayout, (AVSampleFormat)oFormat, oSampleRate,
					iLayout, (AVSampleFormat)iFormat, iSampleRate,
					0, NULL);
	if (m_pSwrCtx == NULL)
		return YY_ERR_MEMORY;

	int nRC = swr_init (m_pSwrCtx);
	if (nRC < 0)
		return YY_ERR_FAILED;

	m_oLayout = oLayout;
	m_oFormat = oFormat;
	m_oSampleRate = oSampleRate;
	m_iLayout = iLayout;
	m_iFormat = iFormat;
	m_iSampleRate = iSampleRate;

	return YY_ERR_NONE;
}

int CFFMpegAudioRSM::Uninit (void)
{
	if (m_pSwrCtx != NULL)
		swr_free (&m_pSwrCtx);
	m_pSwrCtx = NULL;

	m_oLayout = 0;
	m_oFormat = 0;
	m_oSampleRate = 0;
	m_iLayout = 0;
	m_iFormat = 0;
	m_iSampleRate = 0;

	return YY_ERR_NONE;
}

int	CFFMpegAudioRSM::ConvertData (unsigned char ** ppInBuff, int nInSamples, unsigned char ** ppOutBuff, int nOutSize)
{
	if (m_pSwrCtx == NULL)
		return YY_ERR_STATUS;

	int nSamples = swr_convert (m_pSwrCtx, (uint8_t **)ppOutBuff, nOutSize, 
											(const uint8_t **)ppInBuff, nInSamples);

	return nSamples;
}

