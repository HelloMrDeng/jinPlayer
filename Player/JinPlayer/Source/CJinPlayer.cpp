/*******************************************************************************
	File:		CJinPlayer.cpp

	Contains:	Jin Player implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CJinPlayer.h"
#include "CBaseKey.h"
#include "CAnimation.h"

#include "CDlgOpenURL.h"
#include "CDlgFileProp.h"
#include "CDlgNewBox.h"
#include "CDlgOpenBox.h"
#include "CDlgImport.h"
#include "CAppUpdate.h"

#include "USystemFunc.h"
#include "UStringFunc.h"
#include "UBitmapFunc.h"
#include "UFFMpegFunc.h"

#include "RPlayerDef.h"
#include "yyLog.h"
#include "resource.h"

#include "stdint.h"
#include "stddef.h"

#pragma warning (disable : 4996)

CJinPlayer::CJinPlayer(HINSTANCE hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hWnd (NULL)
	, m_hMenu (NULL)
	, m_pLang (NULL)
	, m_pReg (NULL)
	, m_pKey (NULL)
	, m_pMedia (NULL)
	, m_pExtSrc (NULL)
	, m_nWndType (0)
	, m_pWndList (NULL)
	, m_pWndPlay (NULL)
	, m_hPlayBmp (NULL)
	, m_bResized (false)
{
	SetObjectName ("CJinPlayer");
	yyInitFFMpeg ();
}	

CJinPlayer::~CJinPlayer(void)
{
	if (m_pWndPlay->GetPlayItem () != NULL)
		m_pWndPlay->GetPlayItem ()->m_nPlayPos = m_pMedia->GetPos ();

	m_pWndList->Stop ();
	m_pWndPlay->Close ();

	YY_DEL_P (m_pMedia);
	YY_DEL_P (m_pExtSrc);

	YY_DEL_P (m_pWndPlay);
	YY_DEL_P (m_pWndList);

	YY_DEL_P (m_pLang);
	YY_DEL_P (m_pReg);
	YY_DEL_P (m_pKey);

	if (m_hPlayBmp != NULL)
		DeleteObject (m_hPlayBmp);
	DestroyMenu (m_hMenu);

	yyFreeFFMpeg ();
}

bool CJinPlayer::Create (HWND hWnd, TCHAR * pFile)
{
	m_hWnd = hWnd;
	m_pReg = new CRegMng (_T("JinPlayer"));
	m_pLang = new CLangText (m_hInst);
	m_pKey = new CBaseKey ();

	TCHAR szThumbPath[1024];
	yyGetDataPath (m_hInst, szThumbPath, sizeof (szThumbPath));
	_tcscat (szThumbPath, _T("thumb"));
	DWORD dwAttr = GetFileAttributes (szThumbPath);
	if (dwAttr == -1)
		CreateDirectory (szThumbPath, NULL);

	m_pMedia = new CMediaEngine ();
	m_pMedia->Init ();
	m_pExtSrc = new CExtSource (m_hInst);
	m_pWndPlay = new CWndPlay (m_hInst);
	m_pWndPlay->Create (m_hWnd, m_pMedia, m_pExtSrc);
	m_pWndList = new CWndList (m_hInst);
	m_pWndList->Create (m_hWnd, m_pMedia, m_pExtSrc);

	if (pFile == NULL)
	{
		SwitchWnd (PLAYER_WND_LIST, false);
	}
	else
	{
		m_pWndList->Stop ();
		SwitchWnd (PLAYER_WND_PLAY, false);
		if (m_pWndPlay != NULL)
			m_pWndPlay->PlaybackFile (pFile, NULL);
	}

	TCHAR * pVer = CRegMng::g_pRegMng->GetTextValue (_T("CurVersion"));
	if (_tcslen (pVer) <= 0)
		CRegMng::g_pRegMng->SetTextValue (_T("CurVersion"), _T("V10_B109"));

	UpdateLang (m_pReg->GetIntValue (_T("Language"), YYLANG_CHN));

	TCHAR	szWinText[1024];
	memset (szWinText, 0, sizeof (szWinText));
	if (m_nWndType == PLAYER_WND_LIST)
		m_pWndList->GetSelFile (szWinText, sizeof (szWinText));
	else
		_tcscpy (szWinText, m_pWndPlay->GetPlayFile ());
	SendMessage (m_hWnd, WM_PLAYER_SetWinText, (WPARAM)szWinText, 0);
	return true;
}

bool CJinPlayer::SwitchWnd (int nType, bool BAnmt)
{
	if (m_nWndType == nType)
		return true;

	m_nWndType = nType;
	if (nType == PLAYER_WND_LIST)
	{
		m_pWndList->Show (true);
		m_pWndPlay->Show (false);
		ShowScrollBar (m_hWnd, SB_VERT, TRUE);
		if (m_bResized)
		{
			CListView * pView = m_pWndList->GetView ();
			pView->CreateDispBmp ();
			m_bResized = false;
		}
		if (BAnmt && m_hPlayBmp != NULL)
		{
			HBITMAP hBmpView = NULL;
			RECT	rcBmpView;
			m_pWndList->GetViewBmp (&hBmpView, &rcBmpView, NULL);

			CAnimation anm (m_hInst);
			anm.SetForeBmp (m_hPlayBmp, NULL, NULL, 0);
			anm.SetBackBmp (hBmpView, NULL, &rcBmpView);
			anm.SetRndWnd (m_hWnd, m_pWndList->GetItemRect (), NULL);
			anm.Show (ANMT_DoorClose);
			return true;
		}
	}
	else
	{
		m_pWndList->Show (false);
		m_pWndPlay->Show (true);
		ShowScrollBar (m_hWnd, SB_VERT, FALSE);
		CListItem * pPlayItem = m_pWndPlay->GetPlayItem ();
		if (BAnmt && pPlayItem != NULL && pPlayItem->m_hThumb != NULL && pPlayItem->m_nWidth > 0 && pPlayItem->m_nHeight > 0)
		{
			CAnimation anm (m_hInst);
			anm.SetForeBmp (pPlayItem->m_hThumb, pPlayItem->m_pBuff, NULL, pPlayItem->m_nPos);

			RECT rcRnd;
			GetClientRect (m_hWnd, &rcRnd);
			int nW = rcRnd.right;
			int nH = rcRnd.bottom;
			yyAdjustVideoSize (&nW, &nH, pPlayItem->m_nWidth, pPlayItem->m_nHeight, pPlayItem->m_nVNum, pPlayItem->m_nVDen);
			SetRect (&rcRnd, (rcRnd.right - nW) / 2, (rcRnd.bottom - nH) / 2, (rcRnd.right - nW) / 2 + nW, (rcRnd.bottom - nH) / 2 + nH);
			anm.SetRndWnd (m_hWnd, m_pWndList->GetItemRect (), NULL);
			anm.Show (ANMT_DoorOpen);
			return true;
		}
	}
	InvalidateRect (m_hWnd, NULL, TRUE);
	return true;
}

int CJinPlayer::OpenMediaFile (void)
{
	TCHAR szFile[1024];
	OPENFILENAME	ofn;
	memset( &(ofn), 0, sizeof(ofn));
	ofn.lStructSize	= sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = TEXT("Media File (*.*)\0*.*\0");	
	_tcscpy (szFile, _T("*.*"));
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = TEXT("Open Media File");
	ofn.Flags = OFN_EXPLORER;		
	if (!GetOpenFileName(&ofn))
		return -1;

	if (m_nWndType == PLAYER_WND_LIST)
		SwitchWnd (PLAYER_WND_PLAY, false);

	int nRC = m_pWndPlay->PlaybackFile (szFile, NULL);
	if (nRC < 0)
	{
		TCHAR szMsg[256];
		_stprintf (szMsg, yyLangGetText (YYTEXT_OpenReturn), szFile, nRC);
		MessageBox (m_hWnd, szMsg, yyLangGetText (YYTEXT_Error), MB_OK);
		SwitchWnd (PLAYER_WND_LIST, false);
		return -1;
	}

	return 0;
}

bool CJinPlayer::CreateVideoBmp (void)
{
	if (m_pMedia->GetStatus () <= YY_PLAY_Open)
		return false;
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	RECT rcRnd;
	m_pMedia->GetParam (YYPLAY_PID_RenderArea, &rcRnd);

	LPBYTE pBuff = NULL;
	if (m_hPlayBmp != NULL)
		DeleteObject (m_hPlayBmp);
	m_hPlayBmp = yyBmpCreate (NULL, rcView.right, rcView.bottom, &pBuff, 0);

	m_pMedia->Pause ();

	YY_VIDEO_BUFF buffVideo;
	memset (&buffVideo, 0, sizeof (YY_VIDEO_BUFF));
	buffVideo.nType = YY_VDT_RGBA;
	buffVideo.nWidth = rcRnd.right - rcRnd.left;
	buffVideo.nHeight = rcRnd.bottom - rcRnd.top;
	buffVideo.pBuff[0] = (unsigned char *)pBuff + rcRnd.top * rcView.right * 4 + rcRnd.left * 4;
	buffVideo.nStride[0] = rcView.right * 4;

	YY_BUFFER yyBuff;
	memset (&yyBuff, 0, sizeof (yyBuff));
	yyBuff.pBuff = (unsigned char *)&buffVideo;
	yyBuff.uSize = sizeof (buffVideo);
	yyBuff.uFlag = YYBUFF_TYPE_VIDEO;
	YY_BUFFER * pVideoBuff = &yyBuff;
	m_pMedia->GetParam (YYPLAY_PID_VideoData, &pVideoBuff);

	return true;
}

LRESULT	CJinPlayer::MsgProc	(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRC = S_OK;
	if (uMsg ==  WM_YYRR_CHKLCS)
		return (LRESULT)CheckExtLicense;
	else if (uMsg == WM_COMMAND)
	{
		lRC = m_pWndPlay->MsgProc (hWnd, uMsg, wParam, lParam);
		if (lRC == S_FALSE)
			lRC = m_pWndList->MsgProc (hWnd, uMsg, wParam, lParam);
		if (lRC != S_FALSE)
			return lRC;
	}

	if (m_nWndType == PLAYER_WND_LIST)
		lRC = m_pWndList->MsgProc (hWnd, uMsg, wParam, lParam);
	else if (m_nWndType == PLAYER_WND_PLAY)
		lRC = m_pWndPlay->MsgProc (hWnd, uMsg, wParam, lParam);
	if (lRC != S_FALSE)
		return lRC;

	int		wmId, wmEvent;
	switch (uMsg)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case ID_FILE_OPENFILE:
			OpenMediaFile ();
			return S_OK;

		case ID_FILE_OPENURL:
		{
			CDlgOpenURL dlgURL (m_hInst, m_hWnd);
			if (dlgURL.OpenDlg () == IDOK)
			{
				if (m_nWndType == PLAYER_WND_LIST)
					SwitchWnd (PLAYER_WND_PLAY, false);
				int nRC = m_pWndPlay->PlaybackFile (dlgURL.GetURL (), NULL);
				if (nRC < 0)
				{
					TCHAR szMsg[256];
					_stprintf (szMsg, yyLangGetText (YYTEXT_OpenReturn), dlgURL.GetURL (), nRC);
					MessageBox (m_hWnd, szMsg, yyLangGetText (YYTEXT_Error), MB_OK);
					SwitchWnd (PLAYER_WND_LIST, false);
				}
			}
			return S_OK;
		}

		case ID_FILE_PROPERTIES:
		{
			TCHAR			szFile[1024];
			memset (szFile, 0, sizeof (szFile));
			if (m_nWndType == PLAYER_WND_LIST)
			{
				CListItem * pItem = m_pWndList->GetSelItem ();
				if (pItem != NULL)
					pItem->GetFile (szFile, sizeof (szFile));
			}
			else
			{
				_tcscpy (szFile, m_pWndPlay->GetPlayFile ());
			}
			if (_tcsstr (szFile, _T("://")) == NULL)
			{
				CDlgFileProp	dlgProp (m_hInst, m_hWnd);
				dlgProp.OpenDlg (szFile);
			}
			else
			{
				TCHAR szMediaInfo[2048];
				memset (szMediaInfo, 0, sizeof (szMediaInfo));
				m_pMedia->MediaInfo (szMediaInfo, sizeof (szMediaInfo));
				MessageBox (m_hWnd, szMediaInfo, yyLangGetText (YYTEXT_Info), MB_OK);
			}
			return S_OK;
		}
		case ID_ITEM_IMPORTBOX:
		{
			CListItem * pItem = m_pWndList->GetSelItem ();
			if (pItem == NULL)
				return S_OK;
			if (pItem->m_nType != ITEM_Video && pItem->m_nType != ITEM_Audio && pItem->m_nType != ITEM_Image)
				return S_OK;

			TCHAR * pBoxRoot = m_pReg->GetTextValue (_T("BoxRootPath"));
			if (!m_pKey->IsUsed ())
			{
				if (_tcslen (pBoxRoot) <= 0)
				{
					CDlgNewBox dlgBox (m_hInst, m_hWnd, CBaseKey::g_Key);
					if (dlgBox.OpenDlg (NULL, true) == IDCANCEL)
						return S_OK;
				}
				else
				{
					CDlgOpenBox dlgBox (m_hInst, m_hWnd, CBaseKey::g_Key);
					if (dlgBox.OpenDlg () == IDCANCEL)
						return S_OK;
				}
			}
			if (_tcslen (pBoxRoot) <= 0)
				pBoxRoot = m_pReg->GetTextValue (_T("BoxRootPath"));
			TCHAR	szBoxPath[1024];
			_tcscpy (szBoxPath, pBoxRoot);
			_tcscat (szBoxPath, _T("\\"));
			_tcscat (szBoxPath, m_pKey->GetWKey ());
			TCHAR	szFile[1024];
			TCHAR *	pFile = szFile;
			memset (szFile, 0, sizeof (szFile));
			pItem->GetFile (szFile, sizeof (szFile));
			CDlgImport dlgImport (m_hInst, m_hWnd, m_pKey);
			dlgImport.SetSource (&pFile, 1);
			dlgImport.OpenDlg (szBoxPath);
			return S_OK;
		}

		case ID_FILE_FOLDERVIEW:
		case ID_FILE_PLAYLISTVIEW:
		case ID_FILE_MYBOXVIEW:
			if (m_nWndType == PLAYER_WND_PLAY)
			{
				CreateVideoBmp ();
				if (m_pWndPlay != NULL)
					m_pWndPlay->Close ();
				SwitchWnd (PLAYER_WND_LIST, true);
			}
			if (wmId == ID_FILE_FOLDERVIEW)
				m_pWndList->SetView (LIST_VIEW_Folder);
			else if (wmId == ID_FILE_PLAYLISTVIEW)
				m_pWndList->SetView (LIST_VIEW_Favor);
			else
				m_pWndList->SetView (LIST_VIEW_MyBox);
			return S_OK;

		case ID_MYBOX_NEWBOX:
		case ID_MYBOX_OPENBOX:
		{
			if (wmId == ID_MYBOX_NEWBOX)
			{
				CDlgNewBox dlgBox (m_hInst, m_hWnd, CBaseKey::g_Key);
				if (dlgBox.OpenDlg (NULL, true) == IDCANCEL)
					return S_OK;
			}
			else
			{
				CDlgOpenBox dlgBox (m_hInst, m_hWnd, CBaseKey::g_Key);
				if (dlgBox.OpenDlg () == IDCANCEL)
					return S_OK;
			}
			if (m_nWndType == PLAYER_WND_PLAY)
			{
				CreateVideoBmp ();
				if (m_pWndPlay != NULL)
					m_pWndPlay->Close ();
				SwitchWnd (PLAYER_WND_LIST, true);
			}
			m_pWndList->SetView (LIST_VIEW_MyBox);
			return S_OK;
		}

		case ID_LANG_CHINESE:
			UpdateLang (YYLANG_CHN);
			return S_OK;
		case ID_LANG_ENGLISH:
			UpdateLang (YYLANG_ENG);
			return S_OK;

		case ID_HELP_UPDATE:
		{
			CAppUpdate appUpdate (m_hInst, m_hWnd);
			appUpdate.OpenDlg ();
			break;
		}

		default:
			break;
		}
		break;

	case WM_DROPFILES:
	{	
		HDROP hDrop = (HDROP) wParam;
		int nFiles = DragQueryFile (hDrop, 0XFFFFFFFF, NULL, 0);
		if (nFiles > 0)
		{
			TCHAR szFile[1024];
			memset (szFile, 0, sizeof (szFile));
			int nNameLen = DragQueryFile (hDrop, 0, szFile, sizeof (szFile));
			if (nNameLen > 0)
			{
				if (m_nWndType == PLAYER_WND_LIST)
					SwitchWnd (PLAYER_WND_PLAY, false);
				m_pWndPlay->PlaybackFile (szFile, NULL);
			}
		}
		break;
	}

	case WM_SIZE:
		if (m_nWndType == PLAYER_WND_PLAY)
			m_bResized = true;
		break;

	case WM_LIST_PlayFile:
	{
		int nRC = m_pWndPlay->PlaybackItem ((CListItem *)wParam, (void *)lParam);
		if (nRC == YY_ERR_NONE)
			SwitchWnd (PLAYER_WND_PLAY, true);
		return S_OK;
	}

	case WM_PLAY_Play:
		//SwitchWnd (PLAYER_WND_PLAY, true);
		if (m_pWndPlay != NULL)
			m_pWndPlay->Start ();
		return S_OK;

	case WM_PLAY_Close:
		if (m_pWndPlay->GetPlayItem () != NULL)
			m_pWndPlay->GetPlayItem ()->m_nPlayPos = m_pMedia->GetPos ();
		if (m_pWndPlay->IsFullScreen ())
			m_pWndPlay->ShowFullScreen ();
		CreateVideoBmp ();
		if (m_pWndPlay != NULL)
			m_pWndPlay->Close ();
		SwitchWnd (PLAYER_WND_LIST, true);
		return S_OK;

	case WM_PLAYER_SetWinText:
	{	
		TCHAR szWinText[1024];
		_tcscpy (szWinText, yyLangGetText (YYTEXT_Player));
		_tcscat (szWinText, _T("  "));
		_tcscat (szWinText, (TCHAR *)wParam);
		SetWindowText (m_hWnd, szWinText);
		return S_OK;
	}

	default:
		break;
	}
	return S_FALSE;
}

bool CJinPlayer::UpdateLang (int nLang)
{
	if (m_hMenu != NULL)
		DestroyMenu (m_hMenu);
	m_pLang->setLang (nLang);
	if (nLang == YYLANG_ENG)
		m_hMenu = LoadMenu (m_hInst, MAKEINTRESOURCE (IDC_JINPLAYER));
	else
		m_hMenu = LoadMenu (m_hInst, MAKEINTRESOURCE (IDC_JINPLAYER_CHN));
	SetMenu (m_hWnd, m_hMenu);

	m_pWndList->UpdateLang ();
	m_pWndPlay->UpdateLang ();

	return true;
}

int CJinPlayer::CheckExtLicense (char * pText, int nSize, yyVerifyLicenseText fVerify, void * pUserData)
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
