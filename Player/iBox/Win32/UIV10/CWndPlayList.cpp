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

#include "CBaseFile.h"
#include "CRegMng.h"
#include "USystemFunc.h"

#include "yyLog.h"

#pragma warning (disable : 4996)

CWndPlayList::CWndPlayList(HINSTANCE hInst, CMediaEngine * pMedia)
	: CWndBase (hInst)
	, m_pMedia (pMedia)
	, m_pExtSrc (NULL)
	, m_pLangText (NULL)
	, m_lvType (LIST_VIEW_MAX)
	, m_hMenuPopup (NULL)
	, m_hMenuItem (NULL)
	, m_hMenuBox (NULL)
	, m_hMenuBoxItem (NULL)
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
	, m_hBmpNewFolder (NULL)
	, m_hBmpNewFile (NULL)
	, m_hDCBmp (NULL)
	, m_hDCItem (NULL)
	, m_hPenLine (NULL)
	, m_nBmpHeight (0)
	, m_nBmpYPos (0)
	, m_nMoveYPos (0)
	, m_bMoving (false)
	, m_bMoved (false)
	, m_hThumbBmp (NULL)
	, m_pThumbBuff (NULL)
	, m_nSteps (10)
	, m_nDrawStep (0)
	, m_dwThumbTimer (0)
	, m_nAnimateType (ITEM_Unknown)
	, m_hVideoBmp (NULL)
	, m_pVideoBmpBuff (NULL)
	, m_pWndEdit (NULL)
	, m_nTimerOnChar (0)
	, m_pFavorItem (NULL)
	, m_pCurItem (NULL)
	, m_pSelectedItem (NULL)
	, m_bFodlerChanged (false)
	, m_nBoxNum (0)
	, m_bFavorModified (false)
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

	memset (m_szFolder, 0, sizeof (m_szFolder));
	memset (m_szFolderBox, 0, sizeof (m_szFolderBox));
	memset (m_szFolderFold, 0, sizeof (m_szFolderFold));

	_tcscpy (m_szRoot, _T(""));
	TCHAR * pRoot = CRegMng::g_pRegMng->GetTextValue (_T("Root"));
	if (_tcslen (pRoot) == 0)
		_tcscpy (m_szRoot, pRoot);
	TCHAR * pFolder = CRegMng::g_pRegMng->GetTextValue (_T("Folder"));
	if (_tcslen (pFolder) == 0)
		_tcscpy (m_szFolderFold, m_szRoot);
	else
		_tcscpy (m_szFolderFold, pFolder);
	_tcscpy (m_szPlayFile, _T(""));

	memset (m_szBoxPath, 0, sizeof (m_szBoxPath));
	memset (m_szBoxName, 0, sizeof (m_szBoxName));
	TCHAR * pBoxPath = CRegMng::g_pRegMng->GetTextValue (_T("BoxPath"));
	if (_tcslen (pBoxPath) > 0)
		_tcscpy (m_szBoxPath, pBoxPath);
	m_nBoxNum = CRegMng::g_pRegMng->GetIntValue (_T("BoxNum"), m_nBoxNum);

	memset (m_szPWText, 0, sizeof (m_szPWText));

	SetRectEmpty (&m_rcThumb);
	SetRectEmpty (&m_rcItem);

	m_nDrawStepTime = 25;
#ifdef _CPU_MSB2531
	m_nDrawStepTime = 5;
#endif // _CPU_MSB2531

	m_dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL;
	memset (&m_sbInfo, 0, sizeof (m_sbInfo));
	m_sbInfo.cbSize = sizeof (m_sbInfo);
	m_sbInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
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

	if (m_hMenuPopup != NULL)
		DestroyMenu (m_hMenuPopup);
	m_hMenuPopup = NULL;

	ReleaseList ();
	ReleaseDCBmp ();

	SaveMyFavor ();
	DeleteItem (m_pFavorItem);

	YY_DEL_P (m_pExtSrc);

	CRegMng::g_pRegMng->SetTextValue (_T("Root"), m_szRoot);
	if (m_lvType == LIST_VIEW_Folder)
		CRegMng::g_pRegMng->SetTextValue (_T("Folder"), m_szFolder);
	else
		CRegMng::g_pRegMng->SetTextValue (_T("Folder"), m_szFolderFold);
	CRegMng::g_pRegMng->SetTextValue (_T("BoxPath"), m_szBoxPath);
	CRegMng::g_pRegMng->SetIntValue (_T("BoxNum"), m_nBoxNum);

	if (m_lvType == LIST_VIEW_MyBox)
		m_lvType = LIST_VIEW_Folder;
	CRegMng::g_pRegMng->SetIntValue (_T("ViewType"), m_lvType);
}

bool CWndPlayList::CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG, bool bFillItems)
{
	if (!CWndBase::CreateWnd (hParent, rcView, clrBG))
		return false;

	GetClientRect (m_hWnd, &m_rcClient);
	m_rcClient.right = m_rcClient.right & ~1;

	UpdateMenuLang ();

	if (bFillItems)
	{
		LV_TYPE lvType = (LV_TYPE)CRegMng::g_pRegMng->GetIntValue (_T("ViewType"), LIST_VIEW_Folder);
		SetViewType (lvType);
	}
	return true;
}

bool CWndPlayList::SetViewType (LV_TYPE nType)
{
	if (m_lvType == nType)
		return false;

	if (m_lvType == LIST_VIEW_Folder && _tcslen (m_szFolder) > 0)
		_tcscpy (m_szFolderFold, m_szFolder);
	else if (m_lvType == LIST_VIEW_MyBox && _tcslen (m_szFolder) > 0)
		_tcscpy (m_szFolderBox, m_szFolder);

	m_lvType = nType;
	m_nAnimateType = ITEM_Unknown;
	_tcscpy (m_szFolder, _T(""));
	if (m_lvType == LIST_VIEW_Folder)
		FillMediaItems (m_szFolderFold);
	else if (m_lvType == LIST_VIEW_Favor)
		FillFavorItems (m_pCurItem);
	else if (m_lvType == LIST_VIEW_MyBox)
	{
		FillMyBoxItems (m_szFolderBox);
		if (m_nBoxNum <= 0)
			AddNewBox ();
	}

	if (m_lvType == LIST_VIEW_MyBox)
	{
		CBaseKey::g_nUsePW = 1;
	}
	else
	{
		memset (CBaseFile::g_szKey, 0, sizeof (CBaseFile::g_szKey));
		CBaseKey::g_nUsePW = 0;
	}

	return true;
}

void CWndPlayList::UpdateMenuLang (void)
{
	if (m_hMenuPopup != NULL)
		DestroyMenu (m_hMenuPopup);

	if (m_pLangText->GetLang () == YYLANG_CHN)
		m_hMenuPopup = LoadMenu (m_hInst, MAKEINTRESOURCE(IDR_MENU_POPUP_CHN));
	else
		m_hMenuPopup = LoadMenu (m_hInst, MAKEINTRESOURCE(IDR_MENU_POPUP));
	m_hMenuItem = GetSubMenu (m_hMenuPopup, 0);
	m_hMenuBox = GetSubMenu (m_hMenuPopup, 1);
	m_hMenuBoxItem = GetSubMenu (m_hMenuPopup, 2);

}

LRESULT CWndPlayList::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MEDIA_Item * pItem = NULL;

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

	case WM_TIMER:
		return OnTimer (uMsg, wParam, lParam);

	case WM_KEYUP:
		return OnKeyUp (uMsg, wParam, lParam);
	case WM_CHAR:
		return OnChar (uMsg, wParam, lParam);

	case WM_PAINT:
		return OnPaint (uMsg, wParam, lParam);

	case WM_ERASEBKGND:
		return S_OK;

	case WM_CLOSE:
		if (m_pWndEdit != NULL)
		{
			SendMessage (m_pWndEdit->GetWnd (), WM_CLOSE, 0, 0);
			delete m_pWndEdit;
			m_pWndEdit = NULL;
		}
		break;

	default:
		break;
	}

	return	CWndBase::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

void CWndPlayList::ItemSelected (MEDIA_Item * pItem)
{
	if (pItem == NULL)
		return;

	if (pItem->nType != ITEM_NewFolder && m_pWndEdit != NULL)
		m_pWndEdit->ShowWnd (SW_HIDE);

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
		if (m_lvType == LIST_VIEW_Favor)
		{
			m_pCurItem = m_pCurItem->pParent;
			ReleaseList ();
			FillFavorItems (m_pCurItem);
		}
		else
		{
			_tcscpy (szFolder, m_szFolder);
			TCHAR * pDir = _tcsrchr (szFolder, _T('\\'));
			if (pDir != NULL)
			{
				TCHAR * pFirstDir = _tcschr (szFolder, _T('\\'));
				if (pFirstDir == pDir)
					_tcscpy (szFolder, _T(""));
				else
					*pDir = 0;
			}
			else
			{
				_tcscpy (szFolder, _T(""));
			}
			if (m_lvType == LIST_VIEW_Folder)
				FillMediaItems (szFolder);
			else
				FillMyBoxItems (szFolder);
		}
	}
	else if (pItem->nType == ITEM_Folder)
	{
		if (m_lvType == LIST_VIEW_Favor)
		{
			MEDIA_Item * pSelItem = NULL;
			CObjectList<MEDIA_Item> * pList = m_pCurItem->pChildList;
			POSITION pos = pList->GetHeadPosition ();
			while (pos != NULL)
			{
				pSelItem = pList->GetNext (pos);
				if (pSelItem->nType == ITEM_Folder)
				{
					if (!_tcscmp (pSelItem->pName, pItem->pName))
					{
						m_pCurItem = pSelItem;
						break;
					}
				}
			}

			ReleaseList ();
			FillFavorItems (m_pCurItem);
		}
		else
		{
			_tcscpy (szFolder, m_szFolder);
			if (_tcslen (szFolder) > 0)
				_tcscat (szFolder, _T("\\"));
			_tcscat (szFolder, pItem->pName);
			if (m_lvType == LIST_VIEW_Folder)
				FillMediaItems (szFolder);
			else
				FillMyBoxItems (szFolder);
		}
	}
	else if (pItem->nType == ITEM_Audio || pItem->nType == ITEM_Video)
	{
		if (pItem->pPath == NULL)
		{
			_tcscpy (m_szPlayFile, m_szFolder);
			if (_tcslen (m_szPlayFile) > 0)
				_tcscat (m_szPlayFile, _T("\\"));
			_tcscat (m_szPlayFile, pItem->pName);
		}
		else
		{
			_tcscpy (m_szPlayFile, pItem->pPath);
		}
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
	else if (pItem->nType == ITEM_NewFolder)
	{
		RECT rcEdit;
		int nLeft = (pItem->nIndex % m_nCols) * m_nItemWidth;
		int nTop = (pItem->nIndex / m_nCols - m_nBmpYPos / m_nItemHeight) * m_nItemHeight - m_nBmpYPos % m_nItemHeight;
		nTop = nTop + m_nIconOffsetY + m_nIconHeight + 8;
		SetRect (&rcEdit, nLeft, nTop, nLeft + m_nItemWidth, nTop + 32);
		if (m_pWndEdit == NULL)
		{
			m_pWndEdit = new CWndBase (m_hInst);
			m_pWndEdit->CreateWnd (m_hWnd, rcEdit, RGB (200, 200, 200));
			::InvalidateRect (m_pWndEdit->GetWnd (), NULL, TRUE);
		}
		else
		{
			m_pWndEdit->ShowWnd (SW_SHOW);
			MoveWindow (m_pWndEdit->GetWnd (), nLeft, nTop, m_nItemWidth, 32, TRUE);
		}
		return;
	}
	else if (pItem->nType == ITEM_NewFile)
	{
		TCHAR	szPath[8192];
		_tcscpy (szPath, _T("*.*"));
		OPENFILENAME	ofn;
		memset( &(ofn), 0, sizeof(ofn));
		ofn.lStructSize	= sizeof(ofn);
		ofn.hInstance = m_hInst;
		ofn.hwndOwner = m_hWnd;
		ofn.lpstrFilter = TEXT("Media File (*.*)\0*.*\0");	
		ofn.lpstrFile = szPath;
		ofn.nMaxFile = sizeof (szPath);
		ofn.lpstrTitle = TEXT("Import Media File");
		ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT;		
		if (!GetOpenFileName(&ofn))
			return;

		Pause ();

		MEDIA_Item *	pNewItem = NULL;
		MEDIA_Item *	pAddItem = NULL;
		POSITION		pos = NULL;
		char *			pExt = NULL;
		TCHAR	szFile[1024];
		TCHAR * pPath = szPath;
		TCHAR * pName = szPath + _tcslen (pPath) + 1;
		if (*pName == 0)
		{
			pName = _tcsrchr (szPath, _T('\\'));
			*pName++ = 0;
		}
		while (*pName != 0)
		{
			_tcscpy (szFile, pPath);
			_tcscat (szFile, _T("\\"));
			_tcscat (szFile, pName);
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

			pName = pName + _tcslen (pName) + 1;
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
}
