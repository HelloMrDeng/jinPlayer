/*******************************************************************************
	File:		CBoxMonitor.cpp

	Contains:	base box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#include "CBoxMonitor.h"
#include "CBoxSource.h"

#include "USYstemFunc.h"
#include "yyLog.h"

static CBoxMonitor * g_pBoxMonitor = NULL;

CBoxRecOne::CBoxRecOne(CBoxBase * pBox, YY_BUFFER * pBuffer, int * pRC)
	: m_pBox (pBox)
	, m_pBuffer (pBuffer)
	, m_pRC (pRC)
{
	m_nThdUsed = yyGetThreadTime (NULL);
	m_nSysUsed = yyGetSysTime ();

	g_pBoxMonitor->StartRead (this);
}

CBoxRecOne::~CBoxRecOne(void)
{
	m_nThdUsed = yyGetThreadTime (NULL) - m_nThdUsed;
	m_nSysUsed = yyGetSysTime() - m_nSysUsed;

	g_pBoxMonitor->EndRead (this);
}

CBoxMonitor::CBoxMonitor(void)
	: CBaseObject ()
	, m_pClock (NULL)
	, m_hFile (NULL)
{
	SetObjectName ("CBoxMonitor");
	g_pBoxMonitor = this;
	TCHAR szFile[1024];
#ifdef _OS_WIN32
	yyGetAppPath (NULL, szFile, sizeof (szFile));
#else
	strcpy (szFile, "/sdcard/");
#endif // _OS_win32
	_tcscat (szFile, _T("yyLog.txt"));
//	m_hFile = yyFileOpen (szFile, YYFILE_WRITE);
}

CBoxMonitor::~CBoxMonitor(void)
{
	ReleaseItems ();
	if (m_hFile != NULL)
		yyFileClose (m_hFile);
	g_pBoxMonitor = NULL;
}

int CBoxMonitor::ReleaseItems (void)
{
	SBoxReadItem * pRead = m_lstRead.RemoveHead ();
	while (pRead != NULL)
	{
		delete pRead;
		pRead = m_lstRead.RemoveHead ();
	}

	return 0;
}

int CBoxMonitor::StartRead (CBoxRecOne * pOne)
{
	return 0;
}

int CBoxMonitor::EndRead (CBoxRecOne * pOne)
{
	CAutoLock lock (&m_mtRead);

	SBoxReadItem * pRead = new SBoxReadItem ();
	pRead->m_pBox = pOne->m_pBox;
	pRead->m_nType = pOne->m_pBuffer->nType;
	pRead->m_nSysUsed = pOne->m_nSysUsed;
	pRead->m_nThdUsed = pOne->m_nThdUsed;
	pRead->m_tmSys = yyGetSysTime ();
	pRead->m_tmBuf = (int)pOne->m_pBuffer->llTime;
	pRead->m_tmClock = (int)m_pClock->GetTime ();
	pRead->m_nRC = *pOne->m_pRC;

	m_lstRead.AddTail (pRead);

	return 0;
}

SBoxReadItem * CBoxMonitor::GetLastItem (CBoxBase * pBox, YYMediaType nType, int nRC)
{
	CAutoLock lock (&m_mtRead);

	SBoxReadItem * pItem = NULL;
	SBoxReadItem * pLast = NULL;
	NODEPOS pos = m_lstRead.GetTailPositionI ();
	while (pos != NULL)
	{
		pItem = m_lstRead.GetPrev (pos);
		if (pItem->m_nRC != nRC)
			continue;

		if (pItem->m_pBox == pBox && pItem->m_nType == nType)
		{
			pLast = pItem;
			break;
		}
	}

	return pLast;
}

int CBoxMonitor::ShowResult (void)
{
	CBoxBase * pBox = GetBox (OMB_TYPE_SOURCE, YY_MEDIA_Audio);
	if (pBox == NULL)
		pBox = GetBox (OMB_TYPE_SOURCE, YY_MEDIA_Video);
	if (pBox != NULL)
	{
		if (((CBoxSource *)pBox)->GetSourceName () != NULL)
		{
			YYLOGI ("Source Name: %s", ((CBoxSource *)pBox)->GetSourceName ());
		}
	}
#ifndef BOX_MONITOR_DISABLE_ALL
//	ShowAudioSrc ();
//	ShowVideoSrc ();
//	ShowAudioDec ();
//	ShowVideoDec ();
//	ShowAudioRnd ();
//	ShowVideoRnd ();

//	ShowPerformInfo ();

#endif // BOX_MONITOR_DISABLE_ALL
	if (m_hFile != NULL)
		yyFileClose (m_hFile);
	m_hFile = NULL;
	return 0;
}

void CBoxMonitor::ShowAudioSrc (void)
{
	CBoxBase * pBox = GetBox (OMB_TYPE_SOURCE, YY_MEDIA_Audio);
	if (pBox != NULL)
	{
		YYLOGI ("Box Source read audio info:");
		ShowBoxInfo (pBox, YY_MEDIA_Audio, true);
	}
}

void CBoxMonitor::ShowVideoSrc (void)
{
	CBoxBase * pBox = GetBox (OMB_TYPE_SOURCE, YY_MEDIA_Video);
	if (pBox != NULL)
	{
		YYLOGI ("Box Source read video info:");
		ShowBoxInfo (pBox, YY_MEDIA_Video, true);
	}
}

void CBoxMonitor::ShowAudioDec (void)
{
	CBoxBase * pBox = GetBox (OMB_TYPE_FILTER, YY_MEDIA_Audio);
	if (pBox != NULL)
	{
		YYLOGI ("Box Audio Dec info:");
		ShowBoxInfo (pBox, YY_MEDIA_Audio, true);
	}
}

void CBoxMonitor::ShowVideoDec (void)
{
	CBoxBase * pBox = GetBox (OMB_TYPE_FILTER, YY_MEDIA_Video);
	if (pBox != NULL)
	{
		YYLOGI ("Box Video Dec info:");
		ShowBoxInfo (pBox, YY_MEDIA_Video, true);
	}
}

void CBoxMonitor::ShowAudioRnd (void)
{
	CBoxBase * pBox = GetBox (OMB_TYPE_RENDER, YY_MEDIA_Audio);
	if (pBox != NULL)
	{
		YYLOGI ("Box Audio Rnd info:");
		ShowBoxInfo (pBox, YY_MEDIA_Audio, true);
	}
}

void CBoxMonitor::ShowVideoRnd (void)
{
	CBoxBase * pBox = GetBox (OMB_TYPE_RENDER, YY_MEDIA_Video);
	if (pBox != NULL)
	{
		YYLOGI ("Box Video Rnd info:");
		ShowBoxInfo (pBox, YY_MEDIA_Video, true);
	}
}

void CBoxMonitor::ShowBoxInfo (CBoxBase * pBox, YYMediaType nType, bool bSuccess)
{
	int				nIndex = 0;
	int				nStartTime = 0;
	SBoxReadItem *	pItem = NULL;
	SBoxReadItem *	pPrev = NULL;
	NODEPOS		pos = m_lstRead.GetHeadPosition ();

	YYLOGI ("Index  UseSys  UseThd    Buffer    Step   Buf-Clk   SysTime   Sys-Clk  Sys-Step");
	if (m_hFile != NULL)
	{
		yyFileWrite (m_hFile, (unsigned char *)pBox->GetName (), strlen (pBox->GetName ()));
		yyFileWrite (m_hFile, (unsigned char *)"\r\n\r\n", 4);
	}
	while (pos != NULL)
	{
		pItem = m_lstRead.GetNext (pos);
		if (pItem->m_pBox != pBox || pItem->m_nType != nType)
			continue;
		if (bSuccess && pItem->m_nRC != YY_ERR_NONE)
			continue;

		if (nIndex == 0)
		{
			nStartTime = pItem->m_tmSys;// - pItem->m_tmClock;
			YYLOGI ("% 5d  % 6d  % 6d  % 8d  % 6d  % 8d  % 8d  % 8d    % 6d",
					nIndex, pItem->m_nSysUsed, pItem->m_nThdUsed,
					pItem->m_tmBuf, 0, pItem->m_tmBuf - pItem->m_tmClock,
					0, pItem->m_tmSys - nStartTime - pItem->m_tmClock, 0);
			if (m_hFile != NULL)
			{
				char szLogText[1024];
				sprintf (szLogText, "% 5d  % 6d  % 6d  % 8d  % 6d  % 8d  % 8d  % 8d    % 6d\r\n",
						nIndex, pItem->m_nSysUsed, pItem->m_nThdUsed,
						pItem->m_tmBuf, 0, pItem->m_tmBuf - pItem->m_tmClock,
						0, pItem->m_tmSys - nStartTime - pItem->m_tmClock, 0);
				yyFileWrite (m_hFile, (unsigned char *)szLogText, strlen (szLogText));
			}
		}
		else
		{
//			if (pItem->m_tmBuf - pPrev->m_tmBuf != 40)
			{
			YYLOGI ("% 5d  % 6d  % 6d  % 8d  % 6d  % 8d  % 8d  % 8d    % 6d",
					nIndex, pItem->m_nSysUsed, pItem->m_nThdUsed,
					pItem->m_tmBuf, pItem->m_tmBuf - pPrev->m_tmBuf, pItem->m_tmBuf - pItem->m_tmClock,
					pItem->m_tmSys - nStartTime, pItem->m_tmSys - nStartTime - pItem->m_tmClock, pItem->m_tmSys - pPrev->m_tmSys);
			}
			if (m_hFile != NULL)
			{
				char szLogText[1024];
				sprintf (szLogText, "% 5d  % 6d  % 6d  % 8d  % 6d  % 8d  % 8d  % 8d    % 6d\r\n",
						nIndex, pItem->m_nSysUsed, pItem->m_nThdUsed,
						pItem->m_tmBuf, pItem->m_tmBuf - pPrev->m_tmBuf, pItem->m_tmBuf - pItem->m_tmClock,
						pItem->m_tmSys - nStartTime, pItem->m_tmSys - nStartTime - pItem->m_tmClock, pItem->m_tmSys - pPrev->m_tmSys);
				yyFileWrite (m_hFile, (unsigned char *)szLogText, strlen (szLogText));
			}
		}

		pPrev = pItem;
		nIndex++;
	}
	if (m_hFile != NULL)
		yyFileWrite (m_hFile, (unsigned char *)"\r\n\r\n\r\n\r\n", 8);
}

CBoxBase * CBoxMonitor::GetBox (OMBOX_TYPE nBoxType, YYMediaType nMediaType)
{
	CBoxBase *		pBox = NULL;
	SBoxReadItem *	pItem = NULL;
	NODEPOS		pos = m_lstRead.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstRead.GetNext (pos);
		if (pItem->m_pBox->GetType () == nBoxType && pItem->m_nType == nMediaType)
		{
			pBox = pItem->m_pBox;
			break;
		}
	}

	return pBox;
}

void CBoxMonitor::ShowPerformInfo (void)
{
	int				nCount = 0;
	int				nTotal = 0;
	int				nThdTime = 0;
	int				nSttTime = 0;
	int				nEndTime = 0;

	YYLOGI ("Show performance info:");

	SBoxReadItem *	pItem = NULL;
	NODEPOS		pos = m_lstRead.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstRead.GetNext (pos);
		if (pItem->m_pBox->GetType () == OMB_TYPE_SOURCE && pItem->m_nType == YY_MEDIA_Audio)
		{
			if (pItem->m_nRC != YY_ERR_NONE)
			{
				nTotal++;
				continue;
			}

			if (nCount == 0)
				nSttTime = pItem->m_tmSys;
			nCount++;
			nTotal++;
			nThdTime += pItem->m_nThdUsed;
			nEndTime = pItem->m_tmSys;
		}
	}
	YYLOGI ("Audio Read: Num % 6d / % 6d,    Thd % 8d,   % 4.2f%%", nCount, nTotal, nThdTime, (nThdTime * 100.0) / (nEndTime - nSttTime));

	nCount = 0;
	nTotal = 0;
	nThdTime = 0;
	pos = m_lstRead.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstRead.GetNext (pos);
		if (pItem->m_pBox->GetType () == OMB_TYPE_FILTER && pItem->m_nType == YY_MEDIA_Audio)
		{
			if (pItem->m_nRC != YY_ERR_NONE)
			{
				nTotal++;
				continue;
			}

			if (nCount == 0)
				nSttTime = pItem->m_tmSys;
			nCount++;
			nTotal++;
			nThdTime += pItem->m_nThdUsed;
			nEndTime = pItem->m_tmSys;
		}
	}
	YYLOGI ("Audio Dec:  Num % 6d / % 6d,    Thd % 8d,   % 4.2f%%", nCount, nTotal, nThdTime, (nThdTime * 100.0) / (nEndTime - nSttTime));

	nCount = 0;
	nTotal = 0;
	nThdTime = 0;
	pos = m_lstRead.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstRead.GetNext (pos);
		if (pItem->m_pBox->GetType () == OMB_TYPE_RENDER && pItem->m_nType == YY_MEDIA_Audio)
		{
			if (pItem->m_nRC != YY_ERR_NONE)
			{
				nTotal++;
				continue;
			}

			if (nCount == 0)
				nSttTime = pItem->m_tmSys;
			nCount++;
			nTotal++;
			nThdTime += pItem->m_nThdUsed;
			nEndTime = pItem->m_tmSys;
		}
	}
	YYLOGI ("Audio Rnd:  Num % 6d / % 6d,    Thd % 8d,   % 4.2f%%", nCount, nTotal, nThdTime, (nThdTime * 100.0) / (nEndTime - nSttTime));


	// Start to analyse video info.
	nCount = 0;
	nTotal = 0;
	nThdTime = 0;
	pos = m_lstRead.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstRead.GetNext (pos);
		if (pItem->m_pBox->GetType () == OMB_TYPE_SOURCE && pItem->m_nType == YY_MEDIA_Video)
		{
			if (pItem->m_nRC != YY_ERR_NONE)
			{
				nTotal++;
				continue;
			}

			if (nCount == 0)
				nSttTime = pItem->m_tmSys;
			nCount++;
			nTotal++;
			nThdTime += pItem->m_nThdUsed;
			nEndTime = pItem->m_tmSys;
		}
	}
	YYLOGI ("Video Read: Num % 6d / % 6d,    Thd % 8d,   % 4.2f%%   Speed  % 4.2f F/S", nCount, nTotal, nThdTime, 
			(nThdTime * 100.0) / (nEndTime - nSttTime), (nCount * 1000.0) / (nEndTime - nSttTime));

	nCount = 0;
	nTotal = 0;
	nThdTime = 0;
	pos = m_lstRead.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstRead.GetNext (pos);
		if (pItem->m_pBox->GetType () == OMB_TYPE_FILTER && pItem->m_nType == YY_MEDIA_Video)
		{
			if (pItem->m_nRC != YY_ERR_NONE)
			{
				nTotal++;
				continue;
			}

			if (nCount == 0)
				nSttTime = pItem->m_tmSys;
			nCount++;
			nTotal++;
			nThdTime += pItem->m_nThdUsed;
			nEndTime = pItem->m_tmSys;
		}
	}
	YYLOGI ("Video Dec:  Num % 6d / % 6d,    Thd % 8d,   % 4.2f%%   Speed  % 4.2f F/S", nCount, nTotal, nThdTime, 
			(nThdTime * 100.0) / (nEndTime - nSttTime), (nCount * 1000.0) / (nEndTime - nSttTime));

	nCount = 0;
	nTotal = 0;
	nThdTime = 0;
	pos = m_lstRead.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstRead.GetNext (pos);
		if (pItem->m_pBox->GetType () == OMB_TYPE_RENDER && pItem->m_nType == YY_MEDIA_Video)
		{
			if (pItem->m_nRC != YY_ERR_NONE)
			{
				nTotal++;
				continue;
			}

			if (nCount == 0)
				nSttTime = pItem->m_tmSys;
			nCount++;
			nTotal++;
			nThdTime += pItem->m_nThdUsed;
			nEndTime = pItem->m_tmSys;
		}
	}
	YYLOGI ("Video Rnd:  Num % 6d / % 6d,    Thd % 8d,   % 4.2f%%   Speed  % 4.2f F/S", nCount, nTotal, nThdTime, 
			(nThdTime * 100.0) / (nEndTime - nSttTime), (nCount * 1000.0) / (nEndTime - nSttTime));
}