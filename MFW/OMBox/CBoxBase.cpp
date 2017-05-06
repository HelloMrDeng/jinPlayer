/*******************************************************************************
	File:		CBoxBase.cpp

	Contains:	base box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#include "CBoxBase.h"

#include "yyMediaPlayer.h"

CBoxBase::CBoxBase(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_nBoxType (OMB_TYPE_BASE)
	, m_nStatus (OMB_STATUS_INIT)
	, m_pNotifyFunc (NULL)
	, m_pUserData (NULL)
	, m_pClock (NULL)
	, m_hView (NULL)
	, m_nDecMode (0)
	, m_pBoxSource (NULL)
	, m_nAudioStreamNum (0)
	, m_nVideoStreamNum (0)
	, m_nAudioStreamPlay (-1)
	, m_nVideoStreamPlay (-1)
	, m_pFmtAudio (NULL)
	, m_pFmtVideo (NULL)
	, m_pBaseBuffer (NULL)
	, m_pCurrBuffer (NULL)
	, m_llSeekPos (0)
	, m_nSeekMode (YY_SEEK_KeyFrame)
	, m_llDbgLastTime (0)
{
	SetObjectName ("CBoxBase");
	strcpy (m_szBoxName, "Base Box");

	m_pBaseBuffer = new YY_BUFFER ();
	memset (m_pBaseBuffer, 0, sizeof (YY_BUFFER));
}

CBoxBase::~CBoxBase(void)
{
	YY_DEL_P (m_pBaseBuffer);
}

int CBoxBase::SetDisplay (void * hView, RECT * pRect)
{
	m_hView = hView;
	memset (&m_rcView, 0, sizeof (RECT));
	if (pRect != NULL)
		memcpy (&m_rcView, pRect, sizeof (RECT));
	return YY_ERR_NONE;
}

int CBoxBase::SetDecMode (int nMode)
{
	m_nDecMode = nMode;
	return YY_ERR_NONE;
}

void CBoxBase::SetNotifyFunc (YYMediaNotifyEvent pFunc, void * pUserData)
{
	m_pNotifyFunc = pFunc;
	m_pUserData = pUserData;
}

int CBoxBase::SetSource (CBoxBase * pSource)
{
	m_pBoxSource = pSource;

	return YY_ERR_NONE;
}

int CBoxBase::GetStreamCount (YYMediaType nType)
{
	if (nType == YY_MEDIA_Video)
		return m_nVideoStreamNum;
	else if (nType == YY_MEDIA_Audio)
		return m_nAudioStreamNum;
	else
		return 0;
}

int CBoxBase::GetStreamPlay (YYMediaType nType)
{
	if (nType == YY_MEDIA_Video)
		return m_nVideoStreamPlay;
	else if (nType == YY_MEDIA_Audio)
		return m_nAudioStreamPlay;
	else
		return -1;
}

int CBoxBase::SetStreamPlay (YYMediaType nType, int nIndex)
{
	return YY_ERR_STATUS;
}

int CBoxBase::GetDuration (void)
{
	if (m_pBoxSource == NULL)
		return 0;

	return m_pBoxSource->GetDuration ();
}

int	CBoxBase::Start (void)
{
	m_nStatus = OMB_STATUS_RUN;

	if (m_pBoxSource != NULL)
		m_pBoxSource->Start ();

	return YY_ERR_NONE;
}

int CBoxBase::Pause (void)
{
	m_nStatus = OMB_STATUS_PAUSE;

	if (m_pBoxSource != NULL)
		m_pBoxSource->Pause ();

	return YY_ERR_NONE;
}

int	CBoxBase::Stop (void)
{
	m_nStatus = OMB_STATUS_STOP;

	if (m_pBoxSource != NULL)
		m_pBoxSource->Stop ();

	return YY_ERR_NONE;
}

int CBoxBase::ReadBuffer (YY_BUFFER * pBuffer, bool bWait)
{
	return YY_ERR_SOURCE;
}

int CBoxBase::RendBuffer (YY_BUFFER * pBuffer, bool bRend)
{
	return YY_ERR_FAILED;
}

int CBoxBase::Convert (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff, RECT * pZoom)
{
	return YY_ERR_IMPLEMENT;
}

int CBoxBase::SetPos (int nPos, bool bSeek)
{
	m_llSeekPos = nPos;

	if (m_pBoxSource != NULL)
		m_pBoxSource->SetPos (nPos, bSeek);

	return YY_ERR_NONE;
}

int CBoxBase::SetSeekMode (int nSeekMode)
{
	m_nSeekMode = nSeekMode;

	if (m_pBoxSource != NULL)
		m_pBoxSource->SetSeekMode (nSeekMode);

	return YY_ERR_NONE;
}

void CBoxBase::SetClock (CBaseClock * pClock)
{
	m_pClock = pClock;
}

CBaseClock * CBoxBase::GetClock (void)
{
	return NULL;
}

int CBoxBase::SetParam (int nID, void * pParam)
{
	return YY_ERR_PARAMID;
}

int CBoxBase::GetParam (int nID, void * pParam)
{
	return YY_ERR_PARAMID;
}

YY_AUDIO_FORMAT * CBoxBase::GetAudioFormat (void) 
{
	if (m_pFmtAudio == NULL && m_pBoxSource != NULL)
		return m_pBoxSource->GetAudioFormat ();
	return m_pFmtAudio;
}

YY_VIDEO_FORMAT * CBoxBase::GetVideoFormat (void) 
{
	if (m_pFmtVideo == NULL && m_pBoxSource != NULL)
		return m_pBoxSource->GetVideoFormat ();
	return m_pFmtVideo;
}
