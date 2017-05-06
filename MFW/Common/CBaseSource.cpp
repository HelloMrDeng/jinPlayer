/*******************************************************************************
	File:		CBaseSource.cpp

	Contains:	base source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "CBaseSource.h"
#include "CBaseConfig.h"

#include <libavformat/avformat.h>

#include "USystemFunc.h"
#include "yyConfig.h"
#include "yyLog.h"

CBaseSource::CBaseSource(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_nSourceType (0)
	, m_bForceClosed (false)
{
	SetObjectName ("CBaseSource");

	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));
	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
	memset (&m_fmtSubTT, 0, sizeof (m_fmtSubTT));

	m_bSubTTEnable = false;
	ResetParam (0);
}

CBaseSource::~CBaseSource(void)
{
	Close ();

	YY_DEL_A (m_fmtAudio.pHeadData);
	YY_DEL_A (m_fmtVideo.pHeadData);
}

int CBaseSource::Open (const TCHAR * pSource, int nType)
{
	ResetParam (0);

	return YY_ERR_NONE;
}

int CBaseSource::Close (void)
{
	return YY_ERR_NONE;
}

int CBaseSource::ForceClose (void)
{
	return YY_ERR_NONE;
}

int CBaseSource::GetStreamCount (YYMediaType nType)
{
	if (nType == YY_MEDIA_Video)
		return m_nVideoStreamNum;
	else if (nType == YY_MEDIA_Audio)
		return m_nAudioStreamNum;
	else if (nType == YY_MEDIA_SubTT)
		return m_nSubTTStreamNum;
	else
		return 0;
}

int CBaseSource::GetStreamPlay (YYMediaType nType)
{
	if (nType == YY_MEDIA_Video)
		return m_nVideoStreamPlay;
	else if (nType == YY_MEDIA_Audio)
		return m_nAudioStreamPlay;
	else if (nType == YY_MEDIA_SubTT)
		return m_nSubTTStreamPlay;
	else
		return -1;
}

int CBaseSource::SetStreamPlay (YYMediaType nType, int nStream)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseSource::EnableSubTT (bool bEnable)
{
	m_bSubTTEnable = bEnable;
	return YY_ERR_NONE;
}

int	CBaseSource::Start (void)
{
	return YY_ERR_NONE;
}

int CBaseSource::Pause (void)
{
	return YY_ERR_NONE;
}

int	CBaseSource::Stop (void)
{
	return YY_ERR_NONE;
}

int CBaseSource::ReadData (YY_BUFFER * pBuff)
{
	if (pBuff == NULL)
		return YY_ERR_ARG;

	if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
	{
		if (abs ((int)(m_llSeekPos - pBuff->llTime)) > 2000)
		{
			return SetPos (pBuff->llTime);
		}
	}

	return YY_ERR_NONE;
}

int CBaseSource::SetPos (long long llPos)
{
	m_llSeekPos = llPos;

	if (GetStreamCount (YY_MEDIA_Audio) > 0)
		m_bAudioNewPos = true;
	if (GetStreamCount (YY_MEDIA_Video) > 0)
		m_bVideoNewPos = true;
	if (GetStreamCount (YY_MEDIA_SubTT) > 0)
		m_bSubTTNewPos = true;

	return YY_ERR_NONE;
}

int CBaseSource::GetPos (void)
{
	return 0;
}

int CBaseSource::GetDuration (void)
{
	return (int)m_llDuration;
}
 
int CBaseSource::SetParam (int nID, void * pParam)
{
	return YY_ERR_PARAMID;
}

int CBaseSource::GetParam (int nID, void * pParam)
{
	return YY_ERR_PARAMID;
}

int CBaseSource::GetMediaInfo (TCHAR * pInfo, int nSize)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseSource::FillMediaInfo (YYINFO_Thumbnail * pInfo)
{
	return YY_ERR_IMPLEMENT;
}

void CBaseSource::ResetParam (int nLevel)
{
	strcpy (m_szSourceName, "");
	_tcscpy (m_szSource, _T(""));
	m_nProtType = YY_PROT_FILE;
	m_llDuration = 0;
	m_llDurAudio = 0;
	m_llDurVideo = 0;
	m_nVideoStreamNum = 0;
	m_nAudioStreamNum = 0;
	m_nSubTTStreamNum = 0;
	m_nAudioStreamPlay = -1;
	m_nVideoStreamPlay = -1;
	m_nSubTTStreamPlay = -1;

	YY_DEL_A (m_fmtAudio.pHeadData);
	YY_DEL_A (m_fmtVideo.pHeadData);
	YY_DEL_A (m_fmtSubTT.pHeadData);
	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));
	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
	memset (&m_fmtSubTT, 0, sizeof (m_fmtSubTT));

	m_llSeekPos = 0;
	m_bVideoNewPos = false;
	m_bAudioNewPos = false;

	m_bEOF = true;
	m_bEOV = true;
	m_bEOA = true;
}

int CBaseSource::CheckConfigLimit (int nCodec, int nWidth, int nHeight)
{
	TCHAR szFile[1024];
	memset (szFile, 0, sizeof (szFile));
	yyGetAppPath (m_hInst, szFile, sizeof (szFile));	
	_tcscat (szFile, _T("yyLmtSize.cfg"));

	CBaseConfig cfgFile;
	if (cfgFile.Open (szFile) == false)
		return YY_ERR_NONE;

	int nEnableCheck = cfgFile.GetItemValue ("Check", "Enable", 1);
	if (nEnableCheck <= 0)
		return YY_ERR_NONE;

	char	szName[32];
	int		nW = 0, nH = 0;
	
	switch (nCodec)
	{
	case AV_CODEC_ID_H265:
		strcpy (szName, "H265");
		break;

	case AV_CODEC_ID_H264:
		strcpy (szName, "H264");
		break;

	case AV_CODEC_ID_MPEG4:
		strcpy (szName, "MPEG4");
		break;

	case AV_CODEC_ID_RV30:
	case AV_CODEC_ID_RV40:
		strcpy (szName, "REAL");
		break;

	case AV_CODEC_ID_WMV3:
		strcpy (szName, "WMV9");
		break;

	case AV_CODEC_ID_VC1:
		strcpy (szName, "VC1");
		break;

	case AV_CODEC_ID_H263:
		strcpy (szName, "H263");
		break;

	case AV_CODEC_ID_MPEG2VIDEO:
		strcpy (szName, "MPEG2");
		break;

	case AV_CODEC_ID_VP8:
		strcpy (szName, "VP8");
		break;

	default:
		strcpy (szName, "MAX");
		break;
	}

	CCfgSect * pCodec = cfgFile.FindSect (szName);
	if (pCodec == NULL)
		return YY_ERR_NONE;

	nW = cfgFile.GetItemValue (szName, "Width", 0);
	nH = cfgFile.GetItemValue (szName, "Height", 0);

	if (nW * nH < nWidth * nHeight)
		return YY_ERR_OUTOFLIMIT;

	return YY_ERR_NONE;
}
