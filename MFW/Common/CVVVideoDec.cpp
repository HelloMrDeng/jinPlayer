/*******************************************************************************
	File:		CVVVideoDec.cpp

	Contains:	The vo video dec wrap implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "CVVVideoDec.h"

#include "yyLog.h"

#include "cmnMemory.h"
#include "voMPEG4.h"
#include "voH264.h"
#include "voH265.h"
#include "voWmv.h"
#include "voVC1.h"

#include "USystemFunc.h"
#include "yyConfig.h"
#include "yyDefine.h"

CVVVideoDec::CVVVideoDec(void * hInst)
	: CBaseVideoDec (hInst)
	, m_hDec (NULL)
	, m_nCodecType (0)
	, m_bDecoded (false)
	, m_pPacket (NULL)
{
	SetObjectName ("CVVVideoDec");
	cmnMemFillPointer (0);
	memset (&m_fAPI, 0, sizeof (m_fAPI));
	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
}

CVVVideoDec::~CVVVideoDec(void)
{
	Uninit ();
}

int CVVVideoDec::Init (YY_VIDEO_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;

	Uninit ();

	int					nCodecID = 0;
    AVCodecContext *	pDecCtx = NULL;
	if (pFmt->pPrivateData != NULL)
		pDecCtx = (AVCodecContext *)pFmt->pPrivateData;
	if (pDecCtx != NULL)
	{
		m_uCodecTag = pDecCtx->codec_tag;
		nCodecID = pDecCtx->codec_id;
	}
	else
	{
		nCodecID = pFmt->nCodecID;
	}
	if (nCodecID == AV_CODEC_ID_H265)
	{
		m_nCodecType = VO_VIDEO_CodingH265;
#ifndef _OS_WINCE
		yyGetHEVCDecFunc (&m_fAPI, 0);
#endif // _OS_WINCE
	}
#ifdef _OS_WINCE
#ifndef _CPU_MSB2531
	else if (pDecCtx->codec_id == AV_CODEC_ID_MPEG4)
	{
		m_nCodecType = VO_VIDEO_CodingMPEG4;
		yyGetMPEG4DecFunc (&m_fAPI, 0);
	}
	else if (pDecCtx->codec_id == AV_CODEC_ID_WMV3 || pDecCtx->codec_id == AV_CODEC_ID_WMV2
		     || pDecCtx->codec_id == AV_CODEC_ID_WMV1)
	{
		m_nCodecType = VO_VIDEO_CodingWMV;
		yyGetWMVDecFunc (&m_fAPI, 0);
	}
#endif // _CPU_MSB2531
#endif // _OS_WINCE	

	if (m_fAPI.Init == NULL)
		return YY_ERR_FAILED;

	VO_CODEC_INIT_USERDATA	initInfo;
	memset (&initInfo, 0, sizeof (VO_CODEC_INIT_USERDATA));
	initInfo.memflag = VO_IMF_USERMEMOPERATOR;
	initInfo.memData = &g_memOP;

	int nRC = m_fAPI.Init (&m_hDec, (VO_VIDEO_CODINGTYPE)m_nCodecType, &initInfo);
	if (m_hDec != NULL)
	{
		nRC = SetDecParam (pFmt);
		if (nRC != YY_ERR_NONE)
			return nRC;	

		nRC = SetHeadData (pFmt);
		if (nRC != YY_ERR_NONE)
			return nRC;
	}

	memcpy (&m_fmtVideo, pFmt, sizeof (m_fmtVideo));
	m_fmtVideo.nCodecID = AV_CODEC_ID_NONE;
	m_fmtVideo.pHeadData = NULL;
	m_fmtVideo.nHeadSize = 0;
	m_fmtVideo.pPrivateData = NULL;

	m_bDropFrame = true;
	m_nDecCount = 0;
	m_pPacket = NULL;
	m_uBuffFlag = 0;
	m_bDecoded = false;

	memset (&m_Input, 0, sizeof (m_Input));
	memset (&m_Output, 0, sizeof (m_Output));
	memset (&m_OutputInfo, 0, sizeof (m_OutputInfo));

	m_uFrameSize = m_fmtVideo.nWidth * m_fmtVideo.nHeight;
	
	return YY_ERR_NONE;
}

int CVVVideoDec::Uninit (void)
{
	if (m_hDec != NULL)
		m_fAPI.Uninit (m_hDec);
	m_hDec = NULL;

	YY_DEL_A (m_pHeadData);

	return YY_ERR_NONE;
}

int CVVVideoDec::Flush (void)
{
//	CAutoLock lock (&m_mtBuffer);
	if (m_hDec != NULL && m_bDecoded)
	{
		VO_U32	nFlush = 1;	
		m_fAPI.SetParam (m_hDec, VO_PID_COMMON_FLUSH, &nFlush);
	}
	m_bDropFrame = true;
	m_nDecCount = 0;

	return YY_ERR_NONE;
}

int CVVVideoDec::SetBuff (YY_BUFFER * pBuff)
{
	if (pBuff == NULL || m_hDec == NULL)
		return YY_ERR_ARG;
//	CAutoLock lock (&m_mtBuffer);
	CBaseVideoDec::SetBuff (pBuff);

	if ((pBuff->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
		Init ((YY_VIDEO_FORMAT *)pBuff->pFormat);
	else if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		Flush ();

	if ((pBuff->uFlag & YYBUFF_DROP_FRAME) == YYBUFF_DROP_FRAME)
	{
		m_bDropFrame = true;
		m_nDecCount = 0;
	}

	if ((pBuff->uFlag & YYBUFF_TYPE_PACKET) != YYBUFF_TYPE_PACKET)
	{
		m_pktData.data = pBuff->pBuff;
		m_pktData.size = pBuff->uSize;
		m_pktData.pts = pBuff->llTime;

		m_pPacket = &m_pktData;
	}
	else
	{
		m_pPacket = (AVPacket *)pBuff->pBuff;
	}

	m_Input.Buffer = (VO_PBYTE)m_pPacket->data;
	m_Input.Length = m_pPacket->size;
	if (m_pPacket->pts == YY_64_INVALID && m_pPacket->dts != YY_64_INVALID)
		m_Input.Time = m_pPacket->dts;
	else
		m_Input.Time = m_pPacket->pts;

	if (pBuff->llDelay > YYCFG_SKIP_BFRAME_TIME)
	{
		if (GetFrameType (&m_Input) == VO_VIDEO_FRAME_B)
			return YY_ERR_NEEDMORE;
	}
	if ((pBuff->uFlag & YYBUFF_DEC_SKIP_BFRAME) == YYBUFF_DEC_SKIP_BFRAME)
	{
		if (GetFrameType (&m_Input) == VO_VIDEO_FRAME_B)
			return YY_ERR_NEEDMORE;
	}

	int nRC = SetInputData (&m_Input);
	nRC = nRC & 0X8000000F;
	FillInfo (pBuff);

	if (nRC == VO_ERR_INPUT_BUFFER_SMALL)
		return YY_ERR_NEEDMORE;
	else if (nRC == VO_ERR_RETRY)
		return YY_ERR_RETRY;
	else if (nRC == VO_ERR_CODEC_UNSUPPORTED)
		return YY_ERR_UNSUPPORT;

	return YY_ERR_NONE;
}

int CVVVideoDec::GetBuff (YY_BUFFER * pBuff)
{
	if (pBuff == NULL || m_hDec == NULL)
		return YY_ERR_ARG;
	if (m_nBuffNum <= 1)
	{
		if (m_Input.Buffer == NULL || m_Input.Length == 0)
			return YY_ERR_RETRY;
		if (m_pPacket != NULL)
		{
			if (m_Input.Buffer != (VO_PBYTE)m_pPacket->data || m_Input.Length != m_pPacket->size)
				return YY_ERR_RETRY;
		}
	}

//	CAutoLock lock (&m_mtBuffer);
	if ((pBuff->uFlag & YYBUFF_DEC_DISABLE) == YYBUFF_DEC_DISABLE)
	{
		m_bWaitKeyFrame = true;

		if (m_pPacket->pts >= 0)
			pBuff->llTime = m_pPacket->pts;
		else if (m_pPacket->dts >= 0)
			pBuff->llTime = m_pPacket->dts;

		return YY_ERR_NONE;
	}
	if (m_bWaitKeyFrame && m_pPacket != &m_pktData)
	{
		if ((m_pPacket->flags & AV_PKT_FLAG_KEY) != AV_PKT_FLAG_KEY)
			return YY_ERR_RETRY;
		else
			m_bWaitKeyFrame = false;
	}

	int nRC = GetOutputData ();

	if (m_Output.Buffer[0] == NULL)
		return YY_ERR_RETRY;

	m_bufVideo.pBuff[0] = m_Output.Buffer[0];
	m_bufVideo.pBuff[1] = m_Output.Buffer[1];
	m_bufVideo.pBuff[2] = m_Output.Buffer[2];

	m_bufVideo.nStride[0] = m_Output.Stride[0];
	m_bufVideo.nStride[1] = m_Output.Stride[1];
	m_bufVideo.nStride[2] = m_Output.Stride[2];

	m_bufVideo.nWidth = m_OutputInfo.Format.Width;
	m_bufVideo.nHeight = m_OutputInfo.Format.Height;

	m_bufVideo.nType = YY_VDT_YUV420_P;

	pBuff->uFlag = YYBUFF_TYPE_VIDEO;
	pBuff->pBuff = (unsigned char *)&m_bufVideo;
	pBuff->uSize = m_nVdoBuffSize;
	if (m_bufVideo.nWidth != m_fmtVideo.nWidth || m_bufVideo.nHeight != m_fmtVideo.nHeight)
	{
		m_fmtVideo.nWidth = m_bufVideo.nWidth;
		m_fmtVideo.nHeight = m_bufVideo.nHeight;
		pBuff->uFlag |= YYBUFF_NEW_FORMAT;
		pBuff->pFormat = &m_fmtVideo;
	}

	pBuff->llTime = m_Input.Time;

	m_bDecoded = true;
	RestoreInfo (pBuff);

	CBaseVideoDec::GetBuff (pBuff);

	return YY_ERR_NONE;
}

int CVVVideoDec::SetInputData (VO_CODECBUFFER * pInData)
{
	int nRC = m_fAPI.SetInputData (m_hDec, pInData);
//	if (nRC != VO_ERR_NONE)
//		YYLOGW ("SetInputData was failed! return %08X", nRC);
	return nRC;
}

int CVVVideoDec::GetOutputData (void)
{
	memset (&m_Output, 0, sizeof (m_Output));
	int nRC = m_fAPI.GetOutputData (m_hDec, &m_Output, &m_OutputInfo);
//	if (nRC != VO_ERR_NONE)
//		YYLOGW ("SetInputData was failed! return %08X", nRC);
	return nRC;	
}

VO_VIDEO_FRAMETYPE CVVVideoDec::GetFrameType (VO_CODECBUFFER * pBuffer)
{
	VO_VIDEO_FRAMETYPE	nFrameType = VO_VIDEO_FRAME_NULL;

	if (m_nCodecType != VO_VIDEO_CodingMPEG4)
		return nFrameType;

	VO_CODECBUFFER		inputBuffer;
	memcpy (&inputBuffer, pBuffer, sizeof (VO_CODECBUFFER));
	inputBuffer.Time = VO_VIDEO_FRAME_NULL;
	VO_U32 nRC = m_fAPI.GetParam (m_hDec, VO_PID_VIDEO_FRAMETYPE, &inputBuffer);
	if (nRC != VO_ERR_NONE)
		return VO_VIDEO_FRAME_NULL;

	return VO_VIDEO_FRAMETYPE (inputBuffer.Time);
}

int CVVVideoDec::SetDecParam (YY_VIDEO_FORMAT * pFmt)
{
#ifdef _OS_LINUX
	int nCPUNum = yyGetCPUNum ();
	m_fAPI.SetParam (m_hDec, VO_PID_COMMON_CPUNUM, &nCPUNum);
	//YYLOGI ("CPU Num %d, return %d", nCPUNum, nRC);	
#endif // _OS_LINUX	

	return YY_ERR_NONE;
}

int CVVVideoDec::SetHeadData (YY_VIDEO_FORMAT * pFmt)
{
	int					nRC = YY_ERR_NONE;
    AVCodecContext *	pDecCtx = NULL;
	if (pFmt->pPrivateData != NULL)
		pDecCtx = (AVCodecContext *)pFmt->pPrivateData;

	VO_CODECBUFFER buffHead;
	if (pDecCtx->codec_id == AV_CODEC_ID_WMV1 || pDecCtx->codec_id == AV_CODEC_ID_WMV2 || pDecCtx->codec_id == AV_CODEC_ID_WMV3)
	{
		YY_DEL_A (m_pHeadData);

		m_nHeadSize = sizeof (VO_BITMAPINFOHEADER) + pDecCtx->extradata_size;
		m_pHeadData = new VO_BYTE[m_nHeadSize];
		memset (m_pHeadData, 0, m_nHeadSize);
		VO_BITMAPINFOHEADER * pBmp = (VO_BITMAPINFOHEADER *)m_pHeadData;
		pBmp->biSize = m_nHeadSize;
		pBmp->biWidth = pFmt->nWidth;
		pBmp->biHeight = pFmt->nHeight;
		pBmp->biPlanes = 1;
		pBmp->biBitCount = 24;
		if (pDecCtx->codec_id == AV_CODEC_ID_WMV1)
			pBmp->biCompression = YYMKBETAG ('1', 'v', 'm', 'w');
		else if (pDecCtx->codec_id == AV_CODEC_ID_WMV2)
			pBmp->biCompression = YYMKBETAG ('2', 'v', 'm', 'w');
		else
			pBmp->biCompression = YYMKBETAG ('3', 'v', 'm', 'w');
		pBmp->biSizeImage = pBmp->biWidth * pBmp->biHeight * 3;

		if (pDecCtx->extradata_size > 0)
			memcpy (m_pHeadData + sizeof (VO_BITMAPINFOHEADER), pDecCtx->extradata, pDecCtx->extradata_size);
		buffHead.Buffer = m_pHeadData;
		buffHead.Length = m_nHeadSize;		

		nRC = m_fAPI.SetParam (m_hDec, VO_PID_COMMON_HEADDATA, &buffHead);
		if (nRC != VO_ERR_NONE)
			return YY_ERR_FAILED;
	}
	else if (pDecCtx->extradata_size > 0)
	{
		buffHead.Buffer = pDecCtx->extradata;
		buffHead.Length = pDecCtx->extradata_size;		

		nRC = m_fAPI.SetParam (m_hDec, VO_PID_COMMON_HEADDATA, &buffHead);
		if (nRC != VO_ERR_NONE)
			return YY_ERR_FAILED;
	}

	return YY_ERR_NONE;
}

#if 0
typedef VO_S32 (VO_API * VOGETVIDEODECAPI) (VO_VIDEO_DECAPI * pDecHandle, VO_U32 uFlag);
int CVVVideoDec::GetDecAPI (void)
{
	HMODULE hDll = LoadLibrary (_T("C:\\Projects\\voRelease\\Win32\\Bin\\XP\\voH265Dec.dll"));
	VOGETVIDEODECAPI pAPI = (VOGETVIDEODECAPI)GetProcAddress (hDll, "voGetH265DecAPI");
	pAPI (&m_fAPI, 0);
	return YY_ERR_NONE;
}
#endif // 0