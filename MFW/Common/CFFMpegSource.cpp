/*******************************************************************************
	File:		CFFMpegSource.cpp

	Contains:	The ffmpeg source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CFFMpegSource.h"

#include "CBaseUtils.h"
#include "CBaseFile.h"
#include "CBaseExtData.h"
#include "CLicenseCheck.h"

#include "yyMetaData.h"

#include "UFFMpegFunc.h"
#include "UStringFunc.h"
#include "UThreadFunc.h"
#include "USystemFunc.h"
#include "UFileFormat.h"

#include "yyConfig.h"
#include "yyLog.h"

// The openstatus value
// -1 force to exit normal file parser.
// 1001 force to exit the ts parser
// 1100 force to exit find stream info immediately.
// >1100 force to exit find stream info after parse - 1100 frames.

static CFFMpegSource * g_ffSource = NULL;

CFFMpegSource::CFFMpegSource(void * hInst)
	: CBaseSource (hInst)
	, m_pFmtCtx (NULL)
	, m_pTmpCtx (NULL)
	, m_llFileStartTime (0)
	, m_bFileStartTimeSet (false)
	, m_nIdxAudio (-1)
	, m_pStmAudio (NULL)
	, m_nIdxVideo (-1)
	, m_pStmVideo (NULL)
	, m_nIdxSubTT (-1)
	, m_pStmSubTT (NULL)
	, m_hReadThread (NULL)
	, m_bReadStop (false)
	, m_bReadPaused (false)
	, m_hDurThread (NULL)
{
	SetObjectName ("CFFSource");

	ResetParam (0);

	g_ffSource = this;
}

CFFMpegSource::~CFFMpegSource(void)
{
	Close ();

	g_ffSource = NULL;
}

int CFFMpegSource::Open (const TCHAR * pSource, int nType)
{
	CAutoLock lock (&m_mtFile);
	// close the opened source first.
	Close ();
	ResetParam (0);

	TCHAR * pSourceName = (TCHAR *)pSource;
	m_nSourceType = nType;
	if ((m_nSourceType & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ)
	{
		YY_READ_EXT_DATA * pExtData = (YY_READ_EXT_DATA *)pSource;
		pSourceName = pExtData->szName;
		CBaseExtData::g_pExtData = pExtData;
		g_ioFileExt.url_read = yy_extio_read;

		if (pExtData->llSize == 0)
			m_bAudioNeedBuffer = true;
		_tcscpy (m_szSource, pSourceName);
	}
	else
	{
		g_ioFileExt.url_read = CBaseUtils::yy_extio_read;
		_tcscpy (m_szSource, pSource);
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
	if (m_nProtType == YY_PROT_FILE || m_nProtType == YY_PROT_PDP)
#else
	if (m_nProtType == YY_PROT_FILE || m_nProtType == YY_PROT_PDP)
#endif // _OS_WIN32
	{
		memset (szFileName, 0, sizeof (szFileName));
		sprintf (szFileName, "ExtIO:%08X", &g_ioFileExt);
		char * pFileName = szFileName + strlen (szFileName);
#ifdef _UNICODE
		CBaseUtils::ConvertDataToBase64 ((unsigned char *)pSourceName, _tcslen (pSourceName) * sizeof (TCHAR),
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
	if (m_pTmpCtx->openstatus == 1001)
		m_pTmpCtx->openstatus = 0;
	m_nReadExtTime = 0;
	m_pFmtCtx = m_pTmpCtx;

//	YYLOGI ("Open source status: File: %d, UUID: %s!", CBaseFile::g_bFileError, CLicenseCheck::m_pLcsChk->GetCMUUID ());
	// It will contine to playback for AVI file.
	if (CBaseFile::g_bFileError)
	{
		// Sino Embed	
		if (!strncmp (CLicenseCheck::m_pLcsChk->GetCMUUID (), "SNPL", 4))
			return YY_ERR_FAILED;

		if (strcmp (m_pFmtCtx->iformat->name, "avi"))
			return YY_ERR_FAILED;
	}
	
	if ((m_nSourceType & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ)
	{
		YYLOGI ("Open source successed!");
		m_pTmpCtx->openstatus = 1130;   
	}

    // retrieve stream information
    nRC = avformat_find_stream_info(m_pFmtCtx, NULL);
	m_pTmpCtx->openstatus = 0;    
	if (nRC < 0)
	{
		YYLOGE ("avformat_find_stream_info failed. %d", nRC);
		return nRC;
	}

	if ((m_nSourceType & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ)
		YYLOGI ("It found the stream info!");

	AVCodecContext * pDecCtx = NULL;
	if ((m_nSourceType & YY_OPEN_SRC_AUDIO) == YY_OPEN_SRC_AUDIO)
		m_nIdxVideo = -1;
	else
		m_nIdxVideo = av_find_best_stream(m_pFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (m_nIdxVideo >= 0)
	{
		// Check the video stream to get best video stream
		int nVideoNum = 0;
		int	nVideoIdx = -1;
		int nMaxW = 0;
		for (int i = 0; i < m_pFmtCtx->nb_streams; i++)
		{
			pDecCtx = m_pFmtCtx->streams[i]->codec;
			if (m_pFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				if (pDecCtx->width > nMaxW)
				{
					nMaxW = pDecCtx->width;
					nVideoIdx = i;
				}
				nVideoNum++;
			}
		}
		if (nVideoNum > 1 && nVideoIdx != m_nIdxVideo)
		{
			if (m_pFmtCtx->streams[nVideoIdx]->nb_frames >= m_pFmtCtx->streams[m_nIdxVideo]->nb_frames)
				m_nIdxVideo = nVideoIdx;
		}

		pDecCtx = m_pFmtCtx->streams[m_nIdxVideo]->codec;
		if (CheckConfigLimit (pDecCtx->codec_id, pDecCtx->width, pDecCtx->height) != YY_ERR_NONE)
			return YY_ERR_OUTOFLIMIT;
	}

	// Find the audio stream, Open the audio decoder
	if ((m_nSourceType & YY_OPEN_SRC_VIDEO) == YY_OPEN_SRC_VIDEO)
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

			// try to select not dts audio stream
			if (m_nAudioStreamNum > 1 && m_pFmtCtx->streams[m_nIdxAudio]->codec->codec_id == AV_CODEC_ID_DTS)
			{
				int nIndexNum = 0;
				for (int i = 0; i < m_pFmtCtx->nb_streams; i++)
				{
					if (m_pFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
					{
						if (i != m_nIdxAudio)
						{
							m_nIdxAudio = i;
							m_nAudioStreamPlay = nIndexNum;
							break;
						}
						nIndexNum++;
					}
				}
			}

			pDecCtx = m_pFmtCtx->streams[m_nIdxAudio]->codec;
			m_pStmAudio = m_pFmtCtx->streams[m_nIdxAudio];
			m_fmtAudio.nCodecID = pDecCtx->codec_id;
			m_fmtAudio.nSampleRate = pDecCtx->sample_rate;
			m_fmtAudio.nChannels = pDecCtx->channels;
			m_fmtAudio.nBits = 16;

			m_fmtAudio.nSourceType = YY_SOURCE_FF;
			m_fmtAudio.pPrivateData = pDecCtx;
		}
	}
	if ((m_nSourceType & YY_OPEN_SRC_VIDEO) == YY_OPEN_SRC_VIDEO || !m_bSubTTEnable)
		m_nIdxSubTT = -1;
	else
		m_nIdxSubTT = av_find_best_stream(m_pFmtCtx, AVMEDIA_TYPE_SUBTITLE, -1, -1, NULL, 0);
	if (m_nIdxSubTT >= 0 && m_nIdxSubTT < m_pFmtCtx->nb_streams)
	{
		pDecCtx = m_pFmtCtx->streams[m_nIdxSubTT]->codec;
		m_pStmSubTT = m_pFmtCtx->streams[m_nIdxSubTT];
		m_fmtSubTT.nSourceType = YY_SOURCE_FF;
		m_fmtSubTT.nCodecID = pDecCtx->codec_id;
		m_fmtSubTT.pPrivateData = pDecCtx;
		m_nSubTTStreamNum = 0;
		for (int i = 0; i < m_pFmtCtx->nb_streams; i++)
		{
			if (m_pFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_SUBTITLE)
			{
				if (i == m_nIdxSubTT)
					m_nSubTTStreamPlay = m_nSubTTStreamNum;
				m_nSubTTStreamNum++;
			}
		}
	}
	else
	{
		m_nIdxSubTT = -1;
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
	m_bFileStartTimeSet = false;
	// Use the first frame PTS as start time 2014-07-30
	if (0)//m_pFmtCtx->start_time > 0) 
	{
		if (m_nIdxAudio >= 0 && m_pStmAudio != NULL)
			m_llFileStartTime = yyBaseToTime (m_pStmAudio->start_time, m_pStmAudio);
		else if (m_nIdxVideo >= 0 && m_pStmVideo != NULL)
			m_llFileStartTime = yyBaseToTime (m_pStmVideo->start_time, m_pStmVideo);
	}

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
	if (m_llDuration <= 0 && m_nProtType == YY_PROT_FILE)
	{
		if ((m_nSourceType & 0X000F0000) == 0) // local file
		{
			int nID = 0;
			yyThreadCreate (&m_hDurThread, &nID, CheckDurProc, this, 0);
		}
	}

	m_bEOF = false;
	m_bEOA = false;
	m_bEOV = false;

	return YY_ERR_NONE;
}

int CFFMpegSource::Close (void)
{
	CAutoLock lock (&m_mtFile);

	ReleasePacket (YY_MEDIA_All);

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

int CFFMpegSource::ForceClose (void)
{
	m_bForceClosed = true;

	if (m_pTmpCtx != NULL)
		m_pTmpCtx->openstatus = -1;

	return YY_ERR_NONE;
}

int	CFFMpegSource::Start (void)
{
	bool bCreateThread = false;
#if _OS_ANDROID
	bCreateThread = true;
#endif // check
	if (!bCreateThread)
	{
		if ((m_nSourceType & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ)
		{
			if (CBaseExtData::g_pExtData->llSize == 0)
				bCreateThread = true;
		}
	}

	if (!bCreateThread)
		return YY_ERR_NONE;

	m_bReadPaused = false;

	if (m_hReadThread != NULL)
		return YY_ERR_NONE;

	m_bReadStop = false;

	int nID = 0;
	yyThreadCreate (&m_hReadThread, &nID, ReadProc, this, 0);

	return YY_ERR_NONE;
}

int CFFMpegSource::Pause (void)
{
//	m_bReadPaused = true;

	return YY_ERR_NONE;
}

int	CFFMpegSource::Stop (void)
{
	m_bReadStop = true;
	int nTryTimes = 0;
	while (m_hReadThread != NULL || m_hDurThread != NULL)
	{
		yySleep (10000);
		nTryTimes++;
		if (nTryTimes > 100)
			break;
	}

	return YY_ERR_NONE;
}

int CFFMpegSource::SetStreamPlay (YYMediaType nType, int nStream)
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

	m_llVideoTrackPos = m_llAudioTrackPos;
	pPacket = m_lstVideo.GetTail ();
	while (pPacket != NULL)
	{
		if (pPacket->pts == YY_64_INVALID)
		{
			m_lstVideo.RemoveTail ();
			pPacket = m_lstVideo.GetTail ();
			continue;
		}
		m_llVideoTrackPos = pPacket->pts;
		break;
	}

	ReleasePacket (YY_MEDIA_Audio);

	long long llPosAudio = yyTimeToBase (m_llAudioTrackPos + m_llFileStartTime, m_pStmAudio);
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

int CFFMpegSource::ReadData (YY_BUFFER * pBuff)
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
	if ((pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY)
		pBuff->uFlag |= YYBUFF_KEY_FRAME;	
	if (pPacket->pts == YY_64_INVALID && pPacket->dts != YY_64_INVALID)
		pBuff->llTime = pPacket->dts;
	else
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
		m_nVideoReadCount++;
		if (pPacket->pts == YY_64_INVALID && pPacket->dts != YY_64_INVALID)
			pPacket->pts = pPacket->dts;
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
	else if (pBuff->nType == YY_MEDIA_SubTT)
	{
		if (m_bSubTTNewPos)
		{
			m_bSubTTNewPos = false;
			pBuff->uFlag |= YYBUFF_NEW_POS;
		}
		m_nSubTTReadCount++;
		CAutoLock lockPacket (&m_mtPacket);
		if (m_pPktSubTT != NULL)
		{
			//av_free_packet (m_pPktSubTT);
			m_lstMedia.AddTail (m_pPktSubTT);
		}
		m_pPktSubTT = pPacket;
	}
	return YY_ERR_NONE;
}

int CFFMpegSource::SetPos (long long llPos)
{
	CAutoLock lock (&m_mtFile);
	if (m_pFmtCtx == NULL)
		return YY_ERR_FAILED;

	CAutoLock lockSeek (&m_mtRead);
	CAutoLock lockPacket (&m_mtPacket);

	int			nRC = 0;
	AVPacket *	pPacket = NULL;
	long long	llNewPos = llPos + m_llFileStartTime;
	long long	llVideoPos = YY_64_MAXVAL;
	bool		bSeekAudio = false;

	unsigned int	nStartTime = yyGetSysTime ();
	long long		llPosAudio = 0;

	while (!m_bFileStartTimeSet)
		ReadPacket ();

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

			if (!bSeekAudio && _tcslen (m_szSource) > 0)
			{
				if (yyffGetType (m_szSource, 0) == YY_MEDIA_Audio)
					bSeekAudio = true;
			}
		}
	}

	// Seek the pos
	if (bSeekAudio)
	{
		if (m_llDurAudio > 0 && llPos > m_llDurAudio)
		{
			llPos = m_llDurAudio - 2000;
			if (llPos < 0)
				llPos = 0;
			llPosAudio = yyTimeToBase (llPos + m_llFileStartTime, m_pStmAudio);
		}

		nRC = av_seek_frame (m_pFmtCtx, m_nIdxAudio, llPosAudio, AVSEEK_FLAG_ANY);
		if (nRC != 0)
		{
			YYLOGI ("Set Pos Failed! return %d", nRC);
			return YY_ERR_FAILED;
		}

		ReleasePacket (YY_MEDIA_All);
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
					llVideoPos = pPacket->pts;
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

		ReleasePacket (YY_MEDIA_All);
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
						llVideoPos = pPacket->pts;
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
			{
				llVideoPos = pPacket->pts;
				break;
			}

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

			ReleasePacket (YY_MEDIA_All);

			llNewPos -= llPos / 5;
			if (llNewPos < 0)
				llNewPos = 0;

			llPosVideo = yyTimeToBase (llNewPos, m_pStmVideo);
			nRC = av_seek_frame (m_pFmtCtx, m_nIdxVideo, llPosVideo, AVSEEK_FLAG_BACKWARD);
			if (nRC < 0)
				break;

			if (yyGetSysTime () - nStartTime > 2000)
				break;
		}
	}
	CBaseSource::SetPos (llPos);

	nStartTime = yyGetSysTime ();
	while (m_nIdxAudio >= 0 && llVideoPos != YY_64_MAXVAL)
	{
		pPacket = m_lstAudio.GetHead ();
		while (pPacket == NULL)
		{
			ReadPacket ();
			pPacket = m_lstAudio.GetHead ();
			if (yyGetSysTime () - nStartTime > 2000)
				break;
		}
		if (pPacket != NULL)
		{
			if (pPacket->pts >= llVideoPos)
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
	return YY_ERR_NONE;
}

AVPacket * CFFMpegSource::GetPacket (YYMediaType  nType, long long llPlayTime)
{
	CAutoLock lock (&m_mtPacket);
	if (m_hReadThread == NULL)
		ReadPacket ();
	AVPacket *	pPacket = NULL;
	if (nType == YY_MEDIA_Video)
	{
		// try to read more video packet for drop frames
		if (m_hReadThread == NULL && m_nVideoReadCount > YYCFG_VIDEO_LIST_FIRST)
		{
			// Try to read more two packet if it was too later.
			AVPacket *	pLastPacket = m_lstVideo.GetTail ();
			int			nPacketCount = m_lstVideo.GetCount ();
			if (nPacketCount < YYCFG_VIDEO_LIST_FIRST || 
				(pLastPacket != NULL && pLastPacket->pts != YY_64_INVALID && pLastPacket->pts < llPlayTime + 5000))
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
			if ((pPacket->flags & AV_PKT_FLAG_KEY) != AV_PKT_FLAG_KEY || m_nKeyFrameNum > m_lstVideo.GetCount () / 2)
			{
				// if it was late than the playing time, try to jump to next key frame.
				if (pPacket->pts != YY_64_INVALID && pPacket->pts < llPlayTime)
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
		if (m_bAudioNeedBuffer)
		{
			AVPacket * pPacket0 = m_lstAudio.GetHead ();
			AVPacket * pPacket1 = m_lstAudio.GetTail ();
			if (pPacket0 == NULL || pPacket1 == NULL)
				return NULL;

			long long llBuffTime = pPacket1->pts - pPacket0->pts;
			if (llBuffTime < 800 && m_lstAudio.GetCount () < 20)
				return NULL;
		}

		if ((m_nSourceType & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ)
		{
			if (m_lstAudio.GetCount () <= 0)
			{
				if (CBaseExtData::g_pExtData->llSize == 0)
				{
					m_bAudioNeedBuffer = true;
					return NULL;
				}
			}
		}
		m_bAudioNeedBuffer = false;

		pPacket = m_lstAudio.RemoveHead ();
		if (pPacket != NULL)
			return pPacket;
	}
	else if (nType == YY_MEDIA_SubTT)
	{
		pPacket = m_lstSubTT.RemoveHead ();
		if (pPacket != NULL)
			return pPacket;
	}

	return NULL;
}

int CFFMpegSource::ReadPacket (void)
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
/*
//#ifdef _CPU_MSB2531
			MEMORYSTATUS	memInfo;
			memset (&memInfo, 0, sizeof (memInfo));
			GlobalMemoryStatus (&memInfo);
			if (memInfo.dwAvailVirtual < 1024 * 1024 * 10)
			{
				m_bEOF = true;
				return -1;
			}
//#endif // _CPU_MSB2531
*/
			pPacket = new AVPacket ();
			if (pPacket == NULL)
			{
				m_bEOF = true;
				return -1;
			}
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
		if (m_llAudioTrackPos > 0)
		{
			if (pPacket->pts == YY_64_INVALID || pPacket->pts <= m_llAudioTrackPos)
			{
				av_free_packet (pPacket);
				m_lstMedia.AddTail (pPacket);
				return 0;
			}
			else
			{
				YYLOGI ("Audio Reach the switch track pos %d", (int)m_llAudioTrackPos);
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

		if (m_nVideoFrameTime == 0 && m_nVideoFrameCount > 1)
			m_nVideoFrameTime = (int)((pPacket->pts - m_llVideoFrameLast) / (m_nVideoFrameCount));
		m_llVideoFrameLast = pPacket->pts;

		m_nVideoFrameCount++;

		if (m_llVideoTrackPos > 0)
		{
			if (pPacket->pts == YY_64_INVALID || pPacket->pts <= m_llVideoTrackPos)
			//if (pPacket->pts != m_llVideoTrackPos)
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
	else if (pPacket->stream_index == m_nIdxSubTT)
	{
		UpdatePacketTime (pPacket);
		if (m_llSubTTTrackPos > 0)
		{
			if (pPacket->pts == YY_64_INVALID || pPacket->pts <= m_llSubTTTrackPos)
			{
				av_free_packet (pPacket);
				m_lstMedia.AddTail (pPacket);
				return 0;
			}
			else
			{
				YYLOGI ("SubTT Reach the switch track pos %d", (int)m_llSubTTTrackPos);
				m_llSubTTTrackPos = -1;
			}
		}

		if (m_lstSubTT.GetCount () < YYCFG_SUBTT_LIST_MAX)
		{
			m_lstSubTT.AddTail (pPacket);
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

void CFFMpegSource::UpdatePacketTime (AVPacket * pPacket)
{
	if (pPacket == NULL)
		return;

	if (pPacket->pts != YY_64_INVALID)
		pPacket->pts = yyBaseToTime (pPacket->pts, m_pFmtCtx->streams[pPacket->stream_index]);
	
	if (pPacket->dts != YY_64_INVALID)
	{
		pPacket->dts = yyBaseToTime (pPacket->dts, m_pFmtCtx->streams[pPacket->stream_index]);
		if (pPacket->pts == YY_64_INVALID)
			pPacket->pts = pPacket->dts;
	}
	if (pPacket->duration == YY_64_INVALID)
		pPacket->duration = 30;
	else
		pPacket->duration = yyBaseToTime (pPacket->duration, m_pFmtCtx->streams[pPacket->stream_index]);

	if (!m_bFileStartTimeSet && (pPacket->pts != YY_64_INVALID || pPacket->dts != YY_64_INVALID))
	{
		if (pPacket->pts != YY_64_INVALID)
			m_llFileStartTime = pPacket->pts;
		else
			m_llFileStartTime = pPacket->dts;
		m_bFileStartTimeSet = true;
		YYLOGI ("The file first start time is % 8d", (int)m_llFileStartTime);
	}
	if (m_llFileStartTime != 0)
	{
		if (pPacket->dts != YY_64_INVALID)
			pPacket->dts -= m_llFileStartTime;
		if (pPacket->pts != YY_64_INVALID)
			pPacket->pts -= m_llFileStartTime;
	}
	if (pPacket->pts < -1000 && pPacket->pts != YY_64_INVALID)
	{
		m_llFileStartTime = pPacket->pts + m_llFileStartTime;
		YYLOGI ("The file start time changed % 8d", (int)m_llFileStartTime);
	}
}

long long CFFMpegSource::GetVideoBuffTime (void)
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

AVPacket * CFFMpegSource::GetNextKeyFrame (long long llTime)
{
	AVPacket *	pPacket = NULL;
	if (m_nKeyFrameNum <= 0)
		return NULL;

	pPacket = m_lstVideo.GetTail ();
	if (pPacket == NULL)
		return NULL;
	if (pPacket->pts != YY_64_INVALID && pPacket->pts < llTime && m_nKeyFrameNum <= 0)
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
		{
			if ((pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY)
				return pPacket;
			else
				return NULL;
		}

		if ((pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY)
		{
			if (m_nKeyFrameNum < m_lstVideo.GetCount () / 2)
				return pPacket;
			else if (pPacket->pts >= llKeyTime)
				return pPacket;
		}

		pPacket = m_lstVideo.GetNext (pPos);
	}
	pPacket = m_lstVideo.GetTail ();
	if (pPacket != NULL && (pPacket->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY)
	{
		if (pPacket->pts != YY_64_INVALID && pPacket->pts <= llKeyTime)
			return pPacket;
	}

	return NULL;
}

bool CFFMpegSource::ReleasePacket (YYMediaType nType)
{
	CAutoLock lock (&m_mtPacket);
	AVPacket * pPacket = NULL;
	
	if (nType == YY_MEDIA_Audio || nType == YY_MEDIA_All)
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
	if (nType == YY_MEDIA_Video || nType == YY_MEDIA_All)
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
	if (nType == YY_MEDIA_SubTT || nType == YY_MEDIA_All)
	{
		if (m_pPktSubTT != NULL)
		{
			av_free_packet (m_pPktSubTT);
			m_lstMedia.AddTail (m_pPktSubTT);
			m_pPktSubTT = NULL;
		}
		pPacket = m_lstSubTT.RemoveHead ();
		while (pPacket != NULL)
		{
			av_free_packet (pPacket);
			m_lstMedia.AddTail (pPacket);
			pPacket = m_lstSubTT.RemoveHead ();
		}
		m_nSubTTReadCount = 0;
	}
	NODEPOS pos = m_lstMedia.GetHeadPosition ();
	while (pos != NULL)
	{
		pPacket = m_lstMedia.GetNext (pos);
		if (pPacket != NULL)
			av_free_packet (pPacket);
	}
	return true;
}

void CFFMpegSource::ResetParam (int nLevel)
{
	CBaseSource::ResetParam (nLevel);

	m_pPktAudio = NULL;
	m_pPktVideo = NULL;
	m_pPktSubTT = NULL;

	m_nVideoReadCount = 0;
	m_nKeyFrameNum = 0;
	m_nVideoDropFrames = 0;
	m_nVideoFrameTime = 0;
	m_nVideoFrameCount = 0;
	m_llVideoFrameLast = 0;

	m_nAudioReadCount = 0;
	m_bNewAudioStream = false;
	m_bAudioNeedBuffer = false;

	m_nSubTTReadCount = 0;

	m_llAudioTrackPos = 0;
	m_llVideoTrackPos = 0;
	m_llSubTTTrackPos = 0;

	m_llReadExtSize = 0;
	m_nReadExtTime = 0;
	m_nExtBitrate = 0;
}

int CFFMpegSource::ReadProc (void * pParam)
{
	CFFMpegSource * pSource = (CFFMpegSource *)pParam;	
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

int CFFMpegSource::ReadLoop (void)
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


int CFFMpegSource::yy_extio_read (URLContext *h, unsigned char *buf, int size)
{
	if (g_ffSource->m_nReadExtTime == 0)
	{
//		g_ffSource->m_nExtBitrate = 52428;
		g_ffSource->m_nReadExtTime = yyGetSysTime ();
	}
				
	int nRC = CBaseUtils::yy_extio_read (h, buf, size);
		
	if (nRC > 0)
		g_ffSource->m_llReadExtSize += nRC;

	if (g_ffSource->m_pFmtCtx == NULL)
	{
		if (yyGetSysTime () - g_ffSource->m_nReadExtTime > 1200)
		{
			if (g_ffSource->m_llReadExtSize > 1024 * 100)
			{
				g_ffSource->m_pTmpCtx->openstatus = 1001;
				//YYLOGT (g_ffSource->m_szObjName, "Force to exit ts parser! Time: %d, Size: %d", yyGetSysTime () - g_ffSource->m_nReadExtTime, (int)g_ffSource->m_llReadExtSize);
			}
		}
	}
	
	if (g_ffSource->m_nExtBitrate > 0)
	{
		while ((yyGetSysTime () - g_ffSource->m_nReadExtTime) * g_ffSource->m_nExtBitrate  < g_ffSource->m_llReadExtSize * 1000)
			yySleep (5000);
	}
	
	return nRC;
}
