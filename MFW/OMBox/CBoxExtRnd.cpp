/*******************************************************************************
	File:		CBoxExtRnd.cpp

	Contains:	The video render box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#include "CBoxExtRnd.h"

#include "CBoxMonitor.h"

#include "USystemFunc.h"
#include "UYYDataFunc.h"

#include "yyConfig.h"
#include "yyMediaPlayer.h"
#include "yyLog.h"


CBoxExtRnd::CBoxExtRnd(void * hInst)
	: CBoxRender (hInst)
	, m_uCodecTag (0)
	, m_pVDec (NULL)
	, m_pVHead (NULL)
	, m_nVSize (0)
{
	SetObjectName ("CBoxExtRnd");
	strcpy (m_szBoxName, "Box Ext Render");

	m_nBoxType = OMB_TYPE_RND_EXT;
	memset (&m_bufRnd, 0, sizeof (m_bufRnd));
	m_bufRnd.nType = YY_MEDIA_Video;
#ifdef _OS_WIN32
	m_bufRnd.uBuffSize = 1024 * 1024;
	m_bufRnd.pBuff = new unsigned char[m_bufRnd.uBuffSize];
#endif // _OS_WIN32
	
	YYLOGI ("Create ext render box!");
}

CBoxExtRnd::~CBoxExtRnd(void)
{
	YY_DEL_P (m_pVDec);
#ifdef _OS_WIN32
	YY_DEL_A (m_bufRnd.pBuff);
#endif // _OS_WIN32
}

int CBoxExtRnd::SetSource (CBoxBase * pSource)
{
	int nRC = CBoxBase::SetSource (pSource);
	YY_VIDEO_FORMAT * pFmt = pSource->GetVideoFormat ();
	if (pFmt != NULL)
	{
		AVCodecContext *	pDecCtx = NULL;
		if (pFmt->pPrivateData != NULL)
		{
			pDecCtx = (AVCodecContext *)pFmt->pPrivateData;
			m_uCodecTag = pDecCtx->codec_tag;

			if (pDecCtx->extradata_size > 0 && pDecCtx->extradata != NULL)
			{
				if (m_uCodecTag == YY_FCC_AVC1 || m_uCodecTag == YY_FCC_HAVC)
				{
					if (m_pVDec == NULL)
						m_pVDec = new CBaseVideoDec (m_hInst);
					if (m_pVDec->ConvertH264HeadData (pDecCtx->extradata, pDecCtx->extradata_size))
						m_nVSize = m_pVDec->GetHeadData (&m_pVHead);
				}
				else
				{
					m_pVHead = pDecCtx->extradata;
					m_nVSize = pDecCtx->extradata_size;		
				}
			}
		}
		else if (pFmt->pHeadData != NULL && pFmt->nHeadSize > 0)
		{
			if (m_uCodecTag == YY_FCC_AVC1 || m_uCodecTag == YY_FCC_HAVC)
			{
				if (m_pVDec == NULL)
					m_pVDec = new CBaseVideoDec (m_hInst);
				m_pVDec->ConvertH264HeadData (pFmt->pHeadData, pFmt->nHeadSize);
				m_nVSize = m_pVDec->GetHeadData (&m_pVHead);
			}
		}
	}
	return nRC;
}

int CBoxExtRnd::RenderFrame (bool bInBox, bool bWait)
{
	int nRC = ReadBuffer (&m_bufRnd, false);
	if (nRC == YY_ERR_NONE)
	{
		while (m_pClock->GetTime () < m_bufRnd.llTime)
		{
			if (m_status != YYRND_RUN)
				break;
			yySleep (2000);
		}
	}
	return nRC;
}

int CBoxExtRnd::ReadBuffer (YY_BUFFER * pBuffer, bool bWait)
{	
	if (pBuffer == NULL || pBuffer->pBuff == NULL)
		return YY_ERR_ARG;
	if (m_pBoxSource == NULL)
		return YY_ERR_STATUS;
	pBuffer->uFlag = 0;
	int nRC = YY_ERR_NONE;
	if (pBuffer->nType == YY_MEDIA_Video)
	{
		if (m_pVHead != NULL && m_nVSize > 0)
		{
			memcpy (pBuffer->pBuff, m_pVHead, m_nVSize);
			pBuffer->uSize = m_nVSize;
			pBuffer->uFlag = YYBUFF_NEW_FORMAT;
			m_pVHead = NULL;
			m_nVSize = 0;
			return YY_ERR_NONE;
		}
	}
	YY_BUFFER * pSrcBuf = m_pCurrBuffer;
	if (pSrcBuf == NULL)
	{
		m_pBaseBuffer->nType = pBuffer->nType;
		nRC = m_pBoxSource->ReadBuffer (m_pBaseBuffer, false);
		pSrcBuf = m_pBaseBuffer;
	}
	if (m_pCurrBuffer != NULL)
		m_pCurrBuffer = NULL;	
	if (nRC != YY_ERR_NONE && nRC != YY_ERR_FINISH)
		return nRC;
		
	if (nRC == YY_ERR_NONE)
	{
		if ((pSrcBuf->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
		{
			if (pBuffer->nType == YY_MEDIA_Video)
			{
				YY_VIDEO_FORMAT * pFmt = (YY_VIDEO_FORMAT *)pSrcBuf->pFormat;
				if (pFmt != NULL && pFmt->nHeadSize > 0)
				{
					unsigned char * pHeadData = pFmt->pHeadData;
					int				nHeadSize = pFmt->nHeadSize;
					if (m_uCodecTag == YY_FCC_AVC1 || m_uCodecTag == YY_FCC_HAVC)
					{
						if (m_pVDec == NULL)
							m_pVDec = new CBaseVideoDec (m_hInst);
						m_pVDec->ConvertH264HeadData (pFmt->pHeadData, pFmt->nHeadSize);
						nHeadSize = m_pVDec->GetHeadData (&pHeadData);
					}

					if (pBuffer->uBuffSize < pFmt->nHeadSize)
						return YY_ERR_MEMORY;
					memcpy (pBuffer->pBuff, pFmt->pHeadData, pFmt->nHeadSize);
					pBuffer->uSize = pFmt->nHeadSize;
					pBuffer->uFlag = YYBUFF_NEW_FORMAT;
					pBuffer->pFormat = m_pBoxSource->GetVideoFormat ();
					m_pCurrBuffer = pSrcBuf;
					m_pCurrBuffer->uFlag = m_pCurrBuffer->uFlag & ~YYBUFF_NEW_FORMAT;
					YYLOGI ("This is new format buffer!");
					return YY_ERR_NONE;
				}
			}
		}
	
		if ((pSrcBuf->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		{
			m_pCurrBuffer = pSrcBuf;
			m_pCurrBuffer->uFlag = m_pCurrBuffer->uFlag & ~YYBUFF_NEW_POS;
			pBuffer->uFlag = YYBUFF_NEW_POS;
			pBuffer->uSize = 0;
			YYLOGI ("This is new pos buffer!");
			return YY_ERR_NONE;
		}
	
		unsigned char * pData = NULL;
		int				nSize = 0;
		pBuffer->uFlag = pSrcBuf->uFlag;
		pBuffer->llTime = pSrcBuf->llTime;
		if ((pSrcBuf->uFlag & YYBUFF_TYPE_PACKET) == YYBUFF_TYPE_PACKET)
		{
			AVPacket * pPacket = (AVPacket *)pSrcBuf->pBuff;
			pData = pPacket->data;
			nSize = pPacket->size;
		}
		else if ((pSrcBuf->uFlag & YYBUFF_TYPE_DATA) == YYBUFF_TYPE_DATA)
		{
			pData = pSrcBuf->pBuff;
			nSize = pSrcBuf->uSize;
		}
		else
		{
			pData = pSrcBuf->pBuff;
			nSize = pSrcBuf->uSize;
		}
		if (pData != NULL)
		{
			if (m_uCodecTag == YY_FCC_AVC1 || m_uCodecTag == YY_FCC_HAVC)
			{
				if (m_pVDec == NULL)
					m_pVDec = new CBaseVideoDec (m_hInst);
				if (m_pVDec->ConvertVideoData (pData, nSize))
				{
					unsigned char * pTmpData = NULL;
					int				nTmpSize = 0;
					nTmpSize = m_pVDec->GetVideoData (&pTmpData);
					if (pTmpData != NULL)
					{
						pData = pTmpData;
						nSize = nTmpSize;
					}
				}
			}
			if (pBuffer->uBuffSize < nSize)
				return YY_ERR_MEMORY;
			memcpy (pBuffer->pBuff, pData, nSize);
			pBuffer->uSize = nSize;
		}
		m_nRndCount++;
	}

	if ((pSrcBuf->uFlag & YYBUFF_EOS) == YYBUFF_EOS || nRC == YY_ERR_FINISH)
	{		
		if (m_bEOS)
		{
			yySleep (10000);
		}
		else
		{
			m_bEOS = true;
			if (m_pNotifyFunc != NULL)
				m_pNotifyFunc (m_pUserData, YY_EV_Play_Complete, this);
		}
	}
	
	return nRC;
}

#ifdef _OS_NDK
int	CBoxExtRnd::Start (CThreadWork * pWork)
{
	int nRC = CBoxBase::Start ();
	m_status = YYRND_RUN;
	m_bEOS = false;	
	return nRC;
}

int CBoxExtRnd::Pause (void)
{
	int nRC = CBoxBase::Pause ();
	m_status = YYRND_PAUSE;
	return nRC;
}

int	CBoxExtRnd::Stop (void)
{
	int nRC = CBoxBase::Pause ();
	m_status = YYRND_STOP;
	return nRC;
}
#endif // _OS_NDK