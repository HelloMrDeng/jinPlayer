/*******************************************************************************
	File:		CBasePDP.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2015-03-14		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "CBasePDP.h"
#include "CSourceIO.h"

#include "UThreadFunc.h"
#include "USystemFunc.h"
#include "UFileFunc.h"
#include "UStringFunc.h"

#include "yyLog.h"

TCHAR	CBasePDP::g_szPDPFile[1024] = {_T("")};

CBasePDP::CBasePDP(void)
	: CBaseIO ()
	, m_pIO (NULL)
	, m_llFileSize (0)
	, m_pBuffer (NULL)
	, m_nBuffRead (0)
	, m_nBuffDown (0)
	, m_hThread (NULL)
	, m_nStatus (YY_PLAY_Init)
{
	SetObjectName ("CBasePDP");
}

CBasePDP::~CBasePDP(void)
{
	close (NULL);
}

int CBasePDP::open (URLContext *h, const TCHAR * filename, int flags)
{
	int		nPrevNum = 3;
	char	szURL[2048];
	memset (szURL, 0, sizeof (szURL));
	strcpy (szURL, "http");
	if (_tcsstr (filename, _T("file:pdp")) != NULL)
		nPrevNum = 8;
#ifdef _OS_WIN32
	WideCharToMultiByte (CP_ACP, 0, filename + nPrevNum, -1, szURL + 4, sizeof (szURL), NULL, NULL);
#else
	strcat (szURL, filename + nPrevNum);
	YYLOGI ("Open URL %s, Org: %s", szURL, filename);
#endif // _OS_WIN32

	m_pIO = new CSourceIO ();
	int nRC = m_pIO->Init (szURL, 0);
	if (nRC != VO_ERR_NONE)
	{	
		YYLOGE ("Init url failed! return %08X", nRC);
		return -1;
	}
	nRC = m_pIO->Open ();
	if (nRC != VO_ERR_NONE)
	if (nRC != VO_ERR_NONE)
	{	
		YYLOGE ("OPen url failed! return %08X", nRC);
		return -1;
	}
	m_llFileSize = m_pIO->GetSize ();
	m_nStatus = YY_PLAY_Run;
	if (m_pBuffer == NULL)
		m_pBuffer = new VO_BYTE[m_llFileSize];
	if (m_hThread == NULL)
	{
		int nID = 0;
		yyThreadCreate (&m_hThread, &nID, DownLoadProc, this, 0);
	}
	return 0;
}

int CBasePDP::read (URLContext *h, unsigned char *buf, int size)
{	
	if (m_pIO == NULL)
		return -1;

	if (m_nBuffRead + size > m_llFileSize)
		size = m_llFileSize - m_nBuffRead;
	if (m_nBuffRead + size > m_nBuffDown)
	{
		int nStart = yyGetSysTime ();
		while (yyGetSysTime () - nStart < 5000)
		{
			yySleep (2000);
			if (m_nBuffRead + size <= m_nBuffDown)
				break;
		}
		if (m_nBuffRead + size > m_nBuffDown)
			return 0;
	}

	CAutoLock lock (&m_mtBuff);
	memcpy (buf, m_pBuffer + m_nBuffRead, size);
	m_nBuffRead += size;

	return size;
}

int CBasePDP::write(URLContext *h, unsigned char *buf, int size)
{	
	unsigned int uWrite = 0;
	return uWrite;
}

int64_t CBasePDP::seek (URLContext *h, int64_t pos, int whence)
{	
	CAutoLock lock (&m_mtBuff);
	int64_t llPos = m_nBuffRead;
	if (whence == AVSEEK_SIZE)
		return m_llFileSize;
	else if (whence == SEEK_SET)
	{
		llPos = pos;
	}
	else if (whence == SEEK_CUR)
	{
		llPos = m_nBuffRead + pos;
	}
	else if (whence == SEEK_END)
	{
		llPos = m_llFileSize - pos;
	}

	if (llPos <= m_nBuffDown)
	{
		m_nBuffRead = llPos;
		return llPos;
	}

	
	return -1;			
}

int CBasePDP::close (URLContext *h)
{	
	if (m_pIO != NULL)
		m_pIO->Close ();
	m_nStatus = YY_PLAY_Stop;
	while (m_hThread != NULL)
	{
		yySleep (10000);
	}
	YY_DEL_P (m_pIO);
	YY_DEL_A (m_pBuffer);
	m_llFileSize = 0;
	m_nBuffRead = 0;
	m_nBuffDown = 0;

	return 0;
}

int CBasePDP::get_handle (URLContext *h)
{			
	return (int)m_pIO;
}

int CBasePDP::check (URLContext *h, int mask)
{	
	if (m_pIO != NULL)
		return 0;
	else
		return -1;
}

int CBasePDP::DownLoadProc (void * pParam)
{
	CBasePDP * pPDP = (CBasePDP *) pParam;

	return pPDP->DownLoadLoop ();
}

int CBasePDP::DownLoadLoop (void)
{
	int				nRC = 0;
	int				nReadStep = 32 * 1024;
	unsigned char *	pBuffer = new unsigned char[nReadStep];
	while (m_nStatus == YY_PLAY_Run || m_nStatus == YY_PLAY_Pause)
	{
		if (m_nStatus == YY_PLAY_Pause)
		{
			yySleep (10000);
			continue;
		}
		VO_S64			nRestSize = m_llFileSize;
		int				nReadSize = 0;
		while (nRestSize > 0)
		{
			nRC = m_pIO->Read (pBuffer, nReadStep, &nReadSize);
			if (nReadSize > 0)
			{
				nRestSize -= nReadSize;
				CAutoLock lock (&m_mtBuff);
				memcpy (m_pBuffer + m_nBuffDown, pBuffer, nReadSize);
				m_nBuffDown += nReadSize;
			}
			if (nRC == VO_ERR_RETRY)
				yySleep (2000);
			else if (nRC != VO_ERR_NONE)
				break;
			if (m_nStatus != YY_PLAY_Run)
				break;
		}
		if (nRC == VO_ERR_FINISH)
			break;
	}
	delete []pBuffer;

	YYLOGI ("PDPFile: %s, DownLoad Size = %d, File: %d", g_szPDPFile, m_nBuffDown, m_llFileSize);
	if (_tcslen (g_szPDPFile) > 0 && m_nBuffDown == m_llFileSize)
	{
		yyFile hFile = yyFileOpen (g_szPDPFile, YYFILE_WRITE);
		if (hFile != NULL)
		{
			int nWrite = 0;
			int nWStep = 32 * 1024;
			while (nWrite < m_nBuffDown)
			{
				if (nWStep > m_nBuffDown - nWrite)
					nWStep = m_nBuffDown - nWrite;
				if (yyFileWrite (hFile, m_pBuffer + nWrite, nWStep) < 0)
					break;
				nWrite += nReadStep;
				yySleep (1000);
			}
			yyFileClose (hFile);
		}
	}

	m_hThread = NULL;
	return 0;
}
