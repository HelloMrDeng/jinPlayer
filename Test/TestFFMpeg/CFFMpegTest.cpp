/*******************************************************************************
	File:		CFFMpegTest.cpp

	Contains:	The ffmpeg source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CFFMpegTest.h"

#include "CBaseUtils.h"
#include "CBaseFile.h"
#include "CBaseExtData.h"

#include "yyMetaData.h"

#include "UFFMpegFunc.h"
#include "UStringFunc.h"
#include "UThreadFunc.h"
#include "USystemFunc.h"

#include "yyConfig.h"
#include "yyLog.h"

CFFMpegTest::CFFMpegTest(void * hInst)
	: CBaseSource (hInst)
	, m_pFmtCtx (NULL)
	, m_pTmpCtx (NULL)
	, m_llFileStartTime (0)
	, m_nIdxAudio (-1)
	, m_pStmAudio (NULL)
	, m_nIdxVideo (-1)
	, m_pStmVideo (NULL)
	, m_hReadThread (NULL)
	, m_bReadStop (true)
	, m_bReadPaused (false)
{
	SetObjectName (__FILE__);

	ResetParam (0);
}

CFFMpegTest::~CFFMpegTest(void)
{
	Close ();
}

int CFFMpegTest::Open (const TCHAR * pSource, int nType)
{
	CAutoLock lock (&m_mtFile);
	// close the opened source first.
	Close ();
	ResetParam (0);

	TCHAR * pSourceName = (TCHAR *)pSource;
	if ((nType & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ)
	{
		YY_READ_EXT_DATA * pExtData = (YY_READ_EXT_DATA *)pSource;
		pSourceName = pExtData->szName;
		CBaseExtData::g_pExtData = pExtData;
	}

	char szFileName[1024];
#ifdef _UNICODE
	memset (szFileName, 0, sizeof (szFileName));
	WideCharToMultiByte (CP_ACP, 0, pSourceName, -1, szFileName, sizeof (szFileName), NULL, NULL);
#else
	strcpy (szFileName, pSourceName);
#endif //_UNICODE
	strcpy (m_szSourceName, szFileName);
	yyChangeFileExtName (szFileName, false);

	m_nProtType = yyGetProtType (pSourceName);
#ifdef _OS_WIN32
	if (m_nProtType == YY_PROT_FILE || m_nProtType == YY_PROT_HTTP)
#else
	if (m_nProtType == YY_PROT_FILE)
#endif // _OS_WIN32
	{
		memset (szFileName, 0, sizeof (szFileName));
		sprintf (szFileName, "ExtIO:%08X", &g_ioFileExt);
		char * pFileName = szFileName + strlen (szFileName);
#ifdef _UNICODE
		CBaseUtils::ConvertDataToBase64 ((const unsigned char *)pSourceName, _tcslen (pSourceName) * sizeof (TCHAR),
											pFileName, sizeof (szFileName) - strlen (szFileName));
#else
		strcat (szFileName, pSourceName);
#endif // UNICODE
	}

//	strcpy (szFileName, "http://42.121.109.19/Files/h264-576.mkv");
//	strcpy (szFileName, "rtsp://alive.rbc.cn/am603?@?life");
//	strcpy (szFileName, "http://qatest.visualon.com:8082/osmp/PD/H264/MP4/6M_H264_720x480_Baseline_24f.mp4");
	
	// open input file, and allocate format context
	m_bForceClosed = false;
	int nRC = avformat_open_input(&m_pTmpCtx, szFileName, NULL, NULL);
	if (m_bForceClosed)
		return YY_ERR_FAILED;
	if (nRC < 0)
	{
		m_pTmpCtx = NULL;

		char szErr[1024];
		av_strerror(nRC, szErr, sizeof (szErr));
		YYLOGE ("Open Source %s failed. Return %d %s", szFileName, nRC, szErr);

		return nRC;
	}
	m_pFmtCtx = m_pTmpCtx;

	// It will contine to playback for AVI file.
	if (CBaseFile::g_bFileError)
	{
		if (strcmp (m_pFmtCtx->iformat->name, "avi"))
			return YY_ERR_FAILED;
	}

    // retrieve stream information
    nRC = avformat_find_stream_info(m_pFmtCtx, NULL);
	if (nRC < 0)
	{
		YYLOGE ("avformat_find_stream_info failed. %d", nRC);
		return nRC;
	}

	AVCodecContext * pDecCtx = NULL;
	if ((nType & YY_OPEN_SRC_AUDIO) == YY_OPEN_SRC_AUDIO)
		m_nIdxVideo = -1;
	else
		m_nIdxVideo = av_find_best_stream(m_pFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	// Find the audio stream, Open the audio decoder
	if ((nType & YY_OPEN_SRC_VIDEO) == YY_OPEN_SRC_VIDEO)
		m_nIdxAudio = -1;
	else
		m_nIdxAudio = av_find_best_stream(m_pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (m_nIdxAudio >= 0)
	{
		pDecCtx = m_pFmtCtx->streams[m_nIdxAudio]->codec;
		if (pDecCtx->codec_id == AV_CODEC_ID_NONE || 
			pDecCtx->codec_id == AV_CODEC_ID_GSM_MS )
		{
			m_nIdxAudio = -1;
		}

		if (m_nIdxAudio >= 0)
		{
			m_pStmAudio = m_pFmtCtx->streams[m_nIdxAudio];
			m_fmtAudio.nCodecID = pDecCtx->codec_id;
			m_fmtAudio.nSampleRate = pDecCtx->sample_rate;
			m_fmtAudio.nChannels = pDecCtx->channels;
			m_fmtAudio.nBits = 16;

			m_fmtAudio.nSourceType = YY_SOURCE_FF;
			m_fmtAudio.pPrivateData = pDecCtx;

			m_nAudioStreamNum = 0;
			for (int i = 0; i < m_pFmtCtx->nb_streams; i++)
			{
				if (m_pFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
				{
					if (i == m_nIdxAudio)
						m_nAudioStreamPlay = m_nAudioStreamNum;
					m_nAudioStreamNum++;
				}
			}
		}
	}

	// Find the Video stream, Open the Video decoder
	if (m_nIdxVideo >= 0)
	{
		pDecCtx = m_pFmtCtx->streams[m_nIdxVideo]->codec;
		if (pDecCtx->codec_id == AV_CODEC_ID_NONE ||
			pDecCtx->codec_id == AV_CODEC_ID_SVQ3)
		{
			m_nIdxVideo = -1;
		}

		if (m_nIdxVideo >= 0)
		{
			m_pStmVideo = m_pFmtCtx->streams[m_nIdxVideo];
			m_fmtVideo.nCodecID = pDecCtx->codec_id;
			m_fmtVideo.nWidth = pDecCtx->width;
			m_fmtVideo.nHeight = pDecCtx->height;
			m_fmtVideo.nNum = m_pStmVideo->sample_aspect_ratio.num;
			m_fmtVideo.nDen = m_pStmVideo->sample_aspect_ratio.den;
			if (m_pStmVideo->avg_frame_rate.den > 0)
				m_fmtVideo.nFrameTime = m_pStmVideo->avg_frame_rate.num / m_pStmVideo->avg_frame_rate.den;
	
			m_fmtVideo.nSourceType = YY_SOURCE_FF;
			m_fmtVideo.pPrivateData = pDecCtx;

			m_nVideoStreamNum = 1;
			m_nVideoStreamPlay = 0;
		}
	}
	
	if (m_nIdxVideo < 0 && m_nIdxAudio < 0)
	{
		YYLOGE ("It can't find audio and video.");
		return YY_ERR_SOURCE;
	}

	// Check the start time.
	m_llFileStartTime = 0;
	if (m_pFmtCtx->start_time > 0)
	{
		if (m_nIdxAudio >= 0 && m_pStmAudio != NULL)
			m_llFileStartTime = yyBaseToTime (m_pStmAudio->start_time, m_pStmAudio);
		else if (m_nIdxVideo >= 0 && m_pStmVideo != NULL)
			m_llFileStartTime = yyBaseToTime (m_pStmVideo->start_time, m_pStmVideo);
	}
	m_llAudioTime = m_llFileStartTime;
	m_llVideoTime = m_llFileStartTime;

	// Get the duration
	long long llDuration = 0;
	if (m_pStmAudio != NULL)
	{
		m_llDurAudio = m_pStmAudio->duration;
		m_llDurAudio = yyBaseToTime (m_llDurAudio, m_pStmAudio);
	}
	if (m_pStmVideo != NULL)
	{
		m_llDurVideo = m_pStmVideo->duration;
		m_llDurVideo = yyBaseToTime (m_llDurVideo, m_pStmVideo);
	}
	if (m_llDurVideo > m_llDurAudio)
		llDuration = m_llDurVideo;
	else
		llDuration = m_llDurAudio;
	if (llDuration == 0 && m_pFmtCtx != NULL)
	{
		if (m_pStmAudio != NULL && m_pStmAudio->time_base.den != 0)
			m_llDurAudio = m_pFmtCtx->duration * m_pStmAudio->time_base.num / m_pStmAudio->time_base.den;
		else if (m_pStmVideo != NULL && m_pStmVideo->time_base.num != 0)
			m_llDurVideo = m_pFmtCtx->duration * m_pStmVideo->time_base.num / m_pStmVideo->time_base.den;
		else
			llDuration = m_pFmtCtx->duration / 1000;
	}

	if (m_llDurAudio > 0 || m_llDurVideo > 0)
	{
		llDuration = m_llDurVideo;
		if (m_llDurAudio > m_llDurVideo)
			llDuration = m_llDurAudio;
	}

	m_llDuration = llDuration;
	if (m_llDuration <= 0)
	{
		if ((nType & YY_OPEN_SRC_VIDEO) != YY_OPEN_SRC_VIDEO)
			ForceCheckDuration ();
	}

	m_bEOF = false;
	m_bEOA = false;
	m_bEOV = false;
/*
#ifdef _CPU_MSB2531
	if (m_nIdxVideo >= 0)
	{
		if (m_fmtVideo.nWidth * m_fmtVideo.nHeight * 3 / 2 > 1024 * 1024)
		{
			YYLOGE ("The video size %d X %d is too large for MSB2531!", m_fmtVideo.nWidth, m_fmtVideo.nHeight);
			return YY_ERR_FAILED;
		}
	}
#endif // _CPU_MSB2531
*/
	return YY_ERR_NONE;
}

int CFFMpegTest::Close (void)
{
	CAutoLock lock (&m_mtFile);

	ReleasePacket (true, true);

	CAutoLock lockPkt (&m_mtPacket);
	AVPacket * pPacket = m_lstMedia.RemoveHead ();
	while (pPacket != NULL)
	{
		av_free_packet (pPacket);
		delete pPacket;
		pPacket = m_lstMedia.RemoveHead ();
	}

	if (m_pFmtCtx != NULL)
		avformat_close_input (&m_pFmtCtx);
	m_pFmtCtx = NULL;
	m_pTmpCtx = NULL;

	ResetParam (0);

	return YY_ERR_IMPLEMENT;
}

int CFFMpegTest::ForceClose (void)
{
	m_bForceClosed = true;

	if (m_pTmpCtx != NULL)
		m_pTmpCtx->openstatus = -1;

	return YY_ERR_NONE;
}

int	CFFMpegTest::Start (void)
{
#if 1
	m_bReadPaused = false;

	if (m_hReadThread != NULL)
		return YY_ERR_NONE;

	m_bReadStop = false;

	int nID = 0;
	yyThreadCreate (&m_hReadThread, &nID, ReadProc, this, 0);
#endif // check

	return YY_ERR_NONE;
}

int CFFMpegTest::Pause (void)
{
//	m_bReadPaused = true;

	return YY_ERR_NONE;
}

int	CFFMpegTest::Stop (void)
{
	m_bReadStop = true;
	int nTryTimes = 0;
	while (m_hReadThread != NULL)
	{
		yySleep (10000);
		nTryTimes++;
		if (nTryTimes > 100)
			break;
	}

	return YY_ERR_NONE;
}

int CFFMpegTest::SetStreamPlay (YYMediaType nType, int nStream)
{
	if (nStream < 0 || nStream >= m_nAudioStreamNum)
		return YY_ERR_ARG;

	int nIndex = 0;
	for (int i = 0; i < m_pFmtCtx->nb_streams; i++)
	{
		if (m_pFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			if (nIndex == nStream)
			{
				nIndex = i;
				break;
			}
			nIndex++;
		}
	}
	if (nIndex == m_nIdxAudio)
		return YY_ERR_NONE;

	CAutoLock lock (&m_mtFile);
	CAutoLock lockPacket (&m_mtPacket);
	int nRC = 0;
	m_nAudioStreamPlay = nStream;
	m_nIdxAudio = nIndex;
	m_pStmAudio = m_pFmtCtx->streams[m_nIdxAudio];

	AVPacket * pPacket = m_lstAudio.GetHead ();
	if (pPacket != NULL)
		m_llAudioTrackPos = pPacket->pts;
	else 
		m_llAudioTrackPos = GetPos ();

	pPacket = m_lstVideo.GetTail ();
	if (pPacket != NULL)
		m_llVideoTrackPos = pPacket->pts;

	ReleasePacket (true, false);

	long long llPosAudio = yyTimeToBase (m_llAudioTrackPos, m_pStmAudio);
	nRC = av_seek_frame (m_pFmtCtx, m_nIdxAudio, llPosAudio, AVSEEK_FLAG_ANY);
	if (nRC < 0)
		nRC = av_seek_frame (m_pFmtCtx, m_nIdxVideo, llPosAudio, AVSEEK_FLAG_ANY);

	AVCodecContext * pDecCtx = m_pStmAudio->codec;
	m_fmtAudio.nCodecID = pDecCtx->codec_id;
	m_fmtAudio.nSampleRate = pDecCtx->sample_rate;
	m_fmtAudio.nChannels = pDecCtx->channels;
	m_fmtAudio.nBits = 16;

	m_fmtAudio.nSourceType = YY_SOURCE_FF;
	m_fmtAudio.pPrivateData = pDecCtx;

	m_bNewAudioStream = true;

	return YY_ERR_NONE;
}


int CFFMpegTest::ReadData (YY_BUFFER * pBuff)
{
	CAutoLock lock (&m_mtFile);
	if (pBuff == NULL)
		return YY_ERR_ARG;

	CBaseSource::ReadData (pBuff);

	pBuff->uFlag = 0;
	AVPacket * pPacket = GetPacket (pBuff->nType, pBuff->llTime);
	if (pPacket == NULL)
	{
		if (m_bEOF)
		{
			pBuff->pBuff = NULL;
			pBuff->uSize = 0;
			pBuff->uFlag = YYBUFF_EOS;

			return YY_ERR_FINISH;
		}
		else
		{	
			if (pBuff->nType == YY_MEDIA_Audio && m_nAudioReadCount < 16)
			{
				if (m_lstVideo.GetCount () > YYCFG_VIDEO_LIST_MAX / 10 || m_bEOA)
				{
					pBuff->pBuff = NULL;
					pBuff->uSize = 0;
					pBuff->uFlag = YYBUFF_EOS;

					m_bEOA = true;

					return YY_ERR_FINISH;
				}
			}
			else if (pBuff->nType == YY_MEDIA_Video && m_nVideoReadCount < 16)
			{
				if (m_lstAudio.GetCount () > YYCFG_AUDIO_LIST_MAX / 10 || m_bEOV)
				{
					pBuff->pBuff = NULL;
					pBuff->uSize = 0;
					pBuff->uFlag = YYBUFF_EOS;

					m_bEOV = true;

					return YY_ERR_FINISH;
				}
			}

			if (m_hReadThread != NULL)
				yySleep (5000);
				
			return YY_ERR_RETRY;
		}
	}

	pBuff->uFlag = YYBUFF_TYPE_PACKET;
	pBuff->llTime = pPacket->pts;
	pBuff->pBuff = (unsigned char *)pPacket;
	pBuff->uSize = g_nPacketSize;

	if (pBuff->nType == YY_MEDIA_Video)
	{
		if (m_nVideoDropFrames > 0)
		{
			pBuff->uFlag |= YYBUFF_DROP_FRAME;
			pBuff->nValue = m_nVideoDropFrames;
		}
		if (m_bVideoNewPos)
		{
			m_bVideoNewPos = false;
			pBuff->uFlag |= YYBUFF_NEW_POS;
		}
		m_llVideoLastTime = pBuff->llTime;
		m_nVideoReadCount++;

		CAutoLock lockPacket (&m_mtPacket);
		if (m_pPktVideo != NULL)
		{
			//av_free_packet (m_pPktVideo);
			m_lstMedia.AddTail (m_pPktVideo);
		}
		m_pPktVideo = pPacket;

		//YYLOGI ("Index: % 6d, Time: % 8d, Step % 8d", m_nVideoReadCount, (int)pBuff->llTime, (int)(pBuff->llTime - m_llDbgPrevTime));
		//m_llDbgPrevTime = pBuff->llTime;
	}
	else if (pBuff->nType == YY_MEDIA_Audio)
	{
		if (m_bAudioNewPos)
		{
			m_bAudioNewPos = false;
			pBuff->uFlag |= YYBUFF_NEW_POS;
		}
		if (m_bNewAudioStream)
		{
			m_bNewAudioStream = false;
			pBuff->uFlag |= YYBUFF_NEW_FORMAT;
			pBuff->pFormat = &m_fmtAudio;
		}

		m_nAudioReadCount++;

		CAutoLock lockPacket (&m_mtPacket);
		if (m_pPktAudio != NULL)
		{
			//av_free_packet (m_pPktAudio);
			m_lstMedia.AddTail (m_pPktAudio);
		}
		m_pPktAudio = pPacket;
	}

	return YY_ERR_NONE;
}

int CFFMpegTest::SetPos (long long llPos)
{
	CAutoLock lock (&m_mtFile);
	if (m_pFmtCtx == NULL)
		return YY_ERR_FAILED;

	CAutoLock lockSeek (&m_mtRead);
	CAutoLock lockPacket (&m_mtPacket);

	int			nRC = 0;
	AVPacket *	pPacket = NULL;
	int			nVideoReadPackets = m_lstVideo.GetCount ();
	long long	llNewPos = llPos + m_llFileStartTime;
	bool		bSeekAudio = false;

	unsigned int	nStartTime = yyGetSysTime ();
	long long		llPosAudio = 0;

	if (m_nIdxAudio >= 0)
	{
		llPosAudio = yyTimeToBase (llNewPos, m_pStmAudio);
		if (m_nIdxVideo < 0)
		{
			bSeekAudio = true;
		}
		else
		{
			if (!strcmp (m_pFmtCtx->iformat->name, "rm") ||
				!strcmp (m_pFmtCtx->iformat->name, "asf") ||
				!strcmp (m_pFmtCtx->iformat->name, "aac") ||
				!strcmp (m_pFmtCtx->iformat->name, "mp3"))
			{
				bSeekAudio = true;
			}
			else if (strstr ("mpegts", m_pFmtCtx->iformat->name) != NULL)
			{
				bSeekAudio = true;
			}

			if (m_llDurVideo > 0 && llPos > m_llDurVideo)
				bSeekAudio = true;
		}
	}

	if (bSeekAudio)
	{
		if (m_llDurAudio > 0 && llPos > m_llDurAudio)
		{
			llPos = m_llDurAudio - 2000;
			if (llPos < 0)
				llPos = 0;
			llPosAudio = yyTimeToBase (llPos, m_pStmAudio);
		}

		nRC = av_seek_frame (m_pFmtCtx, m_nIdxAudio, llPosAudio, AVSEEK_FLAG_ANY);
		if (nRC != 0)
		{
			YYLOGI ("Set Pos Failed! return %d", nRC);
			return YY_ERR_FAILED;
		}

		ReleasePacket (true, true);
		m_bEOF = false;

		llPosAudio = 0;
		while (m_nIdxVideo >= 0)
		{
			if (!strcmp (m_pFmtCtx->iformat->name, "aac") || !strcmp (m_pFmtCtx->iformat->name, "mp3"))
				break;

			ReadPacket ();
			pPacket = m_lstVideo.GetHead ();
			if (pPacket != NULL)
			{
				if ((pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY)
				{
					llPosAudio = pPacket->pts;
					break;
				}
				else
				{
					pPacket = m_lstVideo.RemoveHead ();
					av_free_packet (pPacket);
					m_lstMedia.AddTail (pPacket);
				}
			}

			if (yyGetSysTime () - nStartTime > 2000)
				break;
		}

		nStartTime = yyGetSysTime ();
		while (m_nIdxAudio >= 0)
		{
			pPacket = m_lstAudio.GetHead ();
			while (pPacket == NULL)
			{
				ReadPacket ();
				pPacket = m_lstAudio.GetHead ();
				if (yyGetSysTime () - nStartTime > 2000)
					break;
			}

			if (pPacket == NULL)
				break;

			if (pPacket->pts >= llPosAudio)
			{
				break;
			}
			else
			{
				pPacket = m_lstAudio.RemoveHead ();
				av_free_packet (pPacket);
				m_lstMedia.AddTail (pPacket);
			}
			if (yyGetSysTime () - nStartTime > 2000)
				break;
		}
	}
	else
	{
		if (m_llDurVideo > 0 && llPos > m_llDurVideo)
		{
			llPos = m_llDurVideo - 2000;
			if (llPos < 0)
				llPos = 0;
		}

		long long	llPosVideo = yyTimeToBase (llNewPos, m_pStmVideo);
		nRC = av_seek_frame (m_pFmtCtx, m_nIdxVideo, llPosVideo, AVSEEK_FLAG_BACKWARD);
		if (nRC != 0)
		{
			YYLOGI ("Set Pos Failed! return %d", nRC);
			return YY_ERR_FAILED;
		}

		ReleasePacket (true, true);
		m_bEOF = false;

		// make sure the first sample is key frame.
		long long		llStartPos = 0;
		while (llPosVideo > 0 && nRC >= 0)
		{
			int nTryFrames = 0;
			while (yyGetSysTime () - nStartTime < 2000)
			{
				ReadPacket ();
				pPacket = m_lstVideo.GetHead ();
				if (pPacket != NULL)
				{
					if ((pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY)
					{
						break;
					}
					else
					{
						pPacket = m_lstVideo.RemoveHead ();
						av_free_packet (pPacket);
						m_lstMedia.AddTail (pPacket);
					}

					nTryFrames++;
					if (nTryFrames > 100)
						break;
				}
			}

			if (pPacket == NULL)
				break;

			if ((pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY)
				break;

			if (llStartPos == 0)
				llStartPos = pPacket->pts;

			// try to find the key frame in next 3000 ms samples
			if (pPacket->pts - llStartPos < 3000)
			{
				pPacket = m_lstVideo.RemoveHead ();
				av_free_packet (pPacket);
				m_lstMedia.AddTail (pPacket);
				continue;
			}

			ReleasePacket (true, true);

			llNewPos -= llPos / 5;
			if (llNewPos < 0)
				llNewPos = 0;
			m_llVideoTime = llNewPos + m_llFileStartTime;

			llPosVideo = yyTimeToBase (llNewPos, m_pStmVideo);
			nRC = av_seek_frame (m_pFmtCtx, m_nIdxVideo, llPosVideo, AVSEEK_FLAG_BACKWARD);
			if (nRC < 0)
				break;

			if (yyGetSysTime () - nStartTime > 2000)
				break;
		}
	}

	return YY_ERR_NONE;
}

AVPacket * CFFMpegTest::GetPacket (YYMediaType  nType, long long llPlayTime)
{
	CAutoLock lock (&m_mtPacket);
	AVPacket *	pPacket = NULL;
	if (nType == YY_MEDIA_Video)
	{
		// try to read more video packet for drop frames
		if (m_hReadThread == NULL && m_nVideoReadCount > YYCFG_VIDEO_LIST_FIRST)
		{
			// Try to read more two packet if it was too later.
			AVPacket *	pLastPacket = m_lstVideo.GetTail ();
			int			nPacketCount = m_lstVideo.GetCount ();
			if (nPacketCount < YYCFG_VIDEO_LIST_FIRST || (pLastPacket != NULL && pLastPacket->pts < llPlayTime + 5000))
			{
				int nTryTimes = 0;
				while (m_lstVideo.GetCount () - nPacketCount < 3)
				{
					if (ReadPacket () < 0)
						break;
					nTryTimes++;
					if (nTryTimes >= 30)
						break;
				}
			}
		}

		pPacket = m_lstVideo.GetHead ();
		if (pPacket == NULL)
		{
			if (m_hReadThread == NULL)
				ReadPacket ();
			return NULL;
		}

		m_nVideoDropFrames = 0;
		if (m_nKeyFrameNum > 0 && m_nVideoReadCount > YYCFG_VIDEO_LIST_FIRST)
		{
			if ((pPacket->flags & AV_PKT_FLAG_KEY) != AV_PKT_FLAG_KEY)
			{
				// if it was late than the playing time, try to jump to next key frame.
				if (pPacket->pts < llPlayTime)
				{
					AVPacket * pKeyPacket = GetNextKeyFrame (llPlayTime);
					if (pKeyPacket != NULL)
					{
						pPacket = m_lstVideo.RemoveHead ();
						while (pPacket != NULL)
						{
							if ((pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY)
								m_nKeyFrameNum--;
							m_nVideoDropFrames++;
							if (pKeyPacket == pPacket)
							{
								// YYLOGI("Time: % 8d, Drops % 4d", (int)pPacket->pts, m_nVideoDropFrames);
								return pPacket;
							}

							av_free_packet (pPacket);
							m_lstMedia.AddTail (pPacket);
							pPacket = m_lstVideo.RemoveHead ();
						}
					}
				}
			}
		}

		pPacket = m_lstVideo.RemoveHead ();
		if (pPacket != NULL)
		{
			if ((pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY)
				m_nKeyFrameNum--;
			return pPacket;
		}
	}
	else if (nType == YY_MEDIA_Audio)
	{
		pPacket = m_lstAudio.RemoveHead ();
		if (pPacket != NULL)
			return pPacket;
	}

	if (m_hReadThread == NULL)
		ReadPacket ();

	return NULL;
}

int CFFMpegTest::ReadPacket (void)
{
	if (m_bEOF)
		return -1;
	int			nRC = 0;
	AVPacket *	pPacket = NULL;
	{
		CAutoLock lock (&m_mtPacket);
		pPacket = m_lstMedia.RemoveHead ();
		if (pPacket == NULL)
		{
			pPacket = new AVPacket ();
			if (pPacket == NULL)
				return -1;
			av_init_packet (pPacket);
		}
		else
		{
			av_free_packet (pPacket);
		}
	}

//	CAutoLock lockFile (&m_mtFile);
	int nStart = yyGetSysTime ();
	nRC = av_read_frame (m_pFmtCtx, pPacket);
	if (yyGetSysTime () - nStart > 2000)
		YYLOGW ("av_read_frame time %d", yyGetSysTime () - nStart);
	if (nRC < 0)
	{
		YYLOGI ("It reach the end of File!");
		m_bEOF = true;
		return -1;
	}

	CAutoLock lockAV (&m_mtPacket);
	if (pPacket->stream_index == m_nIdxAudio)
	{
		UpdatePacketTime (pPacket);
		if (pPacket->pts == 0 && m_lstAudio.GetCount () > 0)
			pPacket->pts = m_llAudioTime;
		if (pPacket->duration == 0)
			m_llAudioTime += 40;
		else
			m_llAudioTime += pPacket->duration;

		if (m_llAudioTrackPos > 0)
		{
			if (pPacket->pts <= m_llAudioTrackPos)
			{
				av_free_packet (pPacket);
				m_lstMedia.AddTail (pPacket);
				return 0;
			}
			else
			{
				YYLOGI ("Audio Reach the switch track pos %d", (int)m_llVideoTrackPos);
				m_llAudioTrackPos = -1;
			}
		}

		if (m_lstAudio.GetCount () < YYCFG_AUDIO_LIST_MAX)
		{
			m_lstAudio.AddTail (pPacket);
		}
		else
		{
			av_free_packet (pPacket);
			m_lstMedia.AddTail (pPacket);
		}
	}
	else if (pPacket->stream_index == m_nIdxVideo)
	{
		UpdatePacketTime (pPacket);
		if (pPacket->pts == 0 && m_lstVideo.GetCount () > 0)
			pPacket->pts = m_llVideoTime;
		if (pPacket->duration == 0)
			m_llVideoTime += 40;
		else
			m_llVideoTime += pPacket->duration;

		if (m_nVideoFrameTime == 0)
		{
			if (pPacket->duration > 0)
				m_nVideoFrameTime = (int)pPacket->duration;
			else
				m_nVideoFrameTime = pPacket->pts;
		}

		if (m_llVideoTrackPos > 0)
		{
			if (pPacket->pts != m_llVideoTrackPos)
			{
				av_free_packet (pPacket);
				m_lstMedia.AddTail (pPacket);
				return 0;
			}
			else
			{
				YYLOGI ("Video Reach the switch track pos %d", (int)m_llVideoTrackPos);
				m_llVideoTrackPos = -1;
			}
		}

		if (m_lstVideo.GetCount () < YYCFG_VIDEO_LIST_MAX)
		{
			if ((pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY)
				m_nKeyFrameNum++;

			m_lstVideo.AddTail (pPacket);
		}
		else
		{
			av_free_packet (pPacket);
			m_lstMedia.AddTail (pPacket);
		}
	}
	else
	{
		av_free_packet (pPacket);
		m_lstMedia.AddTail (pPacket);
	}

	return 0;
}

void CFFMpegTest::UpdatePacketTime (AVPacket * pPacket)
{
	if (pPacket == NULL)
		return;

	if (pPacket->pts == 0X8000000000000000)
		pPacket->pts = 0;
	else
		pPacket->pts = yyBaseToTime (pPacket->pts, m_pFmtCtx->streams[pPacket->stream_index]);
	if (pPacket->pts == 0X8000000000000000)
		pPacket->dts = 0;
	else
		pPacket->dts = yyBaseToTime (pPacket->dts, m_pFmtCtx->streams[pPacket->stream_index]);
	if (pPacket->duration == 0X8000000000000000)
		pPacket->duration = 0;
	else
		pPacket->duration = yyBaseToTime (pPacket->duration, m_pFmtCtx->streams[pPacket->stream_index]);
	if (pPacket->pts < pPacket->dts)
		pPacket->pts = pPacket->dts;

	if (m_llFileStartTime > 0)
	{
		pPacket->dts -= m_llFileStartTime;
		pPacket->pts -= m_llFileStartTime;
	}
}

long long CFFMpegTest::GetVideoBuffTime (void)
{
	AVPacket *	pPacketFirst = NULL;
	AVPacket *	pPacketLast = NULL;
	long long	llTime = 0;

	if (m_lstVideo.GetCount () <= 1)
		return llTime;

	pPacketFirst = m_lstVideo.GetHead ();
	pPacketLast = m_lstVideo.GetTail ();

	if (pPacketFirst->pts > 0)
		llTime = pPacketLast->pts - pPacketFirst->pts;
	else if (pPacketFirst->dts > 0)
		llTime = pPacketLast->dts - pPacketFirst->dts;

	return llTime;
}

AVPacket * CFFMpegTest::GetNextKeyFrame (long long llTime)
{
	AVPacket *	pPacket = NULL;
	if (m_nKeyFrameNum <= 0)
		return NULL;

	pPacket = m_lstVideo.GetTail ();
	if (pPacket == NULL)
		return NULL;
	if (pPacket->pts < llTime && m_nKeyFrameNum <= 0)
		return NULL;

	int			nCount = 0;
	int			nStep = 35;
	long long	llKeyTime = llTime;
	long long	llFirstTime = 0;
	void *		pPos = m_lstVideo.GetHeadPosition ();
	pPacket = m_lstVideo.GetNext (pPos);
	while (pPacket != NULL)
	{
		if (llFirstTime == 0)
			llFirstTime = pPacket->pts;

		nCount++;
		if (nCount >= 4 && nCount < 32)
		{
			llKeyTime = llTime + (nCount - 3) * nStep;
			if (llKeyTime > llTime + (llTime - llFirstTime))
				llKeyTime = llTime + (llTime - llFirstTime);
		}

		if (pPacket->pts >= llKeyTime)
			return NULL;

		if ((pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY)
		{
			return pPacket;
		}

		pPacket = m_lstVideo.GetNext (pPos);
	}

	return NULL;
}

bool CFFMpegTest::ReleasePacket (bool bAudio, bool bVideo)
{
	CAutoLock lock (&m_mtPacket);
	AVPacket * pPacket = NULL;
	
	if (bAudio)
	{
		if (m_pPktAudio != NULL)
		{
			av_free_packet (m_pPktAudio);
			m_lstMedia.AddTail (m_pPktAudio);
			m_pPktAudio = NULL;
		}

		pPacket = m_lstAudio.RemoveHead ();
		while (pPacket != NULL)
		{
			av_free_packet (pPacket);
			m_lstMedia.AddTail (pPacket);
			pPacket = m_lstAudio.RemoveHead ();
		}
	}

	if (bVideo)
	{
		if (m_pPktVideo != NULL)
		{
			av_free_packet (m_pPktVideo);
			m_lstMedia.AddTail (m_pPktVideo);
			m_pPktVideo = NULL;
		}

		pPacket = m_lstVideo.RemoveHead ();
		while (pPacket != NULL)
		{
			av_free_packet (pPacket);
			m_lstMedia.AddTail (pPacket);
			pPacket = m_lstVideo.RemoveHead ();
		}

		m_nKeyFrameNum = 0;
		m_nVideoReadCount = 0;
	}

	POSITION pos = m_lstMedia.GetHeadPosition ();
	while (pos != NULL)
	{
		pPacket = m_lstMedia.GetNext (pos);
		if (pPacket != NULL)
			av_free_packet (pPacket);
	}
	return true;
}

int CFFMpegTest::GetMediaInfo (TCHAR * pInfo, int nSize)
{
#ifdef _OS_WIN32
	int		nInfoSize = 1024 * 32;
	char *	pInfoText = new char[nInfoSize];
	memset (pInfoText, 0, nInfoSize);
	char	szInfoLine[256];

	sprintf (szInfoLine, "%s  %s\r\n", GetLineText ("File Format:"), m_pFmtCtx->iformat->long_name);
	strcat (pInfoText, szInfoLine);

	sprintf (szInfoLine, "%s  %d\r\n", GetLineText ("Stream Num:"), m_pFmtCtx->nb_streams);
	strcat (pInfoText, szInfoLine);

	for (int i = 0; i < m_pFmtCtx->nb_streams; i++)
	{
		if (i != m_nIdxAudio && i != m_nIdxVideo)
			continue;

		AVStream *			pStream = m_pFmtCtx->streams[i];
		AVCodecContext *	pCodec = pStream->codec;

		sprintf (szInfoLine, "Stream %d\r\n", i);
		strcat (pInfoText, szInfoLine);

		sprintf (szInfoLine, "%s  %s\r\n", GetLineText ("Media Type:"), av_get_media_type_string (pCodec->codec_type));
		strcat (pInfoText, szInfoLine);

		if (pCodec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			sprintf (szInfoLine, "%s  %s\r\n", GetLineText ("Video Codec:"), avcodec_get_name (pCodec->codec_id));
			strcat (pInfoText, szInfoLine);

			sprintf (szInfoLine, "%s  %d X %d\r\n", GetLineText ("Video Size:"), pCodec->width, pCodec->height);
			strcat (pInfoText, szInfoLine);
			sprintf (szInfoLine, "%s  %d / %d\r\n", GetLineText ("Aspect Ratio:"), pStream->sample_aspect_ratio.num, pStream->sample_aspect_ratio.den);
			strcat (pInfoText, szInfoLine);
		}
		else if (pCodec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			sprintf (szInfoLine, "%s  %s\r\n", GetLineText ("Audio Codec:"), avcodec_get_name (pCodec->codec_id));
			strcat (pInfoText, szInfoLine);

			sprintf (szInfoLine, "%s  %d  %d\r\n", GetLineText ("Audio Format:"), pCodec->sample_rate, pCodec->channels);
			strcat (pInfoText, szInfoLine);
		}

		sprintf (szInfoLine, "%s  %lld\r\n", GetLineText ("Duration:"), pStream->duration * 1000 * pStream->time_base.num / pStream->time_base.den);
		strcat (pInfoText, szInfoLine);

		sprintf (szInfoLine, "%s  %d\r\n", GetLineText ("Extradata Size:"), pCodec->extradata_size);
		strcat (pInfoText, szInfoLine);
	}

	if (strlen (pInfoText) * sizeof (TCHAR) > nSize)
	{
		delete []pInfoText;
		return YY_ERR_MEMORY;
	}

#ifdef _UNICODE
	MultiByteToWideChar (CP_ACP, 0, pInfoText, -1, pInfo, nSize);
#else
	strcpy (pInfo, pInfoText);
#endif // _UNICODE

	delete []pInfoText;
#endif // _OS_WIN32
	return 0;
}

char * CFFMpegTest::GetLineText (char * pLine)
{
	memset (m_szLineText, 0, sizeof (m_szLineText));

	int	nWidth = 32;
	int nLen = 0;
	if (strlen (pLine) < nWidth)
		nLen = nWidth - strlen (pLine);
	strcpy (m_szLineText, pLine);
	for (int i = 0; i < nLen; i++)
		strcat (m_szLineText, " ");

	return m_szLineText;
}

int CFFMpegTest::FillMediaInfo (YYINFO_Thumbnail * pInfo)
{
	if (pInfo == NULL)
		return YY_ERR_ARG;

	pInfo->nDuration = GetDuration ();
	pInfo->nBitrate = m_pFmtCtx->bit_rate;
	if (m_nIdxVideo >= 0)
	{
#ifdef UNICODE
		memset (pInfo->szVideoCodec, 0, sizeof (pInfo->szVideoCodec));
		MultiByteToWideChar (CP_ACP, 0,  avcodec_get_name (m_pStmVideo->codec->codec_id), -1, pInfo->szVideoCodec, sizeof (pInfo->szVideoCodec));
#else
		_tcscpy (pInfo->szVideoCodec, avcodec_get_name (m_pStmVideo->codec->codec_id));
#endif // UNICODE
		pInfo->nVideoWidth	= m_fmtVideo.nWidth;;
		pInfo->nVideoHeight	= m_fmtVideo.nHeight;
	}

	int nAudioIndex = m_nIdxAudio;
	if (nAudioIndex < 0)
		nAudioIndex = av_find_best_stream(m_pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (nAudioIndex >= 0 && m_pFmtCtx->streams[nAudioIndex]->codec->codec_id == AV_CODEC_ID_NONE)
		nAudioIndex = -1;

	if (nAudioIndex >= 0)
	{
		AVCodecContext * pAudioCodec = m_pFmtCtx->streams[nAudioIndex]->codec;
#ifdef UNICODE
		memset (pInfo->szAudioCodec, 0, sizeof (pInfo->szAudioCodec));
		MultiByteToWideChar (CP_ACP, 0,  avcodec_get_name (pAudioCodec->codec_id), -1, pInfo->szAudioCodec, sizeof (pInfo->szAudioCodec));
#else
		_tcscpy (pInfo->szAudioCodec, avcodec_get_name (pAudioCodec->codec_id));
#endif // UNICODE
		pInfo->nSampleRate		= pAudioCodec->sample_rate;
		pInfo->nChannels		= pAudioCodec->channels;
	}

	return YY_ERR_NONE;
}

int CFFMpegTest::SetParam (int nID, void * pParam)
{
	return YY_ERR_PARAMID;
}

int CFFMpegTest::GetParam (int nID, void * pParam)
{
	if (nID == YY_PLAY_BASE_META)
	{
		if (m_pFmtCtx == NULL || m_pFmtCtx->metadata == NULL)
			return YY_ERR_SOURCE;
		if (pParam == NULL)
			return YY_ERR_ARG;

		AVDictionaryEntry * pEntry = NULL;
		YYMETA_Value *		pValue = (YYMETA_Value *)pParam;
		memset (pValue->szValue, 0, sizeof (pValue->szValue));
		pValue->nSize = 0;

		pEntry = av_dict_get (m_pFmtCtx->metadata, pValue->szKey, NULL, 0);
		if (pEntry == NULL)
			return YY_ERR_FAILED;

		if (strlen (pEntry->value) > sizeof (pValue->szValue))
			return YY_ERR_MEMORY;

#ifdef _OS_WIN32
		char			szText[256];
		unsigned char *	pText = NULL;
		int				nTxtLen = 0;
		bool			bGB2312 = false;
		int				i = 0;

//		if ((unsigned char)pEntry->value[0] > 0XE0 || (unsigned char)pEntry->value[0] < 'z' )
		if (yyCheckTextUTF8 (pEntry->value))
		{
			MultiByteToWideChar (CP_UTF8, MB_ERR_INVALID_CHARS, pEntry->value, -1, pValue->szValue, sizeof (pValue->szValue));
		
			pText = (unsigned char *)pValue->szValue;
			nTxtLen = _tcslen (pValue->szValue) * 2;
			for (i = 0; i < nTxtLen; i+=2)
			{
				if (pText[i+1] != 0)
				{
					bGB2312 = false;
					break;
				}
				if (pText[i] > 'z')
					bGB2312 = true;
			}

			if (bGB2312)
			{
				memset (szText, 0, sizeof (szText));
				for (i = 0; i < nTxtLen / 2; i++)
					szText[i] = (char)pValue->szValue[i];

				memset (pValue->szValue, 0, sizeof (pValue->szValue));
				MultiByteToWideChar (936, MB_ERR_INVALID_CHARS, szText, -1, pValue->szValue, sizeof (pValue->szValue));
			}
		}
		else
		{
			MultiByteToWideChar (CP_ACP, 0, pEntry->value, -1, pValue->szValue, sizeof (pValue->szValue));
		}
#else
		strcpy (pValue->szValue, pEntry->value);
#endif // _OS_WIN32;

		return YY_ERR_NONE;
	}

	return YY_ERR_PARAMID;
}

bool CFFMpegTest::ForceCheckDuration (void)
{
	if (m_pFmtCtx == NULL)
		return false;

	if (strstr ("mpegts", m_pFmtCtx->iformat->name) == NULL)
		return false;

	AVPacket *	pPacket = new AVPacket ();
	if (pPacket == NULL)
		return false;;
	av_init_packet (pPacket);

	int nRC = av_read_frame (m_pFmtCtx, pPacket);
	if (nRC < 0)
	{
		delete pPacket;
		return false;
	}

	m_llFileStartTime = 0;
	if (m_nIdxAudio >= 0 && m_pStmAudio != NULL)
		m_llFileStartTime = yyBaseToTime (pPacket->pts, m_pStmAudio);
	else if (m_nIdxVideo >= 0 && m_pStmVideo != NULL)
		m_llFileStartTime = yyBaseToTime (pPacket->pts, m_pStmVideo);
	m_llAudioTime = m_llFileStartTime;
	m_llVideoTime = m_llFileStartTime;

	av_free_packet (pPacket);

	int			nStartTime = yyGetSysTime ();
	long long	llDur = 0;
	long long	llPos = 10000;
	AVStream *	pStream = m_pStmVideo;
	int			nIndex = m_nIdxVideo;
	if (m_nIdxAudio >= 0)
	{
		nIndex = m_nIdxAudio;
		pStream = m_pStmAudio;
	}

	nRC = 0;
	while (nRC >= 0)
	{
		nRC = av_read_frame (m_pFmtCtx, pPacket);
		if (nRC >= 0)
		{
			llDur = yyBaseToTime (pPacket->pts, pStream);
			av_free_packet (pPacket);

			if (llDur >= m_llDuration)
				m_llDuration = llDur;
		}
		else
		{
			break;
		}

		if (m_bForceClosed)
		{
			av_free_packet (pPacket);
			delete pPacket;
			return false;;
		}

		if (yyGetSysTime () - nStartTime > 10000)
			break;

		nRC = av_seek_frame (m_pFmtCtx, nIndex, yyTimeToBase (llPos, pStream), AVSEEK_FLAG_ANY);
		llPos += 10000;
	}

	av_free_packet (pPacket);
	delete pPacket;
	m_llDuration = m_llDuration - m_llFileStartTime;

	nRC = av_seek_frame (m_pFmtCtx, nIndex, yyTimeToBase (m_llFileStartTime, pStream), AVSEEK_FLAG_ANY);

	YYLOGI ("Check Duration Used: %d", yyGetSysTime () - nStartTime);
	return true;
}

void CFFMpegTest::ResetParam (int nLevel)
{
	CBaseSource::ResetParam (nLevel);

	m_pPktAudio = NULL;
	m_pPktVideo = NULL;

	m_nVideoReadCount = 0;
	m_llVideoTime = 0;
	m_nKeyFrameNum = 0;
	m_nVideoDropFrames = 0;
	m_nVideoFrameTime = 0;
	m_llVideoLastTime = 0;

	m_nAudioReadCount = 0;
	m_llAudioTime = 0;
	m_bNewAudioStream = false;

	m_llAudioTrackPos = 0;
	m_llVideoTrackPos = 0;
}

int CFFMpegTest::ReadProc (void * pParam)
{
	CFFMpegTest * pSource = (CFFMpegTest *)pParam;	
	yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_ABOVE_NORMAL);

	while (!pSource->m_bReadStop)
	{
		if (pSource->m_bReadPaused)
		{
			yySleep (10000);
			continue;
		}

		if (pSource->ReadLoop () < 0)
			yySleep (10000);
	}

	pSource->m_hReadThread = NULL;
	
	return 0;
}	

#define MIN_A_BUFFER	100
#define	MIN_V_BUFFER	100

int CFFMpegTest::ReadLoop (void)
{
	if (m_bEOF)
		return -1;

	CAutoLock lockSeek (&m_mtRead);
	if (m_lstAudio.GetCount () >= YYCFG_AUDIO_LIST_MAX - 2 || m_lstVideo.GetCount () >= YYCFG_VIDEO_LIST_MAX - 2)
		return -1;

	bool bRead = false;
	if (m_nIdxAudio >= 0 && m_lstAudio.GetCount () < 10)
	{
		ReadPacket ();
		bRead = true;
	}
	if (m_nIdxVideo >= 0)
	{
		if (m_nKeyFrameNum < 2 || m_lstVideo.GetCount () < 30)
		{
			ReadPacket ();
			bRead = true;
		}
	}

	if (m_nIdxVideo >= 0 && m_nIdxAudio >= 0)
	{
		if (m_nVideoReadCount < 150)
		{
			if (m_lstAudio.GetCount () >= 2 && m_lstVideo.GetCount () > 2)
			{
				if (m_lstVideo.GetCount () >= m_nVideoReadCount)
					bRead = false;
			}
		}
		else
		{
			if (m_nKeyFrameNum >= 1 && m_lstVideo.GetCount () >= 10 && m_lstAudio.GetCount () >= 5)
				bRead = false;
		}
	}
	else if (m_nIdxVideo >= 0)
	{
		if (m_nVideoReadCount < 150)
		{
			if (m_lstVideo.GetCount () >= m_nVideoReadCount && m_lstVideo.GetCount () > 10)
				bRead = false;
		}
		else
		{
			if (m_nKeyFrameNum >= 1 && m_lstVideo.GetCount () >= 10)
				bRead = false;
		}
	}
	else if (m_nIdxAudio >= 0)
	{
		if (m_lstAudio.GetCount () >= 5)
			bRead = false;
	}

//	YYLOGI ("Key % 4d, Video % 6d   Audio % 6d", m_nKeyFrameNum, m_lstVideo.GetCount (), m_lstAudio.GetCount ());

	return bRead ? 0 : -1;
}
