/*******************************************************************************
	File:		CDataConvert.cpp

	Contains:	The data convert implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CDataConvert.h"

#include "USYstemFunc.h"

#include "CLicenseCheck.h"
#include "yyConfig.h"
#include "yyLog.h"
#include "yyLogoData.h"
#include "UFFMpegFunc.h"

CDataConvert::CDataConvert(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_pVideoRCC (NULL)
	, m_pFFMpegASM (NULL)
	, m_ppVRCC (NULL)
	, m_pInBuff (NULL)
	, m_pAVFrame (NULL)
	, m_pVideoBuff (NULL)
	, m_fSpeed (1.0)
	, m_nLicenseStatus (0)
{
	SetObjectName ("CDataConvert");

	m_nCPUNum = yyGetCPUNum ();
#ifdef _OS_WIN32
	m_nCPUNum = 1;
#endif // _OS_WIN32
	
#ifdef _OS_NDK
	char szPath[1024];
	yyGetAppPath (NULL, szPath, 1024);
	if (strstr (szPath, "com.cansure.yysampleplayer") != NULL)
		m_nLicenseStatus = 20130311;
#endif // _OS_NDK
}

CDataConvert::~CDataConvert(void)
{
	YY_DEL_P (m_pVideoRCC);
	YY_DEL_P (m_pFFMpegASM);

	if (m_ppVRCC != NULL)
	{
		for (int i = 0; i < m_nCPUNum; i++)
			delete m_ppVRCC[i];
		delete []m_ppVRCC;
	}
	m_ppVRCC = NULL;

	YY_DEL_A (m_pInBuff);
	YY_DEL_A (m_pAVFrame);
	YY_DEL_A (m_pVideoBuff);
}

int CDataConvert::Convert (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff, RECT * pZoom)
{
	CAutoLock lock (&m_mtConvert);

	if (pInBuff == NULL || pOutBuff == NULL)
		return YY_ERR_ARG;

//	int nStart = yyGetSysTime ();
	
	int nRC = YY_ERR_NONE;
	if (pInBuff->nType == YY_MEDIA_Video)
		nRC = ConvertVideo (pInBuff, pOutBuff, pZoom);
	else if (pInBuff->nType == YY_MEDIA_Audio)
		nRC = ConvertAudio (pInBuff, pOutBuff);
		
//	YYLOGI ("Color Convert used % 8d", yyGetSysTime () - nStart);

	return nRC;
}

void CDataConvert::SetSpeed (float fSpeed)
{
	if (m_fSpeed == fSpeed)
		return;

	m_fSpeed = fSpeed;

	if (m_pFFMpegASM != NULL)
		delete m_pFFMpegASM;
	m_pFFMpegASM = NULL;

	return;
}

int CDataConvert::ConvertAudio (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff)
{
	unsigned char **	ppData = (unsigned char **)pOutBuff->pBuff;
	int					nSize = pOutBuff->uSize;
	YY_AUDIO_FORMAT *	pFmtAudio = (YY_AUDIO_FORMAT *)pOutBuff->pFormat;

	AVFrame *		pFrame = NULL;
	bool			bConvert = false;

	if ((pInBuff->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
	{
		pFrame = (AVFrame *)pInBuff->pBuff;
		if (pFrame->format != AV_SAMPLE_FMT_S16 || pFrame->channels > 2 || m_fSpeed != 1.0)
		{
			bConvert = true;
		}
		else
		{
			pOutBuff->uSize = pFrame->nb_samples * pFrame->channels * 2;
			memcpy (*ppData, pFrame->data[0], pOutBuff->uSize);
		}
	}
	else if ((pInBuff->uFlag & YYBUFF_TYPE_DATA) == YYBUFF_TYPE_DATA)
	{
		if (m_fSpeed != 1.0)
		{
			bConvert = true;
		}
		else
		{			
			memcpy (*ppData, pInBuff->pBuff, pInBuff->uSize);
			pOutBuff->uSize = pInBuff->uSize;
		}
	}

	if (bConvert)
	{
		if (m_pFFMpegASM == NULL)
			m_pFFMpegASM = new CFFMpegAudioRSM (m_hInst);

		uint64_t		nChannelLayout = AV_CH_LAYOUT_STEREO;
		int				nSampleRate = pFmtAudio->nSampleRate;
		AVSampleFormat	nSampleFormat = AV_SAMPLE_FMT_S16;
		uint64_t		nSampleLayout = AV_CH_LAYOUT_STEREO;

		if (pFrame != NULL)
		{
			nChannelLayout = pFrame->channel_layout;
			if (pFrame->channels > 2)
				nChannelLayout = AV_CH_LAYOUT_STEREO;
			nSampleRate = pFrame->sample_rate;
			nSampleFormat = (AVSampleFormat) pFrame->format;
			nSampleLayout = pFrame->channel_layout;
		}

		if (nChannelLayout <= 0)
		{
			if (pFrame->channels == 1)
				nChannelLayout = AV_CH_LAYOUT_MONO;
			else
				nChannelLayout = AV_CH_LAYOUT_STEREO;
		}
		if (nSampleLayout <= 0)
		{
			if (pFrame->channels == 1)
				nSampleLayout = AV_CH_LAYOUT_MONO;
			else
				nSampleLayout = AV_CH_LAYOUT_STEREO;
		}
		m_pFFMpegASM->Init (nChannelLayout, AV_SAMPLE_FMT_S16, nSampleRate / m_fSpeed, 
							nSampleLayout, nSampleFormat, nSampleRate);

		int					nSourceSamples = 0;
		unsigned char **	ppSourceBuff = NULL;
		if (pFrame != NULL)
		{
			nSourceSamples = pFrame->nb_samples;
			ppSourceBuff = &pFrame->data[0];
		}
		else
		{
			nSourceSamples = pInBuff->uSize / (pFmtAudio->nChannels * pFmtAudio->nBits / 8);
			ppSourceBuff = &pInBuff->pBuff;
		}

		int nSamples = m_pFFMpegASM->ConvertData (ppSourceBuff, nSourceSamples, ppData, nSize);
		if (pFmtAudio->nChannels > 2)
			pOutBuff->uSize = nSamples * 2 * 2;
		else
			pOutBuff->uSize = nSamples * pFmtAudio->nChannels * 2;
	}

	return YY_ERR_NONE;
}

int CDataConvert::ConvertVideo (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff, RECT * pZoom)
{
	if ((pOutBuff->uFlag & YYBUFF_TYPE_VIDEO) != YYBUFF_TYPE_VIDEO)
		return YY_ERR_ARG;	

	int				nRC = YY_ERR_NONE;

	AVFrame *		pFrmVideo = NULL;
	YY_VIDEO_BUFF * pBufVideo = NULL;
	if ((pInBuff->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
		pFrmVideo = (AVFrame *)pInBuff->pBuff;
	else if ((pInBuff->uFlag & YYBUFF_TYPE_VIDEO) == YYBUFF_TYPE_VIDEO)
		pBufVideo = (YY_VIDEO_BUFF *)pInBuff->pBuff;

	YY_VIDEO_BUFF * pVideoBuff = (YY_VIDEO_BUFF *)pOutBuff->pBuff;
	
	if (pVideoBuff->nType == YY_VDT_YUV420_P || m_nCPUNum <= 1 || pZoom != NULL ||
		(pFrmVideo != NULL && pFrmVideo->format != AV_PIX_FMT_YUV420P))
	{
		if (m_pVideoRCC == NULL)
			m_pVideoRCC = new CFFMpegVideoRCC (m_hInst);		
		nRC = m_pVideoRCC->ConvertBuff (pInBuff, pVideoBuff, pZoom);
		if (m_nLicenseStatus != 20130311)
			OverLogo (pVideoBuff);

		return nRC;
	}
	
	int i = 0;
	if (m_ppVRCC == NULL)
	{
		m_ppVRCC = new CVideoRCCThread*[m_nCPUNum];
		for (i = 0; i < m_nCPUNum; i++)
			m_ppVRCC[i] = new CVideoRCCThread (m_hInst, i);

		m_pInBuff = new YY_BUFFER[m_nCPUNum];
		m_pAVFrame = new AVFrame[m_nCPUNum];
		m_pVideoBuff = new YY_VIDEO_BUFF[m_nCPUNum];
		
		bool bReady = false;
		while (!bReady)
		{
			bReady = true;
			for (i = 0; i < m_nCPUNum; i++)
			{
				if (!m_ppVRCC[i]->WaitTask ())
				{
					bReady = false;
					break;
				}
			}
			if (bReady)
				break;
			yySleep (1000);
		}	
	}
	

	int	nStep = 0;
	for (i = 0; i < m_nCPUNum; i++)
	{
		memcpy (&m_pInBuff[i], pInBuff, sizeof (YY_BUFFER));
		if (pFrmVideo != NULL)
		{
			nStep = pFrmVideo->height / m_nCPUNum;
			nStep = nStep & 0XFFFFFFFE;

			m_pAVFrame[i].format = pFrmVideo->format;
			m_pAVFrame[i].width = pFrmVideo->width;
			m_pAVFrame[i].height = nStep;
			m_pAVFrame[i].data[0] = pFrmVideo->data[0] + i * pFrmVideo->linesize[0] * nStep;
			m_pAVFrame[i].data[1] = pFrmVideo->data[1] + i * pFrmVideo->linesize[1] * (nStep / 2);
			m_pAVFrame[i].data[2] = pFrmVideo->data[2] + i * pFrmVideo->linesize[2] * (nStep / 2);
			m_pAVFrame[i].linesize[0] = pFrmVideo->linesize[0];
			m_pAVFrame[i].linesize[1] = pFrmVideo->linesize[1];
			m_pAVFrame[i].linesize[2] = pFrmVideo->linesize[2];
		}
		else
		{
			nStep = pBufVideo->nHeight / m_nCPUNum;
			nStep = nStep & 0XFFFFFFFE;

			m_pAVFrame[i].format = AV_PIX_FMT_YUV420P;
			m_pAVFrame[i].width = pBufVideo->nWidth;
			m_pAVFrame[i].height = nStep;
			m_pAVFrame[i].data[0] = pBufVideo->pBuff[0] + i * pBufVideo->nStride[0] * nStep;
			m_pAVFrame[i].data[1] = pBufVideo->pBuff[1] + i * pBufVideo->nStride[1] * (nStep / 2);
			m_pAVFrame[i].data[2] = pBufVideo->pBuff[2] + i * pBufVideo->nStride[2] * (nStep / 2);
			m_pAVFrame[i].linesize[0] = pBufVideo->nStride[0];
			m_pAVFrame[i].linesize[1] = pBufVideo->nStride[1];
			m_pAVFrame[i].linesize[2] = pBufVideo->nStride[2];
		}
		m_pInBuff[i].pBuff = (unsigned char *)&m_pAVFrame[i];
		m_pInBuff[i].uSize = g_nAVFrameSize;
		m_pInBuff[i].uFlag = YYBUFF_TYPE_AVFrame;

		memcpy (&m_pVideoBuff[i], pVideoBuff, sizeof (YY_VIDEO_BUFF));
		nStep = pVideoBuff->nHeight / m_nCPUNum;
		nStep = nStep & 0XFFFFFFFE;
		m_pVideoBuff[i].nHeight = nStep;
		m_pVideoBuff[i].pBuff[0] = pVideoBuff->pBuff[0] + i * pVideoBuff->nStride[0] * nStep;
		m_pVideoBuff[i].pBuff[1] = pVideoBuff->pBuff[1] + i * pVideoBuff->nStride[1] * nStep / 2;
		m_pVideoBuff[i].pBuff[2] = pVideoBuff->pBuff[2] + i * pVideoBuff->nStride[2] * nStep / 2;
		m_ppVRCC[i]->ConvertBuff (&m_pInBuff[i], &m_pVideoBuff[i]);
	}

	bool bFinish = false;
	while (!bFinish)
	{
		bFinish = true;
		for (i = 0; i < m_nCPUNum; i++)
		{
			if (!m_ppVRCC[i]->Finished ())
			{
				bFinish = false;
				break;
			}
		}
		if (bFinish)
			break;

		yySleep (1000);
	}

	if (m_nLicenseStatus != 20130311)
		OverLogo (pVideoBuff);

	return nRC;
}

int CDataConvert::OverLogo (YY_VIDEO_BUFF * pVideoBuff)
{
	if (CLicenseCheck::m_pLcsChk != NULL)
	{
		if (CLicenseCheck::m_pLcsChk->m_nLcsStatus1 == YY_LCS_V1 &&
			CLicenseCheck::m_pLcsChk->m_nLcsStatus2 == YY_LCS_V2 &&
			CLicenseCheck::m_pLcsChk->m_nLcsStatus3 == YY_LCS_V3)
		{
			return YY_ERR_NONE;
		}
	}

	int				h = 0;
	unsigned char *	pLOGOBuff = (unsigned char *)yyLogoBuffY;

	if (pVideoBuff->nType == YY_VDT_YUV420_P)
	{
		for (h = 0; h < YYLOGO_HEIGHT; h++)
			memcpy (pVideoBuff->pBuff[0] + h * pVideoBuff->nStride[0], pLOGOBuff + h * YYLOGO_WIDTH, YYLOGO_WIDTH);

		pLOGOBuff = (unsigned char *)yyLogoBuffY + YYLOGO_WIDTH * YYLOGO_HEIGHT;
//		for (h = 0; h < YYLOGO_HEIGHT / 2; h++)
//			memcpy (pVideoBuff->pBuff[2] + h * pVideoBuff->nStride[2], pLOGOBuff + h * YYLOGO_WIDTH / 2, YYLOGO_WIDTH / 2);

		pLOGOBuff = (unsigned char *)yyLogoBuffY + (YYLOGO_WIDTH * YYLOGO_HEIGHT) * 5 / 4;
		for (h = 0; h < YYLOGO_HEIGHT / 2; h++)
			memcpy (pVideoBuff->pBuff[1] + h * pVideoBuff->nStride[1], pLOGOBuff + h * YYLOGO_WIDTH / 2, YYLOGO_WIDTH / 2);
	}
	else if (pVideoBuff->nType == YY_VDT_RGBA)
	{
		pLOGOBuff = (unsigned char *)yyLogoBuffRGBA;
		for (h = 0; h < YYLOGO_HEIGHT; h++)
			memcpy (pVideoBuff->pBuff[0] + h * pVideoBuff->nStride[0], pLOGOBuff + (YYLOGO_HEIGHT - h -1) * YYLOGO_WIDTH * 4, YYLOGO_WIDTH * 4);
	}
	else if (pVideoBuff->nType == YY_VDT_ARGB)
	{
		pLOGOBuff = (unsigned char *)yyLogoBuffRGBA;
		for (h = 0; h < YYLOGO_HEIGHT; h++)
			memcpy (pVideoBuff->pBuff[0] + h * pVideoBuff->nStride[0], pLOGOBuff + (YYLOGO_HEIGHT - h -1) * YYLOGO_WIDTH * 4, YYLOGO_WIDTH * 4);
	}
	else if (pVideoBuff->nType == YY_VDT_RGB565)
	{
	}
	else if (pVideoBuff->nType == YY_VDT_RGB24)
	{
		pLOGOBuff = (unsigned char *)yyLogoBuffRGB24;
		for (h = 0; h < YYLOGO_HEIGHT; h++)
			memcpy (pVideoBuff->pBuff[0] + h * pVideoBuff->nStride[0], pLOGOBuff + h * YYLOGO_WIDTH * 3, YYLOGO_WIDTH * 3);
	}

	return YY_ERR_NONE;
}