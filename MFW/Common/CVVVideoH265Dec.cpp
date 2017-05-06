/*******************************************************************************
	File:		CVVVideoH265Dec.cpp

	Contains:	The vo video dec wrap implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "CVVVideoH265Dec.h"

#include "voH265.h"

#include "yyLog.h"
#include "USystemFunc.h"
#include "yyDefine.h"
#include "yyConfig.h"

CVVVideoH265Dec::CVVVideoH265Dec(void * hInst)
	: CVVVideoDec (hInst)
	, m_bEOS (false)
	, m_nDgbIndex (0)
	, m_llDbgLastTime (0)
{
	SetObjectName ("CVVHEVCDec");

	m_nBuffNum = 8;
	memset (&m_dataInput, 0, sizeof (VO_CODECBUFFER));
}

CVVVideoH265Dec::~CVVVideoH265Dec(void)
{
	Uninit ();

	YY_DEL_A (m_pVideoData);
}

int CVVVideoH265Dec::Uninit (void)
{
	if (m_hDec != NULL)
	{
		if (m_Output.CodecData != NULL)
			m_fAPI.SetParam (m_hDec, VO_PID_COMMON_FRAME_BUF_BACK, &m_Output);
		memset (&m_Output, 0, sizeof (m_Output));

		int nFlush = 1;
		int nRC = m_fAPI.SetParam(m_hDec, VO_PID_DEC_H265_FLUSH_PICS, (VO_PTR*)&nFlush);
		// Get one frame decoded data, out_data_info.InputUsed returens the used length of input buffer
		do {
			nRC = m_fAPI.GetOutputData (m_hDec, &m_Output, &m_OutputInfo);
			if(nRC == VO_ERR_NONE)
			{
				if(m_OutputInfo.Format.Type != VO_VIDEO_FRAME_NULL)
					m_fAPI.SetParam(m_hDec, VO_PID_COMMON_FRAME_BUF_BACK, &m_Output);
			}
		}while(m_OutputInfo.Flag);
	}
	return CVVVideoDec::Uninit ();
}

int CVVVideoH265Dec::Flush (void)
{
	if (m_hDec != NULL)
	{
		if (m_Output.CodecData != NULL)
			m_fAPI.SetParam (m_hDec, VO_PID_COMMON_FRAME_BUF_BACK, &m_Output);
		memset (&m_Output, 0, sizeof (m_Output));
	}
	m_bEOS = false;;
	return CVVVideoDec::Flush ();
}

int CVVVideoH265Dec::GetBuff (YY_BUFFER * pBuff)
{
	if (m_hDec == NULL)
		return YY_ERR_FAILED;

//	VO_U32 uFastMode = VO_FM_B_SAO | VO_FM_SAO | VO_FM_DEBLOCKEDGE | VO_FM_B_DBLOCK | VO_FM_DEBLOCK | VO_FM_DROP_UNREF;
	VO_U32 uFastMode = 0;
	if (pBuff->llDelay > YYCFG_DDEBLOCK_TIME)
		uFastMode = VO_FM_SAO | VO_FM_DEBLOCKEDGE | VO_FM_DEBLOCK;
	if (pBuff->llDelay > YYCFG_SKIP_BFRAME_TIME)
		uFastMode = uFastMode | VO_FM_DROP_UNREF;
	if ((pBuff->uFlag & YYBUFF_DEC_SKIP_BFRAME) == YYBUFF_DEC_SKIP_BFRAME)
		uFastMode = uFastMode | VO_FM_DROP_UNREF;
//	uFastMode = VO_FM_AUTO_FASTMODE;
	m_fAPI.SetParam (m_hDec, VO_PID_DEC_H265_FASTMODE, &uFastMode);

	int nRC = CVVVideoDec::GetBuff (pBuff);	
	if (nRC == YY_ERR_NONE)
	{
		if (m_Output.Time == YY_64_INVALID)
		{
			if (m_llFrameTime > 0)
				m_Output.Time = m_llLastTime + m_llFrameTime;
			else
				m_Output.Time = m_llLastTime + 40;
		}

		pBuff->llTime = m_Output.Time;

		if (abs((int) (m_llLastTime - pBuff->llTime)) > 100)
		{
			if (m_nDecCount > 1)
				m_llFrameTime = (pBuff->llTime - m_llFirstTime) / (m_nDecCount - 1);
			else
				m_llFrameTime = pBuff->llTime - m_llFirstTime;
		}
		m_llLastTime = pBuff->llTime;
		if (m_llFirstTime == 0)
			m_llFirstTime = pBuff->llTime;
	}
	else if (m_bEOS)
	{
		pBuff->uFlag |= YYBUFF_EOS;
	}

	return nRC;
}

int CVVVideoH265Dec::SetInputData (VO_CODECBUFFER * pInData)
{
	if (m_hDec == NULL)
		return YY_ERR_FAILED;

	if (m_uCodecTag != YY_FCC_HVC1 && m_uCodecTag != YY_FCC_HEV1)
		return CVVVideoDec::SetInputData (pInData);

	if (m_dataInput.Time != pInData->Time || m_dataInput.Length != pInData->Length)
	{
		if (!ConvertVideoData (pInData->Buffer, pInData->Length))
			return YY_ERR_FAILED;
	}

	if (m_pVideoData != NULL)
	{
		pInData->Buffer = m_pVideoData;
		pInData->Length = m_uVideoSize;
	}

	m_dataInput.Length = pInData->Length;
	int nRC = m_fAPI.SetInputData (m_hDec, pInData);
	if (nRC == VO_ERR_RETRY)
		m_dataInput.Time = pInData->Time;
	else
		m_dataInput.Length = 0;

	return nRC;
}

int CVVVideoH265Dec::GetOutputData (void)
{
	if (m_hDec == NULL)
		return YY_ERR_FAILED;

	if (m_Output.CodecData != NULL)
		m_fAPI.SetParam (m_hDec, VO_PID_COMMON_FRAME_BUF_BACK, &m_Output);
	memset (&m_Output, 0, sizeof (m_Output));
	int nRC = m_fAPI.GetOutputData (m_hDec, &m_Output, &m_OutputInfo);

	return nRC;
}

int CVVVideoH265Dec::SetHeadData (YY_VIDEO_FORMAT * pFmt)
{
	if (m_hDec == NULL)
		return YY_ERR_FAILED;

	int					nRC = YY_ERR_NONE;
    AVCodecContext *	pDecCtx = NULL;
	if (pFmt->pPrivateData != NULL)
		pDecCtx = (AVCodecContext *)pFmt->pPrivateData;

	if (pDecCtx != NULL)
	{
		if (pDecCtx->extradata_size > 0 && pDecCtx->extradata != NULL)
		{
			VO_CODECBUFFER buffHead;
			if (m_uCodecTag == YY_FCC_HVC1 || m_uCodecTag == YY_FCC_HEV1)
			{
				if (!ConvertHEVCHeadData (pDecCtx->extradata, pDecCtx->extradata_size))
					return YY_ERR_FAILED;
				buffHead.Buffer = m_pHeadData;
				buffHead.Length = m_nHeadSize;		
			}
			else
			{
				buffHead.Buffer = pDecCtx->extradata;
				buffHead.Length = pDecCtx->extradata_size;		
			}

			nRC = m_fAPI.SetParam (m_hDec, VO_PID_COMMON_HEADDATA, &buffHead);
			if (nRC != VO_ERR_NONE)
				return YY_ERR_FAILED;
		}
	}
	else if (pFmt->pHeadData != NULL && pFmt->nHeadSize > 0)
	{
		VO_CODECBUFFER buffHead;
		buffHead.Buffer = pFmt->pHeadData;
		buffHead.Length = pFmt->nHeadSize;		
		nRC = m_fAPI.SetParam (m_hDec, VO_PID_COMMON_HEADDATA, &buffHead);
		if (nRC != VO_ERR_NONE)
			return YY_ERR_FAILED;
	}

	return YY_ERR_NONE;
}

int CVVVideoH265Dec::SetDecParam (YY_VIDEO_FORMAT * pFmt)
{
	if (m_hDec == NULL)
		return YY_ERR_FAILED;

	int nCPUNum = yyGetCPUNum ();
	int nRC = m_fAPI.SetParam (m_hDec, VO_PID_COMMON_CPUNUM, &nCPUNum);

	VO_BOOL	bThumbNail = VO_FALSE;
	nRC = m_fAPI.SetParam (m_hDec, VO_PID_VIDEO_THUMBNAIL_MODE, &bThumbNail);

//	nRC = m_fAPI.SetParam (m_hDec, VO_PID_COMMON_FRAME_BUF_EX, &m_nBuffNum);

	return YY_ERR_NONE;
}
