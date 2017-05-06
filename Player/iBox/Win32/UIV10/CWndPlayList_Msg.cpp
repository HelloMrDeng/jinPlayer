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
#include "CDlgFileProp.h"
#include "resource.h"

#include "USystemFunc.h"
#include "UFileFunc.h"

#include "yyLog.h"

#pragma warning (disable : 4996)

//CWndBase::OnReceiveMessage(m_hWnd, uMsg, wParam, lParam);
LRESULT CWndPlayList::OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int		wmId, wmEvent;
	wmId    = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	switch (wmId)
	{
	case ID_ITEM_PLAY:
		ItemSelected (m_pSelectedItem);
		break;

	case ID_FILE_PROPERTIES:
	{
		if (m_pSelectedItem == NULL)
			return S_OK;
		if (m_pSelectedItem->nType != ITEM_Video && m_pSelectedItem->nType != ITEM_Audio)
			return S_OK;
		CDlgFileProp dlgProp (m_hInst, m_hWnd);
		TCHAR szFile[1024];
		if (m_pSelectedItem->pPath != NULL)
			_tcscpy (szFile, m_pSelectedItem->pPath);
		else
		{
			_tcscpy (szFile, m_szFolder);
			_tcscat (szFile, _T("\\"));
			_tcscat (szFile, m_pSelectedItem->pName);
		}
		dlgProp.OpenDlg (szFile);
		break;
	}

	case ID_ITEM_DELETE:
		DeleteSelItem ();
		break;

	case ID_BOXITEM_EXPORT:
		ExportFileInBox ();
		break;

	case ID_MYBOX_NEWBOX:
		if (_tcslen (m_szBoxPath) > 0 && CBaseKey::g_pBoxPW == NULL)
			return S_OK;
		AddNewBox ();
		break;

	case ID_MYBOX_OPENBOX:
		OpenBox (NULL);
		break;

	case ID_VIEW_NEWBOX:
		SetViewType (LIST_VIEW_MyBox);
		if (_tcslen (m_szBoxPath) > 0 && CBaseKey::g_pBoxPW == NULL)
			return S_OK;
		AddNewBox ();

	case ID_VIEW_OPENBOX:
		SetViewType (LIST_VIEW_MyBox);
		OpenBox (NULL);
		break;

	default:
		break;
	}
	return S_OK;
}

LRESULT CWndPlayList::OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pWndEdit == NULL || !m_pWndEdit->IsShow ())
		return CWndBase::OnReceiveMessage(m_hWnd, uMsg, wParam, lParam);

	if (wParam == VK_ESCAPE)
	{
		m_pWndEdit->SetText (_T(""));
		m_pWndEdit->ShowWnd (SW_HIDE);
	}
	else if (wParam == VK_BACK)
	{
		TCHAR * pText = m_pWndEdit->GetText ();
		if (_tcslen (pText) > 0)
		{
			pText = pText + _tcslen (pText);
			*(pText - 1) = 0;
			InvalidateRect (m_pWndEdit->GetWnd (), NULL, TRUE);
		}
	}
	else if (wParam == VK_RETURN)
	{
		TCHAR * pText = m_pWndEdit->GetText ();
		if (_tcslen (pText) > 0)
		{
			MEDIA_Item *	pNewItem = NULL;
			MEDIA_Item *	pAddItem = NULL;
			MEDIA_Item *	pItem = NULL;
			POSITION		pos = NULL;

			if (m_lvType == LIST_VIEW_Favor)
			{
				if (m_pCurItem->pChildList != NULL)
				{
					pos = m_pCurItem->pChildList->GetHeadPosition ();
					while (pos != NULL)
					{
						pItem = m_pCurItem->pChildList->GetNext (pos);
						if (pItem->nType == ITEM_Folder && !_tcscmp (pItem->pName, pText))
							return S_OK;
					}
				}
			}
			else if (m_lvType == LIST_VIEW_MyBox)
			{
				pos = m_lstItem.GetHeadPosition ();
				while (pos != NULL)
				{
					pItem = m_lstItem.GetNext (pos);
					if (pItem->nType == ITEM_Folder && !_tcscmp (pItem->pName, pText))
						return S_OK;
				}
				if (!NewFolderInBox (pText))
					return S_OK;
			}

			pos = m_lstItem.GetHeadPosition ();
			while (pos != NULL)
			{
				pItem = m_lstItem.GetNext (pos);
				if (pItem->nType == ITEM_NewFolder)
				{
					pNewItem = CreateItem (pText, ITEM_Folder);
					pNewItem->nIndex = pItem->nIndex;

					pItem = m_lstItem.GetPrev (pos);
					m_lstItem.AddBefore (pos, pNewItem);
					pItem = m_lstItem.GetNext (pos);

					if (m_lvType == LIST_VIEW_Favor)
					{
						if (m_pCurItem->pChildList == NULL)
							m_pCurItem->pChildList = new CObjectList<MEDIA_Item> ();
						pAddItem = CloneItem (pNewItem);
						pAddItem->pParent = m_pCurItem;
						m_pCurItem->pChildList->AddTail (pAddItem);
						m_bFavorModified = true;
					}

					int nBmpPos = m_nBmpYPos;
					CreateBitmapBG ();
					m_nBmpYPos = nBmpPos;
					ShowBitmapBG ();
				}
				if (pNewItem != NULL)
					pItem->nIndex += 1;
			}
		}
		m_pWndEdit->SetText (_T(""));
		m_pWndEdit->ShowWnd (SW_HIDE);
	}
	return S_OK;
}

LRESULT CWndPlayList::OnChar (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_RETURN || wParam == VK_BACK)
		return CWndBase::OnReceiveMessage(m_hWnd, uMsg, wParam, lParam);

	if (m_pWndEdit != NULL && m_pWndEdit->IsShow ())
	{
		if (m_lvType == LIST_VIEW_Favor || m_lvType == LIST_VIEW_MyBox)
		{
			int		nTextSize = m_pWndEdit->GetTextSize ();
			TCHAR * pText = m_pWndEdit->GetText ();
			if (_tcslen (pText) >= nTextSize)
				return S_OK;
			pText = pText + _tcslen (pText);
			*(pText++) = (TCHAR)wParam;			
			*(pText++) = 0;			
			InvalidateRect (m_pWndEdit->GetWnd (), NULL, TRUE);
		}
	}
	else
	{
		if (m_nTimerOnChar != 0)
			KillTimer (m_hWnd, TIMER_CHAR_LASTTEXT);
		m_nTimerOnChar = 0;

		TCHAR * pText = m_szPWText;
		pText = pText + _tcslen (pText);
		*(pText++) = (TCHAR)wParam;			
		*(pText++) = 0;	
		if (_tcslen (m_szPWText) == 6)
		{
			OpenBox (m_szPWText);
			memset (m_szPWText, 0, sizeof (m_szPWText));
		}
		else
			m_nTimerOnChar = SetTimer (m_hWnd, TIMER_CHAR_LASTTEXT, 3000, NULL);
	}

	return S_OK;
}

LRESULT CWndPlayList::OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_dwThumbTimer != 0)
		return S_OK;

	MEDIA_Item * pItem = NULL;
	SetCapture (m_hWnd);
	m_nMoveYPos = HIWORD (lParam);
	m_bMoving = true;
	m_bMoved = false;

	m_ptDown.x = LOWORD (lParam);
	m_ptDown.y = HIWORD (lParam);

	pItem = GetSelectItem (LOWORD (lParam), HIWORD (lParam));
	if (pItem != NULL)
	{
		m_pSelectedItem = pItem;
		InvalidateRect (m_hWnd, NULL, FALSE);
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
}

LRESULT CWndPlayList::OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_dwThumbTimer != 0)
		return S_OK;

	MEDIA_Item * pItem = NULL;
	ReleaseCapture ();
	m_bMoving = false;		
	if (abs (m_ptDown.x - LOWORD (lParam)) < 12 && abs (m_ptDown.x - LOWORD (lParam)) < 12)
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

LRESULT CWndPlayList::OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	pt.x = LOWORD (lParam);
	pt.y = HIWORD (lParam);
	ClientToScreen (m_hWnd, &pt);

	if (m_dwThumbTimer != 0)
		return S_OK;
	MEDIA_Item *  pItem = GetSelectItem (LOWORD (lParam), HIWORD (lParam));
	if (pItem == NULL)
	{
		if (m_lvType == LIST_VIEW_MyBox)
			TrackPopupMenu (m_hMenuBox, TPM_LEFTALIGN, pt.x, pt.y, 0, m_hWnd, NULL);
		return S_OK;
	}
	
	m_pSelectedItem = pItem;
	InvalidateRect (m_hWnd, NULL, FALSE);

	if (m_lvType == LIST_VIEW_Folder)
	{
		if (pItem->nType == ITEM_Video || pItem->nType == ITEM_Audio)
			TrackPopupMenu (m_hMenuItem, TPM_LEFTALIGN, pt.x, pt.y, 0, m_hWnd, NULL);
	}
	else if (m_lvType == LIST_VIEW_Favor)
	{
		if (pItem->nType == ITEM_Folder || pItem->nType == ITEM_Video || pItem->nType == ITEM_Audio)
			TrackPopupMenu (m_hMenuItem, TPM_LEFTALIGN, pt.x, pt.y, 0, m_hWnd, NULL);
	}
	else if (m_lvType == LIST_VIEW_MyBox)
	{
		if (pItem->nType == ITEM_Video || pItem->nType == ITEM_Audio)
			TrackPopupMenu (m_hMenuBoxItem, TPM_LEFTALIGN, pt.x, pt.y, 0, m_hWnd, NULL);
	}
	
	return S_OK;
}

LRESULT CWndPlayList::OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_dwThumbTimer != 0)
		return S_OK;
	if (wParam != MK_LBUTTON)
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		
	m_bMoved = true;
	int nYPos = HIWORD (lParam);
	if (m_nMoveYPos > 0 && m_nMoveYPos != nYPos)
	{
		m_nBmpYPos = m_nBmpYPos + (m_nMoveYPos - nYPos);
		if (m_nBmpYPos < 0)
			m_nBmpYPos = 0;
		else if (m_nBmpYPos > m_nBmpHeight - m_rcClient.bottom)
			m_nBmpYPos = m_nBmpHeight - m_rcClient.bottom;
		InvalidateRect (m_hWnd, NULL, FALSE);
		PostMessage (m_hParent, WM_YYLIST_MOVING, (WPARAM)m_nBmpYPos, NULL);

		GetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo);
		m_sbInfo.nPos = m_nBmpYPos;
		SetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo, TRUE);
	}
	m_nMoveYPos = nYPos;

	return S_OK;
}

LRESULT CWndPlayList::OnMouseWheel (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD (wParam) == WHEEL_DELTA)
		m_nBmpYPos = m_nBmpYPos - 20;
	else
		m_nBmpYPos = m_nBmpYPos + 20;
	if (m_nBmpYPos < 0)
		m_nBmpYPos = 0;
	else if (m_nBmpYPos > m_nBmpHeight - m_rcClient.bottom)
		m_nBmpYPos = m_nBmpHeight - m_rcClient.bottom;
	GetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo);
	m_sbInfo.nPos = m_nBmpYPos;
	SetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo, TRUE);
	InvalidateRect (m_hWnd, NULL, FALSE);
	return S_OK;
}

LRESULT CWndPlayList::OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == TIMER_FILL_MEDIA_ITEMS)
	{
		KillTimer (m_hWnd, TIMER_FILL_MEDIA_ITEMS);
		FillMediaItems (m_szFolder);
	}
	else if (wParam == TIMER_SHOW_ITEM_INFO)
	{
		KillTimer (m_hWnd, TIMER_SHOW_ITEM_INFO);
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
			MessageBox (m_hWnd, szInfo, m_pLangText->GetText (YYTEXT_FileInfo), MB_OK);
		}
	}
	else if (wParam == TIMER_DRAW_THUMBNAIL)
	{
		AnimateThumbnail ();
	}
	else if (wParam == TIMER_HIDE_LASTVIDEO)
	{
		if (m_hThumbBmp == NULL)
		{
			KillTimer (m_hWnd, TIMER_HIDE_LASTVIDEO);
			m_dwThumbTimer = 0;
			InvalidateRect (m_hWnd, NULL, TRUE);
			return S_OK;
		}
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
	else if (TIMER_CHAR_LASTTEXT)
	{
		if (m_nTimerOnChar != 0)
			KillTimer (m_hWnd, TIMER_CHAR_LASTTEXT);
		m_nTimerOnChar = 0;
		memset (m_szPWText, 0, sizeof (m_szPWText));
	}
	return S_OK;
}

LRESULT CWndPlayList::OnVScroll (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nScrollCode = (int)LOWORD(wParam);
	int nPos = (short int)HIWORD(wParam);
	GetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo);

	switch (nScrollCode)
	{
	case SB_LINEUP:
		m_sbInfo.nPos = m_sbInfo.nPos - 5; 
		break;
	case SB_LINEDOWN:
		m_sbInfo.nPos = m_sbInfo.nPos + 5; 
		break;		
	case SB_PAGEUP:
		m_sbInfo.nPos = m_sbInfo.nPos - m_nBmpHeight / 10; 
		break;
	case SB_PAGEDOWN:
		m_sbInfo.nPos = m_sbInfo.nPos + m_nBmpHeight / 10; 
		break;
	case SB_THUMBPOSITION:
		m_sbInfo.nPos = nPos; 
		break;
	case SB_THUMBTRACK:
		m_sbInfo.nPos = nPos; 
		YYLOGT ("PlayList", "Pos is % 8d", nPos);
		break;
	}
	if (m_sbInfo.nPos < m_sbInfo.nMin)
		m_sbInfo.nPos = m_sbInfo.nMin;
	if (m_sbInfo.nPos > m_sbInfo.nMax)
		m_sbInfo.nPos = m_sbInfo.nMax;
	SetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo, TRUE);
	m_nBmpYPos = m_sbInfo.nPos;
	InvalidateRect (m_hWnd, NULL, FALSE);
	return S_OK;
}

void CWndPlayList::OnDropFiles (WPARAM wParam)
{
	if (m_lvType == LIST_VIEW_Folder)
		return;

	Pause ();

	MEDIA_Item *	pNewItem = NULL;
	MEDIA_Item *	pAddItem = NULL;
	POSITION		pos = NULL;
	char *			pExt = NULL;
	TCHAR *			pName = NULL;

	HDROP	hDrop = (HDROP) wParam;
	TCHAR	szFile[1024];
	int		nFiles = DragQueryFile (hDrop, 0XFFFFFFFF, NULL, 0);
	for (int i = 0; i < nFiles; i++)
	{
		memset (szFile, 0, sizeof (szFile));
		int nNameLen = DragQueryFile (hDrop, i, szFile, sizeof (szFile));
		if (nNameLen > 0)
		{
			pName = _tcsrchr (szFile, _T('\\'));
			if (pName == NULL)
				continue;
			pName++;

			pExt = (char *)_tcsrchr (szFile, _T('.'));
			if (pExt == NULL)
				continue;
			while (*pExt != 0)
			{
				if (*pExt >= ('A') && *pExt <= ('Z'))
					*pExt += ('a') - ('A');
				pExt += 2;
			}
			TCHAR * pExtName = _tcsrchr (szFile, _T('.'));
			if (_tcsstr (m_szVideoExt, pExtName) != NULL)
				pNewItem = CreateItem (szFile, pName, ITEM_Video);
			else if (_tcsstr (m_szAudioExt, pExtName) != NULL)
				pNewItem = CreateItem (szFile, pName, ITEM_Audio);
			else
				continue;

			if (m_lvType == LIST_VIEW_Favor)
			{
				if (m_pCurItem->pChildList == NULL)
					m_pCurItem->pChildList = new CObjectList<MEDIA_Item> ();
				pAddItem = CloneItem (pNewItem);
				pAddItem->pParent = m_pCurItem;
				m_pCurItem->pChildList->AddTail (pAddItem);
				m_bFavorModified = true;
			}
			else if (m_lvType == LIST_VIEW_MyBox)
			{
				TCHAR szTarget[1024];
				if (!ImportFileInBox (szFile, szTarget))
				{
					DeleteItem (pNewItem);
					break;
				}
				if (pNewItem->pPath != NULL)
					delete []pNewItem->pPath;
				pNewItem->pPath = new TCHAR[1024];
				_tcscpy (pNewItem->pPath, szTarget);
			}

			pNewItem->nIndex = m_lstItem.GetCount () - 2;
			pos = m_lstItem.GetTailPosition ();
			m_lstItem.GetPrev (pos);
			m_lstItem.AddBefore (pos, pNewItem);
		}
	}

	pos = m_lstItem.GetTailPosition ();
	pNewItem = m_lstItem.GetPrev (pos);
	pNewItem->nIndex = m_lstItem.GetCount () - 1;
	pNewItem = m_lstItem.GetPrev (pos);
	pNewItem->nIndex = m_lstItem.GetCount () - 2;

	m_bPause = false;

	int nBmpPos = m_nBmpYPos;
	CreateBitmapBG ();
	m_nBmpYPos = nBmpPos;
	ShowBitmapBG ();
	return;
}

LRESULT CWndPlayList::OnPaint (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hWnd, &ps);
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
	EndPaint(m_hWnd, &ps);

	return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

void CWndPlayList::OnSize (void)
{
	RECT rcView;
	GetClientRect (m_hParent, &rcView);
	SetWindowPos (m_hWnd, NULL, 0, 0, rcView.right, rcView.bottom, 0);

	if (!CreateBitmapBG ())
		return;

	m_nAnimateType = ITEM_Unknown;
	ShowBitmapBG ();
}