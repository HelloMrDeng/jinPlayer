/*******************************************************************************
	File:		CFFMpegSourceTool.cpp

	Contains:	The ffmpeg source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CFFMpegSource.h"

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

int CFFMpegSource::GetMediaInfo (TCHAR * pInfo, int nSize)
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

char * CFFMpegSource::GetLineText (char * pLine)
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

int CFFMpegSource::FillMediaInfo (YYINFO_Thumbnail * pInfo)
{
	if (pInfo == NULL)
		return YY_ERR_ARG;

	AVCodec * pCodec = NULL;
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
		pCodec = avcodec_find_decoder(m_pFmtCtx->streams[m_nIdxVideo]->codec->codec_id);
		if (pCodec == NULL)
			_tcscpy (pInfo->szVideoCodec, _T("unknown"));

		pInfo->nVideoWidth	= m_fmtVideo.nWidth;;
		pInfo->nVideoHeight	= m_fmtVideo.nHeight;
		pInfo->nVNum = m_fmtVideo.nNum;
		pInfo->nVDen = m_fmtVideo.nDen;
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
		pCodec = avcodec_find_decoder(m_pFmtCtx->streams[nAudioIndex]->codec->codec_id);
		if (pCodec == NULL)
			_tcscpy (pInfo->szAudioCodec, _T("unknown"));
		pInfo->nSampleRate		= pAudioCodec->sample_rate;
		pInfo->nChannels		= pAudioCodec->channels;
	}

	return YY_ERR_NONE;
}

int CFFMpegSource::SetParam (int nID, void * pParam)
{
	return YY_ERR_PARAMID;
}

int CFFMpegSource::GetParam (int nID, void * pParam)
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

int CFFMpegSource::CheckDurProc (void * pParam)
{
	yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_LOWEST);

	CFFMpegSource * pSource = (CFFMpegSource *)pParam;

	int nStart = yyGetSysTime ();
	
	int nRC = pSource->CheckDurFunc ();
	
	YYLOGT ("Local File", "The Duration is % 8d, used time: %d", (int)pSource->m_llDuration, yyGetSysTime () - nStart);
		
	pSource->m_hDurThread = NULL;

	return nRC;

}

int CFFMpegSource::CheckDurFunc (void)
{
	int					nRC = 0;
	char				szFileName[1024];
	URLProtocol			ioFileExt;
	AVFormatContext *	pFmtCtx = NULL;
	

	CBaseUtils::FillExtIOFunc (&ioFileExt);

	memset (szFileName, 0, sizeof (szFileName));
	sprintf (szFileName, "ExtIO:%08X", &ioFileExt);
	char * pFileName = szFileName + strlen (szFileName);
#ifdef _UNICODE
	CBaseUtils::ConvertDataToBase64 ((unsigned char *)m_szSource, _tcslen (m_szSource) * sizeof (TCHAR),
										pFileName, sizeof (szFileName) - strlen (szFileName));
#else
	strcat (szFileName, m_szSource);
#endif // UNICODE

	nRC = avformat_open_input(&pFmtCtx, szFileName, NULL, NULL);
	if (nRC < 0) return nRC;
    nRC = avformat_find_stream_info(pFmtCtx, NULL);

	int nIdxVideo = av_find_best_stream(pFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	int nIdxAudio = av_find_best_stream(pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

	AVPacket *	pPacket = new AVPacket ();
	av_init_packet (pPacket);
	nRC = av_read_frame (pFmtCtx, pPacket);
	if (nRC < 0)
	{
		avformat_close_input (&pFmtCtx);
		delete pPacket;
		return -1;
	}

	m_llFileStartTime = 0;
	if (nIdxAudio >= 0)
		m_llFileStartTime = yyBaseToTime (pPacket->pts, pFmtCtx->streams[nIdxAudio]);
	else if (nIdxVideo >= 0)
		m_llFileStartTime = yyBaseToTime (pPacket->pts, pFmtCtx->streams[nIdxVideo]);
	av_free_packet (pPacket);

	int			nIndex = nIdxVideo;
	AVStream *	pStream = NULL;
	if (nIdxVideo >= 0)
		pStream = pFmtCtx->streams[nIdxVideo];
	if (nIdxAudio >= 0)
	{
		nIndex = nIdxAudio;
		pStream = pFmtCtx->streams[nIdxAudio];
	}

	long long	llDur = 0;
	long long	llPos = 10000 + m_llFileStartTime;
	long long	llLastPts = -1;
	while (nRC >= 0)
	{
		nRC = av_read_frame (pFmtCtx, pPacket);
		if (nRC < 0)
			break;
		if (pPacket->pts != YY_64_INVALID)
		{
			if (llLastPts < 0)
				llLastPts = pPacket->pts;
			else if (pPacket->pts <= llLastPts)
				break;
			llLastPts = pPacket->pts;
		}

		llDur = yyBaseToTime (pPacket->pts, pStream) - m_llFileStartTime;
		av_free_packet (pPacket);

		if (m_bForceClosed || m_bReadStop)
			break;

		nRC = av_seek_frame (pFmtCtx, nIndex, yyTimeToBase (llPos, pStream), AVSEEK_FLAG_ANY);
		llPos += 30000;
	}

	av_free_packet (pPacket);
	delete pPacket;

	avformat_close_input (&pFmtCtx);

	m_llDuration = llDur;

	return 0;
}

