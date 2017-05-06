/*******************************************************************************
	File:		CBoxAudioRndExt.cpp

	Contains:	The audio render box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#include "CBoxAudioRndExt.h"
#include "CBoxMonitor.h"

#include "USystemFunc.h"
#include "yyMediaPlayer.h"
#include "UYYDataFunc.h"

#include "yyLog.h"

CBoxAudioRndExt::CBoxAudioRndExt(void * hInst)
	: CBoxAudioRnd (hInst)
	, m_llDbgPrevTime (0)
{
	SetObjectName ("CBoxAudioRndExt");
	strcpy (m_szBoxName, "Audio Render Box Ext");

	m_nBoxType = OMB_TYPE_RND_EXT;
}

CBoxAudioRndExt::~CBoxAudioRndExt(void)
{
}

int CBoxAudioRndExt::SetAudioRndType (YY_PLAY_ARType nType)
{
	return YY_ERR_NONE;
}

int CBoxAudioRndExt::SetSource (CBoxBase * pSource)
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

	return YY_ERR_NONE;
}

int CBoxAudioRndExt::ReadBuffer (YY_BUFFER * pBuffer, bool bWait)
{
	CAutoLock lock (&m_mtRnd);

	if (m_status != YYRND_RUN && m_status != YYRND_PAUSE)
	{	
		yySleep (2000);
		return YY_ERR_STATUS;
	}
		
	if (pBuffer == NULL)
		return YY_ERR_ARG;

//	YYLOGI ("Buffer Time: % 8d    % 4d    % 6d", (int)pBuffer->llTime, (int)(pBuffer->llTime - m_llDbgPrevTime), pBuffer->uSize);
//	m_llDbgPrevTime = pBuffer->llTime;

	if (m_nRndCount > 0 && m_pDataClock != NULL)
	{
		if (pBuffer->llTime > 0 && !m_bEOS)
			m_pDataClock->SetTime (pBuffer->llTime);
	}

	int nRC = CBoxAudioRnd::RenderFrame (false, false);
	if (nRC != YY_ERR_NONE)
		return nRC;

	memcpy (pBuffer, m_pBaseBuffer, sizeof (YY_BUFFER));

	return YY_ERR_NONE;
}

int CBoxAudioRndExt::Convert (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff, RECT * pZoom)
{
	CAutoLock lock (&m_mtRnd);

	if (m_pDataCnvt == NULL)
	{
		m_pDataCnvt = new CDataConvert (m_hInst);
		m_pDataCnvt->SetSpeed (m_fSpeed);
	}

	return m_pDataCnvt->Convert (pInBuff, pOutBuff, pZoom);
}

int	CBoxAudioRndExt::Start (CThreadWork * pWork)
{
	CBoxBase::Start ();

	m_bEOS = false;
	m_status = YYRND_RUN;

	if (m_pDataClock != NULL)
		m_pDataClock->Start ();

	return YY_ERR_NONE;
}

int CBoxAudioRndExt::Pause (void)
{
	CBoxBase::Pause ();

	m_status = YYRND_PAUSE;
	if (m_pDataClock != NULL)
		m_pDataClock->Pause ();

	return YY_ERR_NONE;
}

int	CBoxAudioRndExt::Stop (void)
{
	CBoxBase::Stop ();

	m_status = YYRND_STOP;

	return YY_ERR_NONE;
}

int CBoxAudioRndExt::SetSpeed (float fSpeed)
{
	CAutoLock lock (&m_mtRnd);
	m_fSpeed = fSpeed;
	if (m_pDataCnvt != NULL)
		m_pDataCnvt->SetSpeed (m_fSpeed);
	return YY_ERR_STATUS;
}

CBaseClock * CBoxAudioRndExt::GetClock (void)
{
	return CBoxRender::GetClock ();
}
