/*******************************************************************************
	File:		CBoxSource.cpp

	Contains:	source box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#include "CBoxSource.h"
#include "CBoxMonitor.h"

#include "CFFMpegSource.h"
#include "CImageSource.h"

#ifndef _OS_WINCE
#include "CStreamSource.h"
#endif // _OS_WINCE

#include "UFileFormat.h"
#include "yyLog.h"

CBoxSource::CBoxSource(void * hInst)
	: CBoxBase (hInst)
	, m_pMediaSource (NULL)
	, m_bEnableSubTT (false)
{
	SetObjectName ("CBoxSource");
	m_nBoxType = OMB_TYPE_SOURCE;
	strcpy (m_szBoxName, "Source Box");
}

CBoxSource::~CBoxSource(void)
{
	YY_DEL_P (m_pMediaSource);
}

int CBoxSource::OpenSource (const TCHAR * pSource, int nFlag)
{
	Stop ();

	YY_DEL_P (m_pMediaSource);

#ifndef _OS_WINCE
	if (yyffIsStreaming (pSource, nFlag))
	{
		YYLOGI ("It is HLS source");
		m_pMediaSource = new CStreamSource (m_hInst);
	}
	else
#endif // _OS_WINCE
	{
		if (yyffGetType (pSource, nFlag) == YY_MEDIA_Image)
			m_pMediaSource = new CImageSource (m_hInst);
		else
			m_pMediaSource = new CFFMpegSource (m_hInst);
	}
	if (m_pMediaSource == NULL)
		return YY_ERR_MEMORY;

	m_pMediaSource->EnableSubTT (m_bEnableSubTT);
	int nRC = m_pMediaSource->Open (pSource, nFlag);
	if (nRC != YY_ERR_NONE)
		return nRC;

	m_nAudioStreamNum = m_pMediaSource->GetStreamCount (YY_MEDIA_Audio);
	m_nVideoStreamNum = m_pMediaSource->GetStreamCount (YY_MEDIA_Video);

	m_pFmtAudio = m_pMediaSource->GetAudioFormat ();
	m_pFmtVideo = m_pMediaSource->GetVideoFormat ();

	return YY_ERR_NONE;
}

int CBoxSource::Close (void)
{
	if (m_pMediaSource == NULL)
		return YY_ERR_STATUS;

	m_pMediaSource->ForceClose ();

	return YY_ERR_NONE;
}

char * CBoxSource::GetSourceName (void)
{
	if (m_pMediaSource == NULL)
		return NULL;

	return m_pMediaSource->GetSourceName ();
}

int CBoxSource::GetStreamPlay (YYMediaType nType)
{
	if (m_pMediaSource == NULL)
		return YY_ERR_STATUS;

	return m_pMediaSource->GetStreamPlay (nType);
}

int CBoxSource::SetStreamPlay (YYMediaType nType, int nIndex)
{
	if (m_pMediaSource == NULL)
		return YY_ERR_STATUS;

	return m_pMediaSource->SetStreamPlay (nType, nIndex);
}

int CBoxSource::GetDuration (void)
{
	if (m_pMediaSource == NULL)
		return 0;

	return m_pMediaSource->GetDuration ();
}

int CBoxSource::GetMediaInfo (TCHAR * pInfo, int nSize)
{
	if (m_pMediaSource != NULL)
		return m_pMediaSource->GetMediaInfo (pInfo, nSize);

	return YY_ERR_STATUS;
}

int	CBoxSource::Start (void)
{
	if (m_pMediaSource != NULL)
		m_pMediaSource->Start ();

	m_nStatus = OMB_STATUS_RUN;

	return YY_ERR_NONE;
}

int CBoxSource::Pause (void)
{
	m_nStatus = OMB_STATUS_PAUSE;

	if (m_pMediaSource != NULL)
		m_pMediaSource->Pause ();

	return YY_ERR_NONE;
}

int	CBoxSource::Stop (void)
{
	m_nStatus = OMB_STATUS_STOP;

	if (m_pMediaSource != NULL)
		m_pMediaSource->Stop ();

	return YY_ERR_NONE;
}

int CBoxSource::ReadBuffer (YY_BUFFER * pBuffer, bool bWait)
{
	if (m_pMediaSource == NULL)
		return YY_ERR_SOURCE;

	int nRC = YY_ERR_NONE;

	BOX_READ_BUFFER_REC_SOURCE

	nRC = m_pMediaSource->ReadData (pBuffer);

#if 0
	if (nRC == YY_ERR_NONE)
	{
		if (pBuffer->nType == YY_MEDIA_Video)
		{
			YYLOGI ("Video Read % 8d, Step % 8d", (int)pBuffer->llTime, (int)(pBuffer->llTime - m_llDbgLastTime));
			m_llDbgLastTime = pBuffer->llTime;
		}
	}
#endif //

	return nRC;
}

int CBoxSource::SetPos (int nPos, bool bSeek)
{
	if (m_pMediaSource == NULL)
		return YY_ERR_SOURCE;

	m_llSeekPos = nPos;

	if (bSeek)
		return m_pMediaSource->SetPos (nPos);
	else
		return YY_ERR_NONE;
}

int CBoxSource::GetPos (void)
{
	if (m_pMediaSource == NULL)
		return 0;
	return m_pMediaSource->GetPos ();
}

int CBoxSource::SetParam (int nID, void * pParam)
{
	if (m_pMediaSource == NULL)
		return YY_ERR_SOURCE;

	return m_pMediaSource->SetParam (nID, pParam);
}

int CBoxSource::GetParam (int nID, void * pParam)
{
	if (m_pMediaSource == NULL)
		return YY_ERR_SOURCE;

	return m_pMediaSource->GetParam (nID, pParam);
}

