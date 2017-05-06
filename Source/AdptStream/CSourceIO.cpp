/*******************************************************************************
	File:		CStreamSource.cpp

	Contains:	adaption stream source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#include "CSourceIO.h"

#include "UThreadFunc.h"
#include "USystemFunc.h"
#include "ULibFunc.h"

#include "yyLog.h"

#include "voSource2.h"

CSourceIO::CSourceIO(void)
	: CBaseObject ()
	, m_llFileSize (0)
	, m_bExitRead (false)
	, m_hSSLDll (NULL)
{
	SetObjectName ("CSourceIO");

	memset (&m_sSSLApi, 0, sizeof (m_sSSLApi));
	memset (&m_fAPI, 0, sizeof (m_fAPI));
	memset (m_szURL, 0, sizeof (m_szURL));

	m_sOpenCB.pUserData = this;
	m_sOpenCB.Async_CallBack = ioOpenNotify;

	m_sImplCB.hHandle = this;
	m_sImplCB.IO_Callback = ioImplNotify;

	yyGetDataIOFunc (&m_fAPI);
}

CSourceIO::~CSourceIO(void)
{
	if (m_fAPI.hHandle != NULL)
		m_fAPI.UnInit (m_fAPI.hHandle);
	FreeSSL ();
}

int CSourceIO::Init (const char * pURL, int nFlag)
{
	CAutoLock lock (&m_mtIO);
	if (m_fAPI.Init == NULL)
		return VO_ERR_FAILED;
	if (m_hSSLDll == NULL && strstr (pURL, "https://") == pURL)
		LoadSSL ();

	strcpy (m_szURL, pURL);
//	strcpy (m_szURL, "http://hls-iis.visualon.com:8082/hls/multibitrate/apple/gear1/fileSequence0.ts");

	int nIOFlag = VO_SOURCE2_IO_FLAG_OPEN_URL;// | VO_SOURCE2_IO_FLAG_OPEN_SYNC;
	int nRC = m_fAPI.Init (&m_fAPI.hHandle, (void *)m_szURL, nIOFlag, NULL);
	if (nRC != VO_SOURCE2_IO_OK)
	{
		YYLOGE ("IO init failed. return %08X! URL: %s", nRC, m_szURL);
		return VO_ERR_FAILED;
	}

	nRC = m_fAPI.SetParam (m_fAPI.hHandle, VO_PID_SOURCE2_WORKPATH, g_szWorkPath);
	nRC = m_fAPI.SetParam (m_fAPI.hHandle, VO_SOURCE2_IO_PARAMID_HTTPIOCALLBACK, &m_sImplCB);

	return VO_ERR_NONE;
}

int CSourceIO::Open (void)
{
	CAutoLock lock (&m_mtIO);
	int nRC = m_fAPI.Open (m_fAPI.hHandle, VO_TRUE);
	if (nRC != VO_SOURCE2_IO_OK)
	{
		YYLOGE ("IO open failed. return %08X! The URL: %s", nRC, m_szURL);
		return VO_ERR_FAILED;
	}

	int	nTryTimes = 0;
	int nRCSize = VO_SOURCE2_IO_RETRY;
	while (nRCSize == VO_SOURCE2_IO_RETRY)
	{
		nTryTimes++;
		if (nTryTimes > 300)
			break;
		yySleep (10000);
		nRCSize = m_fAPI.GetSize (m_fAPI.hHandle, &m_llFileSize);
		if (nRCSize == VO_SOURCE2_IO_RETRY)
			continue;
		else if (nRCSize != VO_SOURCE2_IO_OK)
			break;
		else if (m_llFileSize > 0)
			break;
	}
	m_bExitRead = false;

	if (nRCSize != VO_SOURCE2_IO_OK)
		YYLOGW ("IO get size failed. return %08X, Size: %d!", nRCSize, (int)m_llFileSize);

	return VO_ERR_NONE;
}

int CSourceIO::Read (VO_PBYTE pData, VO_U32 nSize, int * pRead)
{
	CAutoLock lock (&m_mtIO);
	if (m_fAPI.hHandle == NULL)
		return VO_ERR_FAILED;

	VO_U32	nRead = 0;
	VO_U32	nReadSize = 0;
	VO_U32	nRestSize = nSize;
	int		nRC = 0;
	while (nReadSize < nSize)
	{
		nRead = 0;
		nRC = m_fAPI.Read (m_fAPI.hHandle, pData, nRestSize, &nRead);
		if (nRead > 0)
		{
			nReadSize += nRead;
			pData += nRead;
			nRestSize -= nRead;
		}
		if (nRC == VO_ERR_RETRY)
			yySleep (2000);
		else if (nRC != VO_ERR_NONE)
			break;
		if (m_bExitRead)
			break;
	}
	*pRead = nReadSize;
	return nRC;
}

int CSourceIO::Seek (VO_S64 llPos)
{
	m_bExitRead = true;
	CAutoLock lock (&m_mtIO);
	VO_SOURCE2_IO_POS	posWhere = VO_SOURCE2_IO_POS_BEGIN;
	VO_S64				llSeekPos = 0;
	return m_fAPI.SetPos (m_fAPI.hHandle, llPos, posWhere, &llSeekPos);
}

VO_U32 CSourceIO::SetParam (VO_U32 uParamID , VO_PTR pParam)
{
	CAutoLock lock (&m_mtIO);
	if (m_fAPI.hHandle == NULL)
		return -1;

	return m_fAPI.SetParam (m_fAPI.hHandle, uParamID, pParam);
}

int CSourceIO::Close (void)
{
	m_bExitRead = true;
	CAutoLock lock (&m_mtIO);
	if (m_fAPI.hHandle != NULL)
	{
		m_fAPI.Close (m_fAPI.hHandle);
		m_fAPI.UnInit (m_fAPI.hHandle);
	}
	m_fAPI.hHandle = NULL;
	return VO_ERR_NONE;
}

VO_U32 CSourceIO::ioOpenNotify (VO_PTR pUserData , VO_U32 uID , VO_U32 uError , VO_PBYTE pBuf , VO_U32 uSizeDone )
{
	CSourceIO * pSource = (CSourceIO *)pUserData;

//	YYLOGT ("SourceID", "ioOpenNotify: ID: %08X ", uID);

	return 0;
}

VO_U32 CSourceIO::ioImplNotify (VO_PTR hHandle , VO_U32 uID , VO_PTR pParam1 , VO_PTR pParam2)
{
	CSourceIO * pSource = (CSourceIO *)hHandle;

//	YYLOGT ("SourceIO", "ioImplNotify: ID: %08X ", uID);

	return 0;
}

bool CSourceIO::LoadSSL (void)
{
	if (m_hSSLDll != NULL)
		return true;
	m_hSSLDll = yyLibLoad (_T("yySSL"), 0);
	if (m_hSSLDll == NULL)
		return false;
	__voGetSSLAPI pGetSSLAPI = NULL;
	pGetSSLAPI = (__voGetSSLAPI)yyLibGetAddr (m_hSSLDll, "yyGetSSLAPI", 0);
	if (pGetSSLAPI == NULL)
		return false;
	pGetSSLAPI (&m_sSSLApi);
	if (m_sSSLApi.SSL_library_init == NULL)
		return false;
	m_sSSLApi.SSL_library_init();
	m_sSSLApi.SSL_load_error_strings();

	m_fAPI.SetParam (m_fAPI.hHandle, VO_PID_SOURCE2_SSLAPI, &m_sSSLApi);
	return true;
}

bool CSourceIO::FreeSSL (void)
{
	if (m_hSSLDll != NULL)
	{
		yyLibFree (m_hSSLDll, 0);
		m_hSSLDll = NULL;
	}
	return true;
}

/*
#ifdef _YYBA_TEST_BITERATE
	int nStartTime = yyGetSysTime ();
	int nDLBitRate = 0;
#ifdef _OS_WIN32
	nDLBitRate = 0;
	if (CRegMng::g_pRegMng != NULL)
		CRegMng::g_pRegMng->GetIntValue (_T("DownloadSpeed"), 0);
#elif defined _OS_NDK
	char szBitrate[64];
	__system_property_get ("com.cansure.media.dlspeed", szBitrate);
	nDLBitRate = atol (szBitrate);
	if (nDLBitRate > 0)
		YYLOGI ("nDLBitRate is % 8d", nDLBitRate);
#endif // _OS_WIN32
#endif // _YYBA_TEST_BITERATE


#ifdef _YYBA_TEST_BITERATE
		if (nDLBitRate > 0)
		{
			while (nDLBitRate * (yyGetSysTime () - nStartTime) < (nFileSize - nRestSize) * 1000)
				yySleep (1000);
		}
#endif // _YYBA_TEST_BITERATE
*/