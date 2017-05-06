/*******************************************************************************
	File:		CStreamSource.cpp

	Contains:	adaption stream source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "CStreamSource.h"
#include "COutSample.h"
#include "COutSampleV2.h"

#include "ULibFunc.h"
#include "USystemFunc.h"
#include "UYYDataFunc.h"
#include "yyLog.h"

CStreamSource::CStreamSource(void * hInst)
	: CBaseSource (hInst)
	, m_pIO (NULL)
	, m_pProgInfo (NULL)
	, m_pProgFunc (NULL)
	, m_pParser (NULL)
	, m_pDLVideo (NULL)
	, m_pOutBuff (NULL)
	, m_pBA (NULL)
	, m_pDRM (NULL)
{
	SetObjectName ("CStreamSource");
	yyGetAppPath (m_hInst, g_szWorkPath, sizeof (g_szWorkPath));

	strcpy (m_szURL, "");

	m_cbParser.pUserData = this;
	m_cbParser.SendEvent = parserNotifyEvent;

	m_progNotify.pUserData = this;
	m_progNotify.fNotify = ProgInfoNotifyEvent;
}

CStreamSource::~CStreamSource(void)
{
	Close ();
}

int CStreamSource::Open (const TCHAR * pSource, int nType)
{
	int nRC = 0;
	m_bForceClosed = false;
	ResetParam (0);
	if (m_pProgInfo == NULL)
	{
		m_pProgInfo = new CProgInfo ();
		m_pProgFunc = m_pProgInfo->GetFunc ();
		m_pProgFunc->RegNotify (m_pProgFunc->hHandle, &m_progNotify, VO_TRUE);
	}
	else
		m_pProgInfo->Reset ();
	if (m_pIO == NULL)
		m_pIO = new CSourceIO ();
	if (m_pDRM == NULL)
		m_pDRM = new CStreamDRM (m_pIO->GetIO (), NULL);

	VO_ADAPTIVESTREAM_PLAYLISTDATA data;
	memset( &data , 0 , sizeof( VO_ADAPTIVESTREAM_PLAYLISTDATA ) );
	memset (m_szURL, 0, sizeof (m_szURL));
#ifdef _OS_WIN32
	WideCharToMultiByte (CP_ACP, 0, pSource, -1, m_szURL, sizeof (m_szURL), NULL, NULL);
#else
	strcpy( m_szURL, pSource);
#endif // _OS_WIN32
	if (m_pDRM != NULL)
	{
		VO_CHAR szNewURL[2048];
		nRC = m_pDRM->PreprocessURL (m_szURL, szNewURL, NULL);
		if (nRC == VO_ERR_NONE)
			strcpy (m_szURL, szNewURL);
	}
	strcpy( data.szUrl, m_szURL);
	data.pProgFunc = m_pProgFunc;

	nRC = FillPlayListData (&data);
	if (nRC != YY_ERR_NONE)
		return YY_ERR_FAILED;
	if (m_pDRM != NULL)
	{
		VO_ADAPTIVESTREAMPARSER_STREAMTYPE nStreamType = VO_ADAPTIVESTREAMPARSER_STREAMTYPE_HLS;
		m_pDRM->SetParameter (VO_PID_AS_DRM2_STREAMING_TYPE, &nStreamType);
		m_pDRM->Info (m_szURL, data.pData, data.uDataSize, NULL);
	}

	if (m_pParser != NULL)
		delete m_pParser;
	m_pParser = new CStreamParser (VO_ADAPTIVESTREAMPARSER_STREAMTYPE_HLS);
	m_pParser->SetProgInfoFunc (m_pProgFunc);

	strcpy( data.szNewUrl, m_szURL);
	nRC = m_pParser->Init (&data, &m_cbParser);
	if (data.pData != NULL)
		delete []data.pData;
	if (nRC != VO_RET_SOURCE2_OK)
	{
		YYLOGE ("Init Parser error return %08X", nRC);
		return YY_ERR_FAILED;
	}

	nRC = m_pParser->Open ();
	if (nRC != VO_RET_SOURCE2_OK)
	{
		YYLOGE ("Open Parser error return %08X", nRC);
		return YY_ERR_FAILED;
	}
	m_pProgInfo->DumpInfo (_T("C:\\Temp\\yyProgInfo.txt"));

	if (m_pOutBuff == NULL)
//		m_pOutBuff = new COutSample ();
		m_pOutBuff = new COutSampleV2 ();

	nRC = m_pParser->Start ();

	if (m_pDLVideo == NULL)
		m_pDLVideo = new CStreamDownLoad (m_pParser);
	m_pDLVideo->SetProgInfoFunc (m_pProgFunc);
	m_pDLVideo->SetOutBuffer (m_pOutBuff);
	m_pDLVideo->SetStreamDRM (m_pDRM);

	if (m_pBA == NULL)
		m_pBA = new CStreamBA ();
	m_pBA->SetParser (m_pParser);
	m_pBA->SetOutBuffer (m_pOutBuff);
	m_pBA->SetProgInfo (m_pProgFunc);

	m_pDLVideo->SetStreamBA (m_pBA);
	m_pOutBuff->SetStreamBA (m_pBA);

	m_pDLVideo->Start (VO_SOURCE2_ADAPTIVESTREAMING_VIDEO);

	long long	llDur = 0;
	VO_U32		uBuffCount = 0;
	int			nStartTime = yyGetSysTime ();
	while (uBuffCount <= 0)
	{
		if (yyGetSysTime () - nStartTime > 60000)
		{
			YYLOGW ("It can't get data in waiting time!");
			return YY_ERR_FAILED;
		}
		if (m_bForceClosed)
			return YY_ERR_FAILED;

		m_pOutBuff->GetBufferInfo (VO_SOURCE2_TT_AUDIO, &llDur, &uBuffCount);
		if (uBuffCount <= 0)
		{
			yySleep (10000);
			continue;
		}
//		m_pOutBuff->GetBufferInfo (VO_SOURCE2_TT_VIDEO, &llDur, &uBuffCount);
//		if (uBuffCount <= 0)
//			continue;
	}

	if (m_pOutBuff->GetVideoFormat ()->nCodecID > 0)
	{
		m_nVideoStreamNum = 1;
		yyDataCloneVideoFormat (&m_fmtVideo, m_pOutBuff->GetVideoFormat ());
	}
	if (m_pOutBuff->GetAudioFormat ()->nCodecID > 0)
	{
		m_nAudioStreamNum = 1;
		yyDataCloneAudioFormat (&m_fmtAudio, m_pOutBuff->GetAudioFormat ());
	}

	nRC = m_pParser->GetDuration ((VO_U64 *)&m_llDuration);

	return YY_ERR_NONE;
}

int CStreamSource::Close (void)
{
	Stop ();

	YY_DEL_P (m_pBA);

	YY_DEL_P (m_pDLVideo);

	YY_DEL_P (m_pOutBuff);

	YY_DEL_P (m_pParser);

	YY_DEL_P (m_pDRM);

	YY_DEL_P (m_pIO);

	YY_DEL_P (m_pProgInfo);

	m_pProgFunc = NULL;

	return YY_ERR_NONE;
}

int CStreamSource::ForceClose (void)
{
	m_bForceClosed = true;
	return YY_ERR_NONE;
}

int	CStreamSource::Start (void)
{
	return YY_ERR_NONE;
}

int	CStreamSource::Stop (void)
{
	if (m_pParser != NULL)
		m_pParser->Stop ();

	if (m_pDLVideo != NULL)
		m_pDLVideo->Stop ();

	return YY_ERR_NONE;
}

int CStreamSource::ReadData (YY_BUFFER * pBuff)
{
	if (m_pOutBuff == NULL)
		return YY_ERR_FAILED;

	int nRC = m_pOutBuff->GetSample (pBuff);
	if (nRC != YY_ERR_NONE)
		return nRC;

	if (pBuff->nType == YY_MEDIA_Video)
	{
		if ((pBuff->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
			yyDataCloneVideoFormat (&m_fmtVideo, (YY_VIDEO_FORMAT *)pBuff->pFormat);
		if (m_bVideoNewPos)
		{
//			if (pBuff->llTime < m_llSeekPos)
//				return YY_ERR_RETRY;
//			if ((pBuff->uFlag & YYBUFF_KEY_FRAME) != YYBUFF_KEY_FRAME)
//				return YY_ERR_RETRY;

			pBuff->uFlag |= YYBUFF_NEW_POS;
			m_bVideoNewPos = false;
		}
	}
	else if (pBuff->nType == YY_MEDIA_Audio)
	{
		if ((pBuff->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
			yyDataCloneAudioFormat (&m_fmtAudio, (YY_AUDIO_FORMAT *)pBuff->pFormat);
		if (m_bAudioNewPos)
		{
//			if (pBuff->llTime < m_llSeekPos)
//				return YY_ERR_RETRY;

			pBuff->uFlag |= YYBUFF_NEW_POS;
			m_bAudioNewPos = false;
		}
	}

	return nRC;
}

int CStreamSource::SetPos (long long llPos)
{
	CBaseSource::SetPos (llPos);

	if (m_pDLVideo != NULL)
		m_pDLVideo->SetPos (llPos);

	return YY_ERR_NONE;
}

int CStreamSource::GetMediaInfo (TCHAR * pInfo, int nSize)
{
	memset (pInfo, 0, nSize);
#ifdef _OS_WIN32
	VO_SOURCE2_STREAM_INFO * pStrmInfo = NULL;
	VO_SOURCE2_PROGRAM_INFO * pProg = m_pProgInfo->GetProg ();
	TCHAR * pLine = pInfo;
	_stprintf (pLine, _T("Stream Num: %d\r\n"), pProg->uStreamCount);
	for (int i = 0; i < pProg->uStreamCount; i++)
	{
		pLine = pLine + _tcslen (pLine);
		pStrmInfo = pProg->ppStreamInfo[i];
		_stprintf (pLine, _T("Stream %d:  -  % 2d     %d\r\n"), i, pStrmInfo->uSelInfo, pStrmInfo->uBitrate);
	}
	pLine = pLine + _tcslen (pLine);
	_stprintf (pLine, _T("\r\nVideo: % 6d X %d \r\n"), m_fmtVideo.nWidth, m_fmtVideo.nHeight);
	pLine = pLine + _tcslen (pLine);
	_stprintf (pLine, _T("Audio: % 6d X %d \r\n"), m_fmtAudio.nSampleRate, m_fmtAudio.nChannels);
#endif // _OS_WIN32
	return YY_ERR_NONE;
}

int g_nIndex = 0;

int CStreamSource::FillPlayListData (VO_ADAPTIVESTREAM_PLAYLISTDATA * pData)
{
	if (pData == NULL)
		return VO_ERR_INVALID_ARG;

	pData->pData = NULL;
	pData->uDataSize = 0;
	pData->uFullDataSize = 0;

	int nRC = 0;
	VO_CHAR szURL[2048];
	memset (szURL, 0, sizeof (szURL));
	if (strstr (pData->szUrl, "http") == pData->szUrl)
	{
		strcpy (szURL, pData->szUrl);
	}
	else
	{
		VO_CHAR szSubFolder[1024];
		strcpy (szSubFolder, "");
		VO_CHAR * pPos = strrchr (pData->szUrl, '/');
		if (pPos != NULL && pPos != pData->szUrl)
		{
			strncpy (szSubFolder, pData->szUrl, pPos - pData->szUrl);
			pPos = strstr (pData->szRootUrl, szSubFolder);
			if (pPos != NULL)
				strncpy (szURL, pData->szRootUrl, pPos - pData->szRootUrl);
		}
		if (strlen (szURL) <= 0)
		{
			pPos = strrchr (pData->szRootUrl, '/');
			if (pPos != NULL)
				strncpy (szURL, pData->szRootUrl, pPos - pData->szRootUrl);
			else
				strcpy (szURL, pData->szRootUrl);
		}
		if (pData->szUrl[0] != '/' && pData->szUrl[1] != ':')
			strcat (szURL, "/");
		strcat (szURL, pData->szUrl);
	}
	strcpy (pData->szNewUrl, szURL);

//	int nStartTime = yyGetSysTime ();

	nRC = m_pIO->Init (szURL, 0);
	if (nRC != VO_ERR_NONE)
	{
		YYLOGE ("IO Init error return %08X", nRC);
		return nRC;
	}
	nRC = m_pIO->Open ();
	if (nRC != VO_ERR_NONE)
	{
		YYLOGE ("IO Open error return %08X", nRC);
		return nRC;
	}

	int nRead = 0;
	int nSize = (int)m_pIO->GetSize ();
	VO_PBYTE pBuff = NULL;
	pBuff = new VO_BYTE[nSize * 2];
	nRC = m_pIO->Read (pBuff, nSize, &nRead);
	nRC = m_pIO->Close ();
	if (nRC < 0)
	{
		delete []pBuff;
		YYLOGE ("IO Read error return %08X", nRC);
		return VO_RET_SOURCE2_FAIL;
	}

	VO_U32 nNewSize = nRead;
	if (m_pDRM != NULL)
	{
		nRC = m_pDRM->PreprocessPlaylist (pBuff, nSize * 2, &nNewSize, NULL);
		if (nRC == VO_ERR_OUTPUT_BUFFER_SMALL)
		{
			VO_PBYTE pNewBuff = new VO_BYTE[nNewSize];
			memcpy (pNewBuff, pBuff, nSize);
			delete []pBuff;
			nRC = m_pDRM->PreprocessPlaylist (pNewBuff, nNewSize, &nNewSize, NULL);
			pBuff = pNewBuff;
		}
	}

	pData->pData = pBuff;
	pData->uDataSize = nNewSize;
	pData->uFullDataSize = nNewSize;

//	YYLOGI ("Download file %s used %d", strrchr (szURL, '/'), yyGetSysTime () - nStartTime);

#if 0
	TCHAR szDumpFile[1024];
	DWORD dwWrite = 0;
	_stprintf (szDumpFile, _T("C:\\Temp\\Dump_%d.m3u8"), g_nIndex);
	g_nIndex++;
	HANDLE hFile = CreateFile(szDumpFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, (DWORD) 0, NULL);
	WriteFile (hFile, pBuff, nSize, &dwWrite, NULL);
	CloseHandle (hFile);
#endif // 0
	return VO_ERR_NONE;
}

VO_S32 CStreamSource::parserNotifyEvent (VO_PTR pUserData, VO_U32 nID, VO_U32 nParam1, VO_U32 nParam2)
{
	CStreamSource * pSource = (CStreamSource *)pUserData;
	if (nID == VO_EVENTID_SOURCE2_ADAPTIVESTREAMING_NEEDPARSEITEM)
	{
		VO_ADAPTIVESTREAM_PLAYLISTDATA * pData = (VO_ADAPTIVESTREAM_PLAYLISTDATA *)nParam1;
		return pSource->FillPlayListData (pData);
	}
		
	return 0;
}

VO_S32 CStreamSource::ProgInfoNotifyEvent (VO_PTR pUserData, VO_U32 nID, VO_U32 uStatus, VO_PTR pValue)
{
	CStreamSource * pSource = (CStreamSource *)pUserData;

	int nRC = VO_ERR_NONE;
	switch (nID)
	{
	case VOCB_PROGINFO_UPDATE_STREAM:
	case VOCB_PROGINFO_UPDATE_TRACK:
	case VOCB_PROGINFO_UPDATE_PROGTYPE:
		break;

	case VOCB_PROGINFO_UPDATE_RESET:
		break;

	default:
		break;
	}

	return nRC;
}
