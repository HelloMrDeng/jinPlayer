/*******************************************************************************
	File:		CBaseHTTP.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "CBaseHTTP.h"

#include "ULibFunc.h"
#include "USystemFunc.h"
#include "yyLog.h"

typedef VO_VOID (* VOGETSOURCEIOAPI) (VO_SOURCE2_IO_API * fAPI);

CBaseHTTP::CBaseHTTP(void)
	: CBaseIO ()
	, m_hDll (NULL)
	, m_hHandle (NULL)
	, m_llFileSize (0)
{
	SetObjectName ("CBaseHTTP");
	memset (&m_fAPI, 0, sizeof (m_fAPI));

	m_hDll = yyLibLoad ((TCHAR *)_T("voSourceIO"), 0);
	if (m_hDll != NULL)
	{
		VOGETSOURCEIOAPI pAPI = NULL;
		pAPI = (VOGETSOURCEIOAPI)yyLibGetAddr (m_hDll, (char *)"voGetSourceIOAPI", 0);
		if (pAPI != NULL)
		{
			pAPI (&m_fAPI);
		}
		else
		{
			YYLOGE ("Init Data IO can't get entry pointer!");
		}
	}
	else
	{
		YYLOGE ("Init Data IO can't be loaded!");
	}
}

CBaseHTTP::~CBaseHTTP(void)
{
	close (NULL);

	if (m_hHandle != NULL)
	{
		m_fAPI.UnInit (m_hHandle);
		m_hHandle = NULL;
	}

	if (m_hDll != NULL)
		yyLibFree (m_hDll, 0);
	m_hDll = NULL;
}

int CBaseHTTP::open (URLContext *h, const TCHAR * filename, int flags)
{
	if (m_fAPI.Init == NULL)
		return -1;

	char szURL[2048];
	memset (szURL, 0, sizeof (szURL));
#ifdef _UNICODE
	WideCharToMultiByte (CP_ACP, 0, filename, -1, szURL, sizeof (szURL), NULL, NULL);
#else
	strcpy (szURL, filename);
#endif // _UNICODE

	m_sOpenCB.pUserData = this;
	m_sOpenCB.Async_CallBack = ioOpenNotify;

	int nFlag = VO_SOURCE2_IO_FLAG_OPEN_URL | VO_SOURCE2_IO_FLAG_OPEN_SYNC;
	int nRC = m_fAPI.Init (&m_hHandle, (void *)szURL, nFlag, &m_sOpenCB);
	if (nRC != VO_SOURCE2_IO_OK)
	{
		YYLOGE ("Init Data IO can't be inited return %08X!", nRC);
		return -1;
	}

	m_sImplCB.hHandle = this;
	m_sImplCB.IO_Callback = ioImplNotify;
	nRC = m_fAPI.SetParam (m_hHandle, VO_SOURCE2_IO_PARAMID_HTTPIOCALLBACK, &m_sImplCB);

	nRC = m_fAPI.Open (m_hHandle, VO_FALSE);
	if (nRC != VO_SOURCE2_IO_OK)
	{
		YYLOGE ("Init Data IO can't be opened return %08X!", nRC);
		return -1;
	}

	m_fAPI.GetSize (m_hHandle, (VO_U64 *)&m_llFileSize);

	return 0;
}

int CBaseHTTP::read (URLContext *h, unsigned char *buf, int size)
{	
	if (m_hHandle == NULL)
		return -1;

	VO_U32	nRead = 0;
	int		nRC = 0;
	int		nTryTimes = 0;
	while (nRead == 0)
	{
		nRC = m_fAPI.Read (m_hHandle, buf, size, &nRead);
		if (nRead == 0)
			yySleep (2000);
		nTryTimes++;
		if (nTryTimes > 1000)
			break;
	}

	return nRead;
}

int CBaseHTTP::write(URLContext *h, const unsigned char *buf, int size)
{	
	if (m_hHandle == NULL)
		return -1;

	VO_U32 nWrite = 0;
	int nRC = m_fAPI.Write (m_hHandle, (void *)buf, size, &nWrite);

	return nWrite;
}

int64_t CBaseHTTP::seek (URLContext *h, int64_t pos, int whence)
{		
	if (m_hHandle == NULL)
		return -1;

	long long			llPos = -1;
	VO_SOURCE2_IO_POS	posWhere;
	if (whence == AVSEEK_SIZE)
		return m_llFileSize;
	else if (whence == SEEK_SET)
		posWhere = VO_SOURCE2_IO_POS_BEGIN;
	else if (whence == SEEK_CUR)
		posWhere = VO_SOURCE2_IO_POS_CURRENT;
	else if (whence == SEEK_END)
		posWhere = VO_SOURCE2_IO_POS_END;

	int nRC = m_fAPI.SetPos (m_hHandle, pos, posWhere, &llPos);

	return llPos;
}

int CBaseHTTP::close (URLContext *h)
{	
	if (m_hHandle == NULL)
		return -1;

	m_fAPI.Close (m_hHandle);

	return 0;
}

int CBaseHTTP::get_handle (URLContext *h)
{	
	return (int)m_hHandle;
}

int CBaseHTTP::check (URLContext *h, int mask)
{	
	if (m_hHandle != NULL)
		return 0;
	else
		return -1;
}

int CBaseHTTP::shutdown (URLContext *h, int flags)
{
	if (m_hHandle != NULL)
	{
		m_fAPI.UnInit (m_hHandle);
		m_hHandle = NULL;
	}

	return 0;
}

VO_U32 CBaseHTTP::ioOpenNotify (VO_PTR pUserData , VO_U32 uID , VO_U32 uError , VO_PBYTE pBuf , VO_U32 uSizeDone )
{
	CBaseHTTP * pHTTP = (CBaseHTTP *)pUserData;

	YYLOGT (pHTTP->m_szObjName, "ID: %08X, Size %d ", uID, uSizeDone);

	return 0;
}

VO_U32 CBaseHTTP::ioImplNotify (VO_PTR hHandle , VO_U32 uID , VO_PTR pParam1 , VO_PTR pParam2)
{
	CBaseHTTP * pHTTP = (CBaseHTTP *)hHandle;

	YYLOGT (pHTTP->m_szObjName, "ID: %08X ", uID);

	return 0;
}