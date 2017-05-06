/*******************************************************************************
	File:		CExtSource.cpp

	Contains:	The ext Source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#include "CExtSource.h"

#include "CBaseKey.h"
#include "yyLog.h"

CExtSource::CExtSource(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hFile (NULL)
	, m_llSize (0)
	, m_bKeyFile (false)
	, m_nKeyLen (0)
	, m_pBuff (NULL)
	, m_nSize (0)
{
	SetObjectName ("ExtSource");
	memset (&m_extData, 0, sizeof (m_extData));
	memset (m_szSource, 0, sizeof (m_szSource));
}

CExtSource::~CExtSource(void)
{
	YY_DEL_A (m_pBuff);

	if (m_hFile != NULL)
		CloseHandle (m_hFile);
	m_hFile = NULL;
}

YY_READ_EXT_DATA * CExtSource::GetExtData (YYExtDataType nType, TCHAR * pSource)
{
	if (nType != YY_EXTDATA_Mux)
		return NULL;

	if (m_hFile != NULL)
		CloseHandle (m_hFile);

	_tcscpy (m_szSource, pSource);

	m_hFile = yyFileOpen (pSource, YYFILE_READ);
	if (m_hFile == NULL)
		return NULL;

	m_bKeyFile = false;
	m_nKeyLen = 0;
	if (CBaseKey::g_Key != NULL)
	{
		unsigned char szKeyData[64];
		yyFileRead (m_hFile, szKeyData, sizeof (szKeyData));
		m_bKeyFile = CBaseKey::g_Key->IsKeyFile (szKeyData, sizeof (szKeyData));
		if (m_bKeyFile)
			m_nKeyLen = YYKEY_LEN;
		yyFileSeek (m_hFile, m_nKeyLen, YYFILE_BEGIN);
	}
	
	DWORD dwHigh = 0;
	DWORD dwSize = GetFileSize (m_hFile, &dwHigh);
	m_llSize = dwHigh;
	m_llSize = m_llSize << 32;
	m_llSize += dwSize;
	m_llSize = m_llSize - m_nKeyLen;

	m_extData.pUser = this;
	m_extData.pRead = ReadExtData;
	m_extData.nType = nType;
	m_extData.llSize = m_llSize;

	TCHAR * pExt = _tcsrchr (pSource, _T('.'));
	_tcscpy (m_extData.szName, _T("file:\\yyextdata"));
	if (_tcslen (pExt) > 8)
		_tcscat (m_extData.szName, _T(".dat"));
	else
		_tcscat (m_extData.szName, pExt);
	m_extData.pSource = m_szSource;
	return &m_extData;
}

int CExtSource::ReadExtData (void * pUser, YY_BUFFER * pData)
{
	CExtSource * pExtData = (CExtSource *)pUser;
	if (pData->nType == YY_MEDIA_Data)
		return pExtData->ReadMuxData (pData);
	else
		return -1;
}

int CExtSource::ReadMuxData (YY_BUFFER * pData)
{
	if (pData->pBuff == NULL)
	{
		if (m_hFile != NULL)
			CloseHandle (m_hFile);
		m_hFile = NULL;
		return 0;
	}

	if (m_hFile == NULL)
	{
		pData->uSize = 0;
		return YY_ERR_FAILED;
	}
	if (pData->llTime >= 0) // Seek
		yyFileSeek (m_hFile, pData->llTime + m_nKeyLen, YYFILE_BEGIN);

	int  nRead = yyFileRead (m_hFile, pData->pBuff, pData->uSize);
	if (nRead <= 0)
		return YY_ERR_FINISH;
	if (m_bKeyFile && CBaseKey::g_Key != NULL)
		CBaseKey::g_Key->DecryptData (pData->pBuff, nRead);

	pData->uSize = nRead;
	return nRead;
}
