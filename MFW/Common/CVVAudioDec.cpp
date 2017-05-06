/*******************************************************************************
	File:		CVVAudioDec.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "CVVAudioDec.h"
#include "yyLog.h"

#include "cmnMemory.h"
#include "UYYDataFunc.h"

#include "voAAC.h"
#include "voAC3.h"

CVVAudioDec::CVVAudioDec(void * hInst)
	: CBaseAudioDec (hInst)
	, m_hDec (NULL)
	, m_pOutBuff (NULL)
	, m_nOutSize (0)
{
	SetObjectName ("CVVAudioDec");
	cmnMemFillPointer (0);
	memset (&m_fAPI, 0, sizeof (m_fAPI));
	memset (&m_Input, 0, sizeof (m_Input));
}

CVVAudioDec::~CVVAudioDec(void)
{
	Uninit ();
}

int CVVAudioDec::Init (YY_AUDIO_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;

	Uninit ();

    AVCodecContext *	pDecCtx = NULL;
	if (pFmt->pPrivateData != NULL)
		pDecCtx = (AVCodecContext *)pFmt->pPrivateData;

	VO_AUDIO_CODINGTYPE nCodecType = VO_AUDIO_CodingAAC;
	if (pDecCtx != NULL)
	{
		if (pDecCtx->codec_id == AV_CODEC_ID_AAC)
			nCodecType = VO_AUDIO_CodingAAC;
		else if (pDecCtx->codec_id == AV_CODEC_ID_AC3)
			nCodecType = VO_AUDIO_CodingAC3;
	}
	else
	{
		if (pFmt->nCodecID == YY_CODEC_ID_AAC)
			nCodecType = VO_AUDIO_CodingAAC;
		else if (pFmt->nCodecID == YY_CODEC_ID_AC3)
			nCodecType = VO_AUDIO_CodingAC3;
	}
#ifdef _OS_WINCE
	if (nCodecType == VO_AUDIO_CodingAAC)
		yyGetAACDecFunc (&m_fAPI);
	else if (nCodecType == VO_AUDIO_CodingAC3)
		yyGetAC3DecFunc (&m_fAPI);
#endif // _OS_WINCE
	if (m_fAPI.Init == NULL)
		return YY_ERR_FAILED;

	VO_CODEC_INIT_USERDATA	initInfo;
	memset (&initInfo, 0, sizeof (VO_CODEC_INIT_USERDATA));
	initInfo.memflag = VO_IMF_USERMEMOPERATOR;
	initInfo.memData = &g_memOP;

	int nRC = m_fAPI.Init (&m_hDec, nCodecType, &initInfo);
	if (m_hDec != NULL)
	{
		if(nCodecType == VO_AUDIO_CodingAAC)
		{
			VOAACFRAMETYPE nFrameType = VOAAC_RAWDATA;
			if (pDecCtx != NULL)
			{
				if (pDecCtx->extradata_size > 0)
					nFrameType = VOAAC_RAWDATA;
				else
					nFrameType = VOAAC_ADTS;
			}
			else
			{
				nFrameType = VOAAC_ADTS;
			}
			nRC = m_fAPI.SetParam (m_hDec, VO_PID_AAC_FRAMETYPE, &nFrameType);
#ifdef _OS_WINCE
			VO_U32 nChannelSpec = VO_CHANNEL_FRONT_LEFT | VO_CHANNEL_FRONT_RIGHT;
			nRC = m_fAPI.SetParam (m_hDec, VO_PID_AAC_SELECTCHS, &nChannelSpec);
#else
			if (pFmt->nChannels > 2)
			{
				VO_AUDIO_CHANNELCONFIG nChnConfig = VO_AUDIO_CHAN_MULDOWNMIX2;
				nRC = m_fAPI.SetParam (m_hDec, VO_PID_AAC_CHANNELSPEC, &nChnConfig);
			}
#endif // _OS_WINCE
			int nDisable = 1;
			nRC = m_fAPI.SetParam (m_hDec, VO_PID_AAC_DISABLEAACPLUSV1, &nDisable);
			nRC = m_fAPI.SetParam (m_hDec, VO_PID_AAC_DISABLEAACPLUSV2, &nDisable);
		}
		else if (nCodecType == VO_AUDIO_CodingAC3)
		{
			int nChannels = 2;
//			nRC = m_fAPI.SetParam (m_hDec, VO_PID_AC3_OUTPUTMODE, &nChannels);
			int nStereo = 1;
//			nRC = m_fAPI.SetParam (m_hDec, VO_PID_AC3_STEREOMODE, &nStereo);
		}

		VO_CODECBUFFER buffHead;
		memset (&buffHead, 0, sizeof (buffHead));
		if (pDecCtx != NULL && pDecCtx->extradata_size > 0)
		{
			buffHead.Buffer = pDecCtx->extradata;
			buffHead.Length = pDecCtx->extradata_size;		
		}
		else if (pFmt->pHeadData != NULL)
		{
			buffHead.Buffer = pFmt->pHeadData;
			buffHead.Length = pFmt->nHeadSize;		
		}
		if (buffHead.Buffer != NULL)
		{
			nRC = m_fAPI.SetParam (m_hDec, VO_PID_COMMON_HEADDATA, &buffHead);
			if (nRC != VO_ERR_NONE)
				return YY_ERR_FAILED;
		}

		m_nOutSize = 1024 * 32;
		m_pOutBuff = new unsigned char[m_nOutSize];
		m_Output.Buffer = m_pOutBuff;
		m_Output.Length = m_nOutSize;
	}

	memcpy (&m_fmtAudio, pFmt, sizeof (m_fmtAudio));
	m_fmtAudio.nCodecID = AV_CODEC_ID_PCM_S16LE;
	m_fmtAudio.pHeadData = NULL;
	m_fmtAudio.nHeadSize = 0;
	m_fmtAudio.pPrivateData = NULL;

	memset (&m_Input, 0, sizeof (m_Input));
	m_uBuffFlag = 0;
	
	return YY_ERR_NONE;
}

int CVVAudioDec::Uninit (void)
{
	if (m_hDec != NULL)
		m_fAPI.Uninit (m_hDec);
	m_hDec = NULL;

	YY_DEL_A (m_pOutBuff);

	return YY_ERR_NONE;
}

int CVVAudioDec::Flush (void)
{
	CAutoLock lock (&m_mtBuffer);

	if (m_hDec != NULL)
	{
		VO_U32	nFlush = 1;	
		m_fAPI.SetParam (m_hDec, VO_PID_COMMON_FLUSH, &nFlush);
	}

	memset (&m_Input, 0, sizeof (m_Input));

	return YY_ERR_NONE;
}

int CVVAudioDec::SetBuff (YY_BUFFER * pBuff)
{
	if (pBuff == NULL || m_hDec == NULL)
		return YY_ERR_ARG;

	CAutoLock lock (&m_mtBuffer);
	CBaseAudioDec::SetBuff (pBuff);

	if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		Flush ();
	
	if ((pBuff->uFlag & YYBUFF_TYPE_PACKET) != YYBUFF_TYPE_PACKET)
	{
		m_Input.Buffer = pBuff->pBuff;
		m_Input.Length = pBuff->uSize;
		m_Input.Time = pBuff->llTime;
	}
	else
	{
		AVPacket * pPacket = (AVPacket *)pBuff->pBuff;
		m_Input.Buffer = (VO_PBYTE)pPacket->data;
		m_Input.Length = pPacket->size;
		m_Input.Time = pPacket->pts;
	}

	int nRC = m_fAPI.SetInputData (m_hDec, &m_Input);
	if (nRC == VO_ERR_INPUT_BUFFER_SMALL)
		return YY_ERR_NEEDMORE;
	else if (nRC == VO_ERR_RETRY)
		return YY_ERR_RETRY;

//	YYLOGI ("Audio Time: % 8d,   % 8d,   % 8d", (int)pBuff->llTime, (int)(pBuff->llTime - m_llDbgTime), pBuff->uSize);
//	m_llDbgTime = pBuff->llTime;

	return YY_ERR_NONE;
}

int CVVAudioDec::GetBuff (YY_BUFFER * pBuff)
{
	if (pBuff == NULL || m_hDec == NULL)
		return YY_ERR_ARG;

	CAutoLock lock (&m_mtBuffer);
	if (m_Input.Length <= 0)
		return YY_ERR_RETRY;

	int nRC = 0;
	int nBuffSize = 0;
	while (true)
	{
		m_Output.Length = m_nOutSize - nBuffSize;
		m_Output.Buffer = m_pOutBuff + nBuffSize;

		nRC = m_fAPI.GetOutputData (m_hDec, &m_Output, &m_OutputInfo);
		if (nRC == VO_ERR_INPUT_BUFFER_SMALL)
		{
			m_Input.Length = 0;
			break;
		}

		nBuffSize += m_Output.Length;
	}

	pBuff->uFlag = YYBUFF_TYPE_DATA;
	pBuff->pBuff = m_pOutBuff;
	pBuff->uSize = nBuffSize;
	pBuff->llTime = m_Input.Time;

	if (m_fmtAudio.nChannels != m_OutputInfo.Format.Channels ||
		m_fmtAudio.nSampleRate !=m_OutputInfo.Format.SampleRate)
	{
		m_fmtAudio.nChannels = m_OutputInfo.Format.Channels;
		m_fmtAudio.nSampleRate =m_OutputInfo.Format.SampleRate;
		pBuff->uFlag |= YYBUFF_NEW_FORMAT;
		pBuff->pFormat = &m_fmtAudio;
	}

	if (nBuffSize <= 0)
		return YY_ERR_RETRY;

//	YYLOGI ("Audio Time: % 8d,   % 8d,   % 8d", (int)pBuff->llTime, (int)(pBuff->llTime - m_llDbgTime), pBuff->uSize);
//	m_llDbgTime = pBuff->llTime;

	CBaseAudioDec::GetBuff (pBuff);

	return YY_ERR_NONE;
}

