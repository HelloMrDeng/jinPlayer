/*******************************************************************************
	File:		CPlayButton.cpp

	Contains:	The play button implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-09		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CPlayButton.h"

#include "USystemFunc.h"
#include "RPlayerDef.h"
#include "yyLog.h"

#pragma warning (disable : 4996)

CPlayButton::CPlayButton(HINSTANCE hInst)
	: CPlayCtrl (hInst)
{
	SetObjectName ("CPlayButton");
}	

CPlayButton::~CPlayButton(void)
{
}

bool CPlayButton::OnDraw (HDC hDC, HBITMAP hBmp, LPBYTE pBuff, RECT * pRect)
{
	if (!m_bShow)
		return false;
	UpdateRect ();
	if (m_rcItem.bottom > pRect->bottom || m_rcItem.right > pRect->right)
	{
		YYLOGI ("The rect of button was large than window! %d X %d   %d X %d", m_rcItem.right, m_rcItem.bottom, pRect->right, pRect->bottom);
		return false;
	}
	int	nW = m_rcItem.right - m_rcItem.left;
	int	nH = m_rcItem.bottom - m_rcItem.top;

	LPBYTE	pBuf = NULL;
	int		nIndex = 0;
	if (!m_bEnable)
		nIndex = 3;
	else if (m_bBtnDown)
		nIndex = 2;
	else if (m_bMusOver)
		nIndex = 1;
	if (nIndex >= m_pBmpFile->GetNum ())
		nIndex = m_pBmpFile->GetNum () - 1;
	HBITMAP	hBmpItem = m_pBmpFile->GetBmpHandle (nIndex, &pBuf);

	unsigned char	cTPN = 0;
	unsigned char * pSrc = pBuf;
	unsigned char * pTar = pBuff;
	for (int h = 0; h < nH; h++)
	{
		pTar = pBuff + pRect->right * 4 * (h + m_rcItem.top) + m_rcItem.left * 4;
		for (int w = 0; w < nW; w++)
		{
			cTPN = pSrc[3];
			*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
			*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
			*pTar =  *pTar++ * (256 - cTPN) / 256 + *pSrc++ * cTPN / 256;
			pTar++; pSrc++;
		}
	}

	return true;
}
