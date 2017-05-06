/*******************************************************************************
	File:		CBoxVDBASmt.cpp

	Contains:	The video dec box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-09-19		Fenger			Create file

*******************************************************************************/
#include "CBoxVDBASmt.h"
#include "CFFMpegVideoDec.h"
#ifdef _OS_NDK
#include "CVVVideoOMXDec.h"
#endif // _OS_NDK

#include "USystemFunc.h"
#include "UYYDataFunc.h"

#include "yyMediaPlayer.h"
#include "yyLog.h"

#define YYBAS_IN_BUFFS	50
#define YYBAS_OUT_BUFFS	12

CBoxVDBASmt::CBoxVDBASmt(void * hInst)
	: CBoxVideoDec (hInst)
	, m_pSoftDec (NULL)
	, m_bInEOS (false)
	, m_bOutEOS (false)
	, m_bThrdStop (false)
	, m_hThrdSoft (NULL)
	, m_llSoftSrc (0)
	, m_llSoftRnd (0)
	, m_hThrdHard (NULL)
	, m_bNewFormat (false)
	, m_pLastFmtBuff (NULL)
{
	SetObjectName ("CBoxVDBASmt");
	strcpy (m_szBoxName, "VDBA Smt Box");
}

CBoxVDBASmt::~CBoxVDBASmt(void)
{
	m_bThrdStop = true;
	while (m_hThrdSoft != NULL)
		yySleep (2000);

	ResetOutBuff (true);
	ResetInBuff (true);

	YY_DEL_P (m_pSoftDec);
}

int CBoxVDBASmt::ReadBuffer (YY_BUFFER * pBuffer, bool bWait)
{
#ifdef _OS_NDK
	if (m_bFF)
		return CBoxVideoDec::ReadBuffer (pBuffer, bWait);
#endif // _OS_NDK

	int nRC = YY_ERR_FAILED;
	YY_BUFFER * pInBuff = NULL;
	YY_BUFFER * pOutBuff = NULL;

	// Try to get the hard dec rest buffer first.
	if (m_hThrdHard == NULL && m_pDec != NULL)
	{
		if (m_bNewFormat)
		{
			nRC = m_pDec->GetBuff (pBuffer);
			if (nRC == YY_ERR_NONE)
			{
				m_nOutCount++;
#ifdef YYDBG_VD_OUTPUT				
				YYLOGI (" Rend Hard dec buffer! % 8d @@@@@@@@  % 8d  sys: % 6d", (int)pBuffer->llTime, (int)(pBuffer->llTime - m_llDbgRndTime), yyGetSysTime () - m_nDbgSysTime);
				m_llDbgRndTime = pBuffer->llTime;
				m_nDbgSysTime = yyGetSysTime ();
#endif // YYDBG_VD_OUTPUT
				return nRC;
			}
		}
		// Create thread to create hard dec again
		if (m_bNewFormat)
		{
			m_bNewFormat = false;
			int nID = 0;
			YYLOGI ("Create thread to create the hard dec again!");
			yyThreadCreate (&m_hThrdHard, &nID, HardDecProc, this, 0);
		}
	}

	// Get the soft dec buffer when Hard dec re-creat!
	if (m_hThrdHard != NULL)
		return GetSoftOutput (pBuffer);

	nRC = GetHardOutput (pBuffer);
	if (pBuffer->llTime < m_llSoftRnd)
	{
		if (nRC == YY_ERR_NONE)
			m_pDec->RndBuff (pBuffer, false);
		if (m_hThrdSoft != NULL)
			return GetSoftOutput (pBuffer);
	}

	if (m_hThrdSoft != NULL)
	{
		if (pBuffer->llTime >= m_llSoftSrc)
		{
			m_bThrdStop = true;
		}
	}
	m_llSoftRnd = 0;
	
	if (nRC == YY_ERR_NONE)
	{
		if ((pBuffer->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
			m_pFmtVideo = m_pDec->GetVideoFormat ();
#ifdef YYDBG_VD_OUTPUT	
		YYLOGI (" Rend Hard dec buffer! % 8d ########  % 8d  sys: % 6d", (int)pBuffer->llTime, (int)(pBuffer->llTime - m_llDbgRndTime), yyGetSysTime () - m_nDbgSysTime);
		m_llDbgRndTime = pBuffer->llTime;
		m_nDbgSysTime = yyGetSysTime ();
#endif // YYDBG_VD_OUTPUT
	}
	return nRC;
}

int CBoxVDBASmt::RendBuffer (YY_BUFFER * pBuffer, bool bRend)
{
	if (pBuffer == NULL)
		return YY_ERR_FAILED;
	if (pBuffer->uFlag & YYBUFF_DEC_SOFT)
		return YY_ERR_NONE;
	
	return CBoxVideoDec::RendBuffer (pBuffer, bRend);	
}

int CBoxVDBASmt::SetPos (int nPos, bool bSeek)
{
	m_bThrdStop = true;
	while (m_hThrdSoft != NULL)
		yySleep (2000);

	int nRC = CBoxVideoDec::SetPos (nPos, bSeek);
	ResetInBuff (false);
	ResetOutBuff (false);

	m_pLastFmtBuff = NULL;
	m_bNewFormat = false;
	m_llSoftSrc = 0;
	m_llSoftRnd = 0;

	return nRC;
}

int CBoxVDBASmt::ReadSource (YY_BUFFER * pBuffer)
{
	int nRC = YY_ERR_NONE;
	YY_BUFFER * pInBuff = NULL;

	m_pBaseBuffer->nType = YY_MEDIA_Video;
	m_pBaseBuffer->uFlag = 0;
	m_pBaseBuffer->llTime = 0;
	m_pBaseBuffer->llDelay = 0;
	if ((pBuffer->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		m_pBaseBuffer->uFlag |= YYBUFF_NEW_POS;
	nRC = m_pBoxSource->ReadBuffer (m_pBaseBuffer, false);
	if ((m_pBaseBuffer->uFlag & YYBUFF_EOS) == YYBUFF_EOS)
		m_bInEOS = true;
	if (nRC == YY_ERR_NONE)
	{
		if ((m_pBaseBuffer->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT && m_lstInFull.GetCount () > 0)
		{
			m_llSoftSrc = m_pBaseBuffer->llTime;
			m_bThrdStop = false;
			CAutoLock lock (&m_mtSoft);		
			m_lstInSoft.RemoveAll ();			
			if (m_hThrdSoft == NULL)
			{
				int nID = 0;
				YYLOGI ("Create thread to create the soft dec!");
				yyThreadCreate (&m_hThrdSoft, &nID, SoftDecProc, this, 0);
			}
		}

		CAutoLock lock (&m_mtIn);
		pInBuff = m_lstInFree.RemoveHead ();
		if (pInBuff == NULL)
		{
			pInBuff = new YY_BUFFER ();
			memset (pInBuff, 0, sizeof (YY_BUFFER));
		}
		yyDataCloneBuffer (pInBuff, m_pBaseBuffer);
		m_lstInFull.AddTail (pInBuff);
		if (m_hThrdSoft != NULL)
		{
			CAutoLock lockSoft (&m_mtSoft);
			m_lstInSoft.AddTail (pInBuff);
		}
	}
	else if (nRC == YY_ERR_FINISH)
	{
		m_bInEOS = true;
	}
	return nRC;
}

int	CBoxVDBASmt::GetSoftOutput (YY_BUFFER * pBuffer)
{
	CAutoLock lock (&m_mtOut);
	if (m_lstOutFull.GetCount () <= 0)
		return YY_ERR_RETRY;
	YY_BUFFER * pOutBuff = m_lstOutFull.RemoveHead ();
	memcpy (pBuffer, pOutBuff, sizeof (YY_BUFFER));
	pBuffer->uFlag |= YYBUFF_DEC_SOFT;
	m_lstOutFree.AddTail (pOutBuff);
	if ((pBuffer->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
		m_pFmtVideo = m_pSoftDec->GetVideoFormat ();
	m_llSoftRnd = pOutBuff->llTime;

#ifdef YYDBG_VD_OUTPUT
	YYLOGI (" Rend soft dec buffer! % 8d ********  % 8d  sys: % 6d", (int)pBuffer->llTime, (int)(pBuffer->llTime - m_llDbgRndTime), yyGetSysTime () - m_nDbgSysTime);
	m_llDbgRndTime = pBuffer->llTime;
	m_nDbgSysTime = yyGetSysTime ();
#endif // YYDBG_VD_OUTPUT
				
	return YY_ERR_NONE;
}

int CBoxVDBASmt::GetHardOutput (YY_BUFFER * pBuffer)
{
	int nRC = YY_ERR_FAILED;
	YY_BUFFER * pInBuff = NULL;
	// try to read more input buffer if not EOS and buffer is not full
	if (!m_bInEOS && m_lstInFull.GetCount () < YYBAS_IN_BUFFS)
	{
		ReadSource (pBuffer);
		ReadSource (pBuffer);
	}
	if (m_pCurrBuffer != NULL)
	{
		pInBuff = m_pCurrBuffer;
	}
	else
	{
		CAutoLock lock (&m_mtIn);
		pInBuff = m_lstInFull.GetHead ();
		if (pInBuff == NULL)
		{
			if (m_bInEOS)
			{
				pBuffer->uFlag |= YYBUFF_EOS;
				return YY_ERR_FINISH;
			}
			else
			{
				return YY_ERR_RETRY;
			}
		}
	}

	// re-create the hard dec if it reach new format buffer
	if ((pInBuff->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT && m_pLastFmtBuff != pInBuff)
	{
		pInBuff->uFlag = pInBuff->uFlag  & ~YYBUFF_NEW_FORMAT;
		if (m_pLastFmtBuff != NULL)
		{
			m_pLastFmtBuff = pInBuff;
			m_bNewFormat = true;
			m_pDec->RndRest ();
			return YY_ERR_RETRY;
		}
		m_pLastFmtBuff = pInBuff;
	}
	if (m_pCurrBuffer == NULL)
		pInBuff = m_lstInFull.RemoveHead ();
	nRC = m_pDec->SetBuff (pInBuff);
	if (nRC == YY_ERR_UNSUPPORT)
		YYLOGI ("nRC = %08X, Count = %d, IsFF = %d", nRC, m_nReadSrcBufs, m_bFF);		
	if (!m_bFF && nRC == YY_ERR_UNSUPPORT && m_nOutCount == 0)
	{
		nRC = SwitchFFDec ();
		if (nRC == YY_ERR_NONE)
			nRC = m_pDec->SetBuff (pInBuff);
	}
	if (nRC == YY_ERR_RETRY)
	{
		m_pCurrBuffer = pInBuff;
	}
	else
	{
		if (m_pCurrBuffer == NULL)
			m_lstInFree.AddTail (pInBuff);
		m_pCurrBuffer = NULL;
	}
	
	if (nRC == YY_ERR_NONE)
		nRC = m_pDec->GetBuff (pBuffer);
	return nRC;
}

void CBoxVDBASmt::ResetInBuff (bool bDel)
{
	CAutoLock lock (&m_mtIn);
	YY_BUFFER * pBuff = m_lstInFull.RemoveHead ();
	while (pBuff != NULL)
	{
		m_lstInFree.AddTail (pBuff);
		pBuff = m_lstInFull.RemoveHead ();
	}

	if (bDel)
	{
		pBuff = m_lstInFree.RemoveHead ();
		while (pBuff != NULL)
		{
			yyDataResetBuffer (pBuff, true);
			pBuff = m_lstInFree.RemoveHead ();
		}
	}
}

void CBoxVDBASmt::ResetOutBuff (bool bDel)
{
	CAutoLock lock (&m_mtOut);
	YY_BUFFER * pBuff = m_lstOutFull.RemoveHead ();
	while (pBuff != NULL)
	{
		m_lstOutFree.AddTail (pBuff);
		pBuff = m_lstOutFull.RemoveHead ();
	}

	if (bDel)
	{
		pBuff = m_lstOutFree.RemoveHead ();
		while (pBuff != NULL)
		{
			yyDataResetVideoBuff (pBuff, true);
			pBuff = m_lstOutFree.RemoveHead ();
		}
	}
}

int CBoxVDBASmt::SoftDecProc (void * pParam)
{
	yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_BELOW_NORMAL);
	CBoxVDBASmt * pDec = (CBoxVDBASmt *)pParam;
	return pDec->SoftDecLoop ();
}

int CBoxVDBASmt::SoftDecLoop (void)
{
	int nRC = YY_ERR_NONE;
	if (m_pSoftDec == NULL)
	{
		YY_VIDEO_FORMAT * pFmt = m_pBoxSource->GetVideoFormat ();
		if (pFmt == NULL)
			return YY_ERR_VIDEO;		
		m_pSoftDec = new CFFMpegVideoDec (m_hInst);
		if (m_pSoftDec == NULL)
			return YY_ERR_MEMORY;
		nRC = m_pSoftDec->Init (pFmt);
		if (nRC != YY_ERR_NONE)
			return nRC;	
	}
	
	YY_BUFFER * pBuff = NULL;
	YY_BUFFER * pCurr = NULL;
	YY_BUFFER * pOutBuff = new YY_BUFFER ();
	YY_BUFFER * pOutSave = NULL;
	int			nOutDecs = 0;
	long long	llLastTime = 0;
	while (!m_bThrdStop)
	{
		if (m_lstOutFull.GetCount () >= YYBAS_OUT_BUFFS)
		{
			yySleep (2000);
			continue;
		}
		if (m_lstInSoft.GetCount () <= 0)
		{
			yySleep (2000);
			continue;
		}
		if (pCurr == NULL)
		{
			CAutoLock lock (&m_mtSoft);
			pBuff = m_lstInSoft.RemoveHead ();
			pCurr = NULL;
		}
		else
		{
			pBuff = pCurr;
		}
		nRC = m_pSoftDec->SetBuff (pBuff);
		if (nRC == YY_ERR_RETRY)
		{
			pCurr = pBuff;
			yySleep (2000);
			continue;
		}		
		if (nRC != YY_ERR_NONE)
			continue;
		
		pOutBuff->nType = YY_MEDIA_Video;
		nRC = m_pSoftDec->GetBuff (pOutBuff);
		if (nRC == YY_ERR_NONE)
		{
			if (nOutDecs == 0 || pOutBuff->llTime - llLastTime >= 80)
			{
				CAutoLock lock (&m_mtOut);
				pOutSave = m_lstOutFree.RemoveHead ();
				if (pOutSave == NULL)
				{
					pOutSave = new YY_BUFFER ();
					memset (pOutSave, 0, sizeof (YY_BUFFER));
				}
				yySleep (2000);
				yyDataCloneVideoBuff (pOutSave, pOutBuff);
				pOutSave->pFormat = NULL;
				m_lstOutFull.AddTail (pOutSave);
				llLastTime = pOutBuff->llTime;
			}
			if (nOutDecs == 0)
			{
				pOutSave->uFlag |= YYBUFF_NEW_FORMAT;
				memcpy (&m_fmtVideo, m_pSoftDec->GetVideoFormat (), sizeof (YY_VIDEO_FORMAT));
				m_fmtVideo.pHeadData = NULL;
				m_fmtVideo.nHeadSize = 0;
				pOutSave->pFormat = &m_fmtVideo;
			}
			nOutDecs++;
		}
		yySleep (4000);
	}
	delete pOutBuff;
	m_pSoftDec->Flush ();
	ResetOutBuff (false);
	CAutoLock lock (&m_mtSoft);		
	m_lstInSoft.RemoveAll ();	
	m_hThrdSoft = NULL;
	YYLOGI ("Exit the soft dec thread!");
	return 0;
}

int CBoxVDBASmt::HardDecProc (void * pParam)
{
	yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_BELOW_NORMAL);
	CBoxVDBASmt * pDec = (CBoxVDBASmt *)pParam;
	return pDec->HardDecLoop ();
}

int CBoxVDBASmt::HardDecLoop (void)
{
	YYLOGI ("Start Hard dec thread!");
	int nRC = YY_ERR_NONE;
	
	YY_VIDEO_FORMAT * pFmt = m_pBoxSource->GetVideoFormat ();
	if (pFmt == NULL)
		return YY_ERR_VIDEO;	
	if (m_pDec == NULL)	
#ifdef _OS_NDK			
		m_pDec = new CVVVideoOMXDec (m_hInst);
#else
		m_pDec = new CFFMpegVideoDec (m_hInst);
#endif // _OS_NDK
	if (m_pDec == NULL)
		return YY_ERR_MEMORY;
		
//	YYLOGI ("Check working!");
//	while (m_pDec->IsWorking ())
//		yySleep (2000);
//	YYLOGI ("Exit working!");
	nRC = m_pDec->Uninit ();
	nRC = m_pDec->Init (pFmt);
#ifdef _OS_WIN32
	yySleep (1000000);
#endif // _OS_WIN32
//	yySleep (1000000);
	YY_BUFFER bufOut;
	memset (&bufOut, 0, sizeof (bufOut));
	while (bufOut.llTime < m_llSoftRnd)
	{
		nRC = GetHardOutput (&bufOut);
		if (nRC == YY_ERR_NONE)
			m_pDec->RndBuff (&bufOut, false);
		yySleep (5000);
		if (m_bThrdStop)
			break;
	}
	m_hThrdHard = NULL;
	YYLOGT ("CBoxVDBASmt", "Exit the hard dec thread!");
	return nRC;	
}