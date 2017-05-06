/*******************************************************************************
	File:		CBoxRender.cpp

	Contains:	The base render box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#include "CBoxRender.h"

#include "USystemFunc.h"
#include "yyLog.h"

CBoxRender::CBoxRender(void * hInst)
	: CBoxBase (hInst)
	, m_nMediaType (YY_MEDIA_Data)
	, m_status (YYRND_INIT)
	, m_pOtherRnd (NULL)
	, m_pDataCB (NULL)
	, m_bEOS (true)
	, m_nRndCount (0)
	, m_pWorkThread (NULL)
	, m_hRndThread (NULL)
	, m_bInRender (false)
	, m_bSetThreadPriority (false)
	, m_pDataClock (NULL)
	, m_pDataCnvt (NULL)
{
	SetObjectName ("CBoxRender");
	m_nBoxType = OMB_TYPE_RENDER;
	strcpy (m_szBoxName, "Base Render Box");
}

CBoxRender::~CBoxRender(void)
{
	YY_DEL_P (m_pDataCnvt);
	YY_DEL_P (m_pDataClock);
}

int CBoxRender::SetOtherRender (CBoxRender * pRnd)
{
	m_pOtherRnd = pRnd;

	return YY_ERR_NONE;
}

int	CBoxRender::Start (CThreadWork * pWork)
{
	if (m_pBoxSource == NULL)
	{
		m_bEOS = true;
		return YY_ERR_NONE;
	}

	int nRC = CBoxBase::Start ();

	m_pWorkThread = pWork;
	m_bEOS = false;
	if (m_status == YYRND_PAUSE)
	{
		m_status = YYRND_RUN;
	}
	else
	{
		m_status = YYRND_RUN;
		if (m_pWorkThread == NULL)
		{
			if (m_hRndThread == NULL)
			{
				int nID = 0;
				yyThreadCreate (&m_hRndThread, &nID, RenderProc, this, 0);
			}
		}
	}

	if (m_pDataClock != NULL)
		m_pDataClock->Start ();

	return nRC;
}

int CBoxRender::Pause (void)
{
	if (m_pBoxSource == NULL)
		return YY_ERR_NONE;

	m_status = YYRND_PAUSE;
	if (m_pDataClock != NULL)
		m_pDataClock->Pause ();

	WaitForExitRender (500);

	return CBoxBase::Pause ();
}

int	CBoxRender::Stop (void)
{
	if (m_pBoxSource == NULL)
		return YY_ERR_NONE;

	m_status = YYRND_STOP;

	WaitForExitRender (5000);

//	CAutoLock lock (&m_mtRnd);
	int nTryTimes = 0;
	while (m_hRndThread != NULL)
	{
		nTryTimes++;
		if (nTryTimes > 100)
		{
			YYLOGW ("It didn't exit render when stop!");
			break;
		}
		yySleep (10000);
	}

	if (m_hRndThread != NULL)
	{
		yyThreadClose (m_hRndThread, 0);
		m_hRndThread = NULL;
	}

	return CBoxBase::Stop ();
}

int CBoxRender::SetPos (int nPos, bool bSeek)
{
	int nRC = YY_ERR_NONE;
	nRC = CBoxBase::SetPos (nPos, bSeek);
	if (nRC == YY_ERR_NONE)
	{
		m_bEOS = false;
		m_nRndCount = 0;
	}

	return nRC;
}

int CBoxRender::SetDataCB (YY_DATACB * pCB)
{
	m_pDataCB = pCB;

	return YY_ERR_NONE;
}

CBaseClock * CBoxRender::GetClock (void)
{
	if (m_pBoxSource == NULL)
		return NULL;

	if (m_pDataClock == NULL)
		m_pDataClock = new CBaseClock ();

	return m_pDataClock;
}

bool CBoxRender::IsEOS (void)
{
	if (m_pBoxSource == NULL)
		return true;

	return m_bEOS;
}

int CBoxRender::WaitRenderTime (YY_BUFFER * pBuff)
{
	return YY_ERR_NONE;
}

void CBoxRender::WaitForExitRender (unsigned int nMaxWaitTime)
{
	unsigned int	nStartTime = yyGetSysTime();
	while (m_bInRender)
	{
		yySleep (1000);

		if (yyGetSysTime () - nStartTime >= nMaxWaitTime)
		{
			YYLOGW ("It exited without stop!");
			break;
		}
	}
}

int CBoxRender::RenderProc (void * pParam)
{
	CBoxRender * pRender = (CBoxRender *) pParam;
	return pRender->RenderLoop ();
}

int CBoxRender::RenderLoop (void)
{
	if (m_nMediaType == YY_MEDIA_Audio)
		yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_ABOVE_NORMAL);
	else if (m_nMediaType == YY_MEDIA_Video)
		yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_NORMAL);

	while (m_status == YYRND_RUN || m_status == YYRND_PAUSE)
	{
		if (m_status != YYRND_RUN)
		{
			yySleep (5000);
			continue;
		}

		RenderFrame (true, true);

		m_bInRender = false;
	}

	if (m_pDataCB != NULL && m_pDataCB->funcCB != NULL)
	{
		if (m_pBaseBuffer != NULL)
		{
			m_pBaseBuffer->uFlag = YYBUFF_EOS;
			m_pDataCB->funcCB (m_pDataCB->userData, m_pBaseBuffer);
		}
	}
	m_hRndThread = NULL;

	return 0;
}

int CBoxRender::RenderFrame (bool bInBox, bool bWait)
{
	if (bInBox)
		m_bInRender = true;
	
	yySleep (10000);

	return 0;
}

int CBoxRender::Convert (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff, RECT * pZoom)
{
	if (m_pDataCnvt == NULL)
		m_pDataCnvt = new CDataConvert (m_hInst);

	return m_pDataCnvt->Convert (pInBuff, pOutBuff, pZoom);
}
