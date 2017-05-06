/*******************************************************************************
	File:		CMediaThumb.cpp

	Contains:	Media Engine implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-24		Fenger			Create file

*******************************************************************************/
#include "CMediaThumb.h"

#include "CFFMpegSource.h"
#include "CImageSource.h"
#include "CFFMpegVideoDec.h"

#include "UStringFunc.h"
#include "USystemFunc.h"
#include "UYYDataFunc.h"
#include "UFFMpegFunc.h"
#include "UFileFormat.h"
#include "yyLog.h"

CMediaThumb::CMediaThumb(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_bCancel (false)
	, m_pMedia (NULL)
	, m_pDec (NULL)
	, m_pVideoRCC (NULL)
	, m_hMemDC (NULL)
	, m_pBmpInfo (NULL)
	, m_pBmpBuff (NULL)
	, m_nPixelBits (32)
	, m_nRndStride (0)
{
	SetObjectName ("CMediaThumb");

	_tcscpy (m_szFile, _T(""));

	memset (&m_buffMedia, 0, sizeof (YY_BUFFER));
	m_buffMedia.nType = YY_MEDIA_Video;

	memset (&m_buffVideo, 0, sizeof (YY_BUFFER));
	m_buffVideo.nType = YY_MEDIA_Video;

}

CMediaThumb::~CMediaThumb(void)
{
	CloseMedia ();

#ifdef _OS_WIN32
	if (m_hMemDC != NULL)
		DeleteDC (m_hMemDC);
	m_hMemDC = NULL;

	YY_DEL_A (m_pBmpInfo);
#endif // _OS_WIN32
}

bool CMediaThumb::Cancel (void)
{
	if (m_pMedia != NULL)
		m_pMedia->ForceClose ();

	m_bCancel = true;

	return true;
}

HBITMAP	CMediaThumb::GetThumb (const TCHAR * pFile, YYINFO_Thumbnail * pThumbInfo)
{
	int	nRC = 0;

	m_bCancel = false;
	if (pThumbInfo == NULL)
		pThumbInfo = &m_thumbInfo;

	if ((pThumbInfo->nInfoType & YYINFO_OPEN_ExtSource) == YYINFO_OPEN_ExtSource)
	{
		CloseMedia ();
	}
	else
	{
		if (_tcscmp (m_szFile, pFile))
		{
			CloseMedia ();
			_tcscpy (m_szFile, pFile);
		}
	}

	if (m_pMedia == NULL)
	{
		YYMediaType yyType = YY_MEDIA_Image;
		if ((pThumbInfo->nInfoType & YYINFO_OPEN_ExtSource) == YYINFO_OPEN_ExtSource)
			yyType = yyffGetType (pFile, YY_OPEN_SRC_READ);
		else
			yyType = yyffGetType (m_szFile, 0);
		if (yyType == YY_MEDIA_Image)
			m_pMedia = new CImageSource (m_hInst);
		else
			m_pMedia = new CFFMpegSource (m_hInst);

		if (m_pMedia == NULL)
		{
			YYLOGE ("Create the source failed!");
			return NULL;
		}

		if ((pThumbInfo->nInfoType & YYINFO_OPEN_ExtSource) == YYINFO_OPEN_ExtSource)
		{
			int nType = YY_OPEN_SRC_VIDEO | YY_OPEN_SRC_READ;
			nRC = m_pMedia->Open (pFile, nType);
		}
		else
		{
			nRC = m_pMedia->Open (m_szFile, YY_OPEN_SRC_VIDEO);
		}
		if ((nRC < 0 && nRC != YY_ERR_SOURCE) || m_bCancel)
		{
			YYLOGE ("Open the source failed! Error: %08X", nRC);
			CloseMedia ();
			return NULL;
		}
	}

	m_pMedia->FillMediaInfo (pThumbInfo);
	if ((pThumbInfo->nInfoType & YYINFO_Get_Thumbnail) != YYINFO_Get_Thumbnail)
	{
		CloseMedia ();
		return NULL;
	}

	if (m_pMedia->GetStreamCount (YY_MEDIA_Video) <= 0)
	{
		YYLOGI ("There is no video!");
		CloseMedia ();
		return NULL;
	}

	int nPos = pThumbInfo->nPos;
	if (nPos >= m_pMedia->GetDuration ())
		nPos = 0;

	if (m_bCancel)
	{
		CloseMedia ();
		return NULL;
	}

	m_buffMedia.uFlag = 0;
	m_buffMedia.llTime = 0;
	m_buffMedia.pBuff = NULL;
	if (nPos > 0)
	{
		m_pMedia->SetPos (nPos);
		m_buffMedia.uFlag = YYBUFF_NEW_POS;
		m_buffMedia.llTime = nPos;
	}

	int nStartTime = yyGetSysTime ();
	nRC = YY_ERR_RETRY;
	while (nRC != YY_ERR_NONE)
	{
		nRC = m_pMedia->ReadData (&m_buffMedia);
		if (nRC == YY_ERR_FINISH)
			break;

		if (nRC == YY_ERR_NONE)
		{
			if (m_pDec == NULL)
			{
				m_pDec = new CFFMpegVideoDec (m_hInst);
				if (m_pDec == NULL)
					break;
				YY_VIDEO_FORMAT * pFmtVideo = m_pMedia->GetVideoFormat ();
				m_pDec->Init (pFmtVideo);
			}

			nRC = m_pDec->SetBuff (&m_buffMedia);
			if (nRC == YY_ERR_NONE)
			{
				nRC = m_pDec->GetBuff (&m_buffVideo);
				if (nRC == YY_ERR_NONE)
				{
					if ((pThumbInfo->nInfoType & YYINFO_Get_NoBlack) == YYINFO_Get_NoBlack)
					{
						if (nPos > 30000 || !IsBlackVideo (&m_buffVideo))
							break;
						nPos += 10000;
						m_pMedia->SetPos (nPos);
						nRC = YY_ERR_RETRY;
					}
					else
					{
						break;
					}
				}
			}

			m_buffMedia.uFlag = 0;
		}
		if (pThumbInfo->nTryTime > 0 && yyGetSysTime () - nStartTime > pThumbInfo->nTryTime)
		{
			CloseMedia ();
			YYLOGI ("It was out of time %d. ", pThumbInfo->nTryTime);
			return NULL;
		}
		if (m_bCancel)
		{
			CloseMedia ();
			return NULL;
		}
	}

	if (m_buffVideo.pBuff == NULL)
	{
		CloseMedia ();
		YYLOGE ("It can not decode video data! ");
		return NULL;
	}

	YY_VIDEO_FORMAT * pFmtVDec = m_pDec->GetVideoFormat ();
	if (pFmtVDec->nHeight <= 0 || pFmtVDec->nWidth <= 0)
	{
		CloseMedia ();
		YYLOGE ("The video size is wrong! ");
		return NULL;
	}
	pThumbInfo->nVNum = pFmtVDec->nNum;
	pThumbInfo->nVDen = pFmtVDec->nDen;

	HBITMAP hBMP = CreateResBMP (pThumbInfo->nThumbWidth, pThumbInfo->nThumbHeight);
	if (hBMP == NULL)
	{
		CloseMedia ();
		YYLOGE ("It create the thumb buffer failed! ");
		return NULL;
	}

	if (m_bCancel)
	{
		CloseMedia ();
		return NULL;
	}

	m_buffThumb.nWidth = pThumbInfo->nThumbWidth;
	m_buffThumb.nHeight = pThumbInfo->nThumbHeight;
	if (pThumbInfo->bKeepAspectRatio)
	{
		int nRndW = pThumbInfo->nThumbWidth;
		int nRndH = pThumbInfo->nThumbHeight;

//		if (pFmtVDec->nWidth * nRndH >= pFmtVDec->nHeight * nRndW)
//			nRndH = nRndW * pFmtVDec->nHeight / pFmtVDec->nWidth;
//		else
//			nRndW = nRndH * pFmtVDec->nWidth / pFmtVDec->nHeight;
		yyAdjustVideoSize (&nRndW, &nRndH, pFmtVDec->nWidth, pFmtVDec->nHeight,pFmtVDec->nNum, pFmtVDec->nDen); 
		m_buffThumb.nWidth = nRndW & ~1;
		m_buffThumb.nHeight = nRndH & ~1;

		int *	pRGB = (int *)m_pBmpBuff;
		int		i = 0;
		int		j = 0;
		if (pThumbInfo->nThumbHeight > m_buffThumb.nHeight)
		{
			int nH = (pThumbInfo->nThumbHeight - m_buffThumb.nHeight) / 2;
			for (i = 0; i < nH; i++)
			{
				pRGB = (int *)(m_pBmpBuff + m_nRndStride * i);
				for (j = 0; j < pThumbInfo->nThumbWidth; j++)
					*pRGB++ = pThumbInfo->nBGColor;
			}
			for (i = m_buffThumb.nHeight + nH; i < pThumbInfo->nThumbHeight; i++)
			{
				pRGB = (int *)(m_pBmpBuff + m_nRndStride * i);
				for (j = 0; j < pThumbInfo->nThumbWidth; j++)
					*pRGB++ = pThumbInfo->nBGColor;
			}
		}

		if (pThumbInfo->nThumbWidth > m_buffThumb.nWidth)
		{
			int nW = (pThumbInfo->nThumbWidth - m_buffThumb.nWidth) / 2;
			for (i = 0; i < pThumbInfo->nThumbHeight; i++)
			{
				pRGB = (int *)(m_pBmpBuff + m_nRndStride * i);
				for (j = 0; j < nW; j++)
					*pRGB++ = pThumbInfo->nBGColor;

				pRGB = (int *)(m_pBmpBuff + m_nRndStride * i + (m_buffThumb.nWidth + nW) * 4);
				for (j = m_buffThumb.nWidth + nW; j < pThumbInfo->nThumbWidth; j++)
					*pRGB++ = pThumbInfo->nBGColor;
			}
		}
	}

	int nBmpOffsetX = (pThumbInfo->nThumbWidth - m_buffThumb.nWidth) * 2;
	int nBmpOffsetY = (pThumbInfo->nThumbHeight - m_buffThumb.nHeight) / 2 * m_nRndStride;

	m_buffThumb.pBuff[0] = m_pBmpBuff + nBmpOffsetX + nBmpOffsetY;
	m_buffThumb.nStride[0] = m_nRndStride;
	m_buffThumb.nType = YY_VDT_RGBA;

	if (m_bCancel)
	{
		CloseMedia ();
		return NULL;
	}

	if (m_pVideoRCC == NULL)
	{
		m_pVideoRCC = new CFFMpegVideoRCC (m_hInst);
		if (m_pVideoRCC == NULL)
		{
			CloseMedia ();
			return NULL;
		}
	}
	nRC = m_pVideoRCC->ConvertBuff (&m_buffVideo, &m_buffThumb, NULL);
	if (nRC != YY_ERR_NONE)
	{
#ifdef _OS_WIN32				
		DeleteObject (hBMP);
#elif defined _OS_NDK
		delete []m_pBmpBuff;
		m_pBmpBuff = NULL;
#endif // _OS_WIN32
		CloseMedia ();
		YYLOGE ("It convert data failed. Error: %08X", nRC);
		return NULL;
	}

	pThumbInfo->nPos = (int)m_buffVideo.llTime;
	pThumbInfo->pBmpBuff = m_pBmpBuff;
	pThumbInfo->hThumbnail = hBMP;

	CloseMedia ();

	return hBMP;
}

HBITMAP CMediaThumb::CreateResBMP (int nW, int nH )
{
	HBITMAP hBitmap = NULL;

#ifdef _OS_WIN32
	int nBmpSize = sizeof(BITMAPINFOHEADER);
	if (m_pBmpInfo == NULL)
		m_pBmpInfo = new BYTE[nBmpSize];
	memset (m_pBmpInfo, 0, nBmpSize);

	BITMAPINFO * pBmpInfo = (BITMAPINFO *)m_pBmpInfo;
	pBmpInfo->bmiHeader.biSize			= nBmpSize;
	pBmpInfo->bmiHeader.biWidth			= nW;
	pBmpInfo->bmiHeader.biHeight		= -nH;
	pBmpInfo->bmiHeader.biBitCount		= (WORD)m_nPixelBits;
	pBmpInfo->bmiHeader.biCompression	= BI_RGB;
	pBmpInfo->bmiHeader.biXPelsPerMeter	= 0;
	pBmpInfo->bmiHeader.biYPelsPerMeter	= 0;
	pBmpInfo->bmiHeader.biPlanes		= 1;

	m_nRndStride = ((pBmpInfo->bmiHeader.biWidth * pBmpInfo->bmiHeader.biBitCount / 8) + 3) & ~3;
	pBmpInfo->bmiHeader.biSizeImage	= m_nRndStride * nH;

	if (m_hMemDC == NULL)
	{
		HDC hDC = GetDC (NULL);
		m_hMemDC = CreateCompatibleDC (hDC);
		ReleaseDC (NULL, hDC);
	}

	hBitmap = CreateDIBSection(m_hMemDC , (BITMAPINFO *)m_pBmpInfo , DIB_RGB_COLORS , (void **)&m_pBmpBuff, NULL , 0);
#elif defined _OS_NDK
	m_nRndStride = nW * 4;
	m_pBmpBuff = new unsigned char[nW * nH * 4];
	hBitmap = m_pBmpBuff;
#endif // _OS_WIN32
	return hBitmap;
}

void CMediaThumb::CloseMedia (void)
{
	YY_DEL_P (m_pVideoRCC);

	if (m_pDec != NULL)
	{
		m_pDec->Uninit ();
		delete m_pDec;
	}
	m_pDec = NULL;

	if (m_pMedia != NULL)
	{
		m_pMedia->Close ();
		delete m_pMedia;
	}
	m_pMedia = NULL;

	m_buffVideo.pBuff = NULL;
	m_buffVideo.uSize = 0;

#ifdef _OS_NDK
	if (m_pBmpBuff != NULL)
		delete []m_pBmpBuff;
	m_pBmpBuff = NULL;
#endif // _OS_NDK
}

bool CMediaThumb::IsBlackVideo (YY_BUFFER * pVideo)
{
	YY_VIDEO_FORMAT *	pFmt = m_pDec->GetVideoFormat ();
	AVFrame *			pFrmVideo = (AVFrame *)pVideo->pBuff;;
	if (pFrmVideo->format != AV_PIX_FMT_YUV420P)
		return false;

	unsigned char *	pY = NULL;
	unsigned char *	pU = NULL;
	unsigned char *	pV = NULL;
	int				nBlight = 0;
	int				nY = 50;
	int				i, j;
	int				nHeight = pFmt->nHeight / 4;
	
	if (nHeight > 20)
		nHeight = 20;
	for (i = pFmt->nHeight / 2; i < pFmt->nHeight / 2 + nHeight; i+=2)
	{
		pY = pFrmVideo->data[0] + i * pFrmVideo->linesize[0];
		for (j = 0; j < pFmt->nWidth; j++)
		{
			if (*(pY++) > nY)
			{
				nBlight++;
				if (nBlight > pFmt->nWidth)
					return false;
			}
		}
	}

	nHeight = nHeight / 2;
	for (i = pFmt->nHeight / 4; i < pFmt->nHeight / 4 + nHeight; i+=2)
	{
		pU = pFrmVideo->data[1] + i * pFrmVideo->linesize[1];
		for (j = 0; j < pFmt->nWidth / 2; j++)
		{
			if (*(pU++) < 80 || *(pU++) > 160)
			{
				nBlight++;
				if (nBlight > pFmt->nWidth)
					return false;
			}
		}
		pV = pFrmVideo->data[2] + i * pFrmVideo->linesize[2];
		for (j = 0; j < pFmt->nWidth / 2; j++)
		{
			if (*(pU++) < 80 || *(pU++) > 160)
			{
				nBlight++;
				if (nBlight > pFmt->nWidth)
					return false;
			}
		}
	}

	return true;
}
