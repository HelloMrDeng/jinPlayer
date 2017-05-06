/*******************************************************************************
	File:		CExtData.cpp

	Contains:	The ext render player implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CExtData.h"

#include "yyConfig.h"
#include "yyLog.h"

CExtData::CExtData(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hFile (NULL)
	, m_llSize (0)
	, m_nReadTimes (0)
	, m_nReadFiles (0)
	, m_hFileVideo (NULL)
	, m_pBuffVideo (NULL)
	, m_nSizeVideo (0)
	, m_nVideoCount (0)
	, m_hFileAudio (NULL)
	, m_pBuffAudio (NULL)
	, m_nSizeAudio (0)
	, m_nAudioCount (0)
{
	SetObjectName ("CExtData");

	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
	m_fmtVideo.nSourceType = YY_SOURCE_YY;
	m_fmtVideo.nCodecID = YY_CODEC_ID_H264;
	m_fmtVideo.nWidth = 800;
	m_fmtVideo.nHeight = 320;

	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));
	m_fmtAudio.nSourceType = YY_SOURCE_YY;
	m_fmtAudio.nCodecID = YY_CODEC_ID_AAC;
	m_fmtAudio.nSampleRate = 44100;
	m_fmtAudio.nChannels = 2;
//	m_fmtAudio.nBits = 16;

	memset (&m_extData, 0, sizeof (m_extData));
	m_extData.pFmtVideo = &m_fmtVideo;
	m_extData.pFmtAudio = &m_fmtAudio;
}

CExtData::~CExtData(void)
{
	if (m_hFile != NULL)
		CloseHandle (m_hFile);
	m_hFile = NULL;

	if (m_hFileVideo != NULL)
		CloseHandle (m_hFileVideo);
	m_hFileVideo = NULL;

	YY_DEL_A (m_pBuffVideo);

	if (m_hFileAudio != NULL)
		CloseHandle (m_hFileAudio);
	m_hFileAudio = NULL;

	YY_DEL_A (m_pBuffAudio);
}

YY_READ_EXT_DATA * CExtData::GetExtData (YYExtDataType nType)
{
	if (nType == YY_EXTDATA_Mux)
	{
		if (m_hFile == NULL)
		{
//			m_hFile = CreateFile(_T("O:\\Data\\Works\\Customers\\SinoEmbed\\2013-12-18\\BenQ.HD.Demo.HDTV.1080i(ED2000.COM).ts"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
//			m_hFile = CreateFile(_T("O:\\Data\\Works\\Customers\\TestFiles\\TS\\qcif._1.0.ts"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
//			m_hFile = CreateFile(_T("O:\\Data\\Works\\Customers\\GeniaTech\\dumpts\\oneseg1.ts"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
//			m_hFile = CreateFile(_T("O:\\Data\\Works\\Customers\\GeniaTech\\dumpts\\oneseg2.ts"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
			m_hFile = CreateFile(_T("O:\\Data\\Works\\Customers\\GeniaTech\\dumpts\\temp.ts"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
//			m_hFile = CreateFile(_T("O:\\Data\\Works\\Customers\\GeniaTech\\dumpts\\dump_mpeg.ts"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
//			m_hFile = CreateFile(_T("O:\\Data\\Works\\Customers\\GeniaTech\\dumpts\\fullseg.ts"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
			if (m_hFile == INVALID_HANDLE_VALUE)
			{
				m_hFile = NULL;
				return NULL;
			}
		}

		long lPos = 188 * 20000;
//		SetFilePointer (m_hFile, lPos, NULL, FILE_BEGIN);

		DWORD dwHigh = 0;
		DWORD dwSize = GetFileSize (m_hFile, &dwHigh);
		m_llSize = dwHigh;
		m_llSize = m_llSize << 32;
		m_llSize += dwSize;
	}

	m_extData.pUser = this;
	m_extData.pRead = ReadExtData;
	m_extData.nType = nType;
	m_extData.llSize = 0;//m_llSize;
	_tcscpy (m_extData.szName, _T("file:\\yyextdata.ts"));

	m_nReadTimes = 0;
	m_nReadDataSize = 0;

	return &m_extData;
}

int CExtData::ReadExtData (void * pUser, YY_BUFFER * pData)
{
	CExtData * pExtData = (CExtData *)pUser;
	if (pData->nType == YY_MEDIA_Data)
		return pExtData->ReadMuxData (pData);
	else if (pData->nType == YY_MEDIA_Video)
		return pExtData->ReadVideoData (pData);
	else if (pData->nType == YY_MEDIA_Audio)
		return pExtData->ReadAudioData (pData);
	else
		return -1;
}

int CExtData::ReadMuxData (YY_BUFFER * pData)
{
	if (m_hFile == NULL)
	{
		pData->uSize = 0;
		return YY_ERR_FAILED;
	}

	DWORD dwRead = 0;

	if (pData->llTime >= 0)
	{
		long lPos = (long)pData->llTime;
		long lHigh = (long)(pData->llTime >> 32);
		SetFilePointer (m_hFile, lPos, &lHigh, FILE_BEGIN);
	}

//	pData->uSize = (rand () % 10) * 1880;
//	if (pData->uSize == 0)
//		pData->uSize = 1880;
/*
	if (m_nReadTimes > 100)
	{
		m_nReadTimes = 0;
		CloseHandle (m_hFile);
		if (m_nReadFiles % 3 == 0)
			m_hFile = CreateFile(_T("O:\\Data\\Works\\Customers\\TestFiles\\TS\\qvga_1.0.ts"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
		else if (m_nReadFiles % 3 == 1)
			m_hFile = CreateFile(_T("O:\\Data\\Works\\Customers\\TestFiles\\TS\\vga_1.0.ts"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
		else
			m_hFile = CreateFile(_T("O:\\Data\\Works\\Customers\\TestFiles\\TS\\qcif._1.0.ts"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
		m_nReadFiles++;
	}
*/

	if (!ReadFile (m_hFile, pData->pBuff, pData->uSize, &dwRead, NULL))
	{
		return YY_ERR_FINISH;
	}

	pData->uSize = dwRead;
	m_nReadTimes++;

	m_nReadDataSize += dwRead;


	YYLOGI ("Read Time: % 5d, Size: % 8d", m_nReadTimes, dwRead);
//	Sleep (100);

	return dwRead;
}

int CExtData::ReadVideoData (YY_BUFFER * pData)
{
	if (m_hFileVideo == NULL)
	{
		m_hFileVideo = CreateFile(_T("C:\\Work\\TestClips\\H264'B10_1000K'1m55s'800x480'29f.264"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
		if (m_hFileVideo == INVALID_HANDLE_VALUE)
		{
			m_hFileVideo = NULL;
			return -1;
		}

		if (m_pBuffVideo == NULL)
		{
			m_nSizeVideo = GetFileSize (m_hFileVideo, NULL);
			m_pBuffVideo = new unsigned char[m_nSizeVideo];
		}

		DWORD dwRead;
		ReadFile (m_hFileVideo, m_pBuffVideo, m_nSizeVideo, &dwRead, NULL);

		m_pVideoPos = m_pBuffVideo;
	}


	DWORD dwSyncWord = 0X01000000;
	unsigned char * pNext = m_pVideoPos;

	while (pNext - m_pBuffVideo < m_nSizeVideo)
	{
		if (!memcmp (pNext, &dwSyncWord, 4))
		{
			if (pNext - m_pVideoPos > 32)
				break;
		}

		pNext++;
	}
			
	pData->pBuff = m_pVideoPos;
	pData->uSize = pNext - m_pVideoPos;
	pData->llTime = m_nVideoCount * 30;

	m_pVideoPos = pNext;

	m_nVideoCount++;

//	YYLOGI ("% 6d,    Size: % 8d", m_nVideoCount, pData->uSize);

	if (pNext - m_pBuffVideo >= m_nSizeVideo)
		m_pVideoPos = m_pBuffVideo;

	return 0;
}

int CExtData::ReadAudioData (YY_BUFFER * pData)
{
	if (m_hFileAudio == NULL)
	{
		m_hFileAudio = CreateFile(_T("C:\\Work\\TestClips\\AAC'LC_2c'48KHz.aac"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
		if (m_hFileAudio == INVALID_HANDLE_VALUE)
		{
			m_hFileAudio = NULL;
			return -1;
		}

		if (m_pBuffAudio == NULL)
		{
			m_nSizeAudio = GetFileSize (m_hFileAudio, NULL);
			m_pBuffAudio = new unsigned char[m_nSizeAudio];
		}

		DWORD dwRead;
		ReadFile (m_hFileAudio, m_pBuffAudio, m_nSizeAudio, &dwRead, NULL);

		m_pAudioPos = m_pBuffAudio;
	}

	DWORD dwSyncWord1 = 0X804CF9FF;
	DWORD dwSyncWord2 = 0X804CF9FF;

	unsigned char * pNext = m_pAudioPos;

	while (pNext - m_pBuffAudio < m_nSizeAudio)
	{
		if (!memcmp (pNext, &dwSyncWord1, 4) || !memcmp (pNext, &dwSyncWord2, 4))
		{
			if (pNext - m_pAudioPos > 32)
				break;
		}

		pNext++;
	}
			
	pData->pBuff = m_pAudioPos;
	pData->uSize = pNext - m_pAudioPos;
	pData->llTime = m_nAudioCount * 22;

	m_pAudioPos = pNext;

	m_nAudioCount++;

//	YYLOGI ("% 6d,    Size: % 8d", m_nAudioCount, pData->uSize);

	if (pNext - m_pBuffAudio >= m_nSizeAudio)
		m_pAudioPos = m_pBuffAudio;

	return 0;
}

