/*******************************************************************************
	File:		CBoxPlayer.cpp

	Contains:	Box Player implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-13		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CBoxPlayer.h"
#include "CDlgFileProp.h"
#include "CDlgOpenURL.h"
#include "CDlgSubTT.h"

#include "USystemFunc.h"
#include "UBitmapFunc.h"
#include "yyLog.h"
#include "resource.h"

#pragma warning (disable : 4996)

int CBoxPlayer::VideoExtRender (void * pUserData, YY_BUFFER * pData)
{
	CBoxPlayer * pPlayer = (CBoxPlayer *)pUserData;
	if (pData == NULL)
		return YY_ERR_ARG;
	return pPlayer->RenderVideo (pData);
}

int CBoxPlayer::RenderVideo (YY_BUFFER * pData)
{
	YY_BUFFER * pVideoBuff = pData;
	RECT		rcView;
	GetClientRect (m_hWnd, &rcView);
	m_pMedia->GetParam (YYPLAY_PID_RenderArea, &rcView);
	rcView.right = rcView.right & ~1;
	int nW = rcView.right - rcView.left;
	int nH = rcView.bottom - rcView.top;
	LPBYTE		pBmpBuff = NULL;
	HDC hDC = GetDC (m_hWnd);
	HBITMAP hBmp = yyBmpCreate (hDC, rcView.right - rcView.left, rcView.bottom - rcView.top, &pBmpBuff, 0);

	YY_BUFFER_CONVERT buffConv;
	buffConv.pSource = pVideoBuff;
	buffConv.pTarget = new YY_BUFFER ();
	memset (buffConv.pTarget, 0, sizeof (YY_BUFFER));

	YY_VIDEO_BUFF buffVideo;
	memset (&buffVideo, 0, sizeof (YY_VIDEO_BUFF));
	buffVideo.nType = YY_VDT_RGBA;
	buffVideo.nWidth = rcView.right - rcView.left;
	buffVideo.nHeight = rcView.bottom - rcView.top;
	buffVideo.pBuff[0] = pBmpBuff;
	buffVideo.nStride[0] = buffVideo.nWidth * 4;

	buffConv.pTarget->nType = YY_MEDIA_Video;
	buffConv.pTarget->pBuff = (unsigned char *)&buffVideo;
	buffConv.pTarget->uFlag = YYBUFF_TYPE_VIDEO;
	buffConv.pTarget->uSize = sizeof (YY_VIDEO_BUFF);

	m_pMedia->GetParam (YYPLAY_PID_ConvertData, &buffConv);

	delete buffConv.pTarget;

	unsigned char * pRGB = pBmpBuff + nW * nH * 2;
	for (int i = 0; i < nH / 2; i++)
	{
		for (int j = 0; j < nW; j++)
		{
			*pRGB = *pRGB++ * 2 / 3;
			*pRGB = *pRGB++ * 2 / 3;
			*pRGB = *pRGB++ * 2 / 3;
			pRGB++;
		}
	}

	if (m_bmpFile.GetWidth () == 0)
	{
		TCHAR szFile[1024];
		yyGetAppPath  (m_hInst, szFile, sizeof (szFile));
		_tcscat (szFile, _T("sureres\\btn_FF.bmp"));
		m_bmpFile.ReadBmpFile (hDC, szFile, 1);
	}

	LPBYTE	pBuf = NULL;
	HBITMAP	hBmpFile = m_bmpFile.GetBmpHandle (0, &pBuf);
	unsigned char * pTar = pBmpBuff + nW * nH * 2;
	unsigned char * pSrc = pBuf;
	for (int i = 0; i < m_bmpFile.GetHeight (); i++)
	{
		pTar = pBmpBuff + nW * nH * 2 + nW * i * 4;
		for (int j = 0; j < m_bmpFile.GetWidth (); j++)
		{
			*pTar = (*pTar++  * (256 - *pSrc)) / 256 + *pSrc++;
			*pTar = (*pTar++  * (256 - *pSrc)) / 256 + *pSrc++;
			*pTar = (*pTar++  * (256 - *pSrc)) / 256 + *pSrc++;
			pSrc++;
			pTar++;
		}
	}


	HDC hBmpDC = ::CreateCompatibleDC (hDC);
	HBITMAP hOld = (HBITMAP)SelectObject (hBmpDC, hBmp);
	BitBlt (hDC, rcView.left, rcView.top, rcView.right - rcView.left, rcView.bottom - rcView.top, hBmpDC, 0, 0, SRCCOPY);
	SelectObject (hBmpDC, hOld);
	DeleteDC (hBmpDC);
	ReleaseDC (m_hWnd, hDC);
	DeleteObject (hBmp);

	return YY_ERR_NONE;
}