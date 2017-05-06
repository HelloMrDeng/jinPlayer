/*******************************************************************************
	File:		CBoxVideoDec.cpp

	Contains:	The video dec box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#include "CBoxVideoDec.h"
#include "CFFMpegVideoDec.h"
#ifdef _EXT_VO
#include "CVVVideoDec.h"
#include "CVVVideoH265Dec.h"
#ifdef _OS_NDK
#include "CVVVideoOMXDec.h"
#endif // _OS_NDK
#endif // _EXT_VO

#ifdef _OS_WIN32
#include "COpenHEVCDec.h"
#endif // _OS_WIN32

#include "CBoxMonitor.h"
#include "USystemFunc.h"

#include "yyMediaPlayer.h"
#include "yyLog.h"

CBoxVideoDec::CBoxVideoDec(void * hInst)
	: CBoxBase (hInst)
	, m_pDec (NULL)
	, m_bFF (true)
	, m_hThread (NULL)
	, m_bExitThread (false)
	, m_bNewFormat (false)
	, m_bEOS (false)
	, m_bSetRndPrio (false)
	, m_nReadSrcBufs (0)
	, m_nOutCount (0)
	, m_lllastOutTime (0)
	, m_lllastChkTime (0)
	, m_nLastSysTime (0)
	, m_llDbgRndTime (0)
	, m_nDbgSysTime (0)
{
	SetObjectName ("CBoxVideoDec");
	m_nBoxType = OMB_TYPE_FILTER;
	strcpy (m_szBoxName, "Video Dec Box");
}

CBoxVideoDec::~CBoxVideoDec(void)
{
	YY_DEL_P (m_pDec);

	ResetListInfo (true);
}

int CBoxVideoDec::SetSource (CBoxBase * pSource)
{
	if (pSource == NULL)
		return YY_ERR_ARG;

	Stop ();

	YY_DEL_P (m_pDec);

	CBoxBase::SetSource (pSource);

	YY_VIDEO_FORMAT * pFmt = pSource->GetVideoFormat ();
	if (pFmt == NULL)
		return YY_ERR_VIDEO;

	m_bFF = false;
#ifdef _EXT_VO
#ifdef _OS_NDK
	if ((m_nDecMode & YY_VDM_IOMX) == YY_VDM_IOMX)
	{
		if (m_pDec == NULL && pFmt->nCodecID == AV_CODEC_ID_H264)
			m_pDec = new CVVVideoOMXDec (m_hInst);
	}
#endif // _OS_NDK
#ifndef _CPU_PRIMA2
#ifndef _CPU_MSB2531
#ifdef _OS_WINCE
	if (m_pDec == NULL)
	{
		if (pFmt->nCodecID == AV_CODEC_ID_MPEG4)
			m_pDec = new CVVVideoDec (m_hInst);
		if (pFmt->nCodecID == AV_CODEC_ID_WMV1 || pFmt->nCodecID == AV_CODEC_ID_WMV2 || pFmt->nCodecID == AV_CODEC_ID_WMV3)
		{
			if (pFmt->nWidth > 480 && pFmt->nHeight > 320)
			m_pDec = new CVVVideoDec (m_hInst);
		}
	}
#else
//	if (m_pDec == NULL && pFmt->nCodecID == AV_CODEC_ID_H265)
//		m_pDec = new CVVVideoH265Dec (m_hInst);	
#endif // _OS_WINCE
#endif // _CPU_MSB2531
#endif // _CPU_PRIMA2
#endif // _EXT_VO

#ifdef _OS_WINPC
	if (m_pDec == NULL && pFmt->nCodecID == AV_CODEC_ID_H265)
		m_pDec = new COpenHEVCDec (m_hInst);
#endif // _OS_WINPC
	if (m_pDec == NULL)
	{
		m_pDec = new CFFMpegVideoDec (m_hInst);
		m_bFF = true;
	}
	if (m_pDec == NULL)
		return YY_ERR_MEMORY;

	int nRC = m_pDec->SetDisplay (m_hView, &m_rcView);
	nRC = m_pDec->Init (pFmt);
	if (nRC != YY_ERR_NONE)
	{
		if (!m_bFF)
		{
			delete m_pDec;
			m_pDec = new CFFMpegVideoDec (m_hInst);
			nRC = m_pDec->Init (pFmt);
		}

		if (nRC != YY_ERR_NONE)
			return nRC;
	}

	m_pFmtVideo = m_pDec->GetVideoFormat ();
	m_bNewFormat = false;
	m_bEOS = false;
	m_bSetRndPrio = false;
	m_nReadSrcBufs = 0;
	m_nOutCount = 0;

	memset (&m_bufOutput, 0, sizeof (YY_BUFFER));

	return YY_ERR_NONE;
}

int CBoxVideoDec::SwitchFFDec (void)
{
	YYLOGI ("It Switch to software dec!!!");
	YY_DEL_P (m_pDec);	
	YY_VIDEO_FORMAT * pFmt = m_pBoxSource->GetVideoFormat ();
	if (pFmt == NULL)
		return YY_ERR_VIDEO;
		
	m_pDec = new CFFMpegVideoDec (m_hInst);
	m_bFF = true;	
	if (m_pDec == NULL)
		return YY_ERR_MEMORY;

	int nRC = m_pDec->SetDisplay (m_hView, &m_rcView);
	nRC = m_pDec->Init (pFmt);
	if (nRC != YY_ERR_NONE)
		return nRC;

	m_pFmtVideo = m_pDec->GetVideoFormat ();
				
	return YY_ERR_NONE;
}

int CBoxVideoDec::SetPos (int nPos, bool bSeek)
{
	int nRC = CBoxBase::SetPos (nPos, bSeek);

	CAutoLock lock (&m_mtReadSrc);
	m_pCurrBuffer = NULL;
	m_nReadSrcBufs = 0;
	m_bNewFormat = false;
	m_bEOS = false;

	ResetListInfo (false);

	return nRC;
}

int CBoxVideoDec::ReadBuffer (YY_BUFFER * pBuffer, bool bWait)
{
	if (m_pDec == NULL)
		return YY_ERR_FAILED;

	int nRC = YY_ERR_NONE;
	if (m_bEOS)
	{
		nRC = m_pDec->GetBuff (pBuffer);
		if (nRC != YY_ERR_NONE)
		{
			pBuffer->uFlag |= YYBUFF_EOS;
			return YY_ERR_FINISH;
		}
		return nRC;
	}
	if (m_bNewFormat)
	{
		nRC = m_pDec->GetBuff (pBuffer);
		if (nRC == YY_ERR_NONE)
		{
#ifdef YYDBG_VD_OUTPUT			
			YYLOGI (" Rend soft dec buffer! % 8d @@@@@@  % 8d  sys: % 6d", (int)pBuffer->llTime, (int)(pBuffer->llTime - m_llDbgRndTime), yyGetSysTime () - m_nDbgSysTime);
			m_llDbgRndTime = pBuffer->llTime;
			m_nDbgSysTime = yyGetSysTime ();
#endif // YYDBG_VD_OUTPUT
			return nRC;
		}
		m_bNewFormat = false;
	}
	if (m_hThread == NULL)
	{
		nRC = DoReadBuffer (pBuffer, bWait);
		if (nRC == YY_ERR_NONE)
		{
			nRC = m_pDec->GetBuff (pBuffer);
			if (nRC == YY_ERR_NONE)
			{
#ifdef YYDBG_VD_OUTPUT				
				YYLOGI (" Rend soft dec buffer! % 8d ******  % 8d  sys: % 6d", (int)pBuffer->llTime, (int)(pBuffer->llTime - m_llDbgRndTime), yyGetSysTime () - m_nDbgSysTime);
				m_llDbgRndTime = pBuffer->llTime;
				m_nDbgSysTime = yyGetSysTime ();
#endif // YYDBG_VD_OUTPUT		
				m_nOutCount++;
			}
		}
		return nRC;
	}
	else
	{
		if (!m_bSetRndPrio)
		{
			m_bSetRndPrio = true;
			yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_ABOVE_NORMAL);
		}

		{
			CAutoLock	lockFlush (&m_mtBuffer);
			memcpy (&m_bufOutput, pBuffer, sizeof (YY_BUFFER));
		}

		if (m_nReadSrcBufs <= 0)
			return YY_ERR_RETRY;

		nRC = m_pDec->GetBuff (pBuffer);
		if (nRC != YY_ERR_NONE && m_bEOS)
		{
			pBuffer->uFlag |= YYBUFF_EOS;
			return YY_ERR_FINISH;
		}
		if (nRC == YY_ERR_NONE)
		{
			m_nOutCount++;
			UpdateListInfo (pBuffer, false);
		}
		if (nRC == YY_ERR_RETRY)
			yySleep (5000);

		return nRC;
	}

	return YY_ERR_FAILED;
}

int CBoxVideoDec::RendBuffer (YY_BUFFER * pBuffer, bool bRend)
{
	if (m_pDec != NULL)
		return m_pDec->RndBuff (pBuffer, bRend);
	return YY_ERR_FAILED;
}

int CBoxVideoDec::DoReadBuffer (YY_BUFFER * pBuffer, bool bWait)
{
	int nRC = YY_ERR_FAILED;
	if (m_pCurrBuffer != NULL)
	{
		nRC = m_pDec->SetBuff (m_pCurrBuffer);
		if (nRC == YY_ERR_RETRY || nRC == YY_ERR_NONE)
		{
			if (nRC == YY_ERR_NONE)
			{
				m_nReadSrcBufs++;
				UpdateListInfo (m_pCurrBuffer, true);
				m_pCurrBuffer = NULL;	
			}
			return nRC;
		}
		m_pCurrBuffer = NULL;
		return nRC;
	}

	m_pBaseBuffer->nType = YY_MEDIA_Video;
	m_pBaseBuffer->uFlag = 0;
	m_pBaseBuffer->llTime = pBuffer->llTime;
	m_pBaseBuffer->llDelay = pBuffer->llDelay;
	if ((pBuffer->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		m_pBaseBuffer->uFlag |= YYBUFF_NEW_POS;

	while (m_nStatus == OMB_STATUS_RUN || m_nStatus == OMB_STATUS_PAUSE)
	{
		if (m_hThread != NULL)
		{
			if (m_lstFull.GetCount () >= 6)
			{
				if (m_lllastOutTime > m_lllastChkTime)
				{
					m_lllastChkTime = m_lllastOutTime;
					return YY_ERR_RETRY;
				}
				else
				{
					if (yyGetSysTime () - m_nLastSysTime < 15)
						return YY_ERR_RETRY;
				}
			}
			m_pBaseBuffer->llTime = pBuffer->llTime + m_lstFull.GetCount () * 15;
			// YYLOGI ("List num : % 6d", m_lstFull.GetCount ());
		}

		nRC = m_pBoxSource->ReadBuffer (m_pBaseBuffer, false);
		if (nRC != YY_ERR_NONE)
		{
			if (nRC == YY_ERR_RETRY)
			{
				if (m_hThread == NULL)
					return nRC;	
				else
					continue;
			}
			if (nRC == YY_ERR_FINISH)
				m_bEOS = true;
			return nRC;
		}

		if ((m_pBaseBuffer->uFlag & YYBUFF_EOS) == YYBUFF_EOS)
			m_bEOS = true;
		if ((m_pBaseBuffer->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT && m_nReadSrcBufs > 0)
		{
			YYLOGI ("Read new format buffer!");
			m_bNewFormat = true;
			m_pCurrBuffer = m_pBaseBuffer;
			m_pDec->RndRest ();
			return YY_ERR_RETRY;
		}

		// save the buffer time from source.
		pBuffer->llTime = m_pBaseBuffer->llTime;
//		YYLOGI ("VideoDec Input % 8d, Step % 8d", (int)pBuffer->llTime, (int)(pBuffer->llTime - m_llDbgLastTime));
//		m_llDbgLastTime = pBuffer->llTime;

		BOX_READ_BUFFER_REC_VIDEODEC

		nRC = m_pDec->SetBuff (m_pBaseBuffer);
		if (nRC == YY_ERR_UNSUPPORT)
			YYLOGI ("nRC = %08X, Count = %d, IsFF = %d", nRC, m_nReadSrcBufs, m_bFF);		
		if (!m_bFF && nRC == YY_ERR_UNSUPPORT && m_nOutCount == 0)
		{
			nRC = SwitchFFDec ();
			if (nRC == YY_ERR_NONE)
				nRC = m_pDec->SetBuff (m_pBaseBuffer);
		}
		if (nRC == YY_ERR_NONE || nRC == YY_ERR_NEEDMORE)
		{
			m_nReadSrcBufs++;
			if (m_hThread != NULL)
				UpdateListInfo (m_pBaseBuffer, true);
			if (nRC == YY_ERR_NONE)
				break;
		}
		else if (nRC == YY_ERR_RETRY)
		{
			m_pCurrBuffer = m_pBaseBuffer;
			break;
		}
		else
			return nRC;
	}

	return nRC;
}

int	CBoxVideoDec::Start (void)
{
	CBoxBase::Start ();

	if (m_pDec->GetBuffNum () <= 1)
		return YY_ERR_NONE;

	if (m_hThread == NULL)
	{
		m_bExitThread = false;
		int nID = 0;
		yyThreadCreate (&m_hThread, &nID, DecProc, this, 0);
	}

	return YY_ERR_NONE;
}

int	CBoxVideoDec::Stop (void)
{
	CBoxBase::Stop ();

	m_bExitThread = true;

	while (m_hThread != NULL)
	{
		yySleep (10000);
	}

	return YY_ERR_NONE;
}

int CBoxVideoDec::DecProc (void * pParam)
{
	yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_BELOW_NORMAL);

	CBoxVideoDec * pBoxDec = (CBoxVideoDec *) pParam;

	return pBoxDec->DecLoop ();
}

int CBoxVideoDec::DecLoop (void)
{
	YY_BUFFER * pBuff = NULL;
	int			nRC = YY_ERR_NONE;

	while (!m_bExitThread)
	{
		m_mtReadSrc.Lock ();
		YY_BUFFER	bufTmpDec;
		{
			CAutoLock	lockFlush (&m_mtBuffer);
			memcpy (&bufTmpDec, &m_bufOutput, sizeof (YY_BUFFER));
		}
		nRC = DoReadBuffer (&bufTmpDec, false);
		m_mtReadSrc.Unlock ();

		if (nRC == YY_ERR_RETRY)
			yySleep (5000);
	}

	m_hThread = NULL;

	return 0;
}

void CBoxVideoDec::UpdateListInfo (YY_BUFFER * pBuffer, bool bInput)
{
	CAutoLock lock (&m_mtInfo);
	YYBOX_VBUFF_INFO * pInfo = NULL;
	if (bInput)
	{
		pInfo = m_lstFree.RemoveHead ();
		if (pInfo == NULL)
			pInfo = new YYBOX_VBUFF_INFO ();
		pInfo->llTime = pBuffer->llTime;
		pInfo->llDelay = pBuffer->llDelay;
		pInfo->nFlag = pBuffer->uFlag;
		m_lstFull.AddTail (pInfo);

		m_nLastSysTime = yyGetSysTime ();
	}
	else
	{
		m_lllastOutTime = pBuffer->llTime;
		NODEPOS pos = m_lstFull.GetHeadPosition ();
		while (pos != NULL)
		{
			pInfo = m_lstFull.GetNext (pos);
			if (pInfo->llTime > pBuffer->llTime + 500)
				break;
			if (pInfo->llTime <= pBuffer->llTime)
			{
				m_lstFull.Remove (pInfo);
				m_lstFree.AddTail (pInfo);
			}
		}
	}
}

void CBoxVideoDec::ResetListInfo (bool bDelete)
{
	CAutoLock lock (&m_mtInfo);
	m_lllastChkTime = 0;
	m_lllastChkTime = 0;
	m_nLastSysTime = 0;

	YYBOX_VBUFF_INFO * pInfo = m_lstFull.RemoveHead ();
	while (pInfo != NULL)
	{
		m_lstFree.AddTail (pInfo);
		pInfo = m_lstFull.RemoveHead ();
	}

	if (bDelete)
	{
		pInfo = m_lstFree.RemoveHead ();
		while (pInfo != NULL)
		{
			delete pInfo;
			pInfo = m_lstFree.RemoveHead ();
		}
	}
}