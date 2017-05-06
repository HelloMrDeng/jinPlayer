/*******************************************************************************
	File:		CStreamDownLoad.cpp

	Contains:	stream download implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#include "CStreamDownLoad.h"

#include "CStreamDemuxTS.h"
#include "CSourceIO.h"
#include "CStreamParser.h"
#include "COutBuffer.h"
#include "CStreamBA.h"
#include "CStreamDRM.h"
#ifdef _OS_WIN32
#include "CRegMng.h"
#elif defined _OS_NDK
#include <sys/system_properties.h>
#endif // _OS_WIN32

#include "USystemFunc.h"
#include "yyCfgBA.h"
#include "yyLog.h"

CStreamDownLoad::CStreamDownLoad(CStreamParser * pParser)
	: m_pParser (pParser)
	, m_pDemux (NULL)
	, m_pOutBuffer (NULL)
	, m_pBA (NULL)
	, m_pDRM (NULL)
	, m_pProgFunc (NULL)
	, m_nChunkType (VO_SOURCE2_ADAPTIVESTREAMING_UNKNOWN)
	, m_nStreamFlag (0)
	, m_bEOS (false)
	, m_bStreamChanged (false)
	, m_hThread (NULL)
	, m_nStatus (YY_PLAY_Init)
	, m_bWorking (false)
	, m_pBuffChunk (NULL)
	, m_nReadSize (YYCFG_BA_READONE_SIZE)
	, m_nDLChunkCount (0)
{
	SetObjectName ("CStreamDownLoad");
	m_pIO = new CSourceIO ();
	m_sProgNotify.pUserData = this;
	m_sProgNotify.fNotify = ProgInfoNotifyEvent;
}	

CStreamDownLoad::~CStreamDownLoad(void)
{
	delete m_pIO;

	if (m_pBuffChunk != NULL)
		delete []m_pBuffChunk;

	if (m_pDemux != NULL)
		delete m_pDemux;

	if (m_pProgFunc != NULL)
		m_pProgFunc->RegNotify (m_pProgFunc->hHandle, &m_sProgNotify, VO_FALSE);
}

VO_VOID CStreamDownLoad::Start(int nChunkType)
{
	m_nChunkType = nChunkType;

	m_nStatus = YY_PLAY_Run;
	if (m_pBuffChunk == NULL)
		m_pBuffChunk = new VO_BYTE[m_nReadSize];

	m_pOutBuffer->SetEOS (m_bEOS);

	if (m_hThread == NULL)
	{
		int nID = 0;
		yyThreadCreate (&m_hThread, &nID, DownLoadProc, this, 0);
	}
}

VO_VOID CStreamDownLoad::Stop()
{
	if (m_pIO != NULL)
		m_pIO->Close ();

	m_nStatus = YY_PLAY_Stop;

	while (m_hThread != NULL)
	{
		yySleep (10000);
	}
}

void CStreamDownLoad::SetProgInfoFunc (VOS2_ProgramInfo_Func * pFunc)
{
	m_pProgFunc = pFunc;
	if (m_pProgFunc != NULL)
		m_pProgFunc->RegNotify (m_pProgFunc->hHandle, &m_sProgNotify, VO_TRUE);
}

int	CStreamDownLoad::SetPos (long long llPos)
{
	if (m_pIO != NULL)
		m_pIO->Close ();

	YYPLAY_STATUS nCurStatus = m_nStatus;
	if (nCurStatus == YY_PLAY_Run)
	{
		m_nStatus = YY_PLAY_Pause;
		while (m_bWorking)
		{
			yySleep (10000);
		}
	}

	int		nRC = YY_ERR_NONE;
	VO_U64	llSeekPos = llPos;
	if (m_pParser != NULL)
        nRC = m_pParser->Seek (&llSeekPos, VO_ADAPTIVESTREAMPARSER_SEEKMODE_OBSOLUTE);

	if (m_pDemux != NULL) 
		m_pDemux->SetPos (llPos);

	if (m_pOutBuffer != NULL)
		nRC = m_pOutBuffer->SetPos (llPos);

	m_nStreamFlag = YYBUFF_NEW_POS;
	m_bEOS = false;
	m_pOutBuffer->SetEOS (m_bEOS);

	if (nCurStatus == YY_PLAY_Run)
		m_nStatus = YY_PLAY_Run;

	return YY_ERR_NONE;
}

int CStreamDownLoad::Download (VO_ADAPTIVESTREAMPARSER_CHUNK * pChunk)
{
	if (pChunk->uFlag & VO_ADAPTIVESTREAMPARSER_CHUNKFLAG_FORMATCHANGE)
	{
		m_nStreamFlag |= YYBUFF_NEW_FORMAT;
		if (m_pDemux != NULL)
			m_pDemux->SetStartTime (pChunk->ullStartTime);
	}

	VO_CHAR szURL[2048];
	memset (szURL, 0, sizeof (szURL));
	if (strstr (pChunk->szUrl, "http") == pChunk->szUrl)
	{
		strcpy (szURL, pChunk->szUrl);
	}
	else
	{
		VO_CHAR szSubFolder[1024];
		strcpy (szSubFolder, "");
		VO_CHAR * pPos = strrchr (pChunk->szUrl, '/');
		if (pPos != NULL && pPos != pChunk->szUrl)
		{
			strncpy (szSubFolder, pChunk->szUrl, pPos - pChunk->szUrl);
			pPos = strstr (pChunk->szRootUrl, szSubFolder);
			if (pPos != NULL)
				strncpy (szURL, pChunk->szRootUrl, pPos - pChunk->szRootUrl);
		}
		if (strlen (szURL) <= 0)
		{
			pPos = strrchr (pChunk->szRootUrl, '/');
			if (pPos != NULL)
				strncpy (szURL, pChunk->szRootUrl, pPos - pChunk->szRootUrl);
			else
				strcpy (szURL, pChunk->szRootUrl);
		}
		if (pChunk->szUrl[0] != '/' && pChunk->szUrl[1] != ':')
			strcat (szURL, "/");
		strcat (szURL, pChunk->szUrl);
	}

	if (m_pBA != NULL)
		m_pBA->DownloadStart (pChunk);

	int nRC = m_pIO->Init (szURL, 0);
	if (nRC != VO_ERR_NONE)
	{
		if (m_pBA != NULL)
			m_pBA->DownloadFinish (0, nRC);
		return nRC;
	}

	//YYLOGI ("Chunk StartTime % 8d, Offset: % 8d,  Size: % 8d", (int)pChunk->ullStartTime, (int)pChunk->ullChunkOffset,  (int)pChunk->ullChunkSize);
	if ((pChunk->ullChunkOffset != YY_64_MAXVAL && pChunk->ullChunkOffset > 0) || 
		(pChunk->ullChunkOffset != YY_64_MAXVAL && pChunk->ullChunkSize > 0))
	{
		VO_SOURCE2_IO_HTTPRANGE range;
		range.ullOffset = pChunk->ullChunkOffset;
		range.ullLength = pChunk->ullChunkSize;
		m_pIO->SetParam (VO_SOURCE2_IO_PARAMID_HTTPRANGE, &range);
	}

	nRC = m_pIO->Open ();
	if (nRC != VO_ERR_NONE)
	{
		if (m_pBA != NULL)
			m_pBA->DownloadFinish (0, nRC);
		return nRC;
	}

	if (m_pDRM != NULL)
		m_pDRM->DataBegin(m_nChunkType, pChunk->pChunkDRMInfo);

	VO_S64 nFileSize = m_pIO->GetSize ();
	if (nFileSize <= 0)
		nFileSize = 0X7FFFFFFF;
	VO_S64	nRestSize = nFileSize;
	int		nReadSize = 0;
	VO_PBYTE	pDrmBuff = NULL;
	VO_U32		nDrmSize = 0;
	VO_BOOL		bChunkEnd = VO_FALSE;
	while (nRestSize > 0)
	{
		nRC = m_pIO->Read (m_pBuffChunk, m_nReadSize, &nReadSize);
		if (m_pBA != NULL)
			m_pBA->Downloading (nReadSize);
		if (nReadSize > 0)
		{
			if (m_pDRM != NULL)
			{
				pDrmBuff = NULL;
				nDrmSize = nReadSize;
				bChunkEnd = (nRestSize ==  nReadSize) ? VO_TRUE : VO_FALSE;
				m_pDRM->DataProcess_Chunk (m_nChunkType, nFileSize - nRestSize, m_pBuffChunk, nReadSize, 
											&pDrmBuff, &nDrmSize, 
											bChunkEnd, pChunk->pChunkDRMInfo);
				if (pDrmBuff == NULL)
					Demux (m_nStreamFlag, m_pBuffChunk, nDrmSize);
				else
					Demux (m_nStreamFlag, pDrmBuff, nDrmSize);
			}
			else
			{
				Demux (m_nStreamFlag, m_pBuffChunk, nReadSize);
			}
			m_nStreamFlag = 0;
			nRestSize -= nReadSize;
		}
		if (nRC == VO_ERR_RETRY)
			yySleep (2000);
		else if (nRC != VO_ERR_NONE)
			break;
		if (m_nStatus != YY_PLAY_Run)
			break;
		if (m_bStreamChanged)
			break;
	}
	if (m_pDRM != NULL)
		m_pDRM->DataEnd(m_nChunkType, pChunk->pChunkDRMInfo);
	nRC = m_pIO->Close ();
	if (m_pBA != NULL)
		m_pBA->DownloadFinish ((int)nFileSize, YY_ERR_NONE);

	return YY_ERR_NONE;
}

int CStreamDownLoad::Demux (int nFlag, VO_PBYTE pBuff, VO_U32 nSize)
{
	if (m_pDemux == NULL) 
	{
		m_pDemux = new CStreamDemuxTS (); 
		m_pDemux->SetOutBuffer (m_pOutBuffer);
	}

	m_pDemux->Demux (nFlag, pBuff, nSize);

	return 0;
}

int CStreamDownLoad::DownLoadLoop (void)
{
	VO_ADAPTIVESTREAMPARSER_CHUNK * pChunk = NULL;
	int								nRC = 0;

	while (m_nStatus == YY_PLAY_Run || m_nStatus == YY_PLAY_Pause)
	{
		m_bWorking = false;
		if (m_nStatus == YY_PLAY_Pause)
		{
			yySleep (10000);
			continue;
		}

		m_bWorking = true;
		m_bStreamChanged = false;
		m_nDLChunkCount++;

		pChunk = NULL;
		nRC = m_pParser->GetChunk ((VO_SOURCE2_ADAPTIVESTREAMING_CHUNKTYPE)m_nChunkType, &pChunk);
		if (nRC == VO_RET_SOURCE2_END)
		{
			if (!m_bEOS)
				m_pOutBuffer->SetEOS (true);

			m_bEOS = true;
			yySleep (10000);
			continue;
		}
		else if (nRC == VO_RET_SOURCE2_NEEDRETRY)
		{
			yySleep (5000);
			continue;
		}
		if (pChunk == NULL)
		{
			yySleep (10000);
			continue;
		}

		if (m_nDLChunkCount == 1)
			pChunk->uFlag = 0;

		Download (pChunk);

		VO_S64	llBuffTime = 0;
		VO_U32	nBuffCount = 0;
		while (m_nStatus == YY_PLAY_Run)
		{
			m_pOutBuffer->GetBufferInfo (VO_SOURCE2_TT_VIDEO, &llBuffTime, &nBuffCount);
			if (llBuffTime > YYCFG_BA_BUFFER_TIME)
				yySleep (10000);
			else
				break;

			if (m_bStreamChanged)
				break;
		}
	}

	m_hThread = NULL;

	return 0;
}

VO_S32 CStreamDownLoad::ProgInfoNotifyEvent (VO_PTR pUserData, VO_U32 nID, VO_U32 uStatus, VO_PTR pValue)
{
	CStreamDownLoad * pDL = (CStreamDownLoad *)pUserData;

	if (nID == VOCB_PROGINFO_UPDATE_STREAM && uStatus == VOS2_PROGINFO_SELECT)
	{
		pDL->m_bStreamChanged = true;
	}

	return VO_ERR_NONE;
}

int CStreamDownLoad::DownLoadProc (void * pParam)
{
	CStreamDownLoad * pDL = (CStreamDownLoad *) pParam;

	return pDL->DownLoadLoop ();
}
