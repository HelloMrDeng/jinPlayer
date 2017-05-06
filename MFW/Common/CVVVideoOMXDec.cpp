/*******************************************************************************
	File:		CVVVideoOMXDec.cpp

	Contains:	The vo video dec wrap implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "CVVVideoOMXDec.h"

#include "voIOMXDec.h"
#include "cmnMemory.h"

#include "yyLog.h"
#include "USystemFunc.h"
#include "yyDefine.h"
#include "yyConfig.h"

#include "OMX_Core.h"
#include "OMX_Video.h"

#include <dlfcn.h>
#include <sys/system_properties.h>

typedef VO_S32 (* YYGETIOMXDECAPI) (VO_VIDEO_DECAPI * pDecHandle, VO_U32 uFlag);
#define XRAW_IS_ANNEXB(p) ( !(*((p)+0)) && !(*((p)+1)) && (*((p)+2)==1))
#define XRAW_IS_ANNEXB2(p) ( !(*((p)+0)) && !(*((p)+1)) && !(*((p)+2))&& (*((p)+3)==1))


CVVVideoOMXDec::CVVVideoOMXDec(void * hInst)
	: CVVVideoDec (hInst)
	, m_hDllOMX (NULL)
	, m_bCheckFrameType (false)
	, m_bEOS (false)
	, m_nDgbIndex (0)
	, m_llDbgLastTime (0)
{
	SetObjectName ("CVVOMXDec");

//	m_nBuffNum = 4;
	memset (&m_dataInput, 0, sizeof (VO_CODECBUFFER));
	__system_property_get ("ro.build.version.release", m_szVer);	
	__system_property_get ("ro.board.platform", m_szBP);		
}

CVVVideoOMXDec::~CVVVideoOMXDec(void)
{
	Uninit ();
}

int CVVVideoOMXDec::Init (YY_VIDEO_FORMAT * pFmt)
{
	if (m_szVer[0] != '4')
		return YY_ERR_FAILED;
	if (pFmt == NULL)
		return YY_ERR_ARG;
	Uninit ();
		
	char szOMXDllName[64];
	if (strstr (m_szVer, "4.0") == m_szVer || strstr (m_szVer, "4.1") == m_szVer ||strstr (m_szVer, "4.2") == m_szVer)
		strcpy (szOMXDllName, "libyyOMXDec_40.so");
	else if (strstr (m_szVer, "4.3") == m_szVer)
		strcpy (szOMXDllName, "libyyOMXDec_42.so");
	else
		strcpy (szOMXDllName, "libyyOMXDec_44.so");
	YYLOGI ("The OMX File is %s. Video Size %d X %d", szOMXDllName, pFmt->nWidth, pFmt->nHeight);
		
	char szPath[256];
	strcpy (szPath, "/data/local/tmp/yylib/");
	strcat (szPath, szOMXDllName);
	m_hDllOMX = dlopen(szPath, RTLD_NOW);
	if (m_hDllOMX == NULL)
	{
		memset (szPath, 0, sizeof (szPath));
		yyGetAppPath (NULL, szPath, sizeof (szPath));
		strcat (szPath, "lib/");
		strcat (szPath, szOMXDllName);
		m_hDllOMX = dlopen(szPath, RTLD_NOW);
		if (m_hDllOMX == NULL)
		{
			YYLOGE ("The omx %s could not be loaed!", szPath);
			return YY_ERR_FAILED;
		}
	}
	YYGETIOMXDECAPI pAPI = (YYGETIOMXDECAPI) dlsym (m_hDllOMX, ("voGetIOMXDecAPI"));
	if (pAPI == NULL)
	{
		YYLOGE ("It could not get OMX API!");
		return YY_ERR_FAILED;
	}
	pAPI (&m_fAPI, 0);
	if (m_fAPI.Init == NULL)
	{
		YYLOGE ("The init function is NULL!");
		return YY_ERR_FAILED;
	}
	
    AVCodecContext *	pDecCtx = NULL;
	if (pFmt->pPrivateData != NULL)
		pDecCtx = (AVCodecContext *)pFmt->pPrivateData;
	if (pDecCtx != NULL)	
		m_uCodecTag = pDecCtx->codec_tag;	
	YYLOGI ("Codec Tag is 0X%08X", m_uCodecTag);

	VO_CODEC_INIT_USERDATA	initInfo;
	memset (&initInfo, 0, sizeof (VO_CODEC_INIT_USERDATA));
	initInfo.memflag = VO_IMF_USERMEMOPERATOR;
	initInfo.memData = &g_memOP;

	m_nCodecType = VO_VIDEO_CodingH264;
	int nRC = m_fAPI.Init (&m_hDec, (VO_VIDEO_CODINGTYPE)m_nCodecType, &initInfo);
	if (m_hDec == NULL)
	{
		YYLOGE ("It call init failed! return %08X!", nRC);
		return YY_ERR_FAILED;
	}
	nRC = SetDecParam (pFmt);
	if (nRC != YY_ERR_NONE)
		return nRC;	
	nRC = SetHeadData (pFmt);
	if (nRC != YY_ERR_NONE)
		return nRC;
	
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
	
	m_bCheckFrameType = false;

	memset (&m_Input, 0, sizeof (m_Input));
	memset (&m_Output, 0, sizeof (m_Output));
	memset (&m_OutputInfo, 0, sizeof (m_OutputInfo));

	m_uFrameSize = m_fmtVideo.nWidth * m_fmtVideo.nHeight;
	
	return YY_ERR_NONE;
}

int CVVVideoOMXDec::Uninit (void)
{
	CVVVideoDec::Uninit ();
	if (m_hDllOMX != NULL)
	{
		dlclose (m_hDllOMX);
		YYLOGI ("Uninit!");
	}
	m_hDllOMX = NULL;
}

int CVVVideoOMXDec::SetBuff (YY_BUFFER * pBuff)
{
//	YYLOGI ("SetBuff: pBuff = %p", pBuff);
		
	int nRC = CVVVideoDec::SetBuff (pBuff);
#if 0
	if (nRC != VO_ERR_RETRY)
	{
		YYLOGI ("Input Time: % 8d, Step: % 8d", (int)pBuff->llTime, (int)(pBuff->llTime - m_llDbgLastTime));	
		m_llDbgLastTime = pBuff->llTime;		
	}
#endif // 0	
	return nRC;
}

int CVVVideoOMXDec::GetBuff (YY_BUFFER * pBuff)
{
//	YYLOGI ("GetBuff: pBuff = %p", pBuff);
		
	int nRC = CVVVideoDec::GetBuff (pBuff);	
	if (nRC == VO_ERR_NONE)
	{
		pBuff->llTime = m_Output.Time;
//		YYLOGI ("Output Time: % 8d, Step: % 8d", (int)pBuff->llTime, (int)(pBuff->llTime - m_llDbgLastTime));	
//		m_llDbgLastTime = pBuff->llTime;		
	}		
	return nRC;
}

int CVVVideoOMXDec::RndBuff (YY_BUFFER * pBuff, bool bRend)
{
#if 0
	if (bRend)
	{
		YYLOGI ("RndBuff: Time = % 8d   ******", (int)pBuff->llTime);
	}
	else
	{
		YYLOGI ("RndBuff: Time = % 8d   ------", (int)pBuff->llTime);
	}
#endif // 000
			
	int nRC = VO_ERR_NONE;
	if (bRend)
		nRC = m_fAPI.SetParam (m_hDec, VO_PID_IOMXDEC_RenderData, &m_Output);
	else
		nRC = m_fAPI.SetParam (m_hDec, VO_PID_IOMXDEC_CancelData, &m_Output);
	if (nRC != VO_ERR_NONE)
	{
		YYLOGE ("render data was failed! return %08X, Rend is %d", nRC, bRend);
		return YY_ERR_FAILED;
	}
	
	pBuff->uFlag |= YYBUFF_DEC_RENDER;
		
	return YY_ERR_NONE;
}

int CVVVideoOMXDec::RndRest (void)
{
	if (m_hDec != NULL)
		m_fAPI.SetParam (m_hDec, VO_PID_IOMXDEC_ForceOutputAll, NULL);
	return YY_ERR_NONE;
}

bool CVVVideoOMXDec::IsWorking (void)
{
	if (m_hDec == NULL)
		return false;
	VO_BOOL bWork = VO_FALSE;
	m_fAPI.GetParam (m_hDec, VO_PID_IOMXDEC_IsWorking, &bWork);
	return bWork ? true : false;
}

int CVVVideoOMXDec::SetInputData (VO_CODECBUFFER * pInData)
{
	if (m_hDec == NULL)
		return YY_ERR_FAILED;
		
	int nRC = YY_ERR_NONE;
	if (m_uCodecTag == YY_FCC_AVC1 || m_uCodecTag == YY_FCC_HAVC)
	{	
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
	}
	
	m_dataInput.Length = pInData->Length;
	nRC = m_fAPI.SetInputData (m_hDec, pInData);
	if (nRC == VO_ERR_IOMXDEC_NeedRetry)
		nRC = VO_ERR_RETRY;
	
	if (nRC == VO_ERR_RETRY)
		m_dataInput.Time = pInData->Time;
	else
		m_dataInput.Length = 0;
//	if (nRC != VO_ERR_NONE)
//		YYLOGW ("SetInputData was failed! return %08X", nRC);
	if (nRC == OMX_ErrorUndefined)
		nRC = VO_ERR_CODEC_UNSUPPORTED;
	return nRC;
}

int CVVVideoOMXDec::SetHeadData (YY_VIDEO_FORMAT * pFmt)
{
	int					nRC = YY_ERR_NONE;
    AVCodecContext *	pDecCtx = NULL;
	if (pFmt->pPrivateData != NULL)
		pDecCtx = (AVCodecContext *)pFmt->pPrivateData;

	if (pDecCtx != NULL)
	{
		if (pDecCtx->extradata_size > 0 && pDecCtx->extradata != NULL)
		{
			VO_CODECBUFFER buffHead;
			if (m_uCodecTag == YY_FCC_AVC1 || m_uCodecTag == YY_FCC_HAVC)
			{
				if (!ConvertH264HeadData (pDecCtx->extradata, pDecCtx->extradata_size))
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

int CVVVideoOMXDec::SetDecParam (YY_VIDEO_FORMAT * pFmt)
{
	if (m_hDec == NULL)
		return YY_ERR_FAILED;
		
	int nRC = 0;
	VO_VIDEO_FORMAT fmtVideo;
	fmtVideo.Width = pFmt->nWidth;	
	fmtVideo.Height = pFmt->nHeight;	
	nRC = m_fAPI.SetParam (m_hDec, VO_PID_VIDEO_FORMAT, &fmtVideo);
			
	nRC = m_fAPI.SetParam (m_hDec, VO_PID_IOMXDEC_SetSurface, m_hView);
	if (nRC != VO_ERR_NONE)
	{
		YYLOGE ("Set surface was failed! return %08X", nRC);
		return YY_ERR_FAILED;
	}

	return YY_ERR_NONE;
}

int CVVVideoOMXDec::FillInfo (YY_BUFFER * pBuff)
{
	return 0;
} 
    
int CVVVideoOMXDec::RestoreInfo (YY_BUFFER * pBuff)
{
	return 0;
}

VO_VIDEO_FRAMETYPE CVVVideoOMXDec::GetH264FrameType(unsigned char * buffer , int size)
{
	int inf,i;
	long byteoffset;      // byte from start of buffer
	int bitoffset;      // bit from start of byte
	int ctr_bit=0;      // control bit for current bit posision
	int bitcounter=1;
	int len;//value;
	int info_bit;
	int totbitoffset = 0;
	int naluType = buffer[0]&0x0f;

	while(naluType!=1&&naluType!=5)//find next NALU
	{
		//buffer = GetNextFrame(buffer,size)
		unsigned char* p = buffer;  
		unsigned char* endPos = buffer+size;
		for (; p < endPos; p++)
		{
			if (XRAW_IS_ANNEXB(p))
			{
				size  -= p-buffer;
				buffer = p+3;
				naluType = buffer[0]&0x0f;
				break;
			}
			if (XRAW_IS_ANNEXB2(p))
			{
				size  -= p-buffer;
				buffer = p+4;
				naluType = buffer[0]&0x0f;
				break;
			}
		}
		if(p>=endPos)
			return VO_VIDEO_FRAME_NULL; 

	}
	if(naluType==5)
		return VO_VIDEO_FRAME_I;//I_FRAME

	buffer++;
	for(i=0;i<2;i++)
	{
		byteoffset= totbitoffset/8;
		bitoffset= 7-(totbitoffset%8);
		ctr_bit = (buffer[byteoffset] & (0x01<<bitoffset));   // set up control bit

		len=1;
		while (ctr_bit==0)
		{                 // find leading 1 bit
			len++;
			bitoffset-=1;           
			bitcounter++;
			if (bitoffset<0)
			{                 // finish with current byte ?
				bitoffset=bitoffset+8;
				byteoffset++;
			}
			ctr_bit=buffer[byteoffset] & (0x01<<(bitoffset));
		}
		// make infoword
		inf=0;                          // shortest possible code is 1, then info is always 0
		for(info_bit=0;(info_bit<(len-1)); info_bit++)
		{
			bitcounter++;
			bitoffset-=1;
			if (bitoffset<0)
			{                 // finished with current byte ?
				bitoffset=bitoffset+8;
				byteoffset++;
			}

			inf=(inf<<1);
			if(buffer[byteoffset] & (0x01<<(bitoffset)))
				inf |=1;
		}
		totbitoffset+=len*2-1;
		if(totbitoffset>48)
			return VO_VIDEO_FRAME_NULL;
	}
	//(int)pow(2,(bitsUsed/2))+info-1;//pow(2,x)==1<<x
	len = (len*2-1)/2;
	inf = (1<<len)+inf-1;
	if (inf>=5)
	{
		inf-=5;
	}
	if(inf<0||inf>2)
		return VO_VIDEO_FRAME_NULL;
	else if (inf == 0)
		return VO_VIDEO_FRAME_P;
	else if (inf == 1)
		return VO_VIDEO_FRAME_B;
	else
		return VO_VIDEO_FRAME_I;

}

/*
	VO_PID_VIDEO_FORMAT:
	VO_PID_IOMXDEC_SetSurface:
	VO_PID_IOMXDEC_SetSecondSurface
	VO_PID_IOMXDEC_RenderData
	VO_PID_IOMXDEC_CancelData
	VO_PID_IOMXDEC_SetCrop
	VO_PID_IOMXDEC_AdaptivePlaybackResolution
	
	VO_PID_COMMON_HEADDATA
	VO_PID_COMMON_FLUSH
	VO_PID_IOMXDEC_ForceOutputAll
*/