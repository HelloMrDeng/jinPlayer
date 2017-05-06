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

#include "CRegMng.h"
#include "USystemFunc.h"

#include "yyLog.h"

#pragma warning (disable : 4996)

#define	TIMER_FILL_MEDIA_ITEMS		1001
#define	TIMER_SHOW_ITEM_INFO		1002
#define TIMER_DRAW_THUMBNAIL		1003
#define TIMER_HIDE_LASTVIDEO		1004

CWndPlayList::CWndPlayList(HINSTANCE hInst, COMBoxMng * pMedia)
	: CWndBase (hInst)
	, m_pMedia (pMedia)
	, m_nCols (4)
	, m_nItemWidth (0)
	, m_nItemHeight (0)
	, m_nIconWidth (120)
	, m_nIconHeight (80)
	, m_nIconOffsetY (8)
	, m_hBmpBG (NULL)
	, m_hBmpBGBuff (NULL)
	, m_hBmpOld (NULL)
	, m_hBmpHome (NULL)
	, m_hBmpFolder (NULL)
	, m_hBmpVideo (NULL)
	, m_hBmpAudio (NULL)
	, m_hDCBmp (NULL)
	, m_hDCItem (NULL)
	, m_hPenLine (NULL)
	, m_nBmpHeight (0)
	, m_nBmpYPos (0)
	, m_nMoveYPos (0)
	, m_bMoving (false)
	, m_bMoved (false)
	, m_pSelectedItem (NULL)
	, m_hThumbBmp (NULL)
	, m_pThumbBuff (NULL)
	, m_nSteps (10)
	, m_nDrawStep (0)
	, m_dwThumbTimer (0)
	, m_nAnimateType (ITEM_Unknown)
	, m_hVideoBmp (NULL)
	, m_pVideoBmpBuff (NULL)
	, m_bFodlerChanged (false)
	, m_hThread (NULL)
	, m_bStop (true)
	, m_bPause (false)
	, m_bWorking (false)
	, m_dwTimerShow (0)
{
	_tcscpy (m_szVideoExt, _T(".avi, .divx, .mp4, .m4v, .mov, .mkv, .3gp, .3g2, "));
	_tcscat (m_szVideoExt, _T(".rmvb, .rm, .real, .rv, "));
	_tcscat (m_szVideoExt, _T(".asf, .wmv, "));
	_tcscat (m_szVideoExt, _T(".flv, .ts, .mpeg, .mpg, .vob "));

	_tcscpy (m_szAudioExt, _T(".mp3, .mp2, .aac, .amr, .ogg, .wav, .ac3, .awb, "));
	_tcscat (m_szAudioExt, _T(".ape, .flac, .wma, .ra, "));

#ifdef _OS_WINCE
	_tcscpy (m_szRoot, _T(""));
#else
	TCHAR * pRoot = CRegMng::g_pRegMng->GetTextValue (_T("Root"));
	if (_tcslen (pRoot) == 0)
		_tcscpy (m_szRoot, _T("c:"));
	else
		_tcscpy (m_szRoot, pRoot);
#endif // WINCE

	TCHAR * pFolder = CRegMng::g_pRegMng->GetTextValue (_T("Folder"));
	if (_tcslen (pFolder) == 0)
		_tcscpy (m_szFolder, m_szRoot);
	else
		_tcscpy (m_szFolder, pFolder);

	_tcscpy (m_szPlayFile, _T(""));

	SetRectEmpty (&m_rcThumb);
	SetRectEmpty (&m_rcItem);

	m_nDrawStepTime = 25;
#ifdef _CPU_MSB2531
	m_nDrawStepTime = 5;
#endif // _CPU_MSB2531
}

CWndPlayList::~CWndPlayList(void)
{
	if (m_hBmpBG != NULL)
	{
		SelectObject (m_hDCBmp, m_hBmpOld);
		DeleteObject (m_hBmpBG);
	}
	m_hBmpBG = NULL;

	if (m_hThumbBmp != NULL)
		DeleteObject (m_hThumbBmp);
	m_hThumbBmp = NULL;

	if (m_hVideoBmp != NULL)
		DeleteObject (m_hVideoBmp);
	m_hVideoBmp = NULL;

	ReleaseList ();

	ReleaseDCBmp ();

	CRegMng::g_pRegMng->SetTextValue (_T("Root"), m_szRoot);
	CRegMng::g_pRegMng->SetTextValue (_T("Folder"), m_szFolder);
}

bool CWndPlayList::CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG, bool bFillItems)
{
	if (!CWndBase::CreateWnd (hParent, rcView, clrBG))
		return false;

	GetClientRect (m_hWnd, &m_rcClient);

//	if (bFillItems)
//		FillMediaItems (m_szFolder);
	SetTimer (m_hWnd, TIMER_FILL_MEDIA_ITEMS, 1000, NULL);

	return true;
}

LRESULT CWndPlayList::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MEDIA_Item * pItem = NULL;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		if (m_dwThumbTimer != 0)
			return S_OK;
		SetCapture (hwnd);
		m_nMoveYPos = HIWORD (lParam);
		m_bMoving = true;
		m_bMoved = false;

		m_ptDown.x = LOWORD (lParam);
		m_ptDown.y = HIWORD (lParam);

		pItem = GetSelectItem (LOWORD (lParam), HIWORD (lParam));
		if (pItem != NULL)
		{
			m_pSelectedItem = pItem;
			InvalidateRect (hwnd, NULL, FALSE);
			if (m_pSelectedItem->nType == ITEM_Folder)
			{
				_tcscpy (m_szPlayFile, m_szFolder);
				_tcscat (m_szPlayFile, _T("\\"));
				_tcscat (m_szPlayFile, pItem->pName);
				PostMessage (m_hParent, WM_YYLIST_SELECT, (WPARAM)m_szPlayFile, NULL);
			}
			else
			{
				PostMessage (m_hParent, WM_YYLIST_SELECT, (WPARAM)pItem->pName, NULL);
			}
		}

		if (m_pSelectedItem != NULL && m_pSelectedItem->nType == ITEM_Video)
			m_dwTimerShow = SetTimer (m_hWnd, TIMER_SHOW_ITEM_INFO, 1000, NULL);

		return S_OK;

	case WM_LBUTTONUP:
	{
		if (m_dwThumbTimer != 0)
			return S_OK;

		ReleaseCapture ();
		m_bMoving = false;
//		InvalidateRect (hwnd, NULL, FALSE);
		
		if (abs (m_ptDown.x - LOWORD (lParam)) < 12 && abs (m_ptDown.x - LOWORD (lParam)) < 12)
//		if (!m_bMoved)
		{
			pItem = GetSelectItem (LOWORD (lParam), HIWORD (lParam));
			if (pItem != NULL)
			{
				m_pSelectedItem = pItem;
				ItemSelected (m_pSelectedItem);
			}
		}
		if (m_dwTimerShow != 0)
			KillTimer (m_hWnd, TIMER_SHOW_ITEM_INFO);
		m_dwTimerShow = 0;

		return S_OK;
	}

	case WM_MOUSEMOVE:
	{
		if (m_dwThumbTimer != 0)
			return S_OK;

		if (wParam != MK_LBUTTON)
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
			
		m_bMoved = true;
		int nYPos = HIWORD (lParam);
		if (m_nMoveYPos > 0 && m_nMoveYPos != nYPos)
		{
			m_nBmpYPos = m_nBmpYPos + (m_nMoveYPos - nYPos);
			if (m_nBmpYPos < 0)
				m_nBmpYPos = 0;
			else if (m_nBmpYPos > m_nBmpHeight - m_rcClient.bottom)
				m_nBmpYPos = m_nBmpHeight - m_rcClient.bottom;
			InvalidateRect (hwnd, NULL, FALSE);
			PostMessage (m_hParent, WM_YYLIST_MOVING, (WPARAM)m_nBmpYPos, NULL);
		}
		m_nMoveYPos = nYPos;
		return S_OK;
	}

	case WM_TIMER:
		if (wParam == TIMER_FILL_MEDIA_ITEMS)
		{
			KillTimer (hwnd, TIMER_FILL_MEDIA_ITEMS);
			FillMediaItems (m_szFolder);
		}
		else if (wParam == TIMER_SHOW_ITEM_INFO)
		{
			KillTimer (hwnd, TIMER_SHOW_ITEM_INFO);
			m_dwTimerShow = 0;

			YYINFO_Thumbnail * pThumb = NULL;
			if (m_pSelectedItem != NULL && m_pSelectedItem->nType == ITEM_Video)
				pThumb = &m_pSelectedItem->sThumbInfo;
			if (pThumb != NULL)
			{
				TCHAR szInfo[256];
				_stprintf (szInfo, _T("Dur: % 6d, Bitrate: % 6d\r\nVideo %s    %d X %d\r\nAudio %s   %d X %d"),
							pThumb->nDuration, pThumb->nBitrate,
							pThumb->szVideoCodec, pThumb->nVideoWidth, pThumb->nVideoHeight,
							pThumb->szAudioCodec, pThumb->nSampleRate, pThumb->nChannels);
				MessageBox (hwnd, szInfo, _T("File Info:"), MB_OK);
			}
		}
		else if (wParam == TIMER_DRAW_THUMBNAIL)
		{
			AnimateThumbnail ();
		}
		else if (wParam == TIMER_HIDE_LASTVIDEO)
		{
			m_nDrawStep--;

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

			if (m_nDrawStep == 0)
			{
				KillTimer (m_hWnd, TIMER_HIDE_LASTVIDEO);
				m_dwThumbTimer = 0;

				InvalidateRect (m_hWnd, NULL, TRUE);
			}
		}
		return S_OK;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		if (m_dwThumbTimer == TIMER_HIDE_LASTVIDEO)
		{
			//if (m_hVideoBmp != NULL)
			//{
			//	HBITMAP hOld = (HBITMAP) SelectObject (m_hDCBmp, m_hVideoBmp);
			//	BitBlt (hdc, 0, 0, m_rcClient.right, m_rcClient.bottom, m_hDCBmp, 0, m_nBmpYPos, SRCCOPY);
			//	SelectObject (m_hDCBmp, hOld);
			//}
		}
		else
		{
			if (m_hBmpBG != NULL)
			{
				SelectObject (m_hDCBmp, m_hBmpBG);
				BitBlt (hdc, 0, 0, m_rcClient.right, m_rcClient.bottom, m_hDCBmp, 0, m_nBmpYPos, SRCCOPY);
			}

			if (m_nBmpHeight > m_rcClient.bottom && m_bMoving)
			{
				int nY = m_nBmpYPos * m_rcClient.bottom / m_nBmpHeight;
				int nH = m_rcClient.bottom / (m_nBmpHeight / m_rcClient.bottom);
				RECT rcBar;
				SetRect (&rcBar, m_rcClient.right - 4, nY, m_rcClient.right, nY + nH);
				HBRUSH hBrush = CreateSolidBrush (RGB (50, 50, 50));
				FillRect (hdc, &rcBar, hBrush);
				DeleteObject (hBrush);
			}

			DrawSelectedItem (hdc);
		}

		EndPaint(hwnd, &ps);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	case WM_ERASEBKGND:
		return S_OK;

	default:
		break;
	}

	return	CWndBase::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
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

		int nBmpW = m_rcClient.right;
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

		if (m_nAnimateType == ITEM_Video && m_pSelectedItem->sThumbInfo.nPos > 200 && m_nDrawStep < m_nSteps / 2)
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
/*
		// Up to Down is not very well. I don't like it.
		HDC hdc = GetDC (m_hWnd);
		HBITMAP hBmp = (HBITMAP)SelectObject (m_hDCBmp, m_hBmpBG);
		int nY =  m_rcClient.bottom * (m_nSteps - m_nDrawStep) / m_nSteps;
		BitBlt (hdc, 0, 0, m_rcClient.right, nY, m_hDCBmp, 0, m_rcClient.bottom - nY, SRCCOPY);
		SelectObject (m_hDCBmp, hBmp);
		ReleaseDC (m_hWnd, hdc);
*/
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
	NODEPOS pos = m_lstItem.GetHeadPosition ();
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

CWndPlayList::MEDIA_Item * CWndPlayList::GetSelectItem (int nX, int nY)
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
		NODEPOS pos = m_lstItem.GetHeadPosition ();
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

void CWndPlayList::ItemSelected (MEDIA_Item * pItem)
{
	if (pItem == NULL)
		return;

	m_nAnimateType = pItem->nType;
	int nX = (pItem->nIndex % m_nCols) * m_nItemWidth + (m_nItemWidth - m_nIconWidth) / 2;
	int nY = (pItem->nIndex / m_nCols - m_nBmpYPos / m_nItemHeight) * m_nItemHeight - m_nBmpYPos % m_nItemHeight + m_nIconOffsetY;
	if (nY < 0)
		nY = 0;
	SetRect (&m_rcItem, nX, nY, nX + m_nIconWidth, nY + m_nIconHeight);

	TCHAR szFolder[1024];
	_tcscpy (szFolder, _T(""));
	if (pItem->nType == ITEM_Home)
	{
		_tcscpy (szFolder, m_szFolder);
		TCHAR * pDir = _tcsrchr (szFolder, _T('\\'));
		if (pDir != NULL)
			*pDir = 0;
	}
	else if (pItem->nType == ITEM_Folder)
	{
		_tcscpy (szFolder, m_szFolder);
		_tcscat (szFolder, _T("\\"));
		_tcscat (szFolder, pItem->pName);
	}
	else
	{
		_tcscpy (m_szPlayFile, m_szFolder);
		_tcscat (m_szPlayFile, _T("\\"));
		_tcscat (m_szPlayFile, pItem->pName);
		Pause ();

		if (m_pSelectedItem->hThumb != NULL)
		{
			m_nSteps = 10;
			m_nDrawStep = m_nSteps;
			m_dwThumbTimer = SetTimer (m_hWnd, TIMER_DRAW_THUMBNAIL, m_nDrawStepTime, NULL);
			if (m_pSelectedItem->sThumbInfo.nPos > 200)
				PostMessage (m_hParent, WM_YYLIST_PLAYFILE, (WPARAM)m_szPlayFile, NULL);
			else
				PostMessage (m_hParent, WM_YYLIST_PLAYFILE, (WPARAM)m_szPlayFile, (LPARAM)m_pSelectedItem->hThumb);
		}
		else
		{
			PostMessage (m_hParent, WM_YYLIST_PLAYFILE, (WPARAM)m_szPlayFile, NULL);
			PostMessage (m_hParent, WM_YYLIST_PLAYBACK, (WPARAM)m_szPlayFile, NULL);
		}
		return;
	}

	FillMediaItems (szFolder);
//	_tcscpy (m_szFolder, szFolder);
//	SetTimer (m_hWnd, TIMER_FILL_MEDIA_ITEMS, 100, NULL);
}

bool CWndPlayList::FillMediaItems (TCHAR * pFolder)
{
	TCHAR	szFilter[1024];
	_tcscpy (szFilter, pFolder);
	_tcscat (szFilter, _T("\\*.*"));
	WIN32_FIND_DATA  data;
	HANDLE  hFind = FindFirstFile(szFilter,&data);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		if (m_lstItem.GetCount () == 0)
		{
#ifdef _OS_WINCE
			_tcscpy (m_szRoot, _T(""));
#else
			_tcscpy (m_szRoot, _T("c:"));
#endif // WINCE
			_tcscpy (m_szFolder, m_szRoot);
			SetTimer (m_hWnd, TIMER_FILL_MEDIA_ITEMS, 10, NULL);
		}
		return false;
	}

	_tcscpy (m_szFolder, pFolder);

	ReleaseList ();
	int				nFolderNum = 0;
	int				nFileNum = 0;
	MEDIA_Item *	pItem = NULL;
	TCHAR *			pExt = NULL;
	int				nNameLen = 0;
	int				nExtLen = 0;
	char *			pExtChar = NULL;

	if (_tcslen (m_szFolder) > 0 && _tcscmp (m_szFolder, m_szRoot))
		AddNewItem (m_szFolder, ITEM_Home);

	do
	{
		if (!_tcscmp (data.cFileName, _T(".")) || !_tcscmp (data.cFileName, _T("..")))
			continue;	
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			AddNewItem (data.cFileName, ITEM_Folder);
			nFolderNum++;
		}
	}while(FindNextFile(hFind, &data));
	FindClose (hFind);

	hFind = FindFirstFile(szFilter,&data);
	do
	{
		if (_tcslen (data.cFileName) < 3)
			continue;	
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			continue;

		pExt = _tcsrchr (data.cFileName, _T('.'));
		if (pExt == NULL)
			continue;

		nExtLen = _tcslen (pExt) * sizeof (TCHAR);
		pExtChar = (char *)pExt;
		for (int i = 0; i < nExtLen; i++)
		{
			if (*(pExtChar + i) >= 'A' && *(pExtChar + i) <= 'Z')
				*(pExtChar + i) += 'a' - 'A';
		}

		if (_tcsstr (m_szVideoExt, pExt) != NULL)
		{
			AddNewItem (data.cFileName, ITEM_Video);
			nFileNum++;
		}
		else if (_tcsstr (m_szAudioExt, pExt) != NULL)
		{
			AddNewItem (data.cFileName, ITEM_Audio);
			nFileNum++;
		}
	}while(FindNextFile(hFind, &data));
	FindClose (hFind);

	int				nIndex = 0;
	MEDIA_Item **	ppFolderItems = NULL;
	MEDIA_Item **	ppFileItems = NULL;
	NODEPOS		pos = NULL;
	if (nFolderNum > 0)
	{
		ppFolderItems = new MEDIA_Item *[nFolderNum];
		pos = m_lstItem.GetHeadPosition ();
		while (pos != NULL)
		{
			pItem = m_lstItem.GetNext (pos);
			if (pItem->nType == ITEM_Folder)
				ppFolderItems[nIndex++] = pItem;
		}
		qsort(ppFolderItems, nFolderNum, sizeof(MEDIA_Item *), compare_filename);
	}

	if (nFileNum > 0)
	{
		nIndex = 0;
		ppFileItems = new MEDIA_Item *[nFileNum];
		pos = m_lstItem.GetHeadPosition ();
		while (pos != NULL)
		{
			pItem = m_lstItem.GetNext (pos);
			if (pItem->nType == ITEM_Audio || pItem->nType == ITEM_Video)
				ppFileItems[nIndex++] = pItem;
		}
		qsort(ppFileItems, nFileNum, sizeof(MEDIA_Item *), compare_filename);
	}

	if (nFileNum > 0 || nFolderNum > 0)
	{
		int				i = 0;
		MEDIA_Item *	pHomeItem = m_lstItem.GetHead ();
		m_lstItem.RemoveAll ();
		if (pHomeItem->nType == ITEM_Home)
			m_lstItem.AddTail (pHomeItem);
		for (i = 0; i < nFolderNum; i++)
		{
			ppFolderItems[i]->nIndex = m_lstItem.GetCount ();
			m_lstItem.AddTail (ppFolderItems[i]);
		}
		for (i = 0; i < nFileNum; i++)
		{
			ppFileItems[i]->nIndex = m_lstItem.GetCount ();
			m_lstItem.AddTail (ppFileItems[i]);
		}
	}
	if (ppFolderItems != NULL)
		delete []ppFolderItems;
	if (ppFileItems != NULL)
		delete []ppFileItems;
	
	if (m_nAnimateType == ITEM_Home)
		CopyLastVideo (m_hWnd);

	CreateBitmapBG ();

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

	return true;
}

bool CWndPlayList::AddNewItem (TCHAR * pName, ITEM_TYPE nType)
{
	MEDIA_Item * pItem = new MEDIA_Item ();
	if (pItem == NULL)
		return false;

	pItem->nType = nType;
	int nNameLen = _tcslen (pName) + 4;
	pItem->pName = new TCHAR[nNameLen];
	memset (pItem->pName, 0, nNameLen * sizeof (TCHAR));
	_tcscpy (pItem->pName, pName);
	pItem->nIndex = m_lstItem.GetCount ();
	m_lstItem.AddTail (pItem);

	return true;
}

bool CWndPlayList::CreateBitmapBG (void)
{
	if (m_lstItem.GetCount () <= 0)
		return false;

	int nLines = (m_lstItem.GetCount () + m_nCols - 1) / m_nCols;

	if (m_hBmpBG != NULL)
	{
		SelectObject (m_hDCBmp, m_hBmpOld);
		DeleteObject (m_hBmpBG);
	}
	m_hBmpBG = NULL;

	if (m_hDCBmp == NULL)
	{
		HDC hDCWnd = GetDC (m_hWnd);
		m_hDCBmp = CreateCompatibleDC (hDCWnd);
		m_hDCItem = CreateCompatibleDC (hDCWnd);
		ReleaseDC (m_hWnd, hDCWnd);
	}

	if (m_nItemWidth == 0)
	{
		m_nItemWidth = m_rcClient.right / m_nCols;
		m_nItemHeight = m_nItemWidth * 3 / 4;
	}

	m_nBmpHeight = nLines * m_nItemHeight;
	if (m_nBmpHeight < m_rcClient.bottom)
		m_nBmpHeight = m_rcClient.bottom;

//	m_hBmpBG = CreateBitmap (m_rcClient.right, m_nBmpHeight, 1, 32, NULL);
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

	int	 x = 0, y = 0, nIndex = 0;
	RECT rcText;

	HBITMAP hBmpIcon = NULL;
	BITMAP	bmpInfo;
	NODEPOS pos = m_lstItem.GetHeadPosition ();
	MEDIA_Item * pItem = m_lstItem.GetNext (pos);
	while (pItem != NULL)
	{	
		if (pItem->nType == ITEM_Home)
			hBmpIcon = m_hBmpHome;
		else if (pItem->nType == ITEM_Folder)
			hBmpIcon = m_hBmpFolder;
		else if (pItem->nType == ITEM_Video)
			hBmpIcon = m_hBmpVideo;
		else if (pItem->nType == ITEM_Audio)
			hBmpIcon = m_hBmpAudio;
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

	if (m_hPenLine != NULL)
		DeleteObject (m_hPenLine);
	m_hPenLine = NULL;
}

void CWndPlayList::ReleaseList (void)
{
	m_bStop = true;
	if (m_pMedia != NULL)
		m_pMedia->SetParam (YYPLAY_PID_Cancel_GetThumb, NULL);
	while (m_hThread != NULL)
		yySleep (10000);

	SelectObject (m_hDCItem, m_hBmpHome);
	MEDIA_Item * pItem = m_lstItem.RemoveHead ();
	while (pItem != NULL)
	{
		if (pItem->pName != NULL)
			delete []pItem->pName;

		if (pItem->hThumb != NULL)
			DeleteObject (pItem->hThumb);

		delete pItem;

		pItem = m_lstItem.RemoveHead ();
	}

	m_pSelectedItem = NULL;
}

int CWndPlayList::GetThumbProc (void * pParam)
{
	SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_BELOW_NORMAL);
	CWndPlayList * pList = (CWndPlayList *)pParam;

	CWndPlayList::MEDIA_Item *	pItem = NULL;

	NODEPOS pos = pList->m_lstItem.GetHeadPosition ();
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
			break;

		pList->GetThumbLoop (pItem);
		yySleep (2000);
	}

	pList->m_hThread = NULL;

	return 0;
}

int CWndPlayList::GetThumbLoop (MEDIA_Item * pItem)
{
	m_bWorking = true;

	if (pItem == NULL)
		return 0;
	if (pItem->nType != ITEM_Video && pItem->nType != ITEM_Audio)
		return 0;
	if (pItem->hThumb != NULL)
		return 0;

	TCHAR szFile[1024];
	_tcscpy (szFile, m_szFolder);
	_tcscat (szFile, _T("\\"));
	_tcscat (szFile, pItem->pName);

//	pItem->sThumbInfo.nInfoType = YYINFO_Get_MediaInfo;
//	pItem->sThumbInfo.nThumbWidth = m_nIconWidth;
//	pItem->sThumbInfo.nThumbHeight = m_nIconHeight;
	pItem->sThumbInfo.nThumbWidth = m_rcClient.right / 2;
	pItem->sThumbInfo.nThumbHeight = m_rcClient.bottom / 2;
	pItem->sThumbInfo.bKeepAspectRatio = true;
	pItem->sThumbInfo.nBGColor = RGB (0, 0, 0);
	pItem->sThumbInfo.nPos = 0;

	int nStartTime = yyGetSysTime ();

	pItem->hThumb = (HBITMAP)m_pMedia->GetThumb (szFile, &pItem->sThumbInfo);

	int nUsedTime = yyGetSysTime () - nStartTime;

	if (pItem->hThumb != NULL)
	{
		if (nUsedTime < 600 && IsBlackThumb (&pItem->sThumbInfo))
		{
			DeleteObject (pItem->hThumb);
			pItem->sThumbInfo.pBmpBuff = NULL;
			pItem->sThumbInfo.hThumbnail = NULL;
			pItem->sThumbInfo.nPos = 10000;
			pItem->hThumb = (HBITMAP)m_pMedia->GetThumb (szFile, &pItem->sThumbInfo);
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
			RECT rcItem;
			SetRect (&rcItem, x, y - m_nBmpYPos, x + m_nItemWidth, y - m_nBmpYPos + m_nItemHeight);
			if (m_dwThumbTimer == 0)
				InvalidateRect (m_hWnd, &rcItem, FALSE);
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

int CWndPlayList::GetMaxPos (void)
{
	int nMax = m_nBmpHeight - m_rcClient.bottom;
	if (nMax < 0)
		nMax = 0;

	return nMax;
}

int CWndPlayList::GetPos (void)
{
	return m_nBmpYPos;
}

int CWndPlayList::SetPos (int nPos)
{
	m_nBmpYPos = nPos;
	InvalidateRect (m_hWnd, NULL, FALSE);
	return 0;
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

	if (m_rcItem.right == 0 || m_rcItem.bottom == 0)
		return -1;

	HDC hdc = GetDC (hWndVideo);

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

	HBITMAP hOld = (HBITMAP)SelectObject (m_hDCBmp, m_hVideoBmp);
	BitBlt (m_hDCBmp, 0, 0, m_rcClient.right, m_rcClient.bottom, hdc, 0, 0, SRCCOPY);
	ReleaseDC (hWndVideo, hdc);

	//hdc = GetDC (m_hWnd);
	//BitBlt (hdc, 0, 0, m_rcClient.right, m_rcClient.bottom, m_hDCBmp, 0, 0, SRCCOPY);
	//ReleaseDC (m_hWnd, hdc);
	//SelectObject (m_hDCBmp, hOld);

	return 0;
}

void CWndPlayList::SetPlayingFile (TCHAR * pFile)
{
	if (pFile == NULL)
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

	NODEPOS pos = m_lstItem.GetHeadPosition ();
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

YYINFO_Thumbnail *	CWndPlayList::GetSelectedItem (void)
{
	if (m_pSelectedItem == NULL)
		return NULL;

	if (m_pSelectedItem->nType != ITEM_Video)
		return NULL;

	return &m_pSelectedItem->sThumbInfo;
}

int __cdecl CWndPlayList::compare_filename(const void *arg1, const void *arg2)
{
	MEDIA_Item * pItem1 = *(MEDIA_Item **)arg1;
	MEDIA_Item * pItem2 = *(MEDIA_Item **)arg2;
	return _tcsicmp (pItem1->pName, pItem2->pName);
}
