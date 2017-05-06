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

float g_VideoZoom[] = {1, 1.5, 2, 2.5, 3, 4, 5, 6, 8, 12};

LRESULT	CBoxPlayer::MsgProc	(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT hr = S_FALSE;
	int		wmId, wmEvent;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case ID_FILE_OPENFILE:
		case IDC_BTN_OPEN:
			m_bAutoPlay = false;
			m_pListFile = NULL;
			OpenMediaFile (NULL, true);
			return S_OK;

		case ID_FILE_OPENURL:
		{
			CDlgOpenURL dlg (m_hInst, m_hWnd);
			dlg.SetLangText (m_pLangText);
			if (dlg.OpenDlg () == IDOK)
			{
				if (_tcslen (dlg.GetURL ()) > 6)
					OpenMediaFile (dlg.GetURL (), true);
			}
			return S_OK;
		}

		case ID_VIEW_FOLDER:
		case ID_VIEW_FAVORITES:
		case ID_VIEW_MYBOX:
			if (wmId == ID_VIEW_FOLDER)
				m_pWndList->SetViewType (LIST_VIEW_Folder);
			else if (wmId == ID_VIEW_FAVORITES)
				m_pWndList->SetViewType (LIST_VIEW_Favor);
			else if (wmId == ID_VIEW_MYBOX)
				m_pWndList->SetViewType (LIST_VIEW_MyBox);
			CheckMenuRadioItem (GetSubMenu (m_hMenuList, 1), ID_VIEW_FOLDER, ID_VIEW_MYBOX, 
								wmId, MF_BYCOMMAND | MF_CHECKED);
			return S_OK;

		case ID_VIEW_NEWBOX:
		case ID_VIEW_OPENBOX:
			CheckMenuRadioItem (GetSubMenu (m_hMenuList, 1), ID_VIEW_FOLDER, ID_VIEW_MYBOX, 
								ID_VIEW_MYBOX, MF_BYCOMMAND | MF_CHECKED);
			SendMessage (m_pWndList->GetWnd(), message, wParam, lParam);
			return S_OK;

		case ID_FILE_PROPERTIES:
		{
			if (yyGetProtType (m_szFile) != YY_PROT_FILE)
			{
				TCHAR szInfo[1024];
				memset (szInfo, 0, sizeof (szInfo));
				m_pMedia->MediaInfo (szInfo, sizeof (szInfo));
				if (_tcslen (szInfo) > 0)
					MessageBox (m_hWnd, szInfo, m_pLangText->GetText (YYTEXT_FileInfo), MB_OK);
				return S_OK;
			}

			CDlgFileProp dlgProp (m_hInst, hWnd);
			if (m_nViewType == YY_VIEW_PLAY)
			{
				m_pMedia->Pause ();
				dlgProp.OpenDlg (m_szFile);
			}
			else
			{
				SendMessage (m_pWndList->GetWnd (), message, wParam, lParam);
			}
			return S_OK;
		}

		case ID_FILE_RETURN:
		case IDC_BTN_LIST:
			if (m_nViewType == YY_VIEW_LIST)
				return S_OK;
			if (m_nVRndType == YY_VRND_GDI)
				m_pWndList->CopyLastVideo (m_hWnd);
			else
				m_pWndList->CopyLastVideo (NULL);
			m_pMedia->SetParam (YYPLAY_PID_Prepare_Close, 0);
			if (m_pMedia->Close () < 0)
			{
				YYLOGT ("DemoUI", "Close media file failed!");
				return S_OK;
			}
			SwitchView (YY_VIEW_LIST);
			return S_OK;

		case IDC_BTN_FULLSRN:
			ShowFullScreen ();
			return S_OK;

		case IDC_BTN_NEXT:
			PlayNextFile (true);
			return S_OK;
			
		case IDC_BTN_PREV:
			PlayNextFile (false);
			return S_OK;

		case ID_SUBTITLE_ENABLE:
		case ID_SUBTITLE_DISABLE:
		case ID_SUBTITLE_SETTING:
		case ID_SUBTITLE_FONT:
			SendMessage (m_pWndSubTT->GetWnd (), message, wParam, lParam);
			return S_OK;

		case ID_LANG_ENGLISH:
			m_pLangText->setLang (YYLANG_ENG);
			UpdateMenuLang ();
			return S_OK;

		case ID_LANG_CHINESE:
			m_pLangText->setLang (YYLANG_CHN);
			UpdateMenuLang ();
			return S_OK;

		case ID_VIDEOZOOM_1:
		case ID_VIDEOZOOM_2:
		case ID_VIDEOZOOM_3:
		case ID_VIDEOZOOM_4:
		case ID_VIDEOZOOM_5:
		case ID_VIDEOZOOM_6:
		case ID_VIDEOZOOM_8:
		case ID_VIDEOZOOM_10:
		case ID_VIDEOZOOM_12:
		case ID_VIDEOZOOM_16:
			m_nZoomNum = wmId - ID_VIDEOZOOM_1;
			CheckMenuRadioItem (m_hMenuZoom, ID_VIDEOZOOM_1, ID_VIDEOZOOM_16, wmId, MF_BYCOMMAND | MF_CHECKED);
			OnZoomVideo ();
			return S_OK;

		case ID_VIDEO_GDI:
			if (m_nVRndType != YY_VRND_GDI)
			{
				m_nVRndType = YY_VRND_GDI;
				m_pMedia->SetView (m_hWnd, m_nVRndType);
				HMENU hMenu = GetSubMenu (m_hMenuPlay, 1);
				hMenu = GetSubMenu (hMenu, 4);
				CheckMenuRadioItem (hMenu, ID_VIDEO_GDI, ID_VIDEO_DDRAW, ID_VIDEO_GDI, MF_BYCOMMAND | MF_CHECKED);
			}
			return S_OK;

		case ID_VIDEO_DDRAW:
			if (m_nVRndType != YY_VRND_DDRAW)
			{
				m_nVRndType = YY_VRND_DDRAW;
				m_pMedia->SetView (m_hWnd, m_nVRndType);
				HMENU hMenu = GetSubMenu (m_hMenuPlay, 1);
				hMenu = GetSubMenu (hMenu, 4);
				CheckMenuRadioItem (hMenu, ID_VIDEO_GDI, ID_VIDEO_DDRAW, ID_VIDEO_DDRAW, MF_BYCOMMAND | MF_CHECKED);
			}
			return S_OK;

		default:
			if (wmId >= ID_AUDIO_AUDIO1 && wmId < ID_AUDIO_AUDIO1 + m_nAudioTrackNum)
			{
				int nTrackPlay = 0;
				m_pMedia->GetParam (YYPLAY_PID_AudioTrackPlay, &nTrackPlay);
				CheckMenuItem (m_hMenuAudio, ID_AUDIO_AUDIO1 + nTrackPlay, MF_BYCOMMAND | MF_UNCHECKED);

				nTrackPlay = wmId - ID_AUDIO_AUDIO1;
				m_pMedia->SetParam (YYPLAY_PID_AudioTrackPlay, (void *)nTrackPlay);
				CheckMenuItem (m_hMenuAudio, ID_AUDIO_AUDIO1 + nTrackPlay, MF_BYCOMMAND | MF_CHECKED);
				return S_OK;
			}
			break;
		}
		break;

	case WM_LBUTTONDOWN:	
		m_nClickCount++;
		if (m_nClickTimer == 0)
			m_nClickTimer = SetTimer (m_hWnd, TIMER_LBUTTON_DOWN, 250, NULL);
		return S_OK;

	case WM_LBUTTONUP:	
		m_nLastZoomX = 0;
		m_nLastZoomY = 0;
		break;

	case WM_RBUTTONUP:
		if (m_nVRndType == YY_VRND_DDRAW)
		{
			m_ptZoom.x = LOWORD (lParam);
			m_ptZoom.y = HIWORD (lParam);
			POINT pt;
			pt.x = LOWORD (lParam);
			pt.y = HIWORD (lParam);
			ClientToScreen (m_hWnd, &pt);
			TrackPopupMenu (m_hMenuZoom, TPM_LEFTALIGN, pt.x, pt.y, 0, m_hWnd, NULL);
		}
		return S_OK;

	case WM_MOUSEMOVE:
		if (m_nViewType == YY_VIEW_LIST)
			break;
		if (abs (m_nLastMoveX - LOWORD (lParam)) > 4 || abs (m_nLastMoveY - HIWORD (lParam)) > 4)
		{
			if (!IsWindowVisible (m_pWndPanel->GetWnd ()) && yyGetSysTime () - m_nLastHideTime > 1000)
				ShowPanel (true);
		}
		m_nLastMoveX = LOWORD (lParam);
		m_nLastMoveY = HIWORD (lParam);
		if (wParam == MK_LBUTTON && m_nZoomNum > 0)
			OnZoomMove (wParam, lParam);
		break;

	case WM_TIMER:
		if (wParam == TIMER_LBUTTON_DOWN)
		{
			KillTimer (m_hWnd, TIMER_LBUTTON_DOWN);
			m_nClickTimer = 0;
			if (m_nClickCount >= 2)
			{
				ShowFullScreen ();
			}
			else
			{
				if (m_pMedia->GetStatus () == YY_PLAY_Run)
				{
					m_pMedia->Pause ();
				}
				else if (m_pMedia->GetStatus () == YY_PLAY_Pause)
				{
					m_pMedia->Run ();
					if (IsWindowVisible (m_pWndPanel->GetWnd ()))
						ShowPanel (false);
				}
			}
			m_nClickCount = 0;
		}
		else if (wParam == TIMER_SHOW_PANEL)
		{
			ShowPanel (false);
		}
		if (wParam == TIMER_PLAY_NEXTFILE)
		{
			KillTimer (m_hWnd, TIMER_PLAY_NEXTFILE);
			PlayNextFile (true);
		}
		else if (wParam == TIMER_UPDATE_AUDIOFILE)
		{
			KillTimer (m_hWnd, TIMER_UPDATE_AUDIOFILE);
			m_nTimerAudio = 0;
			m_hBmpThumb = NULL;
			InvalidateRect (m_hWnd, NULL, TRUE);
		}
		break;

	case WM_SIZE:
		if (m_hWnd != NULL)
		{
			if (m_nViewType == YY_VIEW_LIST)
				m_pWndList->OnSize ();
			else
				m_pMedia->SetView (m_hWnd, m_nVRndType);
		}
		MovePanel ();
		break;

	case WM_MOVE:
		MovePanel ();
		break;

	case WM_KEYDOWN:
		if (m_nViewType == YY_VIEW_LIST)
			return SendMessage (m_pWndList->GetWnd (), message, wParam, lParam);
		if (wParam == VK_ESCAPE && IsFullScreen ()) // ESC
			ShowFullScreen ();
		break;
	case WM_KEYUP:
		if (m_nViewType == YY_VIEW_LIST)
			return SendMessage (m_pWndList->GetWnd (), message, wParam, lParam);
		break;
	case WM_CHAR:
	case WM_UNICHAR:
		if (m_nViewType == YY_VIEW_LIST)
			return SendMessage (m_pWndList->GetWnd (), message, wParam, lParam);
		break;
	case WM_MOUSEWHEEL:
		if (m_nViewType == YY_VIEW_LIST)
			return SendMessage (m_pWndList->GetWnd (), message, wParam, lParam);
		break;

	case WM_DROPFILES:
		if (m_nViewType == YY_VIEW_PLAY)
		{
			HDROP hDrop = (HDROP) wParam;
			int nFiles = DragQueryFile (hDrop, 0XFFFFFFFF, NULL, 0);
			if (nFiles > 0)
			{
				memset (m_szFile, 0, sizeof (m_szFile));
				int nNameLen = DragQueryFile (hDrop, 0, m_szFile, sizeof (m_szFile));
				if (nNameLen > 0)
					OpenMediaFile (m_szFile, true);
			}
		}
		else
		{
			m_pWndList->OnDropFiles (wParam);
		}
		break;

	case WM_ERASEBKGND:
		if (m_hBmpThumb == NULL)
		{
			HDC hdc = (HDC)wParam;
			RECT rcView;
			GetClientRect (m_hWnd, &rcView);
			if (m_nViewType == YY_VIEW_PLAY)
				FillRect (hdc, &rcView, m_hBrushBlack);
		}
		return S_OK;

	case WM_PAINT:
	{
		RECT rcView;
		GetClientRect (m_hWnd, &rcView);
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(m_hWnd, &ps);
		if (m_hBmpThumb != NULL)
		{
			BITMAP bmpInfo;
			GetObject (m_hBmpThumb, sizeof (BITMAP), &bmpInfo);
			if (m_hMemDC == NULL)
				m_hMemDC = CreateCompatibleDC (hdc);
			HBITMAP hBmpOld = (HBITMAP) SelectObject (m_hMemDC, m_hBmpThumb);
			StretchBlt (hdc, 0, 0, rcView.right, rcView.bottom,
						m_hMemDC, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY);
			SelectObject (m_hMemDC, hBmpOld);
			EndPaint(m_hWnd, &ps);
			return S_OK;
		}
		EndPaint(m_hWnd, &ps);
		m_pMedia->UpdateView (NULL);
		break;
	}

	case WM_WINDOWPOSCHANGED:
	{
		WINDOWPOS * pPos = (WINDOWPOS *)lParam;
		if (pPos->cx < 200 || pPos->cy < 40)
			ShowPanel (false);
		break;
	}

	case WM_YYPANEL_COMMAND:
		ShowPanel (true);
		return S_OK;

	case WM_YYLIST_PLAYFILE:
		m_hBmpThumb = (HBITMAP)lParam;
		m_bAutoPlay = false;
		m_pListFile = m_pWndList->GetFileList ();
		OpenMediaFile ((TCHAR *)wParam, false);
		return S_OK;

	case WM_YYLIST_PLAYBACK:
		SwitchView (YY_VIEW_PLAY);
		return S_OK;

	case WM_YYRR_CHKLCS:
	case WM_YYYF_CHKLCS:
	case WM_YYSM_CHKLCS:
		return (LRESULT)CheckExtLicense;

	default:
		break;
	}
	return S_FALSE;
}

void CBoxPlayer::NotifyEvent (void * pUserData, int nID, void * pV1)
{
	((CBoxPlayer *)pUserData)->HandleEvent (nID, pV1);;
}

void CBoxPlayer::HandleEvent (int nID, void * pV1)
{
	int nRC = 0;
	if (m_bMediaClose)
		return;

	switch (nID)
	{
	case YY_EV_Open_Complete:
	{
		int nTryTimes = 0;
		while (m_nViewType == YY_VIEW_LIST)
		{
			yySleep (10000);
			nTryTimes++;
			if (nTryTimes > 300)
				break;
		}

		YY_VIDEO_FORMAT * pFmtVideo = NULL;
		m_pMedia->GetParam (YYPLAY_PID_Fmt_Video, &pFmtVideo);
		if (pFmtVideo != NULL)
		{
			SetRect (&m_rcVideo, 0, 0, pFmtVideo->nWidth, pFmtVideo->nHeight);
			SetRect (&m_rcZoom, 0, 0, pFmtVideo->nWidth, pFmtVideo->nHeight);
		}
		UpdatePlayMenu ();

		nRC = m_pMedia->Run ();
		if (nRC < 0)
		{
			if (nRC != YY_ERR_STATUS)
				MessageBox (m_hWnd, m_pLangText->GetText (YYTEXT_PlayFail), m_pLangText->GetText (YYTEXT_Error), MB_OK);
		}
		m_bOpening = false;

		if (m_pWndPanel != NULL)
			m_pWndPanel->HandleEvent (nID, pV1);

		if (m_nViewType == YY_VIEW_LIST)
			SwitchView (YY_VIEW_PLAY);

		if (nRC >= 0)
			m_nTimerAudio = SetTimer (m_hWnd, TIMER_UPDATE_AUDIOFILE, 2000, NULL);
		return;
	}

	case YY_EV_Open_Failed:
		m_bOpening = false;
		if (m_bAutoPlay)
			SetTimer (m_hWnd, TIMER_PLAY_NEXTFILE, 100, NULL);
		else
		{
			MessageBox (m_hWnd, m_pLangText->GetText (YYTEXT_OpenFail), m_pLangText->GetText (YYTEXT_Error), MB_OK);
			SwitchView (YY_VIEW_LIST);
		}
		break;

	case YY_EV_Play_Complete:
		YYLOGT ("DemoUI", "Playback file completed!");
		if (m_pListFile == NULL || _tcsstr (m_szFile, _T(":/")) != NULL)
		{
			PostMessage (m_hWnd, WM_COMMAND, IDC_BTN_LIST, 0);
		}
		else
		{
			m_bAutoPlay = true;
			SetTimer (m_hWnd, TIMER_PLAY_NEXTFILE, 100, NULL);
		}		
		break;

	case YY_EV_Play_Status:
		if (m_pMedia->GetStatus () == YY_PLAY_Pause)
		{
			ShowPanel (true);
		}
		else 
		{
			if (IsWindowVisible (m_pWndPanel->GetWnd ()))
			{
				if (m_dwTimerPanel == 0)
					m_dwTimerPanel = SetTimer (m_hWnd, TIMER_SHOW_PANEL, PANEL_SHOW_TIME, NULL);
			}
		}
		break;

	case YY_EV_Draw_FirstFrame:
		if (m_nTimerAudio != 0)
		{
			KillTimer (m_hWnd, TIMER_UPDATE_AUDIOFILE);
			m_nTimerAudio = 0;
		}
		m_hBmpThumb = NULL;

		RECT rcDraw;
		RECT rcRnd;
		m_pMedia->GetParam (YYPLAY_PID_RenderArea, &rcRnd);
		RECT rcWnd;
		GetClientRect (m_hWnd, &rcWnd);
		if (rcRnd.left > rcWnd.left)
		{
			SetRect (&rcDraw, rcWnd.left, rcWnd.top, rcRnd.left, rcWnd.bottom);
			InvalidateRect (m_hWnd, &rcDraw, TRUE);
		}
		if (rcRnd.right < rcWnd.right)
		{
			SetRect (&rcDraw, rcRnd.right, rcWnd.top, rcWnd.right, rcWnd.bottom);
			InvalidateRect (m_hWnd, &rcDraw, TRUE);
		}
		if (rcRnd.top > rcWnd.top)
		{
			SetRect (&rcDraw, rcWnd.left, rcWnd.top, rcWnd.right, rcRnd.top);
			InvalidateRect (m_hWnd, &rcDraw, TRUE);
		}
		if (rcRnd.bottom < rcWnd.bottom)
		{
			SetRect (&rcDraw, rcWnd.left, rcRnd.bottom, rcWnd.right, rcWnd.bottom);
			InvalidateRect (m_hWnd, &rcDraw, TRUE);
		}
		break;

	default:
		break;
	}

	if (m_pWndPanel != NULL)
		m_pWndPanel->HandleEvent (nID, pV1);
}

void CBoxPlayer::OnZoomVideo (void)
{
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);

	int nW = m_rcZoom.right - m_rcZoom.left;
	int nH = m_rcZoom.bottom - m_rcZoom.top;

	int nXC = m_rcZoom.left + m_ptZoom.x * nW / rcView.right;
	int nYC = m_rcZoom.top + m_ptZoom.y * nH / rcView.bottom;

	nW = m_rcVideo.right / g_VideoZoom[m_nZoomNum];
	nH = m_rcVideo.bottom / g_VideoZoom[m_nZoomNum];

	int nL = nXC - nW / 2;
	if (nL < 0) nL = 0;
	if (nL + nW > m_rcVideo.right) nL = m_rcVideo.right - nW;

	int nT = nYC - nH / 2;
	if (nT < 0) nT = 0;
	if (nT + nH > m_rcVideo.bottom) nT = m_rcVideo.bottom - nH;

	SetRect (&m_rcZoom, nL, nT, nL + nW, nT + nH);

	m_pMedia->SetParam (YYPLAY_PID_VideoZoomIn, &m_rcZoom);
	if (m_pMedia->GetStatus () == YY_PLAY_Pause)
		m_pMedia->UpdateView (NULL);
}

void CBoxPlayer::OnZoomMove (WPARAM wParam, LPARAM lParam)
{
	if (m_nLastZoomX != 0 || m_nLastZoomY != 0)
	{
		int nW = m_rcZoom.right - m_rcZoom.left;
		int nH = m_rcZoom.bottom - m_rcZoom.top;

		int nX = LOWORD (lParam) - m_nLastZoomX;
		int nY = HIWORD (lParam) - m_nLastZoomY;
		m_rcZoom.left -= nX;
		if (m_rcZoom.left < 0) m_rcZoom.left = 0;
		if (m_rcZoom.left + nW > m_rcVideo.right) m_rcZoom.left = m_rcVideo.right - nW;
		m_rcZoom.right = m_rcZoom.left + nW;

		m_rcZoom.top -= nY;
		if (m_rcZoom.top < 0) m_rcZoom.top = 0;
		if (m_rcZoom.top + nH > m_rcVideo.bottom) m_rcZoom.top = m_rcVideo.bottom - nH;
		m_rcZoom.bottom = m_rcZoom.top + nH;

		m_pMedia->SetParam (YYPLAY_PID_VideoZoomIn, &m_rcZoom);
		if (m_pMedia->GetStatus () == YY_PLAY_Pause)
			m_pMedia->UpdateView (NULL);
	}

	m_nLastZoomX = LOWORD (lParam);
	m_nLastZoomY = HIWORD (lParam);
}
