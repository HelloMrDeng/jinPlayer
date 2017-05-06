/*******************************************************************************
	File:		CMultiPlayer.cpp

	Contains:	Mutex lock implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-10-23		Fenger			Create file

*******************************************************************************/
#include "CMultiPlayer.h"

#include "CLessonInfo.h"
#include "USystemFunc.h"
#include "UStringFunc.h"

#include "yyLog.h"

CMultiPlayer::CMultiPlayer(void)
	: m_ppEng (NULL)
	, m_nEngs (0)
	, m_pPrevEng (NULL)
	, m_pLsn (NULL)
	, m_bPrevPlay (false)
	, m_nPrevComp (0)
	, m_nOpenStat (0)
	, m_nPlayComp (0)
	, m_nSeekPos (0)
	, m_nSeekComp (0)
	, m_pWorkAudio (NULL)
	, m_pPCMBuff (NULL)
	, m_nPCMSize (0)
	, m_pRndBuff (NULL)
	, m_nRndSize (0)
	, m_pRndAudio (NULL)	
{
	SetObjectName ("CMultiPlayer");

	memset (&m_bufBoxAudio, 0, sizeof (m_bufBoxAudio));
	memset (&m_bufAudioRnd, 0, sizeof (m_bufAudioRnd));
	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));
	m_bufBoxAudio.nType = YY_MEDIA_Audio;
	m_bufAudioRnd.nType = YY_MEDIA_Audio;

	m_bufAudioRnd.pFormat = &m_fmtAudio;
	m_dataConvertAudio.pSource = &m_bufBoxAudio;
	m_dataConvertAudio.pTarget = &m_bufAudioRnd;
	m_dataConvertAudio.pZoom = NULL;
}

CMultiPlayer::~CMultiPlayer(void)
{
	Close ();
#ifdef _OS_WIN32
	YY_DEL_P (m_pWorkAudio);
#endif // _OS_WIN32
	ReleaseAudio ();
}

int CMultiPlayer::Open (CLessonInfo * pLesson)
{
	Close ();
	m_pLsn = pLesson;
	TCHAR * pPrevFile = m_pLsn->GetPrevFile ();
	if (_tcslen (pPrevFile) > 0)
	{
		if (CreateEng (&m_pPrevEng, pPrevFile) != YY_ERR_NONE)
			return YY_ERR_FAILED;		
	}
	CLsnItem * pItem = m_pLsn->GetItem ();
	m_nEngs = pItem->m_nTrackNum;
	m_ppEng = new CMediaEngine*[m_nEngs];
	for (int i = 0; i < m_nEngs; i++)
	{
		if (CreateEng (&m_ppEng[i], pItem->m_ppTrack[i]->m_szFile) != YY_ERR_NONE)
			return YY_ERR_FAILED;
	}

	if (m_pRndAudio == NULL)
	{
		if (m_pPCMBuff != NULL)
			delete []m_pPCMBuff;
		YY_AUDIO_FORMAT * pFmtAudio = NULL;
		m_ppEng[0]->GetParam (YYPLAY_PID_Fmt_Audio, &pFmtAudio);
		memcpy (&m_fmtAudio, pFmtAudio, sizeof (m_fmtAudio));
		m_nPCMSize = pFmtAudio->nSampleRate;
		m_pPCMBuff = new unsigned char[m_nPCMSize];
		m_nRndSize = pFmtAudio->nSampleRate;
		m_pRndBuff = new unsigned char[m_nPCMSize];

#ifdef _OS_WIN32
		m_pRndAudio = new CWaveOutRnd (NULL, false);
		m_pRndAudio->Init (pFmtAudio);
		m_pRndAudio->Start ();
#endif // _OS_WIN32
	}

	m_bPrevPlay = false;
	m_nPrevComp = 0;
	m_nOpenStat = 0;
	m_nPlayComp = 0;
	m_nSeekPos = 0;

	return YY_ERR_NONE;
}

int CMultiPlayer::CreateEng (CMediaEngine ** ppEng, TCHAR * pFile)
{
	if (ppEng == NULL || pFile == NULL)
		return YY_ERR_FAILED;

	TCHAR	szSource[1024];
	int		nFlag = YY_OPEN_SRC_AUDIO | YY_OPEN_RNDA_EXT;
	*ppEng = new CMediaEngine ();
	if ((*ppEng)->Init (NULL) != YY_ERR_NONE)
	{
		YYLOGE ("Init media egnine was failed!");
		return YY_ERR_FAILED;
	}
	(*ppEng)->SetNotify (NotifyEvent, this);

	_tcscpy (szSource, m_pLsn->GetPath ());
	_tcscat (szSource, pFile);
	//_tcscpy (szSource, _T("O:\\Data\\Bang\\Music\\��ı���.mp3"));
	m_nOpenStat = 0;
	(*ppEng)->Open (szSource, nFlag);
	while (m_nOpenStat == 0)
	{
		yySleep (2000);
		if (m_nOpenStat < 0)
		{
			YYLOGE ("Open track file failed!");
			return YY_ERR_FAILED;
		}
	}
	return YY_ERR_NONE;
}

int CMultiPlayer::Close (void)
{
	if (m_ppEng == NULL)
		return YY_ERR_NONE;
	Stop ();

	if (m_pPrevEng != NULL)
	{
		m_pPrevEng->Uninit ();
		delete m_pPrevEng;
		m_pPrevEng = NULL;
	}
	for (int i = 0; i < m_nEngs; i++)
	{
		if (m_ppEng[i] != NULL)
		{
			m_ppEng[i]->Uninit ();
			delete m_ppEng[i];
		}
	}
	delete []m_ppEng;
	m_ppEng = NULL;
	m_nEngs = 0;
	return YY_ERR_NONE;
}

int CMultiPlayer::Run (void)
{
	CheckRepeat (-1);
	if (m_pWorkAudio == NULL)
		m_pWorkAudio = new CThreadWork (AudioPlayProc, this);
	bool bPlayPrev = false;
	if (m_pPrevEng != NULL && !m_bPrevPlay)
	{
		YYWORK_STATUS nStatus = m_pWorkAudio->GetStatus ();
		if (nStatus == YYWORK_Pause && GetPos () <= 50)
			bPlayPrev = true;
		else if (nStatus != YYWORK_Run && nStatus != YYWORK_Pause)
			bPlayPrev = true;
		if (bPlayPrev)
		{
			m_pPrevEng->SetPos (0);
			m_pPrevEng->Run ();
			m_bPrevPlay = true;
			m_nPrevComp = 0;
		}
	}
	for (int i = 0; i < m_nEngs; i++)
		m_ppEng[i]->Run ();
	m_pWorkAudio->Start ();
	return YY_ERR_NONE;
}

int CMultiPlayer::Pause (void)
{
	for (int i = 0; i < m_nEngs; i++)
		m_ppEng[i]->Pause ();
	if (m_pWorkAudio != NULL)
		m_pWorkAudio->Pause ();
	return YY_ERR_NONE;
}

int CMultiPlayer::Stop (void)
{
	for (int i = 0; i < m_nEngs; i++)
		m_ppEng[i]->Stop ();
	if (m_pWorkAudio != NULL)
		m_pWorkAudio->Stop ();
	return YY_ERR_NONE;
}

int CMultiPlayer::SetPos (int nPos)
{
	if (m_bPrevPlay)
		return YY_ERR_FAILED;
		
	nPos = nPos / m_pLsn->GetChapTime () * m_pLsn->GetChapTime ();
	if (nPos != 0 && nPos == m_nSeekPos)
		return YY_ERR_NONE;
	m_nSeekPos = nPos;
	m_nPlayComp = 0;
	YYWORK_STATUS status = YYWORK_Init;
	if (m_pWorkAudio != NULL)
	{
		status = m_pWorkAudio->GetStatus ();
		if (status == YYWORK_Run)
			m_pWorkAudio->Pause ();
	}

#ifdef _OS_WIN32
	if (m_pRndAudio != NULL)
		m_pRndAudio->Flush ();
#endif // _OS_WIN32
	for (int i = 0; i < m_nEngs; i++)
		m_ppEng[i]->SetPos (nPos);

	if (status == YYWORK_Run)
		m_pWorkAudio->Start ();

	return YY_ERR_NONE;
}

int CMultiPlayer::GetPos (void)
{
	if (m_bPrevPlay)
		return 0;
			
	if (m_ppEng == NULL)
		return 0;
	if (m_bPrevPlay)
		return 0;
	YYWORK_STATUS status = YYWORK_Init;
	if (m_pWorkAudio != NULL)
	{
		status = m_pWorkAudio->GetStatus ();
		if (status == YYWORK_Init)
			return 	m_nSeekPos;
	}
	else
	{
		return m_nSeekPos;
	}
	return m_ppEng[0]->GetPos ();
}

int CMultiPlayer::GetDur (void)
{
	if (m_ppEng == NULL)
		return 0;
	return m_ppEng[0]->GetDur ();
}

int CMultiPlayer::SetVolume (int nVolume)
{
	return YY_ERR_NONE;
}

int CMultiPlayer::GetVolume (void)
{
	return YY_ERR_NONE;
}

void CMultiPlayer::NotifyEvent (void * pUserData, int nID, void * pV1)
{
	CMultiPlayer * pPlayer = (CMultiPlayer *)pUserData;
	pPlayer->HandleEvent (nID, pV1);
}

int CMultiPlayer::HandleEvent (int nID, void * pV1)
{
	if (nID == YY_EV_Play_Complete)
	{
		if (m_bPrevPlay)
		{
			m_pPrevEng->Pause ();
			m_pPrevEng->SetPos (0);
			m_nPrevComp = 1;
			m_bPrevPlay = false;
			return YY_ERR_NONE;
		}
		m_nPlayComp++;
		if (m_nPlayComp == m_nEngs)
		{
			m_nPlayComp = 0;
			Pause ();
			SetPos (0);
		}
		return YY_ERR_NONE;
	}
	else if (nID == YY_EV_Open_Complete)
	{
		m_nOpenStat = 1;
	}
	else if (nID == YY_EV_Open_Failed)
	{
		m_nOpenStat = -1;
	}
	else if (nID == YY_EV_Play_Duration)
	{
		return YY_ERR_FAILED;
	}
	else if (nID == YY_EV_Seek_Complete)
	{
		m_nSeekComp++;
	}
	else if (nID == YY_EV_Seek_Failed)
	{
		m_nSeekComp++;
	}
	else if (nID == YY_EV_Draw_FirstFrame)
	{
		return YY_ERR_FAILED;
	}
	return YY_ERR_NONE;
}

int CMultiPlayer::AudioPlayProc (void * pParam)
{
	CMultiPlayer * pPlayer = (CMultiPlayer *)pParam;
	return pPlayer->RendAudio ();
}

int CMultiPlayer::RendAudio (void)
{
	int		i = 0;
	int		nRC = 0;
	short *	pPCMData;
	short * pRndData;
	int		nAudio;

	if (m_bPrevPlay)
	{
		nRC = YY_ERR_RETRY;
		m_bufBoxAudio.llTime = 0;
		while (nRC == YY_ERR_RETRY)
			nRC = m_pPrevEng->GetParam (YYPLAY_PID_ReadData, &m_bufBoxAudio);
		if (nRC != YY_ERR_NONE)
			return YY_ERR_NONE;
		m_bufAudioRnd.uFlag = YYBUFF_TYPE_PPOINTER;
		m_bufAudioRnd.pBuff = (unsigned char *)&m_pRndBuff;
		m_bufAudioRnd.uSize = m_nPCMSize;
		nRC = m_pPrevEng->GetParam (YYPLAY_PID_ConvertData, &m_dataConvertAudio);
		if (nRC != YY_ERR_NONE)
			return YY_ERR_FAILED;
	}
	else
	{
		CLsnTrack * pTrack = NULL;
		memset (m_pRndBuff, 0, m_nRndSize);
		for (i = 0; i < m_nEngs; i++)
		{
			pTrack = m_pLsn->GetItem ()->m_ppTrack[i];
			if (pTrack == NULL)
				continue;
			nRC = YY_ERR_RETRY;
			m_bufBoxAudio.llTime = 0;
			while (nRC == YY_ERR_RETRY)
				nRC = m_ppEng[i]->GetParam (YYPLAY_PID_ReadData, &m_bufBoxAudio);
			//YYLOGI ("*** %d  Time: % 8d, RC = %08X", i, m_bufBoxAudio.llTime, nRC);
			if (nRC != YY_ERR_NONE)
				continue;
			
			m_bufAudioRnd.uFlag = YYBUFF_TYPE_PPOINTER;
			m_bufAudioRnd.pBuff = (unsigned char *)&m_pPCMBuff;
			m_bufAudioRnd.uSize = m_nPCMSize;
			nRC = m_ppEng[i]->GetParam (YYPLAY_PID_ConvertData, &m_dataConvertAudio);
			if (nRC != YY_ERR_NONE)
				return YY_ERR_FAILED;

			if (pTrack->m_bEnable)
			{
				pPCMData = (short *)m_pPCMBuff;
				pRndData = (short *)m_pRndBuff;
				for (int j = 0; j < m_bufAudioRnd.uSize / 2; j++)
				{
					nAudio = *pPCMData;
					nAudio = nAudio * pTrack->m_nVolume / 100 + *pRndData;
					if (nAudio > 32767)
						*pRndData = 32767;
					else if (nAudio < -32768)
						*pRndData = -32768;
					else
						*pRndData = nAudio;
					pPCMData++;
					pRndData++;
				}
			}
		}
		//YYLOGI ("#######################################################");
	}
	if (CheckRepeat (m_bufBoxAudio.llTime) == YY_ERR_NONE)
		return YY_ERR_NONE;

	WriteAudio ();

	return YY_ERR_NONE;
}

int CMultiPlayer::WriteAudio (void)
{
#ifdef _OS_WIN32
	m_bufAudioRnd.uFlag = YYBUFF_TYPE_DATA;
	m_bufAudioRnd.pBuff = m_pRndBuff;
	nRC = m_pRndAudio->Render (&m_bufAudioRnd);
	while (nRC == YY_ERR_RETRY)
	{
		Sleep (5);
		nRC = m_pRndAudio->Render (&m_bufAudioRnd);
	}
#endif // _OS_WIN32
	return YY_ERR_NONE;	
}

bool CMultiPlayer::ReleaseAudio (void)
{
#ifdef _OS_WIN32	
	if (m_pRndAudio != NULL)
	{
		m_pRndAudio->Stop ();
		m_pRndAudio->Uninit ();
		delete m_pRndAudio;
	}
	m_pRndAudio = NULL;
#endif // _OS_WIN32
	YY_DEL_A (m_pPCMBuff);
	m_nPCMSize = 0;
	YY_DEL_A (m_pRndBuff);
	m_nRndSize = 0;

	return true;
}

int CMultiPlayer::CheckRepeat (int nTime)
{
	int nSeekNum = 0;
	int	nAdjustTime = 40;
	if (m_pLsn == NULL)
		return YY_ERR_IMPLEMENT;
	CLsnRepeat * pRpt = m_pLsn->GetRepeat ();
	if (pRpt == NULL)
		return YY_ERR_IMPLEMENT;
	if (nTime < 0)
	{
		for (int i = 0; i < m_nEngs; i++)
			m_ppEng[i]->SetPos (pRpt->m_nStart);
		return YY_ERR_NONE;
	}
	if (nTime > pRpt->m_nStart && nTime < pRpt->m_nEnd)
		return YY_ERR_IMPLEMENT;

	int nChapTime = m_pLsn->GetChapTime ();
	if ((nTime % nChapTime) < nAdjustTime)
	{				
		m_nSeekComp = 0;
		nSeekNum = 0;
		for (int i = 0; i < m_nEngs; i++)
		{
			if (m_ppEng[i]->SetPos (pRpt->m_nStart) == YY_ERR_NONE)
				nSeekNum++;
		}
		while (m_nSeekComp < nSeekNum)
			yySleep (1000);
		return YY_ERR_NONE;
	}
	return YY_ERR_IMPLEMENT;
}

