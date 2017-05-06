/*******************************************************************************
	File:		CBoxExtData.cpp

	Contains:	source box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#include "CBoxExtData.h"

#include "CBoxMonitor.h"

#include <libavcodec/avcodec.h>
#include "UYYDataFunc.h"
#include "yyLog.h"

CBoxExtData::CBoxExtData(void * hInst)
	: CBoxSource (hInst)
{
	SetObjectName ("CBoxExtData");
	m_nBoxType = OMB_TYPE_SOURCE;
	strcpy (m_szBoxName, "Ext Data Box");

	memset (&m_extData, 0, sizeof (m_extData));

	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));
}

CBoxExtData::~CBoxExtData(void)
{
}

int CBoxExtData::OpenSource (const TCHAR * pSource, int nFlag)
{
	m_nAudioStreamNum = 1;
	m_nVideoStreamNum = 1;
	if ((nFlag & YY_OPEN_SRC_VIDEO) == YY_OPEN_SRC_VIDEO)
		m_nAudioStreamNum = 0;
	if ((nFlag & YY_OPEN_SRC_AUDIO) == YY_OPEN_SRC_AUDIO)
		m_nVideoStreamNum = 0;

	memcpy (&m_extData, (void *)pSource, sizeof (m_extData));
	if (m_extData.pFmtVideo != NULL)
	{
		yyDataCloneVideoFormat (&m_fmtVideo, m_extData.pFmtVideo);
	}
	else
	{
		m_fmtVideo.nSourceType = YY_SOURCE_YY;
		m_fmtVideo.nCodecID = YY_CODEC_ID_H264;
		m_fmtVideo.nWidth = 800;
		m_fmtVideo.nHeight = 480;
	}
	if (m_fmtVideo.nCodecID == YY_CODEC_ID_H264)
		m_fmtVideo.nCodecID = AV_CODEC_ID_H264;
	m_pFmtVideo = &m_fmtVideo;

	if (m_extData.pFmtAudio != NULL)
	{
		yyDataCloneAudioFormat (&m_fmtAudio, m_extData.pFmtAudio);
	}
	else
	{
		m_fmtAudio.nSourceType = YY_SOURCE_YY;
		m_fmtAudio.nCodecID = YY_CODEC_ID_AAC;
		m_fmtAudio.nSampleRate = 44100;
		m_fmtAudio.nChannels = 2;
	}
	if (m_fmtAudio.nCodecID == YY_CODEC_ID_AAC)
		m_fmtAudio.nCodecID = AV_CODEC_ID_AAC;
	m_pFmtAudio = &m_fmtAudio;

	return YY_ERR_NONE;
}

int CBoxExtData::GetDuration (void)
{
	return 0;
}

int CBoxExtData::ReadBuffer (YY_BUFFER * pBuffer, bool bWait)
{
	int nRC = YY_ERR_NONE;

	BOX_READ_BUFFER_REC_SOURCE

	nRC = m_extData.pRead (m_extData.pUser, pBuffer);

	return nRC;
}

