/*******************************************************************************
	File:		CImageSource.cpp

	Contains:	image source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-06-02		Fenger			Create file

*******************************************************************************/
#include "CImageSource.h"

#include "USystemFunc.h"
#include "UYYDataFunc.h"
#include "UFFMpegFunc.h"

#include "yyLog.h"

CImageSource::CImageSource(void * hInst)
	: CBaseSource (hInst)
	, m_pExtSrc (NULL)
	, m_hFile (NULL)
	, m_nCodecID (AV_CODEC_ID_NONE)
	, m_nStartTime (0)
	, m_nSampleTime (0)
{
	SetObjectName ("CImageSource");
	memset (&m_pktData, 0, sizeof (m_pktData));
	m_llDuration = 5000;
}

CImageSource::~CImageSource(void)
{
	Close ();
}

int CImageSource::Open (const TCHAR * pSource, int nType)
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
	TCHAR * pExt = _tcsrchr (m_szSource, _T('.'));
	if (pExt == NULL)
		return YY_MEDIA_Data;
	int nExtLen = _tcslen (pExt) * sizeof (TCHAR);
	char * pExtChar = (char *)pExt;
	for (int i = 0; i < nExtLen; i++)
	{
		if (*(pExtChar + i) >= 'A' && *(pExtChar + i) <= 'Z')
			*(pExtChar + i) += 'a' - 'A';
	}
	if (_tcsstr (pExt, _T(".png")) != NULL)
		m_nCodecID = AV_CODEC_ID_PNG;
	else if (_tcsstr (pExt, _T(".jpg")) != NULL || _tcsstr (pExt, _T(".jpeg")) != NULL)
		m_nCodecID = AV_CODEC_ID_MJPEG;
	else
		return YY_ERR_FAILED;

	m_nVideoStreamNum = 1;
	m_fmtVideo.nSourceType = YY_SOURCE_YY;
	m_fmtVideo.nCodecID = m_nCodecID;

	return YY_ERR_NONE;
}

int CImageSource::Close (void)
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

int CImageSource::ReadData (YY_BUFFER * pBuff)
{
	if (m_nStartTime == 0)
		m_nStartTime = yyGetSysTime ();
	if (m_nStartTime > 0 && yyGetSysTime () - m_nStartTime >= m_llDuration)
	{
		yySleep (10000);
		pBuff->uFlag = YYBUFF_EOS;
		pBuff->llTime = 0;
		pBuff->uSize = 0;
		return YY_ERR_FINISH;
	}
	if (m_pktData.data == NULL)
	{
		int nFileSize = 0;
		if (m_hFile != NULL)
			nFileSize = (int)yyFileSize (m_hFile);
		else
			nFileSize = (int)m_pExtSrc->llSize;
		m_pktData.size = nFileSize;
		m_pktData.data = new unsigned char[nFileSize];
		if (m_hFile != NULL)
			yyFileRead (m_hFile, m_pktData.data, nFileSize);
		else
		{
			YY_BUFFER buff;
			memset (&buff, 0, sizeof (buff));
			buff.nType = YY_MEDIA_Data;
			buff.pBuff = m_pktData.data;
			buff.uSize = nFileSize;
			m_pExtSrc->pRead (m_pExtSrc->pUser, &buff);
		}
	}

	pBuff->uFlag = YYBUFF_TYPE_PACKET;
	m_nSampleTime += 1000;
	pBuff->llTime = m_nSampleTime;
	m_pktData.pts = m_nSampleTime;
	pBuff->pBuff = (unsigned char *)&m_pktData;
	pBuff->uSize = g_nPacketSize;

	return YY_ERR_NONE;
}
