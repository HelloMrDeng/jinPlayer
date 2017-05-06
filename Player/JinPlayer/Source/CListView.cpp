/*******************************************************************************
	File:		CListView.cpp

	Contains:	The base list view implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CListView.h"
#include "CAnimation.h"
#include "CListStore.h"

#include "USystemFunc.h"
#include "UStringFunc.h"
#include "UBitmapFunc.h"

#include "RPlayerDef.h"
#include "yyLog.h"

#pragma warning (disable : 4996)

CListView::CListView(HINSTANCE hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hWnd (NULL)
	, m_bLBtnDown (false)
	, m_pMedia (NULL)
	, m_pExtSrc (NULL)
	, m_pRes (NULL)
	, m_hMemDC (NULL)
	, m_hBmpDC (NULL)
	, m_hBmpOld (NULL)
	, m_hBmpList (NULL)
	, m_nBmpHeight (0)
	, m_nBmpCols (0)
	, m_nBmpYPos (0)
	, m_nMoveYPos (0)
	, m_hTxtFont (NULL)
	, m_nActvType (ITEM_Unknown)
	, m_pSelItem (NULL)
	, m_pNewFolder (NULL)
	, m_pNewMedia (NULL)
	, m_nFolderLevel (0)
	, m_hThread (NULL)
	, m_nStatus (YYTHRD_Stop)
	, m_bWorking (false)
{
	SetObjectName ("CListView");
	SetRectEmpty (&m_rcView);
	SetRectEmpty (&m_rcLastItem);

	memset (m_szRoot, 0, sizeof (m_szRoot));
	memset (m_szFolder, 0, sizeof (m_szFolder));

	yyGetDataPath (m_hInst, m_szThumb, sizeof (m_szThumb));
	_tcscat (m_szThumb, _T("thumb\\"));

	memset (&m_sbInfo, 0, sizeof (m_sbInfo));
	m_sbInfo.cbSize = sizeof (m_sbInfo);
	m_sbInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
}	

CListView::~CListView(void)
{
	Stop ();
	ReleaseItems ();
	ReleaseBmpDC ();
	YY_DEL_P (m_pNewMedia);
	YY_DEL_P (m_pNewFolder);
}

void CListView::SetMediaSource (CMediaEngine * pMedia, CExtSource * pSource)
{
	m_pMedia = pMedia;
	m_pExtSrc = pSource;
}

bool CListView::Create (HWND hWnd, CListRes * pRes)
{
	m_hWnd = hWnd;
	m_pRes = pRes;

	return true;
}

bool CListView::FillItem (TCHAR * pPath)
{
	return true;
}

bool CListView::CreateDispBmp (void)
{
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	if (abs (m_rcView.right - rcView.right) < 2 && abs (m_rcView.bottom - rcView.bottom) < 2)
		return false;
	GetClientRect (m_hWnd, &m_rcView);

	int		nLines = 0;
	m_nBmpCols = rcView.right / ITEM_WIDTH;
	if (m_nBmpCols < 1)
	{
		m_nBmpCols = 1;
		return false;
	}
	if (m_lstItem.GetCount () > 0)
		nLines = (m_lstItem.GetCount () + m_nBmpCols - 1) / m_nBmpCols;
	m_nBmpHeight = nLines * ITEM_HEIGHT;
	if (m_nBmpHeight <= rcView.bottom)
	{
		m_nBmpHeight = rcView.bottom;
		EnableScrollBar (m_hWnd, SB_VERT, ESB_DISABLE_BOTH);
	}
	else
	{
		GetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo);
		m_sbInfo.nPos = 0;
		m_sbInfo.nMin = 0;
		m_sbInfo.nMax = m_nBmpHeight - rcView.bottom;
		SetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo, TRUE);
		EnableScrollBar (m_hWnd, SB_VERT, ESB_ENABLE_BOTH);
	}
	GetClientRect (m_hWnd, &rcView);
	if (m_hBmpList != NULL)
	{
		SelectObject (m_hBmpDC, m_hBmpOld);
		DeleteObject (m_hBmpList);
		m_hBmpList = NULL;
	}
	if (m_hBmpDC == NULL)
	{
		HDC hDCWnd = GetDC (m_hWnd);
		m_hBmpDC = CreateCompatibleDC (hDCWnd);
		m_hMemDC = CreateCompatibleDC (hDCWnd);
		ReleaseDC (m_hWnd, hDCWnd);
	}
	m_hBmpList = yyBmpCreate (m_hBmpDC, rcView.right, m_nBmpHeight, &m_hBmpBuff, RGB (239, 239, 239));
	if (m_hBmpList == NULL)
		return false;
	m_hBmpOld = (HBITMAP)SelectObject (m_hBmpDC, m_hBmpList);
	int		x = 0, y = 0, nIndex = 0;
	int		nItemWidth = rcView.right / m_nBmpCols;
	RECT	rcText;
	HBITMAP hBmpIcon = NULL;
	BITMAP	bmpInfo;
	HDC hItemDC = CreateCompatibleDC (m_hBmpDC);
	CListItem * pItem = NULL;
	NODEPOS pos = m_lstItem.GetHeadPosition ();
	while (pos != NULL)
	{	
		pItem = m_lstItem.GetNext (pos);
		if (pItem->m_nType == ITEM_Exit)
			hBmpIcon = m_pRes->m_hBmpExit;
		else if (pItem->m_nType == ITEM_Home)
			hBmpIcon = m_pRes->m_hBmpHome;
		else if (pItem->m_nType == ITEM_Folder)
			hBmpIcon = m_pRes->m_hBmpFolder;
		else if (pItem->m_nType == ITEM_Video)
		{
			if (pItem->m_hThumb == NULL)
				hBmpIcon = m_pRes->m_hBmpVideo;
			else
				hBmpIcon = pItem->m_hThumb;
		}
		else if (pItem->m_nType == ITEM_Audio)
		{
			if (pItem->m_hThumb == NULL)
				hBmpIcon = m_pRes->m_hBmpAudio;
			else
				hBmpIcon = pItem->m_hThumb;
		}
		else if (pItem->m_nType == ITEM_Image)
		{
			if (pItem->m_hThumb == NULL)
				hBmpIcon = m_pRes->m_hBmpImage;
			else
				hBmpIcon = pItem->m_hThumb;
		}
		else if (pItem->m_nType == ITEM_NewFolder)
			hBmpIcon = m_pRes->m_hBmpNewFolder;
		else if (pItem->m_nType == ITEM_NewFile)
			hBmpIcon = m_pRes->m_hBmpNewFile;
		SelectObject (hItemDC, hBmpIcon);
		GetObject (hBmpIcon, sizeof (BITMAP), &bmpInfo);
		x = (nIndex % m_nBmpCols) * nItemWidth + (ITEM_WIDTH - ICON_WIDTH) / 2 + (nItemWidth - ITEM_WIDTH) / 2;
		y = (nIndex / m_nBmpCols) * ITEM_HEIGHT + ICON_OFF_Y;
		if (pItem->m_hThumb != NULL)
			DrawItemThumb (pItem);
		else
			StretchBlt (m_hBmpDC, x, y, ICON_WIDTH, ICON_HEIGHT, hItemDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);
		if (m_hTxtFont == NULL)
			CreateTextFont ();
		HFONT hOldFont = (HFONT)SelectObject (m_hBmpDC, m_hTxtFont);
		SetBkMode (m_hBmpDC, TRANSPARENT);
		SetRect (&rcText, (nIndex % m_nBmpCols) * nItemWidth + 8, y + ICON_HEIGHT + ICON_OFF_Y, 
						  (nIndex % m_nBmpCols + 1) * nItemWidth - 16, y + ITEM_HEIGHT - ICON_OFF_Y * 2);

		TCHAR	szName[1024];
		_tcscpy (szName, pItem->m_pName);
		int		nNameLen = _tcslen (szName);
		SIZE szTxt;
		GetTextExtentPoint (m_hBmpDC, szName, nNameLen, &szTxt);
		while (szTxt.cx > (rcText.right - rcText.left))
		{
			nNameLen--;
			GetTextExtentPoint (m_hBmpDC, szName, nNameLen, &szTxt);
		}
		DrawText (m_hBmpDC, szName, nNameLen, &rcText, DT_CENTER);
		if (nNameLen < _tcslen (szName))
		{
			rcText.top = rcText.top + (rcText.bottom - rcText.top) / 2;
			_tcscpy (szName, pItem->m_pName + nNameLen);
			int	 nStartTxt = 0;
			GetTextExtentPoint (m_hBmpDC, szName + nStartTxt, _tcslen (szName + nStartTxt), &szTxt);
			while (szTxt.cx > (rcText.right - rcText.left))
			{
				nStartTxt++;
				GetTextExtentPoint (m_hBmpDC, szName + nStartTxt, _tcslen (szName + nStartTxt), &szTxt);
			}
			DrawText (m_hBmpDC, szName + nStartTxt, _tcslen (szName + nStartTxt), &rcText, DT_CENTER);
		}
		SelectObject (m_hBmpDC, hOldFont);

		nIndex++;
	}
	SelectObject (m_hBmpDC, m_hBmpOld);
	m_nBmpYPos = 0;
	m_nMoveYPos = 0;
	if (hItemDC != NULL)
		DeleteDC (hItemDC);

	return true;
}

bool CListView::ShowDispBmp (CListView * pPrev)
{
	if (m_hBmpDC == NULL || m_hBmpList == NULL)
		return false;

	CAutoLock lock (&m_mtBmp);
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	CAnimation anm (m_hInst);
	anm.SetRndWnd (m_hWnd, &m_rcLastItem, &rcView);
	if (pPrev != NULL)
	{
		HBITMAP hPrevBmp = NULL;
		LPBYTE	pPrevBuf = NULL;
		RECT	rcPrev;
		pPrev->GetViewBmp (&hPrevBmp, &rcPrev, &pPrevBuf);
		anm.SetBackBmp (hPrevBmp, pPrevBuf, &rcPrev);
		anm.SetForeBmp (m_hBmpList, m_hBmpBuff, &rcView, 0);
		anm.Show (ANMT_Transition);
	}
	else if (m_nActvType == ITEM_Folder)
	{
		anm.SetForeBmp (m_hBmpList, m_hBmpBuff, &rcView, 0);
		anm.Show (ANMT_Expand);
	}
	else if (m_nActvType == ITEM_Home)
	{
		rcView.top = m_nBmpYPos;
		rcView.bottom += m_nBmpYPos;
		anm.SetBackBmp (m_hBmpList, m_hBmpBuff, &rcView);
		anm.Show (ANMT_Shrink);
	}
	else
	{
		anm.SetForeBmp (m_hBmpList, m_hBmpBuff, &rcView, 0);
		anm.Show (ANMT_None);
	}
	return true;
}

bool CListView::GetViewBmp (HBITMAP * ppBmp, RECT * pRect, LPBYTE * ppBuff)
{
	if (m_hBmpList == NULL || pRect == NULL)
		return false;

	*ppBmp = m_hBmpList;
	if (ppBuff != NULL)
		*ppBuff = m_hBmpBuff + m_rcView.right * 4 * m_nBmpYPos;
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	SetRect (pRect, 0, m_nBmpYPos, rcView.right, m_nBmpYPos + rcView.bottom);

	return true;
}

bool CListView::CreateTextFont (void)
{
	LOGFONT lgFont;
	memset (&lgFont, 0, sizeof (lgFont));
	lgFont.lfHeight = -16;
	lgFont.lfWeight = FW_MEDIUM;
	lgFont.lfCharSet = GB2312_CHARSET;
	lgFont.lfOutPrecision = 1;
	lgFont.lfClipPrecision = 2;
	lgFont.lfQuality = 1;
	lgFont.lfPitchAndFamily	= 49;
	_tcscpy (lgFont.lfFaceName, _T("Times New Roman"));

	if (m_hTxtFont != NULL)
		DeleteObject (m_hTxtFont);
    m_hTxtFont = CreateFontIndirect(&lgFont); 

	return true;
}

bool CListView::ReleaseBmpDC (void)
{
	if (m_hBmpList != NULL)
	{
		SelectObject (m_hBmpDC, m_hBmpOld);
		DeleteObject (m_hBmpList);
		m_hBmpList = NULL;
	}

	if (m_hBmpDC != NULL)
		DeleteDC (m_hBmpDC);
	m_hBmpDC = NULL;
	if (m_hMemDC != NULL)
		DeleteDC (m_hMemDC);
	m_hMemDC = NULL;

	if (m_hTxtFont != NULL)
		DeleteObject (m_hTxtFont);
	m_hTxtFont = NULL;

	return true;
}

bool CListView::ReleaseItems (void)
{
	Pause ();
	CListItem * pItem = NULL;
	if (m_lstItem.GetCount () > 1)
	{
		bool bModified = false;
		NODEPOS pos = m_lstItem.GetHeadPosition ();
		while (pos != NULL && !bModified)
		{
			pItem = m_lstItem.GetNext (pos);
			if (pItem->m_bModified)
				bModified = true;
		}
		if (bModified)
			SaveItemThumb (m_szFolder);
	}
	CAutoLock lock (&m_mtList);
	pItem = m_lstItem.RemoveHead ();
	while (pItem != NULL)
	{
		delete pItem;
		pItem = m_lstItem.RemoveHead ();
	}
	return true;
}

bool CListView::FindItem (TCHAR * pName, ITEM_TYPE nType)
{
	CListItem * pItem = NULL;
	NODEPOS pos = m_lstItem.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstItem.GetNext (pos);
		if (pItem->m_nType == nType)
		{
			if (!_tcscmp (pItem->m_pName, pName))
				return true;
		}
	}
	return false;
}

bool CListView::SortItems (void)
{
	int			nFolderNum = 0;
	int			nFileNum = 0;
	NODEPOS	pos = NULL;
	CListItem *	pItem = NULL;

	if (m_lstItem.GetCount () <= 0)
		return false;

	CListItem *	pItemHome = NULL;
	CListItem *	pItemNewFile = NULL;
	CListItem *	pItemNewFolder = NULL;
	pos = m_lstItem.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstItem.GetNext (pos);
		if (pItem->m_nType == ITEM_Folder)
			nFolderNum++;
		else if (pItem->m_nType == ITEM_Video || pItem->m_nType == ITEM_Audio || pItem->m_nType == ITEM_Image)
			nFileNum++;
		else if (pItem->m_nType == ITEM_Home || pItem->m_nType == ITEM_Exit)
			pItemHome = pItem;
		else if (pItem->m_nType == ITEM_NewFile)
			pItemNewFile = pItem;
		else if (pItem->m_nType == ITEM_NewFolder)
			pItemNewFolder = pItem;

	}

	CListItem **	ppFolderItems = NULL;
	CListItem **	ppFileItems = NULL;
	int				nIdxFile = 0;
	int				nIdxFold = 0;
	if (nFolderNum > 0 || nFileNum > 0)
	{
		if (nFolderNum > 0)
			ppFolderItems = new CListItem *[nFolderNum];
		if (nFileNum > 0)
			ppFileItems = new CListItem *[nFileNum];

		pos = m_lstItem.GetHeadPosition ();
		while (pos != NULL)
		{
			pItem = m_lstItem.GetNext (pos);
			if (pItem->m_nType == ITEM_Folder)
				ppFolderItems[nIdxFold++] = pItem;
			if (pItem->m_nType == ITEM_Video || pItem->m_nType == ITEM_Audio || pItem->m_nType == ITEM_Image)
				ppFileItems[nIdxFile++] = pItem;
		}
		if (ppFolderItems != NULL)
			qsort(ppFolderItems, nFolderNum, sizeof(CListItem *), compare_filename);
		if (ppFileItems != NULL)
			qsort(ppFileItems, nFileNum, sizeof(CListItem *), compare_filename);
	}

	m_lstItem.RemoveAll ();
	if (pItemHome != NULL)
	{
		pItemHome->m_nIndex = 0;
		m_lstItem.AddTail (pItemHome);
	}
	int i = 0;
	for (i = 0; i < nFolderNum; i++)
	{
		ppFolderItems[i]->m_nIndex = m_lstItem.GetCount ();
		m_lstItem.AddTail (ppFolderItems[i]);
	}
	for (i = 0; i < nFileNum; i++)
	{
		ppFileItems[i]->m_nIndex = m_lstItem.GetCount ();
		m_lstItem.AddTail (ppFileItems[i]);
	}
	if (pItemNewFolder != NULL)
	{
		pItemNewFolder->m_nIndex = m_lstItem.GetCount ();
		m_lstItem.AddTail (pItemNewFolder);
	}
	if (pItemNewFile != NULL)
	{
		pItemNewFile->m_nIndex = m_lstItem.GetCount ();
		m_lstItem.AddTail (pItemNewFile);
	}

	if (ppFolderItems != NULL)
		delete []ppFolderItems;
	if (ppFileItems != NULL)
		delete []ppFileItems;

	return true;
}

int __cdecl CListView::compare_filename(const void *arg1, const void *arg2)
{
	CListItem * pItem1 = *(CListItem **)arg1;
	CListItem * pItem2 = *(CListItem **)arg2;
	return _tcsicmp (pItem1->m_pName, pItem2->m_pName);
}

CListItem *	CListView::GetFocusItem (int nX, int nY)
{
	if (m_lstItem.GetCount () <= 0 || m_nBmpCols == 0)
		return NULL;

	RECT rcView;
	GetClientRect (m_hWnd, &rcView);

	CListItem * pItem = NULL;
	int nLine = (m_nBmpYPos + nY) / ITEM_HEIGHT;
	int nCol = nX / (rcView.right / m_nBmpCols);
	int nIndex = nLine * m_nBmpCols + nCol;
	
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

bool CListView::SaveItemThumb (TCHAR * pFolder)
{
	TCHAR szThumb[1024];
	_tcscpy (szThumb, m_szThumb);
	_tcscat (szThumb, pFolder);
	for (int i = _tcslen (m_szThumb); i < _tcslen (szThumb); i++)
	{
		if (szThumb[i] == _T('\\') || szThumb[i] == _T(':'))
			szThumb[i] = _T('-');
	}
	_tcscat (szThumb, _T(".tmb"));

	CListStore store (m_hInst);
	return store.Save (szThumb, &m_lstItem);
}

bool CListView::LoadItemThumb (TCHAR * pFolder)
{
	TCHAR szThumb[1024];
	_tcscpy (szThumb, m_szThumb);
	_tcscat (szThumb, pFolder);
	for (int i = _tcslen (m_szThumb); i < _tcslen (szThumb); i++)
	{
		if (szThumb[i] == _T('\\') || szThumb[i] == _T(':'))
			szThumb[i] = _T('-');
	}
	_tcscat (szThumb, _T(".tmb"));

	CListStore store (m_hInst);
	CObjectList<CListItem>	lstThumb;
	store.Load (szThumb, &lstThumb);

	CListItem * pItemFile = NULL;
	CListItem * pItemThumb = NULL;
	NODEPOS	posThumb = NULL;
	NODEPOS posItem = m_lstItem.GetHeadPosition ();
	while (posItem != NULL)
	{
		pItemFile = m_lstItem.GetNext (posItem);
		if (pItemFile->m_nType == ITEM_Video || pItemFile->m_nType == ITEM_Audio || pItemFile->m_nType == ITEM_Image)
		{
			posThumb = lstThumb.GetHeadPosition ();
			while (posThumb != NULL)
			{
				pItemThumb = lstThumb.GetNext (posThumb);
				if (pItemThumb->m_pName == NULL)
					continue;
				if (!_tcscmp (pItemThumb->m_pName, pItemFile->m_pName))
				{
					//YYLOGI ("It was same. Name %s", pItemFile->m_pName);
					pItemThumb->MoveTo (pItemFile);
					break;
				}
			}
		}
	}

	CListItem * pItem = lstThumb.RemoveHead ();
	while (pItem != NULL)
	{
		delete pItem;
		pItem = lstThumb.RemoveHead ();
	}
	return true;
}

bool CListView::DrawItemRect (HDC hDC, CListItem * pItem)
{
	if (pItem == NULL)
		return false;

	RECT rcView;
	GetClientRect (m_hWnd, &rcView);

	int nItemWidth = rcView.right / m_nBmpCols;
	int nX = pItem->m_nIndex % m_nBmpCols * nItemWidth;
	int nY = pItem->m_nIndex / m_nBmpCols * ITEM_HEIGHT;

	if (nY + ITEM_HEIGHT < m_nBmpYPos)
		return false;
	else if (nY > m_nBmpYPos + rcView.bottom)
		return false;

	int		nH = nY - m_nBmpYPos;
	RECT	rcItem;
	if (nY < m_nBmpYPos)
	{
		int nH = ITEM_HEIGHT - (m_nBmpYPos - nY);
		SetRect (&rcItem, nX + 8, 0, nX + nItemWidth - 8, nH - 4);
	}
	else if (nY >= m_nBmpYPos && nY < m_nBmpYPos + rcView.bottom)
		SetRect (&rcItem, nX + 8, nH + 4, nX + nItemWidth - 8, nH + ITEM_HEIGHT - 4);
	else
		SetRect (&rcItem, nX + 8, nH + 4, nX + nItemWidth - 8, rcView.bottom);
	HPEN hPen = ::CreatePen (PS_SOLID, 2, RGB (200, 200, 200));
	HPEN hOldPen = (HPEN) SelectObject (hDC, hPen);
	MoveToEx (hDC, rcItem.left, rcItem.top, NULL);
	LineTo (hDC, rcItem.left, rcItem.bottom);
	LineTo (hDC, rcItem.right, rcItem.bottom);
	LineTo (hDC, rcItem.right, rcItem.top);
	LineTo (hDC, rcItem.left, rcItem.top);
	SelectObject (hDC, hOldPen);
	DeleteObject (hPen);

	LPBYTE pBmpBuff = NULL;
	int nW = rcItem.right - rcItem.left;
	nH = rcItem.bottom - rcItem.top;
	if (nW <= 0 || nH <= 0)
		return true;
	HBITMAP hBmpItem = yyBmpCreate (hDC, nW, nH, &pBmpBuff, 0);
	HDC hItemDC = CreateCompatibleDC (hDC);
	HBITMAP hOld = (HBITMAP)SelectObject (hItemDC, hBmpItem);
	BitBlt (hItemDC, 0, 0, nW, nH, hDC, rcItem.left, rcItem.top, SRCCOPY);
	unsigned char * pRGB = pBmpBuff;
	for (int i = 0; i < nH; i++)
	{
		for (int j = 0; j < nW; j++)
		{
			*pRGB = *pRGB++ * 4 / 5;
			*pRGB = *pRGB++ * 4 / 5;
			*pRGB = *pRGB++ * 4 / 5;
			*pRGB++;
		}
	}
	BitBlt (hDC, rcItem.left, rcItem.top, nW, nH, hItemDC, 0, 0, SRCCOPY);
	SelectObject (hItemDC, hOld);
	DeleteObject (hBmpItem);
	DeleteDC (hItemDC);

	return true;
}

bool CListView::DrawItemThumb (CListItem * pItem)
{
	if (pItem == NULL || pItem->m_hThumb == NULL)
		return false;

	int		nItemWidth = m_rcView.right / m_nBmpCols;
	int		x = (pItem->m_nIndex % m_nBmpCols) * nItemWidth + (ITEM_WIDTH - ICON_WIDTH) / 2 + (nItemWidth - ITEM_WIDTH) / 2;
	int		y = (pItem->m_nIndex / m_nBmpCols) * ITEM_HEIGHT + ICON_OFF_Y;
	BITMAP	bmpInfo;
	GetObject (pItem->m_hThumb, sizeof (BITMAP), &bmpInfo);
	int	*	pBmpBuff = NULL;
	int *	pThmBuff = (int *)pItem->m_pBuff;
	int		nStep = bmpInfo.bmWidth / ICON_WIDTH;
	int		nArc = 4;
	for (int h = 0; h < ICON_HEIGHT; h++)
	{
		pBmpBuff = (int *)(m_hBmpBuff + (y + h) * m_rcView.right * 4 + x * 4);
		pThmBuff = (int *)(pItem->m_pBuff + h * bmpInfo.bmWidth * 4 * nStep);
		for (int w = 0; w < ICON_WIDTH; w++)
		{
			if (h < nArc)
			{
				if (w < nArc - h)
				{
					pBmpBuff ++;
					continue;
				}
				else if (w > ICON_WIDTH - nArc + h)
				{
					pBmpBuff ++;
					continue;
				}
			}
			else if (h > ICON_HEIGHT - nArc)
			{
				if (w < nArc - (ICON_HEIGHT - h))
				{
					pBmpBuff ++;
					continue;
				}
				else if (w > ICON_WIDTH - nArc + (ICON_HEIGHT - h))
				{
					pBmpBuff ++;
					continue;
				}
			}

			*pBmpBuff++ = *pThmBuff;
			pThmBuff += nStep;
		}
	}
	return true;
}

LRESULT	CListView::MsgProc (HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		return OnCommand (uMsg, wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown (uMsg, wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp (uMsg, wParam, lParam);
	case WM_RBUTTONUP:
		return OnRButtonUp (uMsg, wParam, lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove (uMsg, wParam, lParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel (uMsg, wParam, lParam);
	case WM_VSCROLL:
		return OnVScroll (uMsg, wParam, lParam);
	case WM_KEYUP:
		return OnKeyUp (uMsg, wParam, lParam);
	case WM_CHAR:
		return OnChar (uMsg, wParam, lParam);
	case WM_DROPFILES:
		return OnDropFiles (uMsg, wParam, lParam);
	case WM_SIZE:
		return OnSize (uMsg, wParam, lParam);
	case WM_TIMER:
		return OnTimer (uMsg, wParam, lParam);
	case WM_ERASEBKGND:
		return OnEraseBG (uMsg, wParam, lParam);
	case WM_PAINT:
		return OnPaint (uMsg, wParam, lParam);
	default:
		break;
	}
	return S_FALSE;
}

LRESULT CListView::OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_FALSE;
}

LRESULT CListView::OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_bLBtnDown = true;
	return S_FALSE;
}

LRESULT CListView::OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!m_bLBtnDown)
		return S_OK;
	m_bLBtnDown = false;

	int nX = LOWORD (lParam);
	int nY = HIWORD (lParam);
	CListItem * pItem = GetFocusItem (nX, nY);
	if (pItem == NULL)
		return S_FALSE;

	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	m_nActvType = pItem->m_nType;
	int nItemWidth = rcView.right / m_nBmpCols;
	nX = pItem->m_nIndex % m_nBmpCols * nItemWidth;
	nY = pItem->m_nIndex / m_nBmpCols * ITEM_HEIGHT;
	int	nH = nY - m_nBmpYPos;
	if (nY < m_nBmpYPos)
		SetRect (&m_rcLastItem, nX + 8, 0, nX + nItemWidth - 8, ITEM_HEIGHT - (m_nBmpYPos - nY) - 4);
	else if (nY >= m_nBmpYPos && nY < m_nBmpYPos + rcView.bottom)
		SetRect (&m_rcLastItem, nX + 8, nH + 4, nX + nItemWidth - 8, nH + ITEM_HEIGHT - 4);
	else
		SetRect (&m_rcLastItem, nX + 8, nH + 4, nX + nItemWidth - 8, rcView.bottom);
	if (m_pSelItem != pItem)
	{
		bool bHadSel = m_pSelItem == NULL ? false : true;
		m_pSelItem = pItem;
		OnSelItemChanged ();
		if (bHadSel)
			return S_OK;
	}
	TCHAR szFolder[1024];
	if (pItem->m_nType == ITEM_Folder)
	{
		m_aBmpPos[m_nFolderLevel] = m_nBmpYPos;
		m_nFolderLevel++;

		_tcscpy (szFolder, m_szFolder);
		if (_tcslen (szFolder) > 0)
			_tcscat (szFolder, _T("\\"));
		_tcscat (szFolder, pItem->m_pName);
		if (FillItem (szFolder))
		{
			SetRectEmpty (&m_rcView);
			if (CreateDispBmp ())
			{
				ShowDispBmp (NULL);
				Start ();
			}
		}
	}
	else if (pItem->m_nType == ITEM_Home)
	{
		_tcscpy (szFolder, m_szFolder);
		TCHAR * pPos = _tcsrchr (szFolder, _T('\\'));
		if (pPos == NULL)
			_tcscpy (szFolder, _T(""));
		else
			*pPos = 0;
		if (FillItem (szFolder))
		{
			SetRectEmpty (&m_rcView);
			if (CreateDispBmp ())
			{
				m_nFolderLevel--;
				if (m_nFolderLevel >= 0)
					m_nBmpYPos = m_aBmpPos[m_nFolderLevel];
				if (m_nFolderLevel < 0)
				{
					m_nFolderLevel = 0;
					m_aBmpPos[m_nFolderLevel] = 0;
				}

				ShowDispBmp (NULL);
				Start ();
			}
		}
	}
	else if (pItem->m_nType == ITEM_Exit)
	{
		PostMessage (m_hWnd, WM_CLOSE, 0, 0);
	}
	else if (pItem->m_nType == ITEM_Video || pItem->m_nType == ITEM_Audio || pItem->m_nType == ITEM_Image)
	{
		Pause ();
		PostMessage (m_hWnd, WM_LIST_PlayFile, (WPARAM)pItem, (LPARAM)&m_lstItem);
	}
	else if (pItem->m_nType == ITEM_NewFile)
	{
		OnNewItemFile ();
	}
	else if (pItem->m_nType == ITEM_NewFolder)
	{
		OnNewItemFolder ();
	}
	
	return S_FALSE;
}

LRESULT CListView::OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_FALSE;
}

LRESULT CListView::OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);

	if (wParam == MK_LBUTTON)
	{
		if (m_nBmpHeight <= rcView.bottom)
			return S_OK;

		int nYPos = HIWORD (lParam);
		if (m_nMoveYPos > 0 && m_nMoveYPos != nYPos)
		{
			m_nBmpYPos = m_nBmpYPos + (m_nMoveYPos - nYPos);
			if (m_nBmpYPos < 0)
				m_nBmpYPos = 0;
			else if (m_nBmpYPos > m_nBmpHeight - rcView.bottom)
				m_nBmpYPos = m_nBmpHeight - rcView.bottom;
			InvalidateRect (m_hWnd, NULL, FALSE);

			GetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo);
			m_sbInfo.nPos = m_nBmpYPos;
			SetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo, TRUE);
		}
		m_nMoveYPos = nYPos;
	}
	else
	{
		int nX = LOWORD (lParam);
		int nY = HIWORD (lParam);
		CListItem * pSelItem = GetFocusItem (nX, nY);
		if (pSelItem != NULL && pSelItem != m_pSelItem)
		{
			m_pSelItem = pSelItem;
			OnSelItemChanged ();
			InvalidateRect (m_hWnd, NULL, FALSE);
		}
	}

	return S_FALSE;
}

LRESULT CListView::OnMouseWheel (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);

	if (HIWORD (wParam) == WHEEL_DELTA)
		m_nBmpYPos = m_nBmpYPos - 20;
	else
		m_nBmpYPos = m_nBmpYPos + 20;
	if (m_nBmpYPos < 0)
		m_nBmpYPos = 0;
	else if (m_nBmpYPos > m_nBmpHeight - rcView.bottom)
		m_nBmpYPos = m_nBmpHeight - rcView.bottom;
	GetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo);
	m_sbInfo.nPos = m_nBmpYPos;
	SetScrollInfo (m_hWnd, SB_VERT, &m_sbInfo, TRUE);
	InvalidateRect (m_hWnd, NULL, FALSE);
	return S_FALSE;
}

LRESULT CListView::OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_FALSE;
}

LRESULT CListView::OnChar (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_FALSE;
}

LRESULT CListView::OnDropFiles (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_FALSE;
}

LRESULT CListView::OnSize (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (CreateDispBmp ())
	{
		InvalidateRect (m_hWnd, NULL, FALSE);
		Start ();
	}
	return S_FALSE;
}

LRESULT CListView::OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_FALSE;
}

LRESULT CListView::OnVScroll (UINT uMsg, WPARAM wParam, LPARAM lParam)
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
	return S_FALSE;
}


LRESULT	CListView::OnEraseBG (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_OK;
}

LRESULT CListView::OnPaint (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_hBmpDC == NULL || m_hBmpList == NULL)
		return S_FALSE;

	CAutoLock lock (&m_mtBmp);
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(m_hWnd, &ps);
	m_hBmpOld = (HBITMAP)SelectObject (m_hBmpDC, m_hBmpList);
	BitBlt (hDC, 0, 0, rcView.right, rcView.bottom, m_hBmpDC, 0, m_nBmpYPos, SRCCOPY);
	SelectObject (m_hBmpDC, m_hBmpOld);
	if (m_pSelItem != NULL)
		DrawItemRect (hDC, m_pSelItem);
	EndPaint(m_hWnd, &ps);

	return S_FALSE;
}

bool CListView::Start (void)
{
	if (m_nStatus < YYTHRD_Run)
		m_nStatus = YYTHRD_Run;
	if (m_hThread == NULL)
		yyThreadCreate (&m_hThread, NULL, GetThumbProc, this, 0);
	return true;
}

bool CListView::Pause (void)
{
	m_nStatus = YYTHRD_Pause;
	while (m_bWorking)
		Sleep (10);
	return true;
}

bool CListView::Stop (void)
{
	m_nStatus = YYTHRD_Stop;
	while (m_hThread != NULL)
		Sleep (10);
	return true;
}

int CListView::GetThumbProc (void * pParam)
{
	CListView * pView = (CListView *)pParam;
	return pView->GetThumbLoop ();
}

int CListView::GetThumbLoop (void)
{
	yyThreadSetPriority (m_hThread, YY_THREAD_PRIORITY_BELOW_NORMAL);
	memset (&m_sThumbInfo, 0, sizeof (m_sThumbInfo));
	m_sThumbInfo.nInfoType = YYINFO_Get_Thumbnail | YYINFO_Get_NoBlack;
	m_sThumbInfo.nThumbWidth = ICON_WIDTH * 4;
	m_sThumbInfo.nThumbHeight = ICON_HEIGHT * 4;
	m_sThumbInfo.bKeepAspectRatio = true;
	
	while (m_nStatus > YYTHRD_Stop)
	{
		m_bWorking = false;
		if (m_nStatus == YYTHRD_Pause)
		{
			Sleep (10);
			continue;
		}
		m_bWorking = true;
		if (m_lstItem.GetCount () <= 0 || m_nBmpCols <= 0)
			break; 

		CAutoLock lock (&m_mtList);
		BITMAP bmpInfo;
		GetObject (m_hBmpList, sizeof (BITMAP), &bmpInfo);
		int nItemWidth = bmpInfo.bmWidth / m_nBmpCols;
		CListItem * pItem = NULL;
		NODEPOS	pos = m_lstItem.GetHeadPosition ();
		while (pos != NULL)
		{
			Sleep (10);
			pItem = m_lstItem.GetNext (pos);
			if (pItem->m_nType != ITEM_Video && pItem->m_nType != ITEM_Audio && pItem->m_nType != ITEM_Image)
				continue;
			if (pItem->m_hThumb != NULL)
				continue;

			m_sThumbInfo.nPos = 0;
			m_sThumbInfo.nTryTime = 3000;
			if (pItem->m_pPath != NULL)
			{
				pItem->m_nPos = GetThumbItem (pItem->m_pPath, &pItem->m_hThumb);
			}
			else
			{
				TCHAR szFile[1024];
				_tcscpy (szFile, m_szFolder);
				_tcscat (szFile, _T("\\"));
				_tcscat (szFile, pItem->m_pName);
				pItem->m_nPos = GetThumbItem (szFile, &pItem->m_hThumb);
			}

			if (pItem->m_hThumb != NULL)
			{
				CAutoLock lock (&m_mtBmp);
				pItem->m_pBuff = m_sThumbInfo.pBmpBuff;
				pItem->m_nPos = m_sThumbInfo.nPos;
				pItem->m_nWidth = m_sThumbInfo.nVideoWidth;
				pItem->m_nHeight = m_sThumbInfo.nVideoHeight;
				pItem->m_nVNum = m_sThumbInfo.nVNum;
				pItem->m_nVDen = m_sThumbInfo.nVDen;
				pItem->m_bModified = true;

				DrawItemThumb (pItem);
				InvalidateRect (m_hWnd, NULL, FALSE);
			}
			if (m_nStatus < YYTHRD_Run)
				break;
		}
		if (pos == NULL)
			break;
	}
	m_bWorking = false;
	m_nStatus = YYTHRD_Stop;
	m_hThread = NULL;
	return YY_ERR_NONE;
}

int CListView::GetThumbItem (TCHAR * pFile, HBITMAP * ppBmp)
{
	m_sThumbInfo.nInfoType |= YYINFO_OPEN_ExtSource;
	TCHAR * pSource = (TCHAR *)m_pExtSrc->GetExtData (YY_EXTDATA_Mux, pFile);
	if (pSource == NULL)
		return -1;

	*ppBmp = (HBITMAP)m_pMedia->GetThumb ((const TCHAR *)pSource, &m_sThumbInfo);

	return m_sThumbInfo.nPos;
}
