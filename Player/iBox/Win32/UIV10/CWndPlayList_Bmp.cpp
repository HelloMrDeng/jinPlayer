/*******************************************************************************
	File:		CWndPlayList.cpp

	Contains:	Window play list view code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CWndPlayList.h"
#include "resource.h"

#include "USystemFunc.h"

#include "yyLog.h"

#define ITEM_WIDTH		240

#pragma warning (disable : 4996)

void CWndPlayList::AnimateThumbnail (void)
{
	m_nDrawStep--;
	if (m_nDrawStep == 0 && m_dwThumbTimer != 0)
	{
		KillTimer (m_hWnd, TIMER_DRAW_THUMBNAIL);
		m_dwThumbTimer = 0;
	}

	if (m_nAnimateType == ITEM_Video || m_nAnimateType == ITEM_Folder)
	{
		if (m_nAnimateType == ITEM_Video && m_pSelectedItem == NULL)
			return;

		m_rcThumb.left	= m_nDrawStep * m_rcItem.left / m_nSteps;
		m_rcThumb.top	= m_nDrawStep * m_rcItem.top / m_nSteps;
		m_rcThumb.right	= m_rcClient.right - m_nDrawStep * (m_rcClient.right - m_rcItem.right) / m_nSteps;
		m_rcThumb.bottom= m_rcClient.bottom - m_nDrawStep * (m_rcClient.bottom - m_rcItem.bottom) / m_nSteps;
		
		BITMAP				bmpInfo;
		HBITMAP				hBmpAnimate = NULL;
		YYINFO_Thumbnail *	pThumb = NULL;
		if (m_pSelectedItem != NULL)
			pThumb = &m_pSelectedItem->sThumbInfo;

		int nBmpW = m_rcClient.right & ~1;
		int nBmpH = m_rcClient.bottom;
		if (m_nAnimateType == ITEM_Video)
		{
			if (pThumb != NULL)
			{
				nBmpW = pThumb->nThumbWidth;
				nBmpH = pThumb->nThumbHeight;
			}
			else
			{
				nBmpW = m_rcClient.right / 2;
				nBmpH = m_rcClient.bottom / 2;
			}
		}

		if (m_nAnimateType == ITEM_Folder || m_pSelectedItem->sThumbInfo.nPos > 200)
		{
			if (m_hThumbBmp != NULL)
			{
				GetObject (m_hThumbBmp, sizeof (BITMAP), &bmpInfo);
				if (bmpInfo.bmWidth != nBmpW || bmpInfo.bmHeight != nBmpH)
				{
					DeleteObject (m_hThumbBmp);
					m_hThumbBmp = NULL;
				}
			}

			if (m_hThumbBmp == NULL)
				m_hThumbBmp = CreateBMP (nBmpW, nBmpH, &m_pThumbBuff);

			if (m_nDrawStep == m_nSteps - 1)
			{
				if (m_nAnimateType == ITEM_Video)
					memcpy (m_pThumbBuff, pThumb->pBmpBuff, nBmpW * nBmpH * 4);
				else
					memcpy (m_pThumbBuff, m_hBmpBGBuff, nBmpW * nBmpH  * 4);
			}

			hBmpAnimate = m_hThumbBmp;
		}
		else
		{
			hBmpAnimate = m_pSelectedItem->hThumb;
		}

		if (m_nAnimateType == ITEM_Video && m_pSelectedItem->sThumbInfo.nPos > 200 && m_nDrawStep < m_nSteps / 3)
		{
			unsigned char *	pRGB = NULL;
			for (int i = 0; i < nBmpH; i++)
			{
				pRGB = m_pThumbBuff + i * nBmpW * 4;
				for (int j = 0; j < nBmpW; j++)
				{
					*(pRGB +j * 4) = *(pRGB + j * 4) * (m_nDrawStep + m_nSteps / 2) / m_nSteps;
					*(pRGB +j * 4 + 1) = *(pRGB + j * 4 + 1) * (m_nDrawStep + m_nSteps / 2) / m_nSteps;
					*(pRGB +j * 4 + 2) = *(pRGB + j * 4 + 2) * (m_nDrawStep + m_nSteps / 2) / m_nSteps;
				}
			}
		}				
						
		HDC hdc = GetDC (m_hWnd);
		HBITMAP hBmp = (HBITMAP)SelectObject (m_hDCBmp, hBmpAnimate);
		StretchBlt (hdc, m_rcThumb.left, m_rcThumb.top, m_rcThumb.right - m_rcThumb.left, m_rcThumb.bottom - m_rcThumb.top,
					m_hDCBmp, 0, 0, nBmpW, nBmpH, SRCCOPY);
		SelectObject (m_hDCBmp, hBmp);
		ReleaseDC (m_hWnd, hdc);

		if (m_nAnimateType == ITEM_Video && m_nDrawStep == 0)
			PostMessage (m_hParent, WM_YYLIST_PLAYBACK, (WPARAM)m_szPlayFile, NULL);
	}
	else if (m_nAnimateType == ITEM_Audio)
	{
		KillTimer (m_hWnd, TIMER_DRAW_THUMBNAIL);
		m_dwThumbTimer = 0;
		m_nDrawStep = 0;
		PostMessage (m_hParent, WM_YYLIST_PLAYBACK, (WPARAM)m_szPlayFile, NULL);
	}
	else if (m_nAnimateType == ITEM_Home)
	{
		if (m_hThumbBmp == NULL)
		{
			int nBmpW = m_rcClient.right;
			int nBmpH = m_rcClient.bottom;
			m_hThumbBmp = CreateBMP (nBmpW, nBmpH, &m_pThumbBuff);
			memcpy (m_pThumbBuff, m_hBmpBGBuff, nBmpW * nBmpH  * 4);
		}

		m_rcThumb.left	= m_rcClient.left + (m_nSteps - m_nDrawStep) * m_rcItem.left / m_nSteps;
		m_rcThumb.top	= m_rcClient.top + (m_nSteps - m_nDrawStep) * m_rcItem.top / m_nSteps;
		m_rcThumb.right	= m_rcClient.right - (m_nSteps - m_nDrawStep) * (m_rcClient.right - m_rcItem.right) / m_nSteps;
		m_rcThumb.bottom= m_rcClient.bottom - (m_nSteps - m_nDrawStep) * (m_rcClient.bottom - m_rcItem.bottom) / m_nSteps;

		HBITMAP hBmpThumb = (HBITMAP)SelectObject (m_hDCBmp, m_hThumbBmp);
		HBITMAP hBmpItem = (HBITMAP)SelectObject (m_hDCItem, m_hBmpBG);
		BitBlt (m_hDCBmp, 0, 0, m_rcClient.right, m_rcClient.bottom, m_hDCItem, 0, m_nBmpYPos, SRCCOPY);

		SelectObject (m_hDCItem, m_hVideoBmp);
		StretchBlt (m_hDCBmp, m_rcThumb.left, m_rcThumb.top, m_rcThumb.right - m_rcThumb.left, m_rcThumb.bottom - m_rcThumb.top,
					m_hDCItem, 0, 0, m_rcClient.right, m_rcClient.bottom, SRCCOPY);
		SelectObject (m_hDCItem, hBmpItem);

		
		HDC hdc = GetDC (m_hWnd);
		BitBlt (hdc, 0, 0, m_rcClient.right, m_rcClient.bottom, m_hDCBmp, 0, 0, SRCCOPY);
		ReleaseDC (m_hWnd, hdc);

		SelectObject (m_hDCBmp, hBmpThumb);
	}

	if (m_nDrawStep == 0)
		InvalidateRect (m_hWnd, NULL, FALSE);
}

void CWndPlayList::DrawSelectedItem (HDC hdc)
{
	if (m_pSelectedItem == NULL)
		return;

	int nIndex = -1;
	POSITION pos = m_lstItem.GetHeadPosition ();
	MEDIA_Item * pItem = m_lstItem.GetNext (pos);
	while (pItem != NULL)
	{
		nIndex++;
		if (pItem == m_pSelectedItem)
			break;
		pItem = m_lstItem.GetNext (pos);
	}
	if (nIndex < 0)
		return;

	int nX = nIndex % m_nCols * m_nItemWidth;
	int nY = nIndex / m_nCols * m_nItemHeight;

	if (nY + m_nItemHeight < m_nBmpYPos)
		return;
	else if (nY > m_nBmpYPos + m_rcClient.bottom)
		return;

	if (m_hPenLine == NULL)
		m_hPenLine = ::CreatePen (PS_SOLID, 4, RGB (0, 0, 220));
	SelectObject (hdc, m_hPenLine);

	if (nY < m_nBmpYPos)
	{
		int nH = m_nIconHeight - (m_nBmpYPos - nY);

		nX = nX + (m_nItemWidth - m_nIconWidth) / 2;
		MoveToEx (hdc, nX, 0, NULL);
		LineTo (hdc, nX, nH);
		LineTo (hdc, nX + m_nIconWidth, nH);
		LineTo (hdc, nX + m_nIconWidth, 0);
	}
	else if (nY >= m_nBmpYPos && nY < m_nBmpYPos + m_rcClient.bottom)
	{
		int nH = nY - m_nBmpYPos + m_nIconOffsetY;

		nX = nX + (m_nItemWidth - m_nIconWidth) / 2;
		MoveToEx (hdc, nX, nH, NULL);
		LineTo (hdc, nX, nH + m_nIconHeight);
		LineTo (hdc, nX + m_nIconWidth, nH + m_nIconHeight);
		LineTo (hdc, nX + m_nIconWidth, nH);
		LineTo (hdc, nX, nH);
	}
	else
	{
		int nH = nY - m_nBmpYPos + m_nIconOffsetY;

		nX = nX + (m_nItemWidth - m_nIconWidth) / 2;
		MoveToEx (hdc, nX, m_rcClient.bottom, NULL);
		LineTo (hdc, nX, nH);
		LineTo (hdc, nX + m_nIconWidth, nH);
		LineTo (hdc, nX + m_nIconWidth, m_rcClient.bottom);
	}
}

MEDIA_Item * CWndPlayList::GetSelectItem (int nX, int nY)
{
	int nW = (m_nItemWidth - m_nIconWidth) / 2;
	for (int i = 0; i < m_nCols; i++)
	{
		if (nX > m_nItemWidth * i && nX < m_nItemWidth * i + nW)
			return NULL;
		else if (nX > m_nItemWidth * i + nW + m_nIconWidth && nX < m_nItemWidth * (i + 1))
			return NULL;
	}

	MEDIA_Item * pItem = NULL;

	int nLine = (m_nBmpYPos + nY) / m_nItemHeight;
	int nCol = nX / m_nItemWidth;
	int nIndex = nLine * m_nCols + nCol;
	
	if (nIndex == 0)
	{
		pItem = m_lstItem.GetHead ();
	}
	else
	{
		POSITION pos = m_lstItem.GetHeadPosition ();
		pItem = m_lstItem.GetNext (pos);
		while (pItem != NULL)
		{
			if (nIndex-- <= 0)
				break;
			pItem = m_lstItem.GetNext (pos);
		}
	}

	return pItem;
}

bool CWndPlayList::CreateBitmapBG (void)
{
	GetClientRect (m_hWnd, &m_rcClient);
	m_rcClient.right = m_rcClient.right & ~1;

	if (m_rcClient.right <= ITEM_WIDTH * 2 || m_rcClient.bottom <= ITEM_WIDTH)
		return false;

	if (m_hBmpBG != NULL)
	{
		SelectObject (m_hDCBmp, m_hBmpOld);
		DeleteObject (m_hBmpBG);
		m_hBmpBG = NULL;
	}

	if (m_hDCBmp == NULL)
	{
		HDC hDCWnd = GetDC (m_hWnd);
		m_hDCBmp = CreateCompatibleDC (hDCWnd);
		m_hDCItem = CreateCompatibleDC (hDCWnd);
		ReleaseDC (m_hWnd, hDCWnd);
	}

	m_nCols = m_rcClient.right / ITEM_WIDTH + 1;
	m_nItemWidth = (m_rcClient.right / m_nCols) & ~3;
	m_nItemHeight = m_nItemWidth * 3 / 4;
	m_nIconWidth = (m_nItemWidth - 40) & ~3;
	m_nIconHeight = (m_nIconWidth * 3 / 4 - 20) & ~1;

	int nLines = 0;
	if (m_lstItem.GetCount () > 0)
		nLines = (m_lstItem.GetCount () + m_nCols - 1) / m_nCols;
	m_nBmpHeight = nLines * m_nItemHeight;
	if (m_nBmpHeight < m_rcClient.bottom)
		m_nBmpHeight = m_rcClient.bottom;
	m_hBmpBG = CreateBMP (m_rcClient.right, m_nBmpHeight, &m_hBmpBGBuff);
	if (m_hBmpBG == NULL)
		return false;
	m_hBmpOld = (HBITMAP)SelectObject (m_hDCBmp, m_hBmpBG);

	RECT rcBmp;
	SetRect (&rcBmp, 0, 0, m_rcClient.right, m_nBmpHeight);
	HBRUSH hBrush = CreateSolidBrush (RGB (255, 255, 255));
	FillRect (m_hDCBmp, &rcBmp, hBrush);
	DeleteObject (hBrush);

	if (m_hBmpHome == NULL)
		m_hBmpHome = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_HOME));
	if (m_hBmpFolder == NULL)
		m_hBmpFolder = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_FOLDER));
	if (m_hBmpVideo == NULL)
		m_hBmpVideo = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_VIDEO));
	if (m_hBmpAudio == NULL)
		m_hBmpAudio = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_AUDIO));
	if (m_hBmpNewFile == NULL)
		m_hBmpNewFile = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_VIDEO_NEW));
	if (m_hBmpNewFolder == NULL)
		m_hBmpNewFolder = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_FOLDER_NEW));

	int	 x = 0, y = 0, nIndex = 0;
	RECT rcText;

	HBITMAP hBmpIcon = NULL;
	BITMAP	bmpInfo;
	POSITION pos = m_lstItem.GetHeadPosition ();
	MEDIA_Item * pItem = m_lstItem.GetNext (pos);
	while (pItem != NULL)
	{	
		if (pItem->nType == ITEM_Home)
			hBmpIcon = m_hBmpHome;
		else if (pItem->nType == ITEM_Folder)
			hBmpIcon = m_hBmpFolder;
		else if (pItem->nType == ITEM_Video)
		{
			if (pItem->hThumb == NULL)
				hBmpIcon = m_hBmpVideo;
			else
				hBmpIcon = pItem->hThumb;
		}
		else if (pItem->nType == ITEM_Audio)
		{
			if (pItem->hThumb == NULL)
				hBmpIcon = m_hBmpAudio;
			else
				hBmpIcon = pItem->hThumb;
		}
		else if (pItem->nType == ITEM_NewFolder)
			hBmpIcon = m_hBmpNewFolder;
		else if (pItem->nType == ITEM_NewFile)
			hBmpIcon = m_hBmpNewFile;
		SelectObject (m_hDCItem, hBmpIcon);
		GetObject (hBmpIcon, sizeof (BITMAP), &bmpInfo);
		x = (nIndex % m_nCols) * m_nItemWidth + (m_nItemWidth - m_nIconWidth) / 2;
		y = (nIndex / m_nCols) * m_nItemHeight + m_nIconOffsetY;
		StretchBlt (m_hDCBmp, x, y, m_nIconWidth, m_nIconHeight, 
					m_hDCItem, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);

		int	 nStartTxt = 0;
		SetRect (&rcText, (nIndex % m_nCols) * m_nItemWidth + 8, y + m_nIconHeight + m_nIconOffsetY, (nIndex % m_nCols + 1) * m_nItemWidth - 16, y + m_nItemHeight - m_nIconOffsetY * 2);
//		if (pItem->nType == ITEM_Home)
		{
			SIZE szTxt;
			GetTextExtentPoint (m_hDCBmp, pItem->pName + nStartTxt, _tcslen (pItem->pName + nStartTxt), &szTxt);
			while (szTxt.cx > (rcText.right - rcText.left))
			{
				nStartTxt++;
				GetTextExtentPoint (m_hDCBmp, pItem->pName + nStartTxt, _tcslen (pItem->pName + nStartTxt), &szTxt);
			}
		}
		
		DrawText (m_hDCBmp, pItem->pName + nStartTxt, _tcslen (pItem->pName + nStartTxt), &rcText, DT_CENTER | DT_VCENTER);

		nIndex++;
		pItem = m_lstItem.GetNext (pos);
	}
	m_nBmpYPos = 0;
	
	PostMessage (m_hParent, WM_YYLIST_UPDATE, (WPARAM)(m_nBmpHeight - m_rcClient.bottom), NULL);
	PostMessage (m_hParent, WM_YYLIST_MOVING, (WPARAM)m_nBmpYPos, NULL);

	if (m_hThread == NULL)
	{
		m_bStop = false;
		m_bPause = false;
		m_bWorking = false;
		DWORD dwID = 0;
		m_hThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) GetThumbProc, this, 0, &dwID);
	}

	return true;
}

void CWndPlayList::ShowBitmapBG (void)
{
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	if (m_nBmpHeight > rcView.bottom + m_nItemHeight / 4)
	{
		GetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo);
		m_sbInfo.nPos = 0;
		m_sbInfo.nMin = 0;
		m_sbInfo.nMax = m_nBmpHeight - rcView.bottom;
		SetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo, TRUE);
		EnableScrollBar (m_hWnd, SB_VERT, ESB_ENABLE_BOTH);
	}
	else
	{
		EnableScrollBar (m_hWnd, SB_VERT, ESB_DISABLE_BOTH);
	}

	if (m_nAnimateType == ITEM_Unknown)
	{
		InvalidateRect (m_hWnd, NULL, FALSE);
	}
	else
	{
		if (m_nAnimateType == ITEM_Home)
		{
			m_nSteps = 10;
			//m_nSteps = 80;
			m_nDrawStep = m_nSteps;
			m_dwThumbTimer = SetTimer (m_hWnd, TIMER_DRAW_THUMBNAIL, m_nDrawStepTime, NULL);
		}
		else
		{
			m_nSteps = 10;
			m_nDrawStep = m_nSteps;
			m_dwThumbTimer = SetTimer (m_hWnd, TIMER_DRAW_THUMBNAIL, m_nDrawStepTime, NULL);
		//	InvalidateRect (m_hWnd, NULL, FALSE);
		}
	}
}

void CWndPlayList::ReleaseDCBmp (void)
{
	if (m_hDCBmp != NULL)
		DeleteDC (m_hDCBmp);
	m_hDCBmp = NULL;
	if (m_hDCItem != NULL)
		DeleteDC (m_hDCItem);
	m_hDCItem = NULL;

	if (m_hBmpHome != NULL)
		DeleteObject (m_hBmpHome);
	m_hBmpHome = NULL;
	if (m_hBmpFolder != NULL)
		DeleteObject (m_hBmpFolder);
	m_hBmpFolder = NULL;
	if (m_hBmpVideo != NULL)
		DeleteObject (m_hBmpVideo);
	m_hBmpVideo = NULL;
	if (m_hBmpAudio != NULL)
		DeleteObject (m_hBmpAudio);
	m_hBmpAudio = NULL;
	if (m_hBmpNewFolder != NULL)
		DeleteObject (m_hBmpNewFolder);
	m_hBmpNewFolder = NULL;
	if (m_hBmpNewFile != NULL)
		DeleteObject (m_hBmpNewFile);
	m_hBmpNewFile = NULL;

	if (m_hPenLine != NULL)
		DeleteObject (m_hPenLine);
	m_hPenLine = NULL;
}

HBITMAP CWndPlayList::CreateBMP (int nW, int nH, LPBYTE * pBmpBuff)
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

	if (m_hDCBmp == NULL)
	{
		HDC hDC = GetDC (NULL);
		m_hDCBmp = CreateCompatibleDC (hDC);
		ReleaseDC (NULL, hDC);
	}

	HBITMAP hBitmap = CreateDIBSection(m_hDCBmp , pBmpInfo , DIB_RGB_COLORS , (void **)pBmpBuff, NULL , 0);

	delete pBmpInfo;

	return hBitmap;
}
	
int CWndPlayList::CopyLastVideo (HWND hWndVideo)
{
	if (m_bFodlerChanged)
	{
		m_bFodlerChanged = false;
		m_nAnimateType = ITEM_Unknown;

		TCHAR * pRoot = _tcschr (m_szFolder, _T('\\'));
		memset (m_szRoot, 0, sizeof (m_szRoot));
		if (pRoot == NULL)
			_tcscpy (m_szRoot, m_szFolder);
		else
			_tcsncpy (m_szRoot, m_szFolder, pRoot - m_szFolder);
		FillMediaItems (m_szFolder);
		return 0;
	}

	BITMAP bmpInfo;
	if (m_hThumbBmp != NULL)
	{
		GetObject (m_hThumbBmp, sizeof (BITMAP), &bmpInfo);
		if (bmpInfo.bmWidth != m_rcClient.right || bmpInfo.bmHeight != m_rcClient.bottom)
		{
			DeleteObject (m_hThumbBmp);
			m_hThumbBmp = NULL;
		}
	}
	if (m_hThumbBmp == NULL)
		m_hThumbBmp = CreateBMP (m_rcClient.right, m_rcClient.bottom, &m_pThumbBuff);

	if (m_hVideoBmp == NULL)
		m_hVideoBmp = CreateBMP (m_rcClient.right, m_rcClient.bottom, &m_pVideoBmpBuff);

	if (hWndVideo != NULL)
	{
		HDC hdc = GetDC (hWndVideo);
		HBITMAP hOld = (HBITMAP)SelectObject (m_hDCBmp, m_hVideoBmp);
		BitBlt (m_hDCBmp, 0, 0, m_rcClient.right, m_rcClient.bottom, hdc, 0, 0, SRCCOPY);
		ReleaseDC (hWndVideo, hdc);
	}
	else
	{
		m_pMedia->Pause ();

		YY_BUFFER * pVideoBuff = NULL;
		m_pMedia->GetParam (YYPLAY_PID_VideoData, &pVideoBuff);
		if (pVideoBuff == NULL)
			return -1;

		YY_BUFFER_CONVERT buffConv;
		buffConv.pSource = pVideoBuff;
		buffConv.pTarget = new YY_BUFFER ();
		memset (buffConv.pTarget, 0, sizeof (YY_BUFFER));

		YY_VIDEO_BUFF buffVideo;
		memset (&buffVideo, 0, sizeof (YY_VIDEO_BUFF));
		buffVideo.nType = YY_VDT_RGBA;
		buffVideo.nWidth = m_rcClient.right;
		buffVideo.nHeight = m_rcClient.bottom;
		buffVideo.pBuff[0] = m_pVideoBmpBuff;
		buffVideo.nStride[0] = buffVideo.nWidth * 4;

		buffConv.pTarget->nType = YY_MEDIA_Video;
		buffConv.pTarget->pBuff = (unsigned char *)&buffVideo;
		buffConv.pTarget->uFlag = YYBUFF_TYPE_VIDEO;
		buffConv.pTarget->uSize = sizeof (YY_VIDEO_BUFF);

		m_pMedia->GetParam (YYPLAY_PID_ConvertData, &buffConv);

		delete buffConv.pTarget;
	}


	return 0;
}

void CWndPlayList::SetPlayingFile (TCHAR * pFile)
{
	if (pFile == NULL || m_lvType != LIST_VIEW_Folder)
		return;
	if (_tcsstr (pFile, _T("://")) != NULL)
	{
		m_bFodlerChanged = true;
		return;
	}

	TCHAR * pFileName = _tcsrchr (pFile, _T('\\'));
	if (pFileName == NULL)
		return;
	pFileName++;

	if (m_pSelectedItem != NULL)
	{
		if (!_tcscmp (pFileName, m_pSelectedItem->pName))
			return;
	}

	_tcscpy (m_szPlayFile, pFile);

	POSITION pos = m_lstItem.GetHeadPosition ();
	MEDIA_Item * pItem = m_lstItem.GetNext (pos);
	while (pItem != NULL)
	{
		if (pItem->nType == ITEM_Video || pItem->nType == ITEM_Audio)
		{
			if (!_tcscmp (pFileName, pItem->pName))
				break;
		}
		pItem = m_lstItem.GetNext (pos);
	}
	if (pItem == NULL)
	{
		memset (m_szFolder, 0, sizeof (m_szFolder));
		_tcsncpy (m_szFolder, pFile, pFileName - pFile - 1);
		m_bFodlerChanged = true;
		return;
	}

	int nX = (pItem->nIndex % m_nCols) * m_nItemWidth + (m_nItemWidth - m_nIconWidth) / 2;
	int nY = (pItem->nIndex / m_nCols - m_nBmpYPos / m_nItemHeight) * m_nItemHeight - m_nBmpYPos % m_nItemHeight + m_nIconOffsetY;
	if (nY < 0)
		nY = 0;
	SetRect (&m_rcItem, nX, nY, nX + m_nIconWidth, nY + m_nIconHeight);

	m_pSelectedItem = pItem;
}

MEDIA_Item * CWndPlayList::GetSelectedItem (void)
{
	if (m_pSelectedItem == NULL)
		return NULL;

	if (m_pSelectedItem->nType != ITEM_Video && m_pSelectedItem->nType != ITEM_Audio)
		return NULL;

	return m_pSelectedItem;
}

int __cdecl CWndPlayList::compare_filename(const void *arg1, const void *arg2)
{
	MEDIA_Item * pItem1 = *(MEDIA_Item **)arg1;
	MEDIA_Item * pItem2 = *(MEDIA_Item **)arg2;
	return _tcsicmp (pItem1->pName, pItem2->pName);
}

void CWndPlayList::Start (void) 
{
	m_bPause = false;

	if (m_rcItem.right == 0 || m_rcItem.bottom == 0)
		return;

	m_nSteps = 10;
	m_nDrawStep = m_nSteps;
#ifdef _CPU_MB2531
	m_dwThumbTimer = SetTimer (m_hWnd, TIMER_HIDE_LASTVIDEO, 5, NULL);
#else
	m_dwThumbTimer = SetTimer (m_hWnd, TIMER_HIDE_LASTVIDEO, 25, NULL);
#endif // _CPU_MB2531
}

void CWndPlayList::Pause (void) 
{
	m_bPause = true;
	if (m_pMedia != NULL)
		m_pMedia->SetParam (YYPLAY_PID_Cancel_GetThumb, NULL);

	while (m_bWorking)
	{
		yySleep (10000);
		if (m_bStop)
			break;
	}
}

int CWndPlayList::GetThumbProc (void * pParam)
{
	SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_BELOW_NORMAL);
	CWndPlayList * pList = (CWndPlayList *)pParam;

	MEDIA_Item *	pItem = NULL;

	POSITION pos = pList->m_lstItem.GetHeadPosition ();
	while (!pList->m_bStop)
	{
		pList->m_bWorking = false;

		if (pList->m_bPause)
		{
			yySleep (5000);
			continue;
		}

		if (pList->m_bStop)
			break;

		pItem = pList->m_lstItem.GetNext (pos);
		if (pItem == NULL)
		{
			if (pList->m_lvType == LIST_VIEW_Favor)
			{
				pos = pList->m_lstItem.GetHeadPosition ();
				continue;
			}
			else
				break;
		}
		else 
		{
			if (pItem->hThumb != NULL || (pItem->nType != ITEM_Video && pItem->nType != ITEM_Audio))
			{
				yySleep (5000);
				continue;
			}
		}

		pList->GetThumbLoop (pItem);
		yySleep (2000);
	}

	pList->m_hThread = NULL;

	return 0;
}

int CWndPlayList::GetThumbLoop (MEDIA_Item * pItem)
{
	m_bWorking = true;

	TCHAR szFile[1024];
	if (pItem->pPath == NULL)
	{
		_tcscpy (szFile, m_szFolder);
		_tcscat (szFile, _T("\\"));
		_tcscat (szFile, pItem->pName);
	}
	else
	{
		_tcscpy (szFile, pItem->pPath);
	}

//	pItem->sThumbInfo.nInfoType = YYINFO_Get_MediaInfo;
	pItem->sThumbInfo.nThumbWidth = m_nIconWidth * 2;
	pItem->sThumbInfo.nThumbHeight = m_nIconHeight * 2;
//	pItem->sThumbInfo.nThumbWidth = (m_rcClient.right / 2) & ~3;
//	pItem->sThumbInfo.nThumbHeight = (m_rcClient.bottom / 2) & ~1;
	pItem->sThumbInfo.bKeepAspectRatio = true;
	pItem->sThumbInfo.nBGColor = RGB (0, 0, 0);
	pItem->sThumbInfo.nPos = 0;

	int nStartTime = yyGetSysTime ();

#if 1
	pItem->sThumbInfo.nInfoType |= YYINFO_OPEN_ExtSource;
	pItem->sThumbInfo.nInfoType |= YYINFO_Get_NoBlack;
	if (m_pExtSrc == NULL)
		m_pExtSrc = new CExtSource (m_hInst);
	TCHAR * pSource = (TCHAR *)m_pExtSrc->GetExtData (YY_EXTDATA_Mux, szFile);
	if (pSource == NULL)
		return 0;
	pItem->hThumb = (HBITMAP)m_pMedia->GetThumb ((const TCHAR *)pSource, &pItem->sThumbInfo);
#else
	pItem->hThumb = (HBITMAP)m_pMedia->GetThumb (szFile, &pItem->sThumbInfo);
#endif // 0

	int nUsedTime = yyGetSysTime () - nStartTime;
	if (pItem->hThumb != NULL)
	{
		if (nUsedTime < 600 && IsBlackThumb (&pItem->sThumbInfo))
		{
			DeleteObject (pItem->hThumb);
			pItem->sThumbInfo.pBmpBuff = NULL;
			pItem->sThumbInfo.hThumbnail = NULL;
			pItem->sThumbInfo.nPos = 10000;
			pItem->hThumb = (HBITMAP)m_pMedia->GetThumb ((const TCHAR *)m_pExtSrc->GetExtData (YY_EXTDATA_Mux, szFile), &pItem->sThumbInfo);
			//pItem->hThumb = (HBITMAP)m_pMedia->GetThumb (szFile, &pItem->sThumbInfo);
		}

		if (pItem->hThumb == NULL)
			return 0;

		int x = (pItem->nIndex % m_nCols) * m_nItemWidth + (m_nItemWidth - m_nIconWidth) / 2;
		int y = (pItem->nIndex / m_nCols) * m_nItemHeight + m_nIconOffsetY;
		HBITMAP hOldBmp = (HBITMAP)SelectObject (m_hDCItem, pItem->hThumb);
		StretchBlt (m_hDCBmp, x, y, m_nIconWidth, m_nIconHeight, 
					m_hDCItem, 0, 0, pItem->sThumbInfo.nThumbWidth, pItem->sThumbInfo.nThumbHeight, SRCCOPY);
		SelectObject (m_hDCItem, hOldBmp);

		if (y >= m_nBmpYPos && y < m_nBmpYPos + m_rcClient.bottom)
		{
			if (m_dwThumbTimer == 0)
				InvalidateRect (m_hWnd, NULL, FALSE);
		}
	}

	return 0;
}

bool CWndPlayList::IsBlackThumb (YYINFO_Thumbnail * pThumb)
{
	if (pThumb == NULL)
		return true;

	unsigned char *	pRGB = NULL;
	int				nBlight = 0;
	int				nRGB = 50;

	for (int i = pThumb->nThumbHeight / 2; i < pThumb->nThumbHeight / 2 + 20; i+=2)
	{
		pRGB = pThumb->pBmpBuff + i * pThumb->nThumbWidth * 4;
		for (int j = 0; j < pThumb->nThumbWidth; j+=4)
		{
			if (*(pRGB + j * 4)> nRGB || *(pRGB + j * 4 + 1)> nRGB || *(pRGB + j * 4 + 2)> nRGB)
			{
				nBlight++;
				if (nBlight > 200)
					return false;
			}
		}
	}

	return true;
}

