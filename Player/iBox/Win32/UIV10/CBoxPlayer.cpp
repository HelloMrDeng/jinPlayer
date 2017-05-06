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
#include "UStringFunc.h"
#include "yyLog.h"
#include "resource.h"

#pragma warning (disable : 4996)

CBoxPlayer::CBoxPlayer(HINSTANCE hInst)
	: m_hInst (hInst)
	, m_hWnd (NULL)
	, m_pMedia (NULL)
	, m_pExtSrc (NULL)
	, m_nClickCount (0)
	, m_nClickTimer (0)
	, m_dwStyle (WS_VISIBLE)
	, m_bMediaClose (false)
	, m_bOpening (false)
	, m_bAutoPlay (false)
	, m_nTimerAudio (0)
	, m_nVRndType (YY_VRND_DDRAW)
	, m_nZoomNum (1)
	, m_nLastZoomX (0)
	, m_nLastZoomY (0)
	, m_nViewType (YY_VIEW_LIST)
	, m_pWndPanel (NULL)
	, m_pWndBar (NULL)
	, m_dwTimerPanel (0)
	, m_nLastMoveX (0)
	, m_nLastMoveY (0)
	, m_nLastHideTime (0)
	, m_pWndSubTT (NULL)
	, m_pWndList (NULL)
	, m_pListFile (NULL)
	, m_hMemDC (NULL)
	, m_hBmpThumb (NULL)
	, m_hBrushBlack (NULL)
	, m_hBrushWhite (NULL)
	, m_hMenuList (NULL)
	, m_hMenuPlay (NULL)
	, m_hMenuAudio (NULL)
	, m_hMenuPopup (NULL)
	, m_hMenuZoom (NULL)
	, m_nAudioTrackNum (0)
	, m_pLangText (NULL)
{
	m_nScreenX = GetSystemMetrics (SM_CXSCREEN);
	m_nScreenY = GetSystemMetrics (SM_CYSCREEN);
	SetRectEmpty (&m_rcView);
	_tcscpy (m_szFile, _T(""));
	_tcscpy (m_szPath, _T(""));
	m_pRegMng = new CRegMng (_T("iBoxPlayer"));
	m_pLangText = new CLangText (m_hInst);
	m_dcbVideoRnd.funcCB = VideoExtRender;
	m_dcbVideoRnd.userData = this;
}	

CBoxPlayer::~CBoxPlayer(void)
{
	if (_tcslen (m_szFile) > 0)
		m_pRegMng->SetTextValue (_T("PlayFile"), m_szFile);
	if (m_pMedia != NULL)
	{
		int nPos = m_pMedia->GetPos ();
		m_pRegMng->SetIntValue (__T("PlayPos"), nPos);
	}
	m_pRegMng->SetIntValue (_T("Language"), m_pLangText->GetLang ());
	m_pRegMng->SetIntValue (_T("RendType"), m_nVRndType);

	if (m_pWndPanel != NULL)
	{
		SendMessage (m_pWndPanel->GetWnd (), WM_CLOSE, 0, 0);
		delete m_pWndPanel;
		m_pWndPanel = NULL;
	}
	if (m_pWndBar != NULL)
	{
		SendMessage (m_pWndBar->GetWnd (), WM_CLOSE, 0, 0);
		delete m_pWndBar;
		m_pWndBar = NULL;
	}
	if (m_pWndSubTT != NULL)
	{
		SendMessage (m_pWndSubTT->GetWnd (), WM_CLOSE, 0, 0);
		delete m_pWndSubTT;
		m_pWndSubTT = NULL;
	}
	if (m_pWndList != NULL)
	{
		SendMessage (m_pWndList->GetWnd (), WM_CLOSE, 0, 0);
		delete m_pWndList;
		m_pWndList = NULL;
	}

	if (m_hMemDC != NULL)
		DeleteDC (m_hMemDC);
	m_hMemDC = NULL;

	if (m_hBrushBlack != NULL)
		DeleteObject (m_hBrushBlack);
	if (m_hBrushWhite != NULL)
		DeleteObject (m_hBrushWhite);

	if (m_hMenuList != NULL)
		DestroyMenu (m_hMenuList);
	m_hMenuList = NULL;
	if (m_hMenuPlay != NULL)
		DestroyMenu (m_hMenuPlay);
	m_hMenuPlay = NULL;
	if (m_hMenuPopup != NULL)
		DestroyMenu (m_hMenuPopup);
	m_hMenuPopup = NULL;

	m_bMediaClose = true;
	YY_DEL_P (m_pMedia);
	YY_DEL_P (m_pExtSrc);
	YY_DEL_P (m_pRegMng);

	YY_DEL_A (CBaseKey::g_pBoxPW);
	YY_DEL_A (CBaseKey::g_pChrPW);
	YY_DEL_A (CBaseKey::g_pDatPW);

	YY_DEL_P (m_pLangText);
}

bool CBoxPlayer::Create (HWND hWnd, TCHAR * pFile)
{
	m_hWnd = hWnd;
	GetWindowRect (hWnd, &m_rcView);
	SetWindowText (m_hWnd, _T("yyDemoPlayerVideoWindow"));

	m_hBrushBlack = CreateSolidBrush (RGB (0, 0, 0));
	m_hBrushWhite = CreateSolidBrush (RGB (255, 255, 255));

	if (m_pMedia == NULL)
	{
		m_pMedia = new CMediaEngine ();
		m_pMedia->Init ();
		m_pMedia->SetNotify (NotifyEvent, this);
		m_pMedia->SetView (m_hWnd, m_nVRndType);
		// m_pMedia->SetParam (YYPLAY_PID_SeekMode, (void *)YY_SEEK_AnyPosition);
	}

	RECT	rcView;
	GetClientRect (m_hWnd, &rcView);
	m_pWndPanel = new CWndPanel (m_hInst);
	SetRect (&rcView, rcView.left + 40, rcView.bottom - 110, rcView.right - 40, rcView.bottom - 30);
	m_pWndPanel->CreateWnd (m_hWnd, rcView, RGB (100, 100, 100), true);
	m_pWndPanel->SetMediaEngine (m_pMedia);
	m_pWndPanel->ShowWnd (SW_HIDE);

	m_pWndSubTT = new CWndSubTT (m_hInst);
	m_pWndSubTT->CreateWnd (m_hWnd);
	m_pWndSubTT->SetMediaEngine (m_pMedia);

	GetClientRect (m_hWnd, &rcView);
	m_pWndList = new CWndPlayList (m_hInst, m_pMedia);
	m_pWndList->SetLangText (m_pLangText);
	m_pWndList->CreateWnd (m_hWnd, rcView, RGB (60, 60, 60), true);

	m_pLangText->setLang (m_pRegMng->GetIntValue (_T("Language"), YYLANG_CHN));
	m_nVRndType = (YYRND_TYPE)m_pRegMng->GetIntValue (_T("RendType"), m_nVRndType);

	SetWindowPos (m_pWndSubTT->GetWnd (), m_pWndList->GetWnd (), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	UpdateMenuLang ();

	if (m_pWndList->GetViewType () == LIST_VIEW_Folder)
		CheckMenuRadioItem (GetSubMenu (m_hMenuList, 1), ID_VIEW_FOLDER, ID_VIEW_MYBOX, ID_VIEW_FOLDER, MF_BYCOMMAND | MF_CHECKED);
	else
		CheckMenuRadioItem (GetSubMenu (m_hMenuList, 1), ID_VIEW_FOLDER, ID_VIEW_MYBOX, ID_VIEW_FAVORITES, MF_BYCOMMAND | MF_CHECKED);

	if (pFile != NULL)
	{
		SwitchView (YY_VIEW_PLAY);
		OpenMediaFile (pFile, true);
	}

	SetFocus (m_pWndList->GetWnd ());
/*
	m_pWndList->ShowWnd (SW_HIDE);
	m_pWndBar = new CWndBar (m_hInst);
	m_pWndBar->SetEngine (m_pMedia);
	m_pWndBar->CreateWnd (m_hWnd, NULL);
*/
	return true;
}

int CBoxPlayer::OpenMediaFile (TCHAR * pFile, bool bShow)
{
	DWORD			dwID = 0;
	OPENFILENAME	ofn;

	if (m_bOpening)
		return -1;
	if (pFile == NULL)
	{		
		memset( &(ofn), 0, sizeof(ofn));
		ofn.lStructSize	= sizeof(ofn);
		ofn.hwndOwner = m_hWnd;
		ofn.lpstrFilter = TEXT("Media File (*.*)\0*.*\0");	
		if (_tcsstr (m_szFile, _T(":/")) != NULL)
			_tcscpy (m_szFile, _T("*.*"));
		ofn.lpstrFile = m_szFile;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = TEXT("Open Media File");
		ofn.Flags = OFN_EXPLORER;		
		if (!GetOpenFileName(&ofn))
			return -1;
		SwitchView (YY_VIEW_PLAY);
	}
	else
	{
		_tcscpy (m_szFile, pFile);
	}
	
	m_pWndList->Pause ();

	int nRC = PlaybackFile (m_szFile);
	if (nRC < 0)
	{
		TCHAR szMsg[256];
		_stprintf (szMsg, m_pLangText->GetText (YYTEXT_OpenReturn), m_szFile, nRC);
		MessageBox (m_hWnd, szMsg, m_pLangText->GetText (YYTEXT_Error), MB_OK);

		m_pWndList->Start ();

		return -1;
	}
	else if (bShow)
	{
		m_pWndList->ShowWnd (SW_HIDE);
	}

	return 0;
}

int CBoxPlayer::PlaybackFile (TCHAR * pFile)
{
	int		nRC = 0;
	TCHAR	szText[1024];
	TCHAR * pName = _tcsrchr (pFile, _T('\\'));
	if (pName == NULL)
		pName = _tcsrchr (pFile, _T('/'));
	pName++;
	_tcscpy (szText, m_pLangText->GetText (YYTEXT_Player));
	_tcscat (szText, _T("  "));
	_tcscat (szText, pName);
	SetWindowText (m_hWnd, szText);

	nRC = m_pMedia->SetParam (YYPLAY_PID_Prepare_Close, 0);
	m_pMedia->Close ();

	SetRectEmpty (&m_rcVideo);
	m_bOpening = true;
	m_nZoomNum = 1;
	CheckMenuRadioItem (m_hMenuZoom, ID_VIDEOZOOM_1, ID_VIDEOZOOM_16, ID_VIDEOZOOM_1, MF_BYCOMMAND | MF_CHECKED);
	if (pFile != m_szFile)
		_tcscpy (m_szFile, pFile);

	if (yyGetProtType (m_szFile) == YY_PROT_FILE)
	{
		int nFlag = YY_OPEN_SRC_READ;
		if (m_pExtSrc == NULL)
			m_pExtSrc = new CExtSource (m_hInst);
		TCHAR * pSource = (TCHAR *)m_pExtSrc->GetExtData (YY_EXTDATA_Mux, m_szFile);
		if (pSource == NULL)
			return YY_ERR_FAILED;
		nRC = m_pMedia->Open (pSource, nFlag);
	}
	else
	{
		nRC = m_pMedia->Open (m_szFile, 0);
	}
	if (nRC < 0)
	{
		m_bOpening = false;
		return nRC;
	}

	_tcscpy (m_szFile, pFile);

	return 0;
}

int CBoxPlayer::PlayNextFile (bool bNext)
{
	if (m_pListFile == NULL)
		return YY_ERR_FAILED;

	TCHAR *	pPlayName = _tcsrchr (m_szFile, _T('\\'));
	if (pPlayName == NULL)
		return YY_ERR_FAILED;
	pPlayName++;

	MEDIA_Item *				pItem = NULL;
	MEDIA_Item *				pPlayItem = NULL;
	CObjectList<MEDIA_Item>		lstFile;
	POSITION pos = m_pListFile->GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_pListFile->GetNext (pos);
		if (pItem->nType != ITEM_Audio && pItem->nType != ITEM_Video)
			continue;
		lstFile.AddTail (pItem);
	}

	if (lstFile.GetCount () > 1)
	{
		if (bNext)
		{
			pos = lstFile.GetHeadPosition ();
			while (pos != NULL)
			{
				pItem = lstFile.GetNext (pos);
				if (!_tcscmp (pItem->pName, pPlayName))
				{
					if (pItem == lstFile.GetTail ())
						pPlayItem = lstFile.GetHead ();
					else
						pPlayItem = lstFile.GetNext (pos);
				}
			}
		}
		else
		{
			pos = lstFile.GetTailPosition ();
			while (pos != NULL)
			{
				pItem = lstFile.GetPrev (pos);
				if (!_tcscmp (pItem->pName, pPlayName))
				{
					if (pItem == lstFile.GetHead ())
						pPlayItem = lstFile.GetTail ();
					else
						pPlayItem = lstFile.GetPrev (pos);
				}
			}
		}
	}
	else
	{
		pPlayItem = lstFile.GetHead ();
	}
	lstFile.RemoveAll ();

	if (pPlayItem == NULL)
		return YY_ERR_FAILED;

	TCHAR	szFile[1024];

	if (pPlayItem->pPath == NULL)
	{
		_tcscpy (szFile, m_pWndList->GetFolder ());
		_tcscat (szFile, _T("\\"));
		_tcscat (szFile, pPlayItem->pName);
	}
	else
	{
		_tcscpy (szFile, pPlayItem->pPath);
	}

	int nRC = PlaybackFile (szFile);
	if (nRC < 0 && bNext)
		SetTimer (m_hWnd, TIMER_PLAY_NEXTFILE, 100, NULL);

	return 0;
}

void CBoxPlayer::SwitchView (int nView)
{
	m_nViewType = nView;
	if (nView == YY_VIEW_LIST)
	{
		if (IsFullScreen ())
			ShowFullScreen ();
		m_pWndSubTT->ShowWnd (SW_HIDE);
		ShowPanel (false);
		m_pWndList->SetPlayingFile (m_szFile);
		m_pWndList->OnSize ();
		m_pWndList->ShowWnd (SW_SHOW);
		SetMenu (m_hWnd, m_hMenuList);
		m_pWndList->Start ();
	}
	else
	{
		m_pWndList->Pause ();
		m_pWndList->ShowWnd (SW_HIDE);
		m_pWndPanel->ShowWnd (SW_SHOW);
		m_pWndSubTT->ShowWnd (SW_SHOW);
		SetMenu (m_hWnd, m_hMenuPlay);
	}
}

int CBoxPlayer::ShowFullScreen (void)
{
	if (IsFullScreen ())
	{
		SetWindowLong (m_hWnd, GWL_STYLE, m_dwStyle);
		SetMenu (m_hWnd, m_hMenuPlay);
		SetWindowPos (m_hWnd, HWND_TOP, m_rcView.left, m_rcView.top, 
						m_rcView.right - m_rcView.left, m_rcView.bottom - m_rcView.top, 0);
	}
	else
	{
		m_dwStyle = GetWindowLong (m_hWnd, GWL_STYLE);
		SetWindowLong (m_hWnd, GWL_STYLE, WS_VISIBLE);
		SetMenu (m_hWnd, NULL);
		GetWindowRect (m_hWnd, &m_rcView);
		SetWindowPos (m_hWnd, HWND_TOP, 0, 0, m_nScreenX, m_nScreenY, 0);
	}
	m_pMedia->SetView (m_hWnd, m_nVRndType);
	return YY_ERR_NONE;
}

bool CBoxPlayer::IsFullScreen (void)
{
	RECT rcView;
	GetWindowRect (m_hWnd, &rcView);
	if (rcView.right == m_nScreenX && rcView.bottom == m_nScreenY)
		return true;
	else
		return false;
}

void CBoxPlayer::MovePanel (void)
{
	RECT	rcView;
	RECT	rcPanel;
	GetWindowRect (m_hWnd, &rcView);
	GetClientRect (m_pWndPanel->GetWnd (), &rcPanel);
	int nLeft = rcView.left + ((rcView.right - rcView.left) - rcPanel.right) / 2;
	int nTop = rcView.bottom - rcPanel.bottom - 40;
	SetWindowPos (m_pWndPanel->GetWnd (), NULL, nLeft, nTop, 0, 0, SWP_NOSIZE);

	m_pWndSubTT->OnSizeMove ();
}

void CBoxPlayer::ShowPanel (bool bShow)
{
	if (m_dwTimerPanel != 0)
		KillTimer (m_hWnd, TIMER_SHOW_PANEL);
	m_dwTimerPanel = 0;

	if (bShow)
	{
		m_pMedia->SetParam (YYPLAY_PID_VideoExtRnd, &m_dcbVideoRnd);
		m_pWndPanel->ShowWnd (SW_SHOW);
		if (m_pMedia->GetStatus () == YY_PLAY_Run)
			m_dwTimerPanel = SetTimer (m_hWnd, TIMER_SHOW_PANEL, PANEL_SHOW_TIME, NULL);
	}
	else
	{
		m_pMedia->SetParam (YYPLAY_PID_VideoExtRnd, NULL);
		m_pWndPanel->ShowWnd (SW_HIDE);
		m_nLastHideTime = yyGetSysTime ();
	}
}

void CBoxPlayer::UpdatePlayMenu (void)
{
	HMENU hMenu = GetSubMenu (m_hMenuPlay, 1);
	hMenu = GetSubMenu (hMenu, 4);
	if (m_nVRndType == YY_VRND_GDI)
		CheckMenuRadioItem (hMenu, ID_VIDEO_GDI, ID_VIDEO_DDRAW, ID_VIDEO_GDI, MF_BYCOMMAND | MF_CHECKED);
	else
		CheckMenuRadioItem (hMenu, ID_VIDEO_GDI, ID_VIDEO_DDRAW, ID_VIDEO_DDRAW, MF_BYCOMMAND | MF_CHECKED);

	if (m_hMenuAudio == NULL)
		return;

	int	i = 0;
	int nCount = GetMenuItemCount (m_hMenuAudio);
	for (i = nCount; i > 1; i--)
		RemoveMenu (m_hMenuAudio, i - 1, MF_BYPOSITION);

	m_nAudioTrackNum = 0;
	m_pMedia->GetParam (YYPLAY_PID_AudioTrackNum, &m_nAudioTrackNum);
	if (m_nAudioTrackNum <= 1)
	{
		CheckMenuItem (m_hMenuAudio, ID_AUDIO_AUDIO1, MF_BYCOMMAND | MF_CHECKED);
		return;
	}

	TCHAR szMenu[32];
	for (i = 1; i < m_nAudioTrackNum; i++)
	{
		_stprintf (szMenu, _T("Audio %d"), i + 1);
		AppendMenu (m_hMenuAudio, MF_BYCOMMAND | MF_STRING, ID_AUDIO_AUDIO1 + i, szMenu);
	}

	CheckMenuItem (m_hMenuAudio, ID_AUDIO_AUDIO1, MF_BYCOMMAND | MF_UNCHECKED);

	int nTrackPlay = 0;
	m_pMedia->GetParam (YYPLAY_PID_AudioTrackPlay, &nTrackPlay);
	CheckMenuItem (m_hMenuAudio, ID_AUDIO_AUDIO1 + nTrackPlay, MF_BYCOMMAND | MF_CHECKED);
}

void CBoxPlayer::UpdateMenuLang (void)
{
	if (m_hMenuList != NULL)
		DestroyMenu (m_hMenuList);
	if (m_hMenuPlay != NULL)
		DestroyMenu (m_hMenuPlay);

	if (m_pLangText->GetLang () == YYLANG_CHN)
	{
		m_hMenuList = LoadMenu (m_hInst, MAKEINTRESOURCE(IDC_MENU_LIST_CHN));
		m_hMenuPlay = LoadMenu (m_hInst, MAKEINTRESOURCE(IDC_MENU_PLAY_CHN));
	}
	else
	{
		m_hMenuList = LoadMenu (m_hInst, MAKEINTRESOURCE(IDC_MENU_LIST));
		m_hMenuPlay = LoadMenu (m_hInst, MAKEINTRESOURCE(IDC_MENU_PLAY));
	}
	m_hMenuAudio = GetSubMenu (GetSubMenu (m_hMenuPlay, 1), 0);
	m_pWndSubTT->SetSubTTMenu (GetSubMenu (GetSubMenu (m_hMenuPlay, 1), 2));

	if (m_nViewType == YY_VIEW_LIST)
		SetMenu (m_hWnd, m_hMenuList);
	else
		SetMenu (m_hWnd, m_hMenuPlay);

	if (m_hMenuPopup == NULL)
	{
		m_hMenuPopup = LoadMenu (m_hInst, MAKEINTRESOURCE(IDR_MENU_POPUP));
		m_hMenuZoom = GetSubMenu (m_hMenuPopup, 3);
	}

	m_pWndList->UpdateMenuLang ();
}

int CBoxPlayer::CheckExtLicense (char * pText, int nSize, yyVerifyLicenseText fVerify, void * pUserData)
{
	if (pText == NULL || nSize <= 0)
		return -2;

	char	szExtLicenseKey[32];
	char *	pExtLicenseText = new char[nSize+1];

	strcpy (pExtLicenseText, pText);
	strcpy (szExtLicenseKey, "yyMediaEngine");
	int nKeyLen = strlen (szExtLicenseKey);

	for (int i = 0; i < nSize; i++)
	{
		for (int j = 0; j < nKeyLen; j++)
		{
			pExtLicenseText[i] = pExtLicenseText[i] ^ szExtLicenseKey[j];
		}
	}

	if (fVerify != NULL)
		fVerify (pUserData, pExtLicenseText, nSize);

	delete []pExtLicenseText;

	return 0;
}
