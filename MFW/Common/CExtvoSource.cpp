/*******************************************************************************
	File:		CExtvoSource.cpp

	Contains:	ext vo source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-01-24		Fenger			Create file

*******************************************************************************/
#include "CExtvoSource.h"

#include "stdlib.h"

#include "ULibFunc.h"
#include "yyLog.h"

typedef VO_S32 ( VO_API *pvoGetParserAPI)(VO_PARSER_API * pParser);

CExtvoSource::CExtvoSource(void * hInst)
	: CBaseSource (hInst)
	, m_pReadBuff (NULL)
	, m_nReadSize (0)
{
	SetObjectName (__FILE__);

	m_hDemux = NULL;
	memset (&m_fAPI, 0, sizeof (m_fAPI));
	memset (&m_extData, 0, sizeof (m_extData));

	m_hModule = yyLibLoad (_T("voTSParser"), 0);
	if (m_hModule != NULL)
	{
		pvoGetParserAPI pAPI = (pvoGetParserAPI) yyLibGetAddr (m_hModule, "voGetParserAPI", 0);
		if (pAPI != NULL)
			pAPI (&m_fAPI);
	}

	m_nReadSize = 18800;
	m_pReadBuff = new unsigned char[m_nReadSize];

	memset (&m_bufRead, 0, sizeof (m_bufRead));
	m_bufRead.pBuff = m_pReadBuff;

	memset (&m_bufDemux, 0, sizeof (m_bufDemux));
	m_bufDemux.pBuf = m_pReadBuff;
}

CExtvoSource::~CExtvoSource(void)
{
	Close ();

	if (m_hModule != NULL)
		yyLibFree (m_hModule, 0);
	m_hModule = NULL;

	YY_DEL_A (m_pReadBuff);
}

int CExtvoSource::Open (const TCHAR * pSource, int nType)
{
	ResetParam (0);

	if (m_fAPI.Open == NULL)
		return YY_ERR_FAILED;

	Close ();

	VO_PARSER_INIT_INFO initInfo;
	memset (&initInfo, 0, sizeof (initInfo));
	initInfo.pUserData = this;
	initInfo.pProc = demuxCallback;

	m_fAPI.Open (&m_hDemux, &initInfo);

	if ((nType & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ)
		memcpy (&m_extData, pSource, sizeof (m_extData));

	while (1)
	{
		m_bufRead.uSize = m_nReadSize;
		m_extData.pRead (m_extData.pUser, &m_bufRead);

		if (m_bufRead.uSize > 0)
		{
			m_bufDemux.nBufLen = m_bufRead.uSize;
			m_fAPI.Process (m_hDemux , &m_bufDemux);
		}
		else
			break;

		Sleep (1);

	}

	return YY_ERR_NONE;
}

int CExtvoSource::Close (void)
{
	if (m_hDemux != NULL)
		m_fAPI.Close (m_hDemux);
	m_hDemux = NULL;

	return YY_ERR_NONE;
}

int CExtvoSource::ForceClose (void)
{
	return YY_ERR_NONE;
}

int	CExtvoSource::Start (void)
{
	return YY_ERR_NONE;
}

int	CExtvoSource::Stop (void)
{
	return YY_ERR_NONE;
}

int CExtvoSource::ReadData (YY_BUFFER * pBuff)
{
	if (pBuff == NULL)
		return YY_ERR_ARG;

	while (1)
	{
		m_bufRead.uSize = m_nReadSize;
		m_extData.pRead (m_extData.pUser, &m_bufRead);

		if (m_bufRead.uSize > 0)
		{
			m_bufDemux.nBufLen = m_bufRead.uSize;
			m_fAPI.Process (m_hDemux , &m_bufDemux);
		}
		else
			break;

		Sleep (1);
	}

	return YY_ERR_NONE;
}

void CExtvoSource::demuxCallback (VO_PARSER_OUTPUT_BUFFER* pData)
{
	CExtvoSource * pSource = (CExtvoSource *)pData->pUserData;

	YYLOGI ("data %p", pData);
}

