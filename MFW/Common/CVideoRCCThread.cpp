/*******************************************************************************
	File:		CVideoRCCThread.cpp

	Contains:	The thread video color convert and resize implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CVideoRCCThread.h"

#include "USystemFunc.h"
#include "yyLog.h"

CVideoRCCThread::CVideoRCCThread(void * hInst, int nID)
	: CFFMpegVideoRCC (hInst)
	, m_nID (nID)
	, m_pInBuff (NULL)
	, m_pOutBuff (NULL)
	, m_bFinished (true)
	, m_pThdSem (NULL)
	, m_hThread (NULL)
	, m_bStop (false)
	, m_bWait (false)
{	
	SetObjectName ("CVideoRCCThread");

	m_pThdSem = new CThreadSem ();

	int nThreadID = 0;
	yyThreadCreate (&m_hThread, &nThreadID, RCCProc, this, 0);
}

CVideoRCCThread::~CVideoRCCThread(void)
{	
	m_bStop = true;
	if (m_bWait)
		m_pThdSem->Broadcast ();

	while (m_hThread != NULL)
	{
		yySleep (10000);
	}

	YY_DEL_P (m_pThdSem);
}

int CVideoRCCThread::ConvertBuff (YY_BUFFER * iBuff, YY_VIDEO_BUFF * oBuff)
{
	m_pInBuff = iBuff;
	m_pOutBuff = oBuff;
	m_bFinished = false;

//	if (m_nID == 1)
//		YYLOGI ("Singal");
	
//	m_pThdSem->Signal ();
	
	return YY_ERR_NONE;
}

bool CVideoRCCThread::Finished (void) 
{
	if (m_bFinished && m_bWait)
		return true;
		
	return false;	
}

int CVideoRCCThread::RCCProc (void * pParam)
{	
	CVideoRCCThread * pRCC = (CVideoRCCThread *)pParam;

	pRCC->RCCLoop ();

	return YY_ERR_NONE;
}

int CVideoRCCThread::RCCLoop (void)
{
//	YYLOGI ("Thread Entry!");
	
	while (!m_bStop)
	{
		m_bWait = true;
		if (m_pOutBuff == NULL)
		{
			yySleep (2000);
			continue;
		}
//		m_pThdSem->Wait ();
		m_bWait = false;
				
//		if (m_nID == 1)
//			YYLOGI ("Convert! ******");
		
		CFFMpegVideoRCC::ConvertBuff (m_pInBuff, m_pOutBuff);
		
		m_pInBuff = NULL;
		m_pOutBuff = NULL;

		m_bFinished = true;
	}

	m_hThread = NULL;

	return YY_ERR_NONE;
}
