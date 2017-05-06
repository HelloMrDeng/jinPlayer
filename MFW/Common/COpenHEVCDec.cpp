/*******************************************************************************
	File:		COpenHEVCDec.cpp

	Contains:	The vo video dec wrap implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "COpenHEVCDec.h"

#include <libavformat/avformat.h>

#include "USystemFunc.h"
#include "ULibFunc.h"

#include "yyLog.h"

COpenHEVCDec::COpenHEVCDec(void * hInst)
	: CBaseVideoDec (hInst)
	, m_hDll (NULL)
	, m_hDec (NULL)
	, m_fInit (NULL)
	, m_fStart (NULL)
	, m_fDec (NULL)
	, m_fGetPicInfo (NULL)
	, m_fCopyExtData (NULL)
	, m_fGetPicSize2 (NULL)
	, m_fGetOutput (NULL)
	, m_fGetOutputCopy (NULL)
	, m_fSetCheckMD5 (NULL)
	, m_fSetDebugMode (NULL)
	, m_fSetTempLayer (NULL)
	, m_fSetNoCrop (NULL)
	, m_fSetActiveDec (NULL)
	, m_fClose (NULL)
	, m_fFlush (NULL)
	, m_fGetVer (NULL)
	, m_llNewPos (0)
{
	SetObjectName ("COHEVCDec");
	// m_nBuffNum = 4;
}

COpenHEVCDec::~COpenHEVCDec(void)
{
	Uninit ();
}

int COpenHEVCDec::Init (YY_VIDEO_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;

	Uninit ();

	m_hDll = yyLibLoad (_T("yyHEVCDec"), 0);
	if (m_hDll == NULL)
		return YY_ERR_FAILED;

	m_fInit			= (LIBOPENHEVCINIT) yyLibGetAddr (m_hDll, "libOpenHevcInit", 0);
	m_fStart		= (LIBOPENHEVCSTARTDECODER) yyLibGetAddr (m_hDll, "libOpenHevcStartDecoder", 0);
	m_fDec			= (LIBOPENHEVCDECODE) yyLibGetAddr (m_hDll, "libOpenHevcDecode", 0);
	m_fGetPicInfo	= (LIBOPENHEVCGETPICTUREINFO) yyLibGetAddr (m_hDll, "libOpenHevcGetPictureInfo", 0);
	m_fCopyExtData	= (LIBOPENHEVCCOPYEXTRADATA) yyLibGetAddr (m_hDll, "libOpenHevcCopyExtraData", 0);
	m_fGetPicSize2	= (LIBOPENHEVCGETPICTURESIZE2) yyLibGetAddr (m_hDll, "libOpenHevcGetPictureSize2", 0);
	m_fGetOutput	= (LIBOPENHEVCGETOUTPUT) yyLibGetAddr (m_hDll, "libOpenHevcGetOutput", 0);
	m_fGetOutputCopy = (LIBOPENHEVCGETOUTPUTCPY) yyLibGetAddr (m_hDll, "libOpenHevcGetOutputCpy", 0);
	m_fSetCheckMD5	= (LIBOPENHEVCSETCHECKMD5) yyLibGetAddr (m_hDll, "libOpenHevcSetCheckMD5", 0);
	m_fSetDebugMode = (LIBOPENHEVCSETDEBUGMODE) yyLibGetAddr (m_hDll, "libOpenHevcSetDebugMode", 0);
	m_fSetTempLayer = (LIBOPENHEVCSETTEMPORALLAYER_ID) yyLibGetAddr (m_hDll, "libOpenHevcSetTemporalLayer_id", 0);
	m_fSetNoCrop	= (LIBOPENHEVCSETNOCROPPING) yyLibGetAddr (m_hDll, "libOpenHevcSetNoCropping", 0);
	m_fSetActiveDec = (LIBOPENHEVCSETACTIVEDECODERS) yyLibGetAddr (m_hDll, "libOpenHevcSetActiveDecoders", 0);
	m_fClose		= (LIBOPENHEVCCLOSE) yyLibGetAddr (m_hDll, "libOpenHevcClose", 0);
	m_fFlush		= (LIBOPENHEVCFLUSH) yyLibGetAddr (m_hDll, "libOpenHevcFlush", 0);
	m_fGetVer		= (LIBOPENHEVCVERSION) yyLibGetAddr (m_hDll, "libOpenHevcVersion", 0);
	if (m_fInit == NULL || m_fClose == NULL)
		return YY_ERR_FAILED;

	int	nRC = 0;
	int nCPUNum = yyGetCPUNum ();
	m_hDec = m_fInit (nCPUNum, 1); // FF_THREAD_FRAME
	if (m_hDec == NULL)
		return YY_ERR_FAILED;

//	m_fSetActiveDec (m_hDec, 1);
//	nRC = m_fStart (m_hDec);

    AVCodecContext *	pDecCtx = NULL;
	if (pFmt->pPrivateData != NULL)
		pDecCtx = (AVCodecContext *)pFmt->pPrivateData;

	if (pDecCtx != NULL && pDecCtx->extradata_size > 0 && pDecCtx->extradata != NULL)
	{
		m_uCodecTag = pDecCtx->codec_tag;

		if (m_uCodecTag == YY_FCC_HVC1 || m_uCodecTag == YY_FCC_HEV1)
		{
			if (!ConvertHEVCHeadData (pDecCtx->extradata, pDecCtx->extradata_size))
				return YY_ERR_FAILED;
			nRC = m_fDec (m_hDec, m_pHeadData, m_nHeadSize, 0);
		}
		else
		{
			nRC = m_fDec (m_hDec, pDecCtx->extradata, pDecCtx->extradata_size, 0);
		}
	}

	memcpy (&m_fmtVideo, pFmt, sizeof (m_fmtVideo));
	m_fmtVideo.nCodecID = AV_CODEC_ID_HEVC;
	m_fmtVideo.pHeadData = NULL;
	m_fmtVideo.nHeadSize = 0;
	m_fmtVideo.pPrivateData = NULL;

	m_bDropFrame = true;
	m_nDecCount = 0;
	m_uBuffFlag = 0;

	memset (&m_frmCopy, 0, sizeof (m_frmCopy));
	m_frmCopy.frameInfo.nBitDepth = 8;
	m_frmCopy.frameInfo.nYPitch = m_fmtVideo.nWidth;
	m_frmCopy.frameInfo.nUPitch = m_frmCopy.frameInfo.nYPitch / 2;
	m_frmCopy.frameInfo.nVPitch = m_frmCopy.frameInfo.nYPitch / 2;
	m_frmCopy.frameInfo.nWidth = m_fmtVideo.nWidth;
	m_frmCopy.frameInfo.nHeight = m_fmtVideo.nHeight;
	m_frmCopy.pvY = new unsigned char [m_fmtVideo.nWidth * m_fmtVideo.nHeight];
	m_frmCopy.pvU = new unsigned char [m_fmtVideo.nWidth * m_fmtVideo.nHeight / 4];
	m_frmCopy.pvV = new unsigned char [m_fmtVideo.nWidth * m_fmtVideo.nHeight / 4];

	return YY_ERR_NONE;
}

int COpenHEVCDec::Uninit (void)
{
	if (m_hDec == NULL)
		return YY_ERR_NONE;

	m_fClose (m_hDec);
	m_hDec = NULL;

	yyLibFree (m_hDll, 0);
	m_hDll = NULL;

	return YY_ERR_NONE;
}

int COpenHEVCDec::Flush (void)
{
	if (m_hDec == NULL)
		return YY_ERR_FAILED;

	CAutoLock lock (&m_mtBuffer);

	m_fFlush (m_hDec);

	return YY_ERR_NONE;
}

int COpenHEVCDec::SetBuff (YY_BUFFER * pBuff)
{
	if (pBuff == NULL || m_hDec == NULL)
		return YY_ERR_ARG;

	CAutoLock lock (&m_mtBuffer);
	CBaseVideoDec::SetBuff (pBuff);

	if ((pBuff->uFlag & YYBUFF_TYPE_PACKET) != YYBUFF_TYPE_PACKET)
		return YY_ERR_ARG;

	AVPacket *	pPacket = (AVPacket *)pBuff->pBuff;
	int			nRC = 0;

	if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
	{
		m_llNewPos = pPacket->pts;
		// YYLOGI ("New Pos % 8d ************************", m_llNewPos);
		Flush ();
	}

	if (m_uCodecTag != YY_FCC_HVC1 && m_uCodecTag != YY_FCC_HEV1)
	{
		nRC = m_fDec (m_hDec, pPacket->data, pPacket->size, pPacket->pts);
	}
	else
	{
		if (!ConvertVideoData (pPacket->data, pPacket->size))
			return YY_ERR_FAILED;
		if (m_pVideoData != NULL)
			nRC = m_fDec (m_hDec, m_pVideoData, m_uVideoSize, pPacket->pts);
		else
			nRC = m_fDec (m_hDec, pPacket->data, pPacket->size, pPacket->pts);
	}

	if (nRC == 0)
		return YY_ERR_NEEDMORE;
	else if (nRC == 1)
		return YY_ERR_NONE;

	return YY_ERR_NONE;
}

int COpenHEVCDec::GetBuff (YY_BUFFER * pBuff)
{
	CAutoLock lock (&m_mtBuffer);
//	memset (&m_frmVideo, 0, sizeof (m_frmVideo));
//	int nRC = m_fGetOutput (m_hDec, 1, &m_frmVideo);
	int nRC = m_fGetOutputCopy (m_hDec, 1, &m_frmCopy);

	if (m_frmCopy.frameInfo.nYPitch == 0)
		return YY_ERR_RETRY;

	if (m_llNewPos > 0 && abs ((int)(m_frmCopy.frameInfo.nTimeStamp - m_llNewPos)) > 1800)
		return YY_ERR_RETRY;
	m_llNewPos = 0;

	if (m_frmCopy.frameInfo.nWidth != m_fmtVideo.nWidth || m_frmCopy.frameInfo.nHeight != m_fmtVideo.nHeight)
	{
		m_fmtVideo.nWidth = m_frmCopy.frameInfo.nWidth;
		m_fmtVideo.nHeight = m_frmCopy.frameInfo.nHeight;
		pBuff->uFlag |= YYBUFF_NEW_FORMAT;
		pBuff->pFormat = &m_fmtVideo;
	}

	m_bufVideo.pBuff[0] = (unsigned char *)m_frmCopy.pvY;
	m_bufVideo.pBuff[1] = (unsigned char *)m_frmCopy.pvU;
	m_bufVideo.pBuff[2] = (unsigned char *)m_frmCopy.pvV;

	m_bufVideo.nStride[0] = m_frmCopy.frameInfo.nYPitch;
	m_bufVideo.nStride[1] = m_frmCopy.frameInfo.nUPitch;
	m_bufVideo.nStride[2] = m_frmCopy.frameInfo.nVPitch;

	m_bufVideo.nWidth = m_frmCopy.frameInfo.nWidth;
	m_bufVideo.nHeight = m_frmCopy.frameInfo.nHeight;

	m_bufVideo.nType = YY_VDT_YUV420_P;

	pBuff->uFlag = YYBUFF_TYPE_VIDEO;
	pBuff->pBuff = (unsigned char *)&m_bufVideo;
	pBuff->uSize = m_nVdoBuffSize;

	pBuff->llTime = m_frmCopy.frameInfo.nTimeStamp;

	CBaseVideoDec::GetBuff (pBuff);

	return YY_ERR_NONE;
}

