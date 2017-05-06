/*******************************************************************************
	File:		CVVSource.cpp

	Contains:	image source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-06-02		Fenger			Create file

*******************************************************************************/
#include "CVVSource.h"

#include "USystemFunc.h"
#include "UYYDataFunc.h"
#include "UFFMpegFunc.h"

#include "yyLog.h"

CVVSource::CVVSource(void * hInst)
	: CBaseSource (hInst)
	, m_pExtSrc (NULL)
	, m_hFile (NULL)
	, m_nCodecID (AV_CODEC_ID_NONE)
	, m_nStartTime (0)
	, m_nSampleTime (0)
{
	SetObjectName ("CVVSource");
	memset (&m_pktData, 0, sizeof (m_pktData));
	m_llDuration = 5000;
}

CVVSource::~CVVSource(void)
{
	Close ();
}

int CVVSource::Open (const TCHAR * pSource, int nType)
{
	if ((nType & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ)
	{
		m_pExtSrc = (YY_READ_EXT_DATA *)pSource;
		_tcscpy (m_szSource, m_pExtSrc->szName);
	}
	else
	{
		_tcscpy (m_szSource, pSource);
		m_hFile = yyFileOpen (m_szSource, YYFILE_READ);
		if (m_hFile == NULL)
			return YY_ERR_FAILED;
	}

	m_nVideoStreamNum = 1;
	m_fmtVideo.nSourceType = YY_SOURCE_VV;
	m_fmtVideo.nCodecID = m_nCodecID;

	return YY_ERR_NONE;
}

int CVVSource::Close (void)
{
	if (m_hFile != NULL)
		yyFileClose (m_hFile);
	m_hFile = NULL;

	if (m_pExtSrc != NULL)
	{
		YY_BUFFER yyBuff;
		memset (&yyBuff, 0, sizeof (YY_BUFFER));
		m_pExtSrc->pRead (m_pExtSrc->pUser, &yyBuff);
	}
	m_pExtSrc = NULL;

	YY_DEL_A (m_pktData.data);

	return YY_ERR_NONE;
}

int CVVSource::ReadData (YY_BUFFER * pBuff)
{
	pBuff->uFlag = YYBUFF_TYPE_PACKET;
	m_nSampleTime += 1000;
	pBuff->llTime = m_nSampleTime;
	m_pktData.pts = m_nSampleTime;
	pBuff->pBuff = (unsigned char *)&m_pktData;
	pBuff->uSize = g_nPacketSize;

	return YY_ERR_NONE;
}
