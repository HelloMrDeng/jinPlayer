/*******************************************************************************
	File:		CBoxVideoRndExt.cpp

	Contains:	The video render box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#include "CBoxVideoRndExt.h"

#include "CBoxMonitor.h"

#include "USystemFunc.h"
#include "UYYDataFunc.h"

#include "yyConfig.h"
#include "yyMediaPlayer.h"
#include "yyLog.h"

//#define YY_EXTRND_MULTI_THREAD

CBoxVideoRndExt::CBoxVideoRndExt(void * hInst)
	: CBoxVideoRnd (hInst)
{
	SetObjectName ("CBoxVideoRndExt");
	strcpy (m_szBoxName, "Video Render Box Ext");

	m_nBoxType = OMB_TYPE_RND_EXT;

#ifdef YY_EXTRND_MULTI_THREAD
	YY_BUFFER * pBuff = NULL;
	for (int i = 0; i < 2; i++)
	{
		pBuff = new YY_BUFFER ();
		memset (pBuff, 0, sizeof (YY_BUFFER));
		pBuff->pData = new AVFrame ();
		m_lstFree.AddTail (pBuff);
	}
#endif // YY_EXTRND_MULTI_THREAD	
}

CBoxVideoRndExt::~CBoxVideoRndExt(void)
{
#ifdef YY_EXTRND_MULTI_THREAD	
	YY_VIDEO_BUFF * pVideo = NULL;
	YY_BUFFER * pBuff = m_lstFree.RemoveHead ();
	while (pBuff != NULL)
	{
		if (pBuff->pData != NULL)
		{
			AVFrame * pFrame = (AVFrame *)pBuff->pData;
			delete pFrame;
		}

		delete pBuff;
		pBuff = m_lstFree.RemoveHead ();
	}
	pBuff = m_lstFull.RemoveHead ();
	while (pBuff != NULL)
	{
		if (pBuff->pData != NULL)
		{
			AVFrame * pFrame = (AVFrame *)pBuff->pData;
			delete pFrame;
		}

		delete pBuff;
		pBuff = m_lstFull.RemoveHead ();
	}
#endif // YY_EXTRND_MULTI_THREAD	
}

int CBoxVideoRndExt::SetRndType (YYRND_TYPE nRndType)
{
	return YY_ERR_NONE;
}

int CBoxVideoRndExt::SetDisplay (void * hView, RECT * pRect)
{
	return YY_ERR_NONE;
}

int CBoxVideoRndExt::UpdateDisp (void)
{
	return YY_ERR_NONE;
}

int CBoxVideoRndExt::SetAspectRatio (int w, int h)
{
	return YY_ERR_NONE;
}

int CBoxVideoRndExt::SetDDMode (YY_PLAY_DDMode nMode)
{
	return YY_ERR_NONE;
}

int CBoxVideoRndExt::DisableVideo (int nFlag)
{
	CAutoLock lock (&m_mtRnd);

	if (m_nDisableFlag == nFlag)
		return YY_ERR_NONE;

	m_nDisableFlag = nFlag;

	return YY_ERR_NONE;
}

int CBoxVideoRndExt::SetSource (CBoxBase * pSource)
{
	CAutoLock lock (&m_mtRnd);
	if (pSource == NULL)
	{
		m_pBoxSource = NULL;
		ResetMembers ();
		return YY_ERR_ARG;
	}

	Stop ();

	CBoxBase::SetSource (pSource);

	YY_VIDEO_FORMAT * pFmt = pSource->GetVideoFormat ();
	if (pFmt == NULL)
		return YY_ERR_VIDEO;
	yyDataCloneVideoFormat (&m_fmtVideo, pFmt);

	return YY_ERR_NONE;
}

int CBoxVideoRndExt::ReadBuffer (YY_BUFFER * pBuffer, bool bWait)
{
	if (m_status != YYRND_RUN && m_status != YYRND_PAUSE)
	{	
		yySleep (2000);
		return YY_ERR_STATUS;
	}

#ifdef YY_EXTRND_MULTI_THREAD	
	if (m_lstFull.GetCount () <= 0)
	{
		yySleep (2000);
		return YY_ERR_RETRY;
	}

	CAutoLock lock (&m_mtList);
	YY_BUFFER * pVideoBuff = m_lstFull.RemoveHead ();
	memcpy (pBuffer, pVideoBuff, sizeof (YY_BUFFER));
	m_lstFree.AddTail (pVideoBuff);

	WaitRenderTime (pBuffer);

	return YY_ERR_NONE;
#else
	int nRC = CBoxVideoRnd::RenderFrame (false, bWait);
	if (nRC == YY_ERR_NONE)
	{
		memcpy (pBuffer, m_pBaseBuffer, sizeof (YY_BUFFER));

		if (m_pClock != NULL)
		{
			if ((m_pBaseBuffer->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
			{
				m_nRndCount = 0;
				m_pClock->SetTime (m_pBaseBuffer->llTime);
			}
			else if (m_nRndCount == 0)
			{
				m_pClock->SetTime (m_pBaseBuffer->llTime);
			}

			pBuffer->llDelay = m_pClock->GetTime () - pBuffer->llTime;
		}
	}

	return nRC;
#endif // YY_EXTRND_MULTI_THREAD
}

int CBoxVideoRndExt::RenderFrame (bool bInBox, bool bWait)
{
#ifdef YY_EXTRND_MULTI_THREAD
	int nRC = CBoxVideoRnd::RenderFrame (bInBox, bWait);
	if (nRC != YY_ERR_NONE)
		return nRC;

	while (m_lstFree.GetCount () <= 0)
	{
		yySleep (2000);
		if (m_status != YYRND_RUN)
			return YY_ERR_STATUS;
	}

	CAutoLock lock (&m_mtList);
	YY_BUFFER * pVideoBuff = m_lstFree.RemoveHead ();

	void * pPrivData = pVideoBuff->pData;

	memcpy (pVideoBuff, m_pBaseBuffer, sizeof (YY_BUFFER));
	memcpy (pPrivData, m_pBaseBuffer->pBuff, sizeof (AVFrame));
	pVideoBuff->pBuff = (unsigned char *)pPrivData;
	pVideoBuff->pData = pPrivData;

	m_lstFull.AddTail (pVideoBuff);

	return YY_ERR_NONE;
	
#else

	return CBoxVideoRnd::RenderFrame (bInBox, bWait);
		
#endif // YY_EXTRND_MULTI_THREAD	
}

int	CBoxVideoRndExt::Start (CThreadWork * pWork)
{
#ifdef YY_EXTRND_MULTI_THREAD
	int nRC = CBoxVideoRnd::Start ();
#else
	int nRC = CBoxBase::Start ();
	m_status = YYRND_RUN;
	m_bEOS = false;	
#endif // YY_EXTRND_MULTI_THREAD

	if (m_pDataClock != NULL)
		m_pDataClock->Start ();

	return nRC;
}

int CBoxVideoRndExt::Pause (void)
{
#ifdef YY_EXTRND_MULTI_THREAD
	int nRC = CBoxVideoRnd::Pause ();
#else
	int nRC = CBoxBase::Pause ();
	m_status = YYRND_PAUSE;
#endif // YY_EXTRND_MULTI_THREAD

	if (m_pDataClock != NULL)
		m_pDataClock->Pause ();

	return nRC;
}

int	CBoxVideoRndExt::Stop (void)
{
#ifdef _YY_PLAYER
	if (m_status != YYRND_STOP && m_nRndCount > 0)
	{
		int nUsedTime = yyGetSysTime ()- m_nDbgStartTime;
		if (nUsedTime > 0)
		{
			YYLOGI ("Status: % 8d : % 8d : % 8d", m_nRndCount, nUsedTime, (m_nRndCount * 100000) / nUsedTime);
		}
	}
#endif // _YY_PLAYER

	if (m_pDataCnvt != NULL)
		delete m_pDataCnvt;
	m_pDataCnvt = NULL;

#ifdef YY_EXTRND_MULTI_THREAD
	int nRC = CBoxVideoRnd::Pause ();
#else
	int nRC = CBoxBase::Pause ();
	m_status = YYRND_STOP;
#endif // YY_EXTRND_MULTI_THREAD

	return nRC;
}

CBaseClock * CBoxVideoRndExt::GetClock (void)
{
	return CBoxRender::GetClock ();
}