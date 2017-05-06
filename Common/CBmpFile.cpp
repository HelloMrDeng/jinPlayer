/*******************************************************************************
	File:		CBmpFile.cpp

	Contains:	the bitmap file implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-27		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "CBmpFile.h"

#include "USystemFunc.h"
#include "UFileFunc.h"
#include "UBitmapFunc.h"

#include "yyLog.h"

CBmpFile::CBmpFile(void)
	: CBaseObject ()
	, m_nBitmaps (1)
	, m_lWidth (0)
	, m_lHeight (0)
{
	SetObjectName ("CBmpFile");
	for (int i = 0; i < MAX_BITMAPS; i++)
	{
		m_pData[i] = NULL;
		m_hBitmap[i] = NULL;
	}
}

CBmpFile::~CBmpFile(void)
{
	ReleaseData ();
}

HBITMAP	CBmpFile::GetBmpHandle (int nIndex, LPBYTE * ppBmpBuff)
{
	if (nIndex < 0 || nIndex >= m_nBitmaps)
		return NULL;

	if (ppBmpBuff != NULL)
		*ppBmpBuff = m_pData[nIndex];

	return m_hBitmap[nIndex];
}

int CBmpFile::ReadBmpFile (HDC hDC, TCHAR * pFile, int nNum)
{
	ReleaseData ();
	m_nBitmaps = nNum;

	HANDLE		hFile = NULL;
	DWORD		dwRead = 0;
	hFile = CreateFile (pFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return YY_ERR_FAILED;

	int nFileSize = GetFileSize (hFile, NULL);
	LPBYTE pFileData = new BYTE[nFileSize];
	ReadFile (hFile, pFileData, nFileSize, &dwRead, NULL);
	CloseHandle (hFile);

	LPBYTE pFileBuffer = pFileData;
	BITMAPFILEHEADER bmpFileInfo;
	memcpy (&bmpFileInfo, pFileBuffer, sizeof (BITMAPFILEHEADER));
	pFileBuffer += sizeof (BITMAPFILEHEADER);

	BITMAPINFOHEADER	bmpHeader;
	memcpy (&bmpHeader, pFileBuffer, sizeof (BITMAPINFOHEADER));

	int nBmpWidth = bmpHeader.biWidth;
	int nBmpHeight = bmpHeader.biHeight;
	if (nBmpWidth % m_nBitmaps != 0 || (bmpHeader.biBitCount != 24 && bmpHeader.biBitCount != 32))
	{
		delete []pFileData;
		return YY_ERR_FAILED;
	}

	m_lWidth = nBmpWidth / m_nBitmaps;
	m_lHeight = nBmpHeight;
	int nFileBytes = bmpHeader.biBitCount / 8;
	int nFileLine = (nFileBytes * bmpHeader.biWidth + 3) & ~3;
	int nBmpLine = m_lWidth * 4;
	int i, j;
	for (i = 0; i < m_nBitmaps; i++)
		m_hBitmap[i] = yyBmpCreate (hDC, m_lWidth, m_lHeight, &m_pData[i], 0);

	unsigned char * pRGBBmp = NULL;
	unsigned char * pRGBFil = NULL;
	for (int nHeight = 0; nHeight < m_lHeight; nHeight++)
	{
		pFileBuffer = pFileData + bmpFileInfo.bfOffBits + nFileLine * nHeight;
		for (i = 0; i < m_nBitmaps; i++)
		{
			if (nFileBytes == 4)
			{
				memcpy (m_pData[i] + (m_lHeight - nHeight - 1) * nBmpLine, pFileBuffer, nBmpLine);
			}
			else
			{
				pRGBBmp = m_pData[i] + (m_lHeight - nHeight - 1) * nBmpLine;
				pRGBFil = pFileBuffer;
				for (j = 0; j < m_lWidth; j++)
				{
					*pRGBBmp++ = *pRGBFil++;
					*pRGBBmp++ = *pRGBFil++;
					*pRGBBmp++ = *pRGBFil++;
					*pRGBBmp++;
				}
			}
			pFileBuffer += nFileBytes * m_lWidth;
		}
	}

	if (pFileData != NULL)
		delete []pFileData;

	return YY_ERR_NONE;
}

void CBmpFile::ReleaseData (void)
{
	for (int i = 0; i < MAX_BITMAPS; i++)
	{
		if (m_hBitmap[i] != NULL)
		{
			DeleteObject (m_hBitmap[i]);
			m_hBitmap[i] = NULL;
		}
	}
}

