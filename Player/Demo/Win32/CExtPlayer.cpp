/*******************************************************************************
	File:		CExtPlayer.cpp

	Contains:	The ext render player implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CExtPlayer.h"

#include "yyConfig.h"
#include "yyLog.h"

CExtPlayer::CExtPlayer(void * hInst, HWND hWnd)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hWnd (hWnd)
	, m_pPlayer (NULL)
	, m_bStop (true)
	, m_bPause (false)
	, m_hThreadAudio (NULL)
	, m_pRndAudio (NULL)
	, m_pPCMBuff (NULL)
	, m_nPCMSize (0)
	, m_hThreadVideo (NULL)
	, m_hWinDC(NULL)
	, m_hMemDC(NULL)
	, m_hBmpVideo (NULL)
	, m_pBmpBuff (NULL)
	, m_pBmpInfo (NULL)
	, m_nBmpW (0)
	, m_nBmpH (0)
	, m_hBmpOld (NULL)
{
	SetObjectName ("CExtPlayer");

	memset (&m_bufBoxAudio, 0, sizeof (m_bufBoxAudio));
	memset (&m_bufAudioRnd, 0, sizeof (m_bufAudioRnd));
	memset (&m_bufBoxVideo, 0, sizeof (m_bufBoxVideo));
	memset (&m_bufVideoDraw, 0, sizeof (m_bufVideoDraw));

	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));
}

CExtPlayer::~CExtPlayer(void)
{
	Stop ();

	ReleaseResBMP ();

	ReleaseAudio ();
}

int CExtPlayer::SetPlayer (COMBoxMng * pPlayer)
{
	ReleaseResBMP ();
	ReleaseAudio ();

	m_pPlayer = pPlayer;

	m_cbData.funcCB = yyMediaDataCB;
	m_cbData.userData = this;
//	m_pPlayer->SetParam (YYPLAY_PID_DataCB, &m_cbData);

	int nOffsetTime = 200;
	m_pPlayer->SetParam (YYPLAY_PID_Clock_OffTime, (void *)nOffsetTime);

	m_bufBoxVideo.nType = YY_MEDIA_Video;
	m_bufBoxAudio.nType = YY_MEDIA_Audio;

	return YY_ERR_NONE;
}

int	CExtPlayer::Start (void)
{
	m_bStop = false;
	m_bPause = false;

	if (m_hThreadAudio == NULL)
	{
		DWORD dwID = 0;
		m_hThreadAudio = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) PlayAudioProc, this, 0, &dwID);
	}

	if (m_hThreadVideo == NULL)
	{
		DWORD dwID = 0;
		m_hThreadVideo = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) PlayVideoProc, this, 0, &dwID);
	}

	return YY_ERR_NONE;
}

int CExtPlayer::Pause (void)
{
	m_bPause = true;

	return YY_ERR_NONE;
}

int	CExtPlayer::Stop (void)
{
	m_bStop = true;
	while (m_hThreadAudio != NULL || m_hThreadVideo != NULL)
	{
		Sleep (10);
	}

	return YY_ERR_NONE;
}

int CExtPlayer::SetPos (int nPos)
{
	return 0;

	if (m_bPause)
	{
		while (PlayVideoLoop () != YY_ERR_NONE)
		{
			Sleep (1);
			if (!m_bPause)
				break;
		}
	}

	return YY_ERR_NONE;
}

int CExtPlayer::PlayAudioProc (void * pParam)
{
	CExtPlayer * pExt = (CExtPlayer *)pParam;

	while (!pExt->m_bStop)
	{
		if (pExt->m_bPause)
		{
			Sleep (10);
			continue;
		}

		pExt->PlayAudioLoop ();
	}

	pExt->m_hThreadAudio = NULL;

	return 0;
}

int CExtPlayer::PlayVideoProc (void * pParam)
{
	CExtPlayer * pExt = (CExtPlayer *)pParam;

	while (!pExt->m_bStop)
	{
		if (pExt->m_bPause)
		{
			Sleep (10);
			continue;
		}

		if (pExt->PlayVideoLoop () != YY_ERR_NONE)
			Sleep (10);
	}

	pExt->m_hThreadVideo = NULL;

	return 0;
}

int CExtPlayer::PlayAudioLoop (void)
{
	int nRC = m_pPlayer->GetParam (YYPLAY_PID_ReadData, &m_bufBoxAudio);
	if (nRC != YY_ERR_NONE)
	{
		Sleep (2);
		return nRC;
	}

	if ((m_bufBoxAudio.uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
		ReleaseAudio ();

	if (m_pRndAudio == NULL)
	{
		if (m_pPCMBuff != NULL)
			delete []m_pPCMBuff;

		YY_AUDIO_FORMAT * pFmtAudio = NULL;
		m_pPlayer->GetParam (YYPLAY_PID_Fmt_Audio, &pFmtAudio);
		memcpy (&m_fmtAudio, pFmtAudio, sizeof (m_fmtAudio));
		m_nPCMSize = pFmtAudio->nSampleRate * 10;
		m_pPCMBuff = new unsigned char[m_nPCMSize];

		m_bufAudioRnd.nType = YY_MEDIA_Audio;

		m_pRndAudio = new CWaveOutRnd (m_hInst, false);
		m_pRndAudio->Init (pFmtAudio);
		m_pRndAudio->Start ();
	}

	m_bufAudioRnd.uFlag = YYBUFF_TYPE_PPOINTER;
	m_bufAudioRnd.pBuff = (unsigned char *)&m_pPCMBuff;
	m_bufAudioRnd.uSize = m_nPCMSize;
	m_bufAudioRnd.pFormat = &m_fmtAudio;

	m_dataConvertAudio.pSource = &m_bufBoxAudio;
	m_dataConvertAudio.pTarget = &m_bufAudioRnd;
	m_dataConvertAudio.pZoom = NULL;
	nRC = m_pPlayer->GetParam (YYPLAY_PID_ConvertData, &m_dataConvertAudio);
	if (nRC != YY_ERR_NONE)
		return YY_ERR_FAILED;
	
	m_bufAudioRnd.uFlag = YYBUFF_TYPE_DATA;
	m_bufAudioRnd.pBuff = m_pPCMBuff;

	nRC = m_pRndAudio->Render (&m_bufAudioRnd);
	while (nRC == YY_ERR_RETRY)
	{
		Sleep (5);
		nRC = m_pRndAudio->Render (&m_bufAudioRnd);
	}

	return YY_ERR_NONE;
}

int CExtPlayer::PlayVideoLoop (void)
{
	int nRC = YY_ERR_NONE;

	nRC = m_pPlayer->GetParam (YYPLAY_PID_ReadData, &m_bufBoxVideo);
	if (nRC != YY_ERR_NONE)
		return nRC;

	if (m_hBmpVideo == NULL)
	{
		YY_VIDEO_FORMAT * pFmtVideo = NULL;
		m_pPlayer->GetParam (YYPLAY_PID_Fmt_Video, &pFmtVideo);
		if (!CreateResBMP (pFmtVideo->nWidth, pFmtVideo->nHeight))
			return YY_ERR_FAILED;
	}

	nRC = m_pPlayer->GetParam (YYPLAY_PID_ConvertData, &m_dataConvertVideo);
	if (nRC != YY_ERR_NONE)
		return YY_ERR_FAILED;

	RECT rcWnd;
	GetClientRect (m_hWnd, &rcWnd);
//	BitBlt(m_hWinDC, 0, 0, rcWnd.right, rcWnd.bottom, m_hMemDC, 0, 0, SRCCOPY);
	StretchBlt (m_hWinDC, 0, 0, rcWnd.right, rcWnd.bottom, 
				m_hMemDC, 0, 0, m_nBmpW, m_nBmpH, SRCCOPY);

	return YY_ERR_NONE;
}

bool CExtPlayer::CreateResBMP (int nW, int nH)
{
	if (m_pBmpInfo != NULL)
		return true;

	m_nBmpW = nW;
	m_nBmpH = nH;

	int nBmpSize = sizeof(BITMAPINFOHEADER);
	m_pBmpInfo = new BYTE[nBmpSize];
	memset (m_pBmpInfo, 0, nBmpSize);

	BITMAPINFO * pBmpInfo = (BITMAPINFO *)m_pBmpInfo;
	pBmpInfo->bmiHeader.biSize			= nBmpSize;
	pBmpInfo->bmiHeader.biWidth			= nW;
	pBmpInfo->bmiHeader.biHeight		= -nH;
	pBmpInfo->bmiHeader.biBitCount		= 32;
	pBmpInfo->bmiHeader.biCompression	= BI_RGB;
	pBmpInfo->bmiHeader.biXPelsPerMeter	= 0;
	pBmpInfo->bmiHeader.biYPelsPerMeter	= 0;
	pBmpInfo->bmiHeader.biPlanes		= 1;

	pBmpInfo->bmiHeader.biSizeImage	= m_nBmpW * m_nBmpH * 4;

	if (m_hWinDC == NULL)
	{
		m_hWinDC = GetDC (m_hWnd);
		m_hMemDC = ::CreateCompatibleDC (m_hWinDC);
	}

	m_hBmpVideo = CreateDIBSection(m_hWinDC , (BITMAPINFO *)m_pBmpInfo , DIB_RGB_COLORS , (void **)&m_pBmpBuff, NULL , 0);
	if (m_pBmpBuff != NULL)
		memset (m_pBmpBuff, 0, ((BITMAPINFO *)m_pBmpInfo)->bmiHeader.biSizeImage);

	m_hBmpOld = (HBITMAP)SelectObject (m_hMemDC, m_hBmpVideo);

	m_bufVideoData.pBuff[0] = m_pBmpBuff;
	m_bufVideoData.nStride[0] = m_nBmpW * 4;
	m_bufVideoData.nType = YY_VDT_RGBA;
	m_bufVideoData.nWidth = m_nBmpW;
	m_bufVideoData.nHeight = m_nBmpH;

	m_bufVideoDraw.uFlag = YYBUFF_TYPE_VIDEO;
	m_bufVideoDraw.pBuff = (unsigned char *)&m_bufVideoData;
	m_bufVideoDraw.uSize = sizeof (m_bufVideoData);

	m_dataConvertVideo.pSource = &m_bufBoxVideo;
	m_dataConvertVideo.pTarget = &m_bufVideoDraw;
	m_dataConvertVideo.pZoom = NULL;

	return true;
}

bool CExtPlayer::ReleaseResBMP (void)
{
	if (m_pBmpInfo != NULL)
		delete []m_pBmpInfo;
	m_pBmpInfo = NULL;

	if (m_hBmpOld != NULL && m_hMemDC != NULL)
		SelectObject (m_hMemDC, m_hBmpOld);

	if (m_hBmpVideo != NULL)
		DeleteObject (m_hBmpVideo);
	m_hBmpVideo = NULL;

	if (m_hWinDC != NULL)
	{
		DeleteDC (m_hMemDC);
		ReleaseDC (m_hWnd, m_hWinDC);
	}

	m_hMemDC = NULL;
	m_hWinDC = NULL;

	return true;
}

bool CExtPlayer::ReleaseAudio (void)
{
	if (m_pRndAudio != NULL)
	{
		m_pRndAudio->Stop ();
		m_pRndAudio->Uninit ();
		delete m_pRndAudio;
	}
	m_pRndAudio = NULL;

	if (m_pPCMBuff != NULL)
		delete []m_pPCMBuff;
	m_pPCMBuff = NULL;
	m_nPCMSize = 0;

	return true;
}

int CExtPlayer::yyMediaDataCB (void * pUser, YY_BUFFER * pData)
{
	if (pData->uFlag == YYBUFF_EOS)
		return 0;

	CExtPlayer * pPlayer = (CExtPlayer *)pUser;

	return ((CExtPlayer *)pUser)->RenderData (pData);
}

int CExtPlayer::RenderData (YY_BUFFER * pData)
{
	int nRC = YY_ERR_NONE;
	if (pData->nType == YY_MEDIA_Video)
	{
		if (m_hBmpVideo == NULL)
		{
			YY_VIDEO_FORMAT * pFmtVideo = NULL;
			m_pPlayer->GetParam (YYPLAY_PID_Fmt_Video, &pFmtVideo);
			if (!CreateResBMP (pFmtVideo->nWidth, pFmtVideo->nHeight))
				return YY_ERR_FAILED;
		}

		m_dataConvertVideo.pSource = pData;
		nRC = m_pPlayer->GetParam (YYPLAY_PID_ConvertData, &m_dataConvertVideo);
		if (nRC != YY_ERR_NONE)
			return YY_ERR_FAILED;

		RECT rcWnd;
		GetClientRect (m_hWnd, &rcWnd);
	//	BitBlt(m_hWinDC, 0, 0, rcWnd.right, rcWnd.bottom, m_hMemDC, 0, 0, SRCCOPY);
		StretchBlt (m_hWinDC, 0, 0, rcWnd.right, rcWnd.bottom, 
					m_hMemDC, 0, 0, m_nBmpW, m_nBmpH, SRCCOPY);
	}
	else if (pData->nType == YY_MEDIA_Audio)
	{
		if ((pData->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
			ReleaseAudio ();

		if (m_pRndAudio == NULL)
		{
			if (m_pPCMBuff != NULL)
				delete []m_pPCMBuff;

			YY_AUDIO_FORMAT * pFmtAudio = NULL;
			m_pPlayer->GetParam (YYPLAY_PID_Fmt_Audio, &pFmtAudio);
			memcpy (&m_fmtAudio, pFmtAudio, sizeof (m_fmtAudio));
			m_nPCMSize = pFmtAudio->nSampleRate * 10;
			m_pPCMBuff = new unsigned char[m_nPCMSize];

			m_bufAudioRnd.nType = YY_MEDIA_Audio;

			m_pRndAudio = new CWaveOutRnd (m_hInst, false);
			m_pRndAudio->Init (pFmtAudio);
			m_pRndAudio->Start ();
		}

		m_bufAudioRnd.uFlag = YYBUFF_TYPE_PPOINTER;
		m_bufAudioRnd.pBuff = (unsigned char *)&m_pPCMBuff;
		m_bufAudioRnd.uSize = m_nPCMSize;
		m_bufAudioRnd.pFormat = &m_fmtAudio;

		m_dataConvertAudio.pSource = pData;
		m_dataConvertAudio.pTarget = &m_bufAudioRnd;
		nRC = m_pPlayer->GetParam (YYPLAY_PID_ConvertData, &m_dataConvertAudio);
		if (nRC != YY_ERR_NONE)
			return YY_ERR_FAILED;
		
		m_bufAudioRnd.uFlag = YYBUFF_TYPE_DATA;
		m_bufAudioRnd.pBuff = m_pPCMBuff;

		nRC = m_pRndAudio->Render (&m_bufAudioRnd);
		while (nRC == YY_ERR_RETRY)
		{
			Sleep (5);
			nRC = m_pRndAudio->Render (&m_bufAudioRnd);
		}
	}
	else if (pData->nType == YY_MEDIA_SubTT)
	{
		OutputDebugString ((TCHAR *)pData->pBuff);
	}


	return YY_ERR_NONE;
}