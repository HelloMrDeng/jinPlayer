/*******************************************************************************
	File:		UBitmapFunc.cpp

	Contains:	The utility for bitmap implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-27		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#include "windows.h"

#include "UBitmapFunc.h"
#include "UStringFunc.h"
#include "UFileFunc.h"

#ifdef _OS_WIN32
#pragma warning (disable : 4996)
#endif // _OS_WIN32

HBITMAP yyBmpCreate (HDC hDC, int nW, int nH, LPBYTE * ppBmpBuff, int nColor)
{
	int nBmpSize = sizeof(BITMAPINFOHEADER);
	BITMAPINFO * pBmpInfo = new BITMAPINFO ();
	pBmpInfo->bmiHeader.biSize			= nBmpSize;
	pBmpInfo->bmiHeader.biWidth			= nW;
	pBmpInfo->bmiHeader.biHeight		= -nH;
	pBmpInfo->bmiHeader.biBitCount		= (WORD)32;
	pBmpInfo->bmiHeader.biCompression	= BI_RGB;
	pBmpInfo->bmiHeader.biXPelsPerMeter	= 0;
	pBmpInfo->bmiHeader.biYPelsPerMeter	= 0;
	pBmpInfo->bmiHeader.biPlanes		= 1;

	int nStride = ((pBmpInfo->bmiHeader.biWidth * pBmpInfo->bmiHeader.biBitCount / 8) + 3) & ~3;
	pBmpInfo->bmiHeader.biSizeImage	= nStride * nH;

	HBITMAP		hBitmap = NULL;
	LPBYTE		pBuffer = NULL;
	LPBYTE *	ppBuffer = &pBuffer;
	if (ppBmpBuff != NULL)
		ppBuffer = ppBmpBuff;

	if (hDC == NULL)
	{
		HDC hDispDC = GetDC (NULL);
		hBitmap = CreateDIBSection(hDispDC , pBmpInfo , DIB_RGB_COLORS , (void **)ppBuffer, NULL , 0);
		ReleaseDC (NULL, hDispDC);
	}
	else
	{
		hBitmap = CreateDIBSection(hDC , pBmpInfo , DIB_RGB_COLORS , (void **)ppBuffer, NULL , 0);
	}

	if (*ppBuffer != NULL)
	{
		char * pRGB = (char *)&nColor;
		if (pRGB[0] == pRGB[1] && pRGB[1] == pRGB[2])
			memset (*ppBuffer, pRGB[0], pBmpInfo->bmiHeader.biSizeImage);
		else
		{
			int * pRGBBuff = (int *)ppBuffer;
			for (int i = 0; i < nH; i++)
			{
				for (int j = 0; j < nW; j++)
					*pRGBBuff++ = nColor;
			}
		}
	}
	delete pBmpInfo;

	return hBitmap;
}
	
HBITMAP yyBmpClone (HBITMAP hBmpSrc, RECT * pRect, LPBYTE * ppBmpBuff)
{
	if (hBmpSrc == NULL)
		return NULL;

	BITMAP bmpInfo;
	GetObject (hBmpSrc, sizeof (BITMAP), &bmpInfo);
	if (pRect != NULL)
	{
		bmpInfo.bmWidth = pRect->right - pRect->left;
		bmpInfo.bmHeight = pRect->bottom - pRect->top;
	}
	HDC hDC = GetDC (NULL);
	HBITMAP hBmpNew = yyBmpCreate (NULL, bmpInfo.bmWidth, bmpInfo.bmHeight, ppBmpBuff, 0);
	if (hBmpNew == NULL)
	{
		ReleaseDC (NULL, hDC);
		return NULL;
	}
	HDC hDCSrc = CreateCompatibleDC (hDC);
	HDC hDCBmp = CreateCompatibleDC (hDC);
	ReleaseDC (NULL, hDC);

	HBITMAP hOldSrc = (HBITMAP) SelectObject (hDCSrc, hBmpSrc);
	HBITMAP hOldBmp = (HBITMAP) SelectObject (hDCBmp, hBmpNew);
	if (pRect == NULL)
		BitBlt (hDCBmp, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, hDCSrc, 0, 0, SRCCOPY);
	else
		BitBlt (hDCBmp, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, hDCSrc, pRect->left, pRect->top, SRCCOPY);
	SelectObject (hDCBmp, hOldBmp);
	SelectObject (hDCSrc, hOldSrc);
	DeleteDC (hDCBmp);
	DeleteDC (hDCSrc);
	return hBmpNew;
}

void yyBmpCheck (HWND hWnd, HBITMAP hBmp)
{
	HDC hDCWin = GetDC (hWnd);
	RECT rcWin;
	GetClientRect (hWnd, &rcWin);
	HDC hDCBmp = CreateCompatibleDC (hDCWin);
	HBITMAP hOld = (HBITMAP)SelectObject (hDCBmp, hBmp);
	BitBlt (hDCWin, 200, 300, rcWin.right, rcWin.bottom, hDCBmp, 0, 0, SRCCOPY);
	SelectObject (hDCBmp, hOld);
	DeleteDC (hDCBmp);
	ReleaseDC (hWnd, hDCWin);
}

bool yyBmpSave (HBITMAP hBmp, LPBYTE pBuff, TCHAR * pFile)
{
	if (hBmp == NULL || pBuff == NULL || pFile == NULL)
		return false;

	BITMAP bmpInfo;
	GetObject (hBmp, sizeof (BITMAP), &bmpInfo);
	BITMAPINFOHEADER	bmpHeader;
	memset (&bmpHeader, 0, sizeof (bmpHeader));
	bmpHeader.biSize = sizeof (bmpHeader);
	bmpHeader.biWidth = bmpInfo.bmWidth; 
	bmpHeader.biHeight = -bmpInfo.bmHeight; 
	bmpHeader.biPlanes = bmpInfo.bmPlanes; 
	bmpHeader.biBitCount = bmpInfo.bmBitsPixel; 
	bmpHeader.biSizeImage = bmpInfo.bmWidthBytes * bmpInfo.bmHeight; 
	bmpHeader.biClrImportant = 1; 

	BITMAPFILEHEADER bmpFileInfo;
	memset (&bmpFileInfo, 0, sizeof (bmpFileInfo));
	bmpFileInfo.bfType = 'MB';
	bmpFileInfo.bfSize = sizeof (bmpFileInfo) + sizeof (bmpHeader) + bmpHeader.biSizeImage;
	bmpFileInfo.bfOffBits = sizeof (bmpFileInfo) + sizeof (bmpHeader);

	yyFile hFile = yyFileOpen (pFile, YYFILE_WRITE);
	if (hFile == NULL)
		return false;
	yyFileWrite (hFile, (unsigned char *)&bmpFileInfo, sizeof (bmpFileInfo));
	yyFileWrite (hFile, (unsigned char *)&bmpHeader, sizeof (bmpHeader));
	yyFileWrite (hFile, pBuff, bmpHeader.biSizeImage);
	yyFileClose (hFile);

	return true;
}
