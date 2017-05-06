/*******************************************************************************
	File:		CVideoRender.cpp

	Contains:	Demo UI implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-14		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"
#include <commctrl.h>
#include <Commdlg.h>
#include <winuser.h>
#include <shellapi.h>

#include "stdint.h"

#include "CDemoUI.h"
#include "CDlgFileInfo.h"
#include "CDlgOpenURL.h"

#include "CBaseUtils.h"

#include "USystemFunc.h"

#include "yyMetaData.h"
#include "yyLog.h"

#include "resource.h"

#define BTN_WIDTH				48
#define	BTN_HEIGHT				32
#define WND_WIDTH				800
#define	WND_HEIGHT				498
#define SLD_HEIGHT				30
#ifdef _OS_WINCE
#define	CMD_BAR_HEIGHT			28
#else
#define	CMD_BAR_HEIGHT			0
#endif // Wince

#define TIMER_UPDATE_STATUS		101
#define TIMER_PLAY_NEXTFILE		102
#define TIMER_UPDATE_AUDIOFILE	103
#define TIMER_TEST_WORK			104

#define	IDC_BTN_LOUD			1101
#define	IDC_SLD_VOICE			1102
#define	IDC_BTN_LOW				1103
#define	IDC_BTN_MUTE			1104
#define	IDC_BTN_AR				1105
#define	IDC_BTN_SPEED			1106
#define	IDC_BTN_AUDIO			1107
#define	IDC_BTN_SUBTITLE		1108

#define	IDC_BTN_CLOSE			1109
#define	IDC_BTN_OPEN			1110
#define	IDC_BTN_NEXT			1111
#define	IDC_BTN_PREV			1112
#define	IDC_BTN_INFO			1113
#define	IDC_BTN_PLAY			1114
#define	IDC_BTN_PAUSE			1115
#define	IDC_BTN_STOP			1116
#define	IDC_BTN_LIST			1117

#define IDC_SLD_POS				1201
#define IDC_TXT_POS				1202
#define IDC_TXT_DUR				1203
#define IDC_TXT_TITLE			1204

#define IDC_CMD_AUDIOTRACK		3204

int		g_nAutoTestCount		= 0;

CDemoUI::CDemoUI(void)
	: m_hInst (NULL)
	, m_hWnd (NULL)
	, m_bPrepareClose (false)
	, m_hMenuAudio (NULL)
	, m_mtWait (NULL)
	, m_mtNormal (NULL)
	, m_bOpening (false)
	, m_bSeeking (false)
	, m_bRunning (false)
	, m_bAutoPlay (false)
	, m_hAudioFileTimer (0)
	, m_pMedia (NULL)
	, m_pExtPlayer (NULL)
	, m_pExtData (NULL)
	, m_nAudioTrackNum (0)
	, m_nAudioTrackMenus (0)
	, m_nBitsVideo (16)
	, m_bOverlay (false)
	, m_ppFiles (NULL)
	, m_nFileNum (0)
	, m_nFileIdx (0)
	, m_nAudioVolume (100)
#ifdef _OS_WINCE
	, m_nVideoRendType (YY_VRND_DDCE6)
//	, m_nVideoRendType (YY_VRND_GDI)
#else
//	, m_nVideoRendType (YY_VRND_GDI)
	, m_nVideoRendType (YY_VRND_DDRAW)
#endif // WINCE
	, m_pWndVideo (NULL)
	, m_pWndList (NULL)
	, m_hPopup (NULL)
	, m_sldPos (NULL)
	, m_pSldPos (NULL)
	, m_pWndTitle (NULL)
	, m_bListView (false)
	, m_uAutoTest (0)
{
	m_pRegMng = new CRegMng (_T("yyPlayer"));

	memset (m_szFile, 0, sizeof (m_szFile));
	_tcscpy (m_szFileExt, _T(".avi, .divx, .mp4, .m4v, .mov, .mkv, .3gp, .3g2, "));
	_tcscat (m_szFileExt, _T(".rmvb, .rm, .ra, .real, .rv, "));
	_tcscat (m_szFileExt, _T(".asf, .wmv, .wma,"));
	_tcscat (m_szFileExt, _T(".flv, .ts, .mpeg, .mpg, .vob"));
	_tcscat (m_szFileExt, _T(".mp3, .mp2, .aac, .amr, .ogg, .wav, .ac3, .awb, "));
	_tcscat (m_szFileExt, _T(".ape, .flac"));

	m_cbSubTT.userData = this;
	m_cbSubTT.funcCB = yyMedeaSubCB;

#ifdef _CPU_MSB2531
//	m_nVideoRendType = YY_VRND_GDI;
#endif // _CPU_MSB2531
}

CDemoUI::~CDemoUI(void)
{
	m_bPrepareClose = true;

	if (_tcslen (m_szFile) > 0)
		m_pRegMng->SetTextValue (_T("PlayFile"), m_szFile);
	if (m_pMedia != NULL)
	{
		int nPos = m_pMedia->GetPos ();
		m_pRegMng->SetIntValue (__T("PlayPos"), nPos);
	}

	if (m_pMedia != NULL)
		m_pMedia->SetParam (YYPLAY_PID_Prepare_Close, 0);

	if (m_pExtPlayer != NULL)
		m_pExtPlayer->Stop ();
	if (m_pMedia != NULL)
		m_pMedia->Stop ();

	ReleasePlayList ();

	if (m_pWndVideo != NULL)
	{
		SendMessage (m_pWndVideo->GetWnd (), WM_CLOSE, 0, 0);
		delete m_pWndVideo;
	}
	m_pWndVideo = NULL;

	if (m_pWndList != NULL)
	{
		SendMessage (m_pWndList->GetWnd (), WM_CLOSE, 0, 0);
		delete m_pWndList;
	}
	m_pWndList = NULL;

	if (m_pSldPos != NULL)
	{
		SendMessage (m_pSldPos->GetWnd (), WM_CLOSE, 0, 0);
		delete m_pSldPos;
	}
	m_pSldPos = NULL;

	if (m_pWndTitle != NULL)
	{
		SendMessage (m_pWndTitle->GetWnd (), WM_CLOSE, 0, 0);
		delete m_pWndTitle;
	}
	m_pWndTitle = NULL;

	if (m_hPopup != NULL)
		DestroyMenu (m_hPopup);
	m_hPopup = NULL;

	if (m_hMenuAudio != NULL)
		DestroyMenu (m_hMenuAudio);
	m_hMenuAudio = NULL;

	if (m_pMedia != NULL)
	{
		m_pMedia->Stop ();
		delete m_pMedia;
	}
	m_pMedia = NULL;

	if (m_pExtPlayer != NULL)
	{
		m_pExtPlayer->Stop ();
		delete m_pExtPlayer;
	}
	m_pExtPlayer = NULL;

	delete m_pRegMng;

	YY_DEL_P (m_pExtData);
}

LRESULT CDemoUI::MsgProc (HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT hr = S_FALSE;
	int		wmId, wmEvent;
	float	fSpeed = 1.0;
	RECT	rcView;
	YYPLAY_ARInfo	arInfo;

//	if (m_pWndList == NULL || m_pWndVideo == NULL)
//		return S_OK;

	if (m_pWndVideo != NULL)
		GetClientRect (m_pWndVideo->GetWnd (), &rcView);
	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_FILE_OPENFILE:
		case IDC_BTN_OPEN:
			if (m_bOpening)
				return S_OK;
			if (m_pMedia != NULL)
			{
				if (m_pMedia->GetStatus () == YY_PLAY_Run)
				{
					m_bRunning = true;
					m_pMedia->Pause ();
				}
			}
			m_bAutoPlay = false;
			OpenMediaFile (NULL, true);
			return S_OK;

		case ID_FILE_OPEN_URL:
		{
			CDlgOpenURL dlgURL (m_hInst, hWnd);
			if (dlgURL.OpenDlg () == IDOK)
			{
				if (_tcslen (dlgURL.GetURL ()) > 6)
					OpenMediaFile (dlgURL.GetURL (), true);
			}
		}
			return S_OK;

		case IDC_BTN_LIST:
			if (IsWindowVisible (m_pWndList->GetWnd ()))
				return S_OK;

			if (m_pMedia != NULL)
			{
				m_pMedia->SetParam (YYPLAY_PID_Prepare_Close, 0);
				if (m_pExtPlayer != NULL)
					m_pExtPlayer->Stop ();
				if (m_pMedia->Close () < 0)
				{
					YYLOGT ("DemoUI", "Close media file failed!");
					return S_OK;
				}
			}
			m_pSldPos->SetRange (0, m_pWndList->GetMaxPos ());
			m_pSldPos->SetPos (m_pWndList->GetPos ());
			m_pWndList->SetPlayingFile (m_szFile);
			m_pWndList->CopyLastVideo (m_pWndVideo->GetWnd ());
			ShowWindow (m_pWndVideo->GetWnd (), SW_HIDE);
			ShowWindow (m_pWndList->GetWnd (), SW_SHOW);
			m_pWndList->Start ();	
			return S_OK;

		case ID_PLAY_GDI:
			m_nVideoRendType = YY_VRND_GDI;
			if (m_pMedia != NULL)
				m_pMedia->SetDisplay (m_pWndVideo->GetWnd (), m_nVideoRendType);
			return S_OK;

		case ID_PLAY_DDRAW:
			m_nVideoRendType = YY_VRND_DDRAW;
			if (m_pMedia != NULL)
				m_pMedia->SetDisplay (m_pWndVideo->GetWnd (), m_nVideoRendType);
			return S_OK;

		case ID_FILE_DDMEMORY:
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_DDMode, (void *)YY_DDM_Memory);
			m_bOverlay = false;
			m_pWndVideo->ShowOverlay (m_bOverlay);
			return S_OK;

		case ID_FILE_DDOVERLAY:
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_DDMode, (void *)YY_DDM_Overlay);
			// MessageBox (hWnd, _T("Test overlay mode!"), _T("Overlay"), MB_OK);
			m_bOverlay = true;
			m_pWndVideo->ShowOverlay (m_bOverlay);
			{
				RECT rcView;
				GetClientRect (m_pWndVideo->GetWnd (), &rcView);
				arInfo.nWidth = rcView.right;
				arInfo.nHeight = rcView.bottom;
				if (m_pMedia != NULL)
					m_pMedia->SetParam (YYPLAY_PID_AspectRatio, &arInfo);
				InvalidateRect (m_pWndVideo->GetWnd (), NULL, TRUE);
			}
			return S_OK;

		case ID_FILE_DISABLEVIDEO:
//			ShowWindow (m_pWndVideo->GetWnd (), SW_HIDE);
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_Disable_Video, (void *)YY_PLAY_VideoDisable_Render);
	//		ShowWindow (m_pWndVideo->GetWnd (), SW_HIDE);
			return S_OK;
		case ID_FILE_DISABLEDECODER:
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_Disable_Video, (void *)YY_PLAY_VideoDisable_Decoder);
			ShowWindow (m_pWndVideo->GetWnd (), SW_HIDE);
			return S_OK;
		case ID_FILE_ENABLEVIDEO:
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_Disable_Video, (void *)YY_PLAY_VideoEnable);
			ShowWindow (m_pWndVideo->GetWnd (), SW_SHOW);
			return S_OK;

		case ID_PLAY_PLAY:
		case IDC_BTN_PLAY:
			if (m_pMedia != NULL)
				m_pMedia->Start ();
			if (m_pExtPlayer != NULL)
				m_pExtPlayer->Start ();
			break;

		case ID_PLAY_PAUSE:
		case IDC_BTN_PAUSE:
			if (m_pExtPlayer != NULL)
				m_pExtPlayer->Pause ();
			if (m_pMedia != NULL)
				m_pMedia->Pause ();
			break;

		case ID_PLAY_STOP:
		case IDC_BTN_STOP:
			if (m_pMedia != NULL)
//				m_pMedia->Stop ();
				m_pWndVideo->SetFullScreen ();
			break;

		case ID_TOOLS_FILEINFO:
		case IDC_BTN_INFO:
			if (m_bOpening)
				return S_OK;
			if (IsWindowVisible (m_pWndList->GetWnd ()))
			{
				YYINFO_Thumbnail * pThumb = m_pWndList->GetSelectedItem ();
				if (pThumb == NULL)
				{
#ifndef WINCE
					CDlgFileInfo dlgFile (m_hInst, hWnd);
					dlgFile.OpenDlg (m_szFile);
#endif // WINCE
					return S_OK;
				}

				TCHAR szInfo[256];
				_stprintf (szInfo, _T("Dur: % 6d, Bitrate: % 6d\r\nVideo %s    %d X %d\r\nAudio %s   %d X %d"),
							pThumb->nDuration, pThumb->nBitrate,
							pThumb->szVideoCodec, pThumb->nVideoWidth, pThumb->nVideoHeight,
							pThumb->szAudioCodec, pThumb->nSampleRate, pThumb->nChannels);
				MessageBox (hWnd, szInfo, _T("File Info:"), MB_OK);
			}
			else
			{
#ifdef _OS_WINCE
				TCHAR szInfo[1024];
				if (m_pMedia != NULL)
				{
					YYPLAY_STATUS status = m_pMedia->GetStatus ();
					if (status == YY_PLAY_Run)
						m_pMedia->Pause ();

					m_pMedia->GetMediaInfo (szInfo, sizeof (szInfo));
					MessageBox (m_hWnd, szInfo, _T("Info"),  MB_OK);

					if (status == YY_PLAY_Run)
					{
						m_pMedia->Start ();
						if (m_pExtPlayer != NULL)
							m_pExtPlayer->Start ();
					}
				}
#else
				CDlgFileInfo dlgFile (m_hInst, hWnd);
				dlgFile.OpenDlg (m_szFile);
#endif // WINCE
			}
			break;

		case ID_TOOLS_METADATA:
		{
			TCHAR szMeta[1024];
			YYMETA_Value metaValue;
			if (m_pMedia == NULL)
				return S_OK;

			_tcscpy (szMeta, _T(""));
			strcpy (metaValue.szKey, "title");
			m_pMedia->GetParam (YY_PLAY_BASE_META, &metaValue);
			_tcscat (szMeta, _T("Title:  ")); _tcscat (szMeta, metaValue.szValue); _tcscat (szMeta, _T("\r\n"));
			
			strcpy (metaValue.szKey, "album");
			m_pMedia->GetParam (YY_PLAY_BASE_META, &metaValue);
			_tcscat (szMeta, _T("album:  ")); _tcscat (szMeta, metaValue.szValue); _tcscat (szMeta, _T("\r\n"));

			strcpy (metaValue.szKey, "artist");
			m_pMedia->GetParam (YY_PLAY_BASE_META, &metaValue);
			_tcscat (szMeta, _T("artist:  ")); _tcscat (szMeta, metaValue.szValue); _tcscat (szMeta, _T("\r\n"));

			strcpy (metaValue.szKey, "date");
			m_pMedia->GetParam (YY_PLAY_BASE_META, &metaValue);
			_tcscat (szMeta, _T("date:  ")); _tcscat (szMeta, metaValue.szValue); _tcscat (szMeta, _T("\r\n"));

			strcpy (metaValue.szKey, "genre");
			m_pMedia->GetParam (YY_PLAY_BASE_META, &metaValue);
			_tcscat (szMeta, _T("genre:  ")); _tcscat (szMeta, metaValue.szValue); _tcscat (szMeta, _T("\r\n"));

			strcpy (metaValue.szKey, "track");
			m_pMedia->GetParam (YY_PLAY_BASE_META, &metaValue);
			_tcscat (szMeta, _T("track:  ")); _tcscat (szMeta, metaValue.szValue); _tcscat (szMeta, _T("\r\n"));

			MessageBox (hWnd, szMeta, _T("MetaData"), MB_OK);
		}
			break;

		case ID_TOOLS_AUTOTEST:
			if (m_uAutoTest == 0)
			{
				m_uAutoTest = SetTimer (hWnd, TIMER_TEST_WORK, 1000, NULL);
			}
			else
			{
				KillTimer (hWnd, TIMER_TEST_WORK);
				m_uAutoTest = 0;
			}
			break;

		case IDC_BTN_NEXT:
			m_bAutoPlay = false;
			PlayNextFile (true);
			break;

		case IDC_BTN_PREV:
			m_bAutoPlay = false;
			PlayNextFile (false);
			break;

		case IDC_BTN_LOUD:
			if (m_pMedia != NULL)
			{
				int nPos = (100 - SendMessage (m_sldVoice, TBM_GETPOS, 0, 0)) + 10;
				if (nPos > 100)
					nPos = 100;

				m_pMedia->SetVolume (nPos);
				SendMessage (m_sldVoice, TBM_SETPOS, TRUE, 100 - nPos);
			}
			break;

		case IDC_BTN_LOW:
			if (m_pMedia != NULL)
			{
				int nPos = (100 - SendMessage (m_sldVoice, TBM_GETPOS, 0, 0)) - 10;
				if (nPos < 0)
					nPos = 0;
				m_pMedia->SetVolume (nPos);
				SendMessage (m_sldVoice, TBM_SETPOS, TRUE, 100 - nPos);
			}
			break;

		case IDC_BTN_MUTE:
			if (m_pMedia != NULL)
			{
				m_pMedia->SetVolume (0);
				SendMessage (m_sldVoice, TBM_SETPOS, TRUE, 100);
			}
			break;

		case IDC_BTN_SUBTITLE:
			if (m_pMedia != NULL)
			{
				int nSubtitle = 0;
				m_pMedia->GetParam (YYPLAY_PID_SubTitle, &nSubtitle);
				if (nSubtitle > 0)
					nSubtitle = 0;
				else
					nSubtitle = 1;
				m_pMedia->SetParam (YYPLAY_PID_SubTitle, (void *)nSubtitle);

				m_pMedia->GetParam (YYPLAY_PID_SubTitle, &nSubtitle);
				if (nSubtitle == 0)
					SetWindowText (m_btnSubTitle, _T("STT -"));
				else
					SetWindowText (m_btnSubTitle, _T("STT +"));
			}
			break;

		case IDC_BTN_AUDIO:
		{
			if (m_pMedia == 0)
				return 0;

			m_pMedia->GetParam (YYPLAY_PID_AudioTrackNum, &m_nAudioTrackNum);
			if (m_nAudioTrackNum < 0)
				return 0;

			RECT	rcBtn;
			BOOL	bRC = FALSE;
			DWORD	dwErr = 0;
			GetWindowRect (m_btnAudio, &rcBtn);
			if (m_nAudioTrackMenus != m_nAudioTrackNum)
			{
				if (m_hMenuAudio != NULL)
					DestroyMenu (m_hMenuAudio);
				m_hMenuAudio = CreatePopupMenu ();

				TCHAR szMenu[32];
				for (int i = 0; i < m_nAudioTrackNum; i++)
				{
					_stprintf (szMenu, _T("Track %d"), i);
					bRC = AppendMenu (m_hMenuAudio, MF_BYCOMMAND | MF_STRING, IDC_CMD_AUDIOTRACK + i, szMenu);
					if (!bRC)
						dwErr = GetLastError ();
				}

				m_nAudioTrackMenus = m_nAudioTrackNum;
			}

			if (m_hMenuAudio == NULL)
				break;

			int nTrackPlay = 0;
			m_pMedia->GetParam (YYPLAY_PID_AudioTrackPlay, &nTrackPlay);
			CheckMenuRadioItem (m_hMenuAudio, 0, m_nAudioTrackMenus, nTrackPlay, MF_BYPOSITION | MF_CHECKED);

			::TrackPopupMenu (m_hMenuAudio, TPM_LEFTALIGN, rcBtn.left, rcBtn.top, 0, m_hWnd, NULL);
		}
			break;

		case IDC_BTN_AR:
		{
			RECT rcBtn;
			GetWindowRect (m_btnAR, &rcBtn);
			HMENU hMenuAR = GetSubMenu (m_hPopup, 1);
			::TrackPopupMenu (hMenuAR, TPM_LEFTALIGN, rcBtn.left, rcBtn.top, 0, m_hWnd, NULL);
		}
			break;

		case IDC_BTN_SPEED:
		{
			RECT rcBtn;
			GetWindowRect (m_btnSpeed, &rcBtn);
			HMENU hMenu = GetSubMenu (m_hPopup, 0);
			::TrackPopupMenu (hMenu, TPM_LEFTALIGN, rcBtn.left, rcBtn.top, 0, m_hWnd, NULL);
		}
			break;

		case ID_SPEED_0:
			fSpeed = (float)0.2;
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_Speed, &fSpeed);
			break;
		case ID_SPEED_1:
			fSpeed = (float)0.5;
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_Speed, &fSpeed);
			break;
		case ID_SPEED_2:
			fSpeed = (float)1.0;
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_Speed, &fSpeed);
			break;
		case ID_SPEED_3:
			fSpeed = (float)1.2;
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_Speed, &fSpeed);
			break;
		case ID_SPEED_4:
			fSpeed = (float)1.5;
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_Speed, &fSpeed);
			break;
		case ID_SPEED_5:
			fSpeed = (float)2.0;
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_Speed, &fSpeed);
			break;
		case ID_SPEED_6:
			fSpeed = (float)4.0;
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_Speed, &fSpeed);
			break;
		case ID_SPEED_8:
			fSpeed = (float)32.0;
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_Speed, &fSpeed);
			break;

		case ID_ASPECTRATIO_4:
			arInfo.nWidth = 4;
			arInfo.nHeight = 3;
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_AspectRatio, &arInfo);
			m_pMedia->UpdateView (&rcView);
		//	InvalidateRect (m_pWndVideo->GetWnd (), NULL, TRUE);
			break;
		case ID_ASPECTRATIO_16:
			arInfo.nWidth = 16;
			arInfo.nHeight = 9;
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_AspectRatio, &arInfo);
			m_pMedia->UpdateView (&rcView);
		//	InvalidateRect (m_pWndVideo->GetWnd (), NULL, TRUE);
			break;
		case ID_ASPECTRATIO_2:
		{
			RECT rcView;
			GetClientRect (m_pWndVideo->GetWnd (), &rcView);
			arInfo.nWidth = rcView.right;
			arInfo.nHeight = rcView.bottom;
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_AspectRatio, &arInfo);
			m_pMedia->UpdateView (&rcView);
		//	InvalidateRect (m_pWndVideo->GetWnd (), NULL, TRUE);
			break;
		}

		case ID_FILE_SEEKKEYFRAME:
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_SeekMode, (void *)YY_SEEK_KeyFrame);
			break;

		case ID_FILE_SEEKANYPOS:
			if (m_pMedia != NULL)
				m_pMedia->SetParam (YYPLAY_PID_SeekMode, (void *)YY_SEEK_AnyPosition);
			break;

		case IDC_BTN_CLOSE:
			DestroyWindow(hWnd);
			break;

		default:
		{
			if (wmId >= IDC_CMD_AUDIOTRACK && wmId < IDC_CMD_AUDIOTRACK + m_nAudioTrackNum)
			{
				if (m_pMedia != NULL)
				{
					int nTrackPlay = 0;
					m_pMedia->GetParam (YYPLAY_PID_AudioTrackPlay, &nTrackPlay);
					HMENU hMenu = GetSubMenu (m_hPopup, 2);
					CheckMenuItem (hMenu, IDC_CMD_AUDIOTRACK + nTrackPlay, MF_BYCOMMAND | MF_UNCHECKED);

					int nTrack = wmId - IDC_CMD_AUDIOTRACK;
					m_pMedia->SetParam (YYPLAY_PID_AudioTrackPlay, (void *)nTrack);
				}
			}
		}
			return hr;
		}
		break;

	case WM_KEYUP:
		if (wParam == 27)// ESC
		{
			if (m_pWndVideo != NULL && m_pWndVideo->IsFullScreen ())
				m_pWndVideo->SetFullScreen ();
		}
		break;

	case WM_HSCROLL:
		if (m_pMedia != NULL)
		{
			int nPos = 0;
			if (lParam = (LPARAM)m_sldPos)
			{
				nPos = SendMessage (m_sldPos, TBM_GETPOS, 0, 0);
				m_bSeeking = true;
				if (m_pMedia->SetPos (nPos * 1000) < 0)
					m_bSeeking = false;
				//m_mtNormal = SetCursor (m_mtWait);
			}
		}
		break;

	case WM_YYSLD_NEWPOS:
		if (m_pMedia != NULL)
		{
			if (IsWindowVisible (m_pWndVideo->GetWnd ()))
			{
				m_bSeeking = true;
				if (m_pMedia->SetPos ((int)wParam) < 0)
					m_bSeeking = false;


//				Sleep (10);
//				PlayNextFile (true);

			}
		}
		if (IsWindowVisible (m_pWndList->GetWnd ()))
		{
			m_pWndList->SetPos ((int)wParam);
		}
		break;

	case WM_VSCROLL:
		if (m_pMedia != NULL)
		{
			int nPos = 0;
			if (lParam = (LPARAM)m_sldVoice)
			{
				nPos = SendMessage (m_sldVoice, TBM_GETPOS, 0, 0);
				m_pMedia->SetVolume (100 - nPos);
			}
		}
		break;

	case WM_TIMER:
		if (wParam == TIMER_UPDATE_STATUS)
		{
			TCHAR szWinText[1024];
//			_stprintf (szWinText, _T("CPU % 3d %%"), CBaseUtils::GetCPULoad ());
//			if (m_pWndTitle != NULL)
//				m_pWndTitle->SetText (szWinText);
			if (m_pMedia != NULL && m_pWndVideo != NULL)// && IsWindowVisible (m_pWndVideo->GetWnd ()))
			{
				GetTimeText (m_pMedia->GetPos (), szWinText);
				SetWindowText (m_txtPos, szWinText);
				if (m_sldPos != NULL)
					SendMessage (m_sldPos, TBM_SETPOS, TRUE, m_pMedia->GetPos () / 1000);
				if (m_pSldPos != NULL)
					m_pSldPos->SetPos (m_pMedia->GetPos ());

//				if (m_pMedia->GetPos () > 20000)
//					SendMessage (m_hWnd, WM_CLOSE, 0, 0);
			}
		}
		else if (wParam == TIMER_PLAY_NEXTFILE)
		{
			KillTimer (m_hWnd, TIMER_PLAY_NEXTFILE);
			PlayNextFile (true);
		}
		else if (wParam == TIMER_UPDATE_AUDIOFILE)
		{
			KillTimer (m_hWnd, TIMER_UPDATE_AUDIOFILE);
			m_hAudioFileTimer = 0;

			m_pWndVideo->SetThumbnail (NULL);
			InvalidateRect (m_pWndVideo->GetWnd (), NULL, TRUE);
		}
		else if (wParam == TIMER_TEST_WORK)
		{
#if 0
			if (m_pMedia != NULL)
			{
				if (g_nAutoTestCount % 2 == 0)
					m_pMedia->SetParam (YYPLAY_PID_Disable_Video, (void *)YY_PLAY_VideoDisable_Render);
				else
					m_pMedia->SetParam (YYPLAY_PID_Disable_Video, (void *)YY_PLAY_VideoEnable);
			}
			g_nAutoTestCount++;
#endif // 0
#if 0
			int nTask = rand () % 10;
			if (nTask < 5)
			{
				int nDur = m_pMedia->GetDuration ();
				int nPos = nDur + 1000;// / (rand () % 10 + 2);
				m_pMedia->SetPos (nPos);
			}
			else if (nTask < 9)
			{
				PlayNextFile (true);
			}
			else
			{
				int nVol = rand () % 100;
			//	m_pMedia->SetVolume (nVol);
				PlayNextFile (true);
			}
#endif // 0
			int nPos = m_pMedia->GetPos () + 10000;
			m_pMedia->SetPos (nPos);
		}
		break;
#ifndef WINCE
	case WM_DROPFILES:
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
		break;
#endif // WINCE

	case WM_YYLIST_PLAYFILE:
		m_pWndVideo->SetThumbnail ((HBITMAP)lParam);
		m_bAutoPlay = false;
		OpenMediaFile ((TCHAR *)wParam, false);
		break;

	case WM_YYLIST_PLAYBACK:
		ShowWindow (m_pWndList->GetWnd (), SW_HIDE);
		ShowWindow (m_pWndVideo->GetWnd (), SW_SHOW);	 
		break;

	case WM_YYLIST_UPDATE:
		if (IsWindowVisible (m_pWndList->GetWnd ()))
			m_pSldPos->SetRange (0, (int)wParam);
		break;

	case WM_YYLIST_MOVING:
		m_pSldPos->SetPos ((int)wParam);
		break;

	case WM_YYLIST_SELECT:
#ifdef _OS_WINCE
		if (m_pWndTitle != NULL)
			m_pWndTitle->SetText ((TCHAR *)wParam);
		else
			SetWindowText (m_hWnd, (TCHAR *)wParam);
#else
		TCHAR szText[1024];
		_tcscpy (szText, _T("yyPlayer    "));
		_tcscat (szText, (TCHAR *)wParam);
		SetWindowText (m_hWnd, szText);
#endif // WINCE		
		break;

	case WM_VIEW_FullScreen:
		if (m_pMedia != NULL)
			m_pMedia->SetDisplay (m_pWndVideo->GetWnd (), m_nVideoRendType);
		break;

	case WM_VIEW_OnPaint:
#ifdef _OS_WIN32
		if (m_pMedia != NULL)
			m_pMedia->UpdateView (NULL);
#endif // _OS_WINPC
		break;

	case WM_VIEW_EVENT:
		if (m_pMedia != NULL)
		{
			if (wParam == 2)
			{
				m_pWndVideo->SetFullScreen ();
				break;
			}
#ifdef _OS_WINCE
			if (wParam == 1)
			{
				m_pWndVideo->SetFullScreen ();
				break;
			}
#endif // WINCE

			YYPLAY_STATUS status = m_pMedia->GetStatus ();
			if (status != YY_PLAY_Run && status != YY_PLAY_Pause)
				break;

			if (wParam == 1)
			{
#ifdef _OS_WINCE
				m_pWndVideo->SetFullScreen ();
#else
				if (m_pMedia->GetStatus () == YY_PLAY_Run)
				{
					m_pMedia->Pause ();
					if (m_pExtPlayer != NULL)
						m_pExtPlayer->Pause ();
				}
				else if (m_pMedia->GetStatus () == YY_PLAY_Pause)
				{
					m_pMedia->Start ();
					if (m_pExtPlayer != NULL)
						m_pExtPlayer->Start ();
				}
#endif // WINCE
			}
		}
		break;

	default:
		return hr;
	}

	return hr;
}

void CDemoUI::NotifyEvent (void * pUserData, int nID, void * pV1)
{
	int nRC = YY_ERR_NONE;
	CDemoUI * pDemo = (CDemoUI *)pUserData;

	if (nID == YY_EV_Play_Complete)
	{
		YYLOGT ("DemoUI", "Playback file completed!");

		if (_tcsstr (pDemo->m_szFile, _T(":/")) != NULL)
		{
			PostMessage (pDemo->m_hWnd, WM_COMMAND, IDC_BTN_LIST, 0);
		}
		else
		{
			pDemo->m_bAutoPlay = true;
			SetTimer (pDemo->m_hWnd, TIMER_PLAY_NEXTFILE, 100, NULL);
		}
	}
	else if (nID == YY_EV_Open_Complete)
	{
		if (pDemo->m_bPrepareClose)
			return;
		int nTryTimes = 0;
		while (!IsWindowVisible (pDemo->m_pWndVideo->GetWnd ()))
		{
			if (pDemo->m_pExtPlayer != NULL)
				break;
			yySleep (10000);
			nTryTimes++;
			if (nTryTimes > 300)
				break;
			if (pDemo->m_bPrepareClose)
				return;
		}

		int nDuration = pDemo->m_pMedia->GetDuration ();
		if (nDuration > 0)
		{
			if (pDemo->m_sldPos != NULL)
				SendMessage (pDemo->m_sldPos, TBM_SETRANGE, TRUE, MAKELONG (0, nDuration / 1000));
			if (pDemo->m_pSldPos != NULL)
				pDemo->m_pSldPos->SetRange (0, nDuration);
		}

		nRC = pDemo->m_pMedia->SetVolume (pDemo->m_nAudioVolume);
		SendMessage (pDemo->m_sldVoice, TBM_SETPOS, TRUE, 100 - pDemo->m_nAudioVolume);

		SetWindowText (pDemo->m_txtPos, _T("0:00:00"));
		TCHAR szTimeText[32];
		pDemo->GetTimeText (nDuration, szTimeText);
		SetWindowText (pDemo->m_txtDur, szTimeText);

//		YYMETA_Value metaValue;
//		strcpy (metaValue.szKey, "title");
//		pDemo->m_pMedia->GetParam (YY_PLAY_BASE_META, &metaValue);

//		pDemo->m_pMedia->SetPos (nDuration - 30000);

		ShowWindow (pDemo->m_pWndVideo->GetWnd (), SW_SHOW);
		ShowWindow (pDemo->m_pWndList->GetWnd (), SW_HIDE);

		int nSubtitle = 0;
		pDemo->m_pMedia->GetParam (YYPLAY_PID_SubTitle, &nSubtitle);
		if (nSubtitle > 0)
			SetWindowText (pDemo->m_btnSubTitle, _T("STT +"));
		else
			SetWindowText (pDemo->m_btnSubTitle, _T("STT -"));

		if (pDemo->m_pExtPlayer != NULL)
			pDemo->m_pExtPlayer->SetPlayer (pDemo->m_pMedia);
//		pDemo->m_pMedia->SetPos (100000);

		nRC = pDemo->m_pMedia->Start ();
		if (nRC < 0)
		{
			if (nRC != YY_ERR_STATUS)
				MessageBox (pDemo->m_hWnd, _T("Start to play failed!"), _T("Error"), MB_OK);
		}
		if (pDemo->m_pExtPlayer != NULL)
			pDemo->m_pExtPlayer->Start ();
//		SetCursor (pDemo->m_mtNormal);

		pDemo->m_bOpening = false;
		if (nRC >= 0)
			pDemo->m_hAudioFileTimer = SetTimer (pDemo->m_hWnd, TIMER_UPDATE_AUDIOFILE, 2000, NULL);

//		SetTimer (pDemo->m_hWnd, TIMER_PLAY_NEXTFILE, 3000, NULL);
	}
	else if (nID == YY_EV_Open_Failed)
	{
		if (pDemo->m_bPrepareClose)
			return;

		SetCursor (pDemo->m_mtNormal);
		pDemo->m_bOpening = false;
		if (pDemo->m_bAutoPlay)
			SetTimer (pDemo->m_hWnd, TIMER_PLAY_NEXTFILE, 100, NULL);
		else
		{
			MessageBox (pDemo->m_hWnd, _T("Open file failed!"), _T("Error"), MB_OK);
			ShowWindow (pDemo->m_pWndList->GetWnd (), SW_SHOW);
			ShowWindow (pDemo->m_pWndVideo->GetWnd (), SW_HIDE);
		}
	}
	else if (nID == YY_EV_Play_Duration)
	{
		int nDuration = pDemo->m_pMedia->GetDuration ();
		if (nDuration > 0)
		{
			if (pDemo->m_sldPos != NULL)
				SendMessage (pDemo->m_sldPos, TBM_SETRANGE, TRUE, MAKELONG (0, nDuration / 1000));
			if (pDemo->m_pSldPos != NULL)
				pDemo->m_pSldPos->SetRange (0, nDuration);

			TCHAR szTimeText[32];
			pDemo->GetTimeText (nDuration, szTimeText);
			SetWindowText (pDemo->m_txtDur, szTimeText);
		}
	}
	else if (nID == YY_EV_Seek_Complete)
	{
		pDemo->m_bSeeking = false;
		if (pDemo->m_pExtPlayer != NULL)
			pDemo->m_pExtPlayer->SetPos (0);
//		SetCursor (pDemo->m_mtNormal);
	}
	else if (nID == YY_EV_Seek_Failed)
	{
		pDemo->m_bSeeking = false;
		SetCursor (pDemo->m_mtNormal);
		MessageBox (pDemo->m_hWnd, _T("Sep Pos failed!"), _T("Error"), MB_OK);
	}
	else if (nID == YY_EV_Draw_FirstFrame)
	{
//		YYLOGI ("Used time: %d", GetTickCount () - pDemo->m_nDbgStartTime);
		if (pDemo->m_hAudioFileTimer != 0)
		{
			KillTimer (pDemo->m_hWnd, TIMER_UPDATE_AUDIOFILE);
			pDemo->m_hAudioFileTimer = 0;
		}

//		SetCursor (pDemo->m_mtNormal);
		pDemo->m_pWndVideo->SetThumbnail (NULL);

		RECT rcDraw;
		RECT rcRnd;
		pDemo->m_pMedia->GetParam (YYPLAY_PID_RenderArea, &rcRnd);
		RECT rcWnd;
		GetClientRect (pDemo->m_pWndVideo->GetWnd (), &rcWnd);
		if (rcRnd.left > rcWnd.left)
		{
			SetRect (&rcDraw, rcWnd.left, rcWnd.top, rcRnd.left, rcWnd.bottom);
			InvalidateRect (pDemo->m_pWndVideo->GetWnd (), &rcDraw, TRUE);
		}
		if (rcRnd.right < rcWnd.right)
		{
			SetRect (&rcDraw, rcRnd.right, rcWnd.top, rcWnd.right, rcWnd.bottom);
			InvalidateRect (pDemo->m_pWndVideo->GetWnd (), &rcDraw, TRUE);
		}
		if (rcRnd.top > rcWnd.top)
		{
			SetRect (&rcDraw, rcWnd.left, rcWnd.top, rcWnd.right, rcRnd.top);
			InvalidateRect (pDemo->m_pWndVideo->GetWnd (), &rcDraw, TRUE);
		}
		if (rcRnd.bottom < rcWnd.bottom)
		{
			SetRect (&rcDraw, rcWnd.left, rcRnd.bottom, rcWnd.right, rcWnd.bottom);
			InvalidateRect (pDemo->m_pWndVideo->GetWnd (), &rcDraw, TRUE);
		}

		int nDDMode = 0;
		pDemo->m_pMedia->GetParam (YYPLAY_PID_DDMode, &nDDMode);
		if (nDDMode == YY_DDM_Overlay)
			InvalidateRect (pDemo->m_pWndVideo->GetWnd (), NULL, TRUE);
	}
}

int CDemoUI::OpenMediaFile (TCHAR * pFile, bool bShow)
{
	DWORD				dwID = 0;
	OPENFILENAME		ofn;

	if (m_bOpening)// || m_bSeeking)
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
		{
			if (m_bRunning)
			{
				m_bRunning = false;
				m_pMedia->Start ();
				if (m_pExtPlayer != NULL)
					m_pExtPlayer->Start ();
			}
			return -1;
		}
	}
	else
	{
		_tcscpy (m_szFile, pFile);
	}

	m_pWndList->Pause ();

	CreatePlayList ();

	int nRC = PlaybackFile (m_szFile);
	if (nRC < 0)
	{
		TCHAR szMsg[256];
		_stprintf (szMsg, _T("Open file %s failed! return 0X%08X."), m_szFile, nRC);
		MessageBox (m_hWnd, szMsg, _T("Error"), MB_OK);

		m_pWndList->Start ();

		return -1;
	}
	else
	{
		if (bShow)
		{
			ShowWindow (m_pWndVideo->GetWnd (), SW_SHOW);
			ShowWindow (m_pWndList->GetWnd (), SW_HIDE);
		}
	}

	return 0;
}

int CDemoUI::PlaybackFile (TCHAR * pFile)
{
	if (m_pWndVideo != NULL)
	{
		m_pWndVideo->ShowLogo (false);
		m_pWndVideo->ShowOverlay (m_bOverlay);
	}

	_tcscpy (m_szFile, pFile);

	TCHAR * pName = _tcsrchr (pFile, _T('\\'));
	if (pName == NULL)
		pName = _tcsrchr (pFile, _T('/'));
	pName++;
#ifdef _OS_WINCE
	if (m_pWndTitle != NULL)
		m_pWndTitle->SetText (pFile);
	else
		SetWindowText (m_hWnd, pName);
#else
	TCHAR szText[1024];
	_tcscpy (szText, _T("yyPlayer    "));
	_tcscat (szText, pName);
	SetWindowText (m_hWnd, szText);
#endif // WINCE

	if (m_pMedia != NULL)
	{
		int nVolume = m_pMedia->GetVolume ();
		if (nVolume >= 0)
			m_nAudioVolume = nVolume;
	}

	if (m_pMedia != NULL)
		m_pMedia->SetParam (YYPLAY_PID_Prepare_Close, 0);

	if (m_pExtPlayer != NULL)
		m_pExtPlayer->Stop ();

	if (m_pMedia != NULL)
	{
/*
		if (m_pMedia->Close () < 0)
		{
			MessageBox (m_hWnd, _T("It was failed when Closed media file!"), _T("Error"), MB_OK);
			return YY_ERR_STATUS;
		}
*/
		if (m_pWndVideo->GetWnd () != NULL)
			m_pMedia->SetDisplay (m_pWndVideo->GetWnd (), m_nVideoRendType);
		if (m_bOverlay)
			m_pMedia->SetParam (YYPLAY_PID_DDMode, (void *)YY_DDM_Overlay);
		else
			m_pMedia->SetParam (YYPLAY_PID_DDMode, (void *)YY_DDM_Memory);

		m_pMedia->SetParam (YYPLAY_PID_SubTitle, (void *) 1);
		m_pMedia->SetParam (YYPLAY_PID_SubTT_View, (void *) m_hWnd);
		m_pMedia->SetParam (YYPLAY_PID_Sub_CallBack, &m_cbSubTT);
	}

//	return m_pMedia->TestPerformance (m_szFile, 0);

//	m_mtNormal = SetCursor (m_mtWait);
	m_bOpening = true;
	int nRC = 0;
	int nFlag = 0;
	if (m_pExtPlayer != NULL)
		nFlag = YY_OPEN_RNDA_EXT | YY_OPEN_RNDV_EXT;
		//nFlag = YY_OPEN_RNDV_CB | YY_OPEN_RNDA_CB;
#if 1
	nRC = m_pMedia->Open (m_szFile, nFlag);
#else
	nFlag = YY_OPEN_SRC_READ;
//	nFlag = YY_OPEN_SRC_BOX;
	if (m_pExtData == NULL)
		m_pExtData = new CExtData (m_hInst);
	if (nFlag == YY_OPEN_SRC_READ)
		nRC = m_pMedia->Open ((const TCHAR *)m_pExtData->GetExtData (YY_EXTDATA_Mux), nFlag);
	else
		nRC = m_pMedia->Open ((const TCHAR *)m_pExtData->GetExtData (YY_EXTDATA_Raw), nFlag);
#endif // 

	if (nRC < 0)
	{
		m_bOpening = false;
		return nRC;
	}

	return 0;
}

int CDemoUI::PlayNextFile (bool bNext)
{
	TCHAR szFile[1024];
	if (m_ppFiles == NULL || m_nFileNum <= 0)
		return -1;

	if (m_bOpening)// || m_bSeeking)
		return -1;

	if (bNext)
		m_nFileIdx++;
	else
		m_nFileIdx--;

	if (m_nFileIdx >= m_nFileNum)
		m_nFileIdx = 0;
	if (m_nFileIdx < 0)
		m_nFileIdx = m_nFileNum - 1;

	_tcscpy (szFile, m_szPath);
	_tcscat (szFile, m_ppFiles[m_nFileIdx]);

	int nRC = PlaybackFile (szFile);
	if (nRC < 0 && bNext)
		SetTimer (m_hWnd, TIMER_PLAY_NEXTFILE, 100, NULL);

	return 0;
}

int __cdecl yydemoui_compare_filename(const void *arg1, const void *arg2)
{
	return _tcsicmp (*((TCHAR **)arg1), *((TCHAR **)arg2));
}

void CDemoUI::CreatePlayList (void)
{
	TCHAR szFileName[1024];
	_tcscpy (szFileName, m_szFile);
	TCHAR * pPath = _tcsrchr (szFileName, _T('\\'));
	if (pPath == NULL)
		pPath = _tcsrchr (szFileName, _T('/'));
	if (pPath == NULL)
		return;
	*pPath = 0;
	_tcscpy (m_szPath, szFileName);
	_tcscat (m_szPath, _T("\\"));
	_tcscpy (szFileName, pPath + 1);

	TCHAR	szFilter[1024];
	_tcscpy (szFilter, m_szPath);
	_tcscat (szFilter, _T("*.*"));
	WIN32_FIND_DATA  data;
	HANDLE  hFind = FindFirstFile(szFilter,&data);
	if (hFind == INVALID_HANDLE_VALUE)
		return ;

	ReleasePlayList ();

	TCHAR * pExt = NULL;
	int		nExtLen = 0;
	char *	pExtChar = NULL;
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

		if (_tcsstr (m_szFileExt, pExt) == NULL)
			continue;

		m_nFileNum++;
	}while(FindNextFile(hFind, &data));
	FindClose (hFind);

	m_ppFiles = new TCHAR * [m_nFileNum];
	for (int i = 0; i < m_nFileNum; i++)
		m_ppFiles[i] = NULL;

	int nIndex = 0;
	int	nNameLen = 0;
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

		if (_tcsstr (m_szFileExt, pExt) == NULL)
			continue;

		nNameLen = (_tcslen (data.cFileName) + 2) * sizeof (TCHAR);
		m_ppFiles[nIndex] = new TCHAR[nNameLen];
		_tcscpy (m_ppFiles[nIndex], data.cFileName);
		nIndex++;
	}while(FindNextFile(hFind, &data));
	FindClose (hFind);	

	qsort(m_ppFiles, m_nFileNum, sizeof(TCHAR *), yydemoui_compare_filename);

	for (int i = 0; i < m_nFileNum; i++)
	{
		if (!_tcscmp (m_ppFiles[i], szFileName))
		{
			m_nFileIdx = i;
			break;
		}
	}
}

void CDemoUI::ReleasePlayList (void)
{
	if (m_ppFiles == NULL)
		return;

	for (int i = 0; i < m_nFileNum; i++)
	{
		if (m_ppFiles[i] != NULL)
			delete []m_ppFiles[i];
	}
	delete []m_ppFiles;
	m_ppFiles = NULL;

	m_nFileNum = 0;
	m_nFileIdx = 0;
}

bool CDemoUI::Create (HINSTANCE hInst, HWND hWnd, bool bFillItems)
{
	m_hInst = hInst;
	m_hWnd = hWnd;

	if (m_pMedia == NULL)
	{
		m_pMedia = new COMBoxMng (m_hInst, NULL);
		m_pMedia->SetNotifyFunc (NotifyEvent, this);

//		m_pMedia->SetParam (YYPLAY_PID_SubTT_Color, (void *)RGB (255, 0, 0));
//		m_pMedia->SetParam (YYPLAY_PID_SubTT_Size, (void *)48);
	}

#ifndef WINCE
	SetWindowLong (m_hWnd, GWL_EXSTYLE, WS_EX_ACCEPTFILES);
	SetWindowPos (m_hWnd, NULL, (GetSystemMetrics (SM_CXSCREEN) - WND_WIDTH) / 2, (GetSystemMetrics (SM_CYSCREEN) - WND_HEIGHT - 100 ) / 2, WND_WIDTH, WND_HEIGHT, 0);
#endif // WINCE

	int		nLeft = 8;
	int		nTop = 4 + CMD_BAR_HEIGHT;
	RECT	rcWnd;
	RECT	rcSld;
	RECT	rcView;

	GetClientRect (m_hWnd, &rcWnd);
	m_btnLoud = CreateButton (_T("Loud"), nLeft, nTop, IDC_BTN_LOUD);
	nTop += BTN_HEIGHT + 4;
	SetRect (&rcSld, nLeft + (BTN_WIDTH - SLD_HEIGHT) / 2, nTop, (BTN_WIDTH + SLD_HEIGHT) / 2, rcWnd.bottom - SLD_HEIGHT - BTN_HEIGHT * 6 - 26);
	m_sldVoice = CreateSlider (rcSld, IDC_SLD_VOICE, false);
	nTop = rcSld.bottom + 4;
	m_btnLow = CreateButton (_T("Low"), nLeft, nTop, IDC_BTN_LOW);
	nTop = nTop + BTN_HEIGHT + 4;
	m_btnMute = CreateButton (_T("Mute"), nLeft, nTop, IDC_BTN_MUTE);
	nTop = nTop + BTN_HEIGHT + 4;
	m_btnSubTitle = CreateButton (_T("SubTT"), nLeft, nTop, IDC_BTN_SUBTITLE);
	nTop = nTop + BTN_HEIGHT + 4;
	m_btnAudio = CreateButton (_T("Audio"), nLeft, nTop, IDC_BTN_AUDIO);
	nTop = nTop + BTN_HEIGHT + 4;
	m_btnAR = CreateButton (_T("Ratio"), nLeft, nTop, IDC_BTN_AR);
	nTop = nTop + BTN_HEIGHT + 4;
	m_btnSpeed = CreateButton (_T("Speed"), nLeft, nTop, IDC_BTN_SPEED);

	if (m_pWndVideo == NULL)
		m_pWndVideo = new CWndView (m_hInst);
	SetRect (&rcView, nLeft + BTN_WIDTH + 4, CMD_BAR_HEIGHT, rcWnd.right - BTN_WIDTH - 8, rcWnd.bottom - SLD_HEIGHT);
#ifdef _CPU_MSB2531
	m_pWndVideo->CreateWnd (m_hWnd, rcView, RGB (16, 8, 16));
#else
	m_pWndVideo->CreateWnd (m_hWnd, rcView, RGB (0, 0, 0));
#endif // _CPU_MSB2531
	if (m_pWndList == NULL)
		m_pWndList = new CWndPlayList (m_hInst, m_pMedia);
	m_pWndList->CreateWnd (m_hWnd, rcView, RGB (60, 60, 60), bFillItems);
	ShowWindow (m_pWndVideo->GetWnd (), SW_HIDE);

	SetRect (&rcSld, nLeft + BTN_WIDTH + 4, rcWnd.bottom - SLD_HEIGHT, rcWnd.right - BTN_WIDTH - 8, rcWnd.bottom);
//	m_sldPos = CreateSlider (rcSld, IDC_SLD_POS, true);
	m_pSldPos = new CWndSlider (m_hInst);
	m_pSldPos->CreateWnd (m_hWnd, rcSld, RGB (100, 100, 100));

	int nBtnDist = (rcWnd.bottom - CMD_BAR_HEIGHT - SLD_HEIGHT) / 9 - BTN_HEIGHT;
	nLeft = rcWnd.right - BTN_WIDTH - 4;
	nTop = 4 + CMD_BAR_HEIGHT;
	m_btnOpen = CreateButton (_T("Close"), nLeft, nTop, IDC_BTN_CLOSE);
	nTop += BTN_HEIGHT + nBtnDist;
	m_btnOpen = CreateButton (_T("Open"), nLeft, nTop, IDC_BTN_OPEN);
	nTop += BTN_HEIGHT + nBtnDist;
	m_btnOpen = CreateButton (_T("List"), nLeft, nTop, IDC_BTN_LIST);
	nTop += BTN_HEIGHT + nBtnDist;
	m_btnNext = CreateButton (_T("Next"), nLeft, nTop, IDC_BTN_NEXT);
	nTop += BTN_HEIGHT + nBtnDist;
	m_btnPrev = CreateButton (_T("Prev"), nLeft, nTop, IDC_BTN_PREV);
	nTop += BTN_HEIGHT + nBtnDist;
	m_btnInfo = CreateButton (_T("Info"), nLeft, nTop, IDC_BTN_INFO);
	nTop += BTN_HEIGHT + nBtnDist;
	m_btnStop = CreateButton (_T("Full"), nLeft, nTop, IDC_BTN_STOP);
	nTop += BTN_HEIGHT + nBtnDist;
	m_btnPause = CreateButton (_T("Pause"), nLeft, nTop, IDC_BTN_PAUSE);
	nTop += BTN_HEIGHT + nBtnDist;
	m_btnPlay = CreateButton (_T("Play"), nLeft, nTop, IDC_BTN_PLAY);

	nLeft = 0;
	nTop = rcWnd.bottom - SLD_HEIGHT;
	m_txtPos = CreateText (_T("0:00:00"), nLeft, nTop, BTN_WIDTH + 12, SLD_HEIGHT, IDC_TXT_POS);
	nLeft = rcWnd.right - BTN_WIDTH - 8;
	m_txtDur = CreateText (_T("0:00:00"), nLeft, nTop, BTN_WIDTH + 12, SLD_HEIGHT, IDC_TXT_DUR);

#ifdef _OS_WINCE
	if (m_pWndTitle == NULL)
	{
		RECT rcTitle;
		GetClientRect (m_hWnd, &rcWnd);
		SetRect (&rcTitle, 172, 0, rcWnd.right - 24, CMD_BAR_HEIGHT - 4);
		m_pWndTitle = new CWndBase (m_hInst);
		m_pWndTitle->CreateWnd (m_hWnd, rcTitle, RGB (188, 188, 188));
	}
#endif // WINCE

	m_mtWait = LoadCursor (NULL, IDC_WAIT);

	m_hPopup = LoadMenu (m_hInst, MAKEINTRESOURCE(IDR_MENU_POPUP));

	SendMessage (m_sldVoice, TBM_SETRANGE, TRUE, MAKELONG (0, 100));
	SetTimer (hWnd, TIMER_UPDATE_STATUS, 500, NULL);

//	m_pExtPlayer = new CExtPlayer (m_hInst, m_pWndVideo->GetWnd ());

//	SetWindowPos (m_pWndVideo->GetWnd (), NULL, 0, 0, 800, 480, 0);

//	OpenMediaFile (_T("\\ResidentFlash\\264.mp4"), true);
//	OpenMediaFile (_T("O:\\Data\\Works\\Customers\\SinoEmbed\\2013-12-18\\GirlTime.mpg"), true);
//	OpenMediaFile (_T("O:\\Data\\Temp\\Youtube.flv"), true);
//	OpenMediaFile (_T("O:\\Data\\Works\\Customers\\GeniaTech\\dumpts\\temp.ts"), true);

//	OpenMediaFile (_T("http://42.121.109.19/Files/h264-576.mkv"), true);
//	m_pMedia->SetParam (YYPLAY_PID_PDPFile, _T("C:\\Temp\\pdp001.mkv"));
//	OpenMediaFile (_T("file:pdp://42.121.109.19/Files/h264-576.mkv"), true);

//	OpenMediaFile (_T("http://qatest.visualon.com:8082/osmp/PD/H264/MP4/6M_H264_720x480_Baseline_24f.mp4"), true);
//	OpenMediaFile (_T("O:/Data/Works/HLS/V8/bipbopall.m3u8"), true);
//	OpenMediaFile (_T("O:/Data/Works/HLS/v10/bipbop_16x9_variant_v10_2.m3u8"), true);
//	OpenMediaFile (_T("http://3glivehntv.doplive.com.cn/video1/index_128k.m3u8?date=20140312163429&uid=&rnd=2014031216342953016&client=imgo&key=f8538258a8e10b327d4c89ce4af5d930"), true);
//	OpenMediaFile (_T("C:\\HLS\\gear1\\fileSequence0.ts"), true);
//	OpenMediaFile (_T("http://hls-iis.visualon.com:8082/hls/multibitrate/apple/bipbopall.m3u8"), true);
//	OpenMediaFile (_T("http://hls-iis.visualon.com:8082/hls/closedcaption/closedcaption/59C_640x480_1327K_29f.m3u8"), true);
//	OpenMediaFile (_T("http://hls-iis.visualon.com:8082/hls/multibitrate/apple/bipbopall.m3u8"), true);
//	OpenMediaFile (_T("http://hls-iis.visualon.com:8082/hls/multibitrate/apple/gear1/fileSequence0.ts"), true);
//	OpenMediaFile (_T("http://42.121.109.19/Files/hls/v8/bipbopall.m3u8"), true);
//	OpenMediaFile (_T("http://42.121.109.19/Files/hls/v10/bipbop_16x9_variant_v10_2.m3u8"), true);
//	OpenMediaFile (_T("http://hot.vrs.sohu.com/ipad1701130.m3u8"), true);

	m_nDbgStartTime = GetTickCount ();

	return true;
}

HWND CDemoUI::CreateButton (TCHAR * pText, int nLeft, int nTop, int nID)
{
	return CreateWindow (_T("button"), pText, WS_VISIBLE | WS_CHILD | BS_PUSHLIKE, 
						 nLeft, nTop, BTN_WIDTH, BTN_HEIGHT, m_hWnd, (HMENU)nID, m_hInst, NULL);
}

HWND CDemoUI::CreateText (TCHAR * pText, int nLeft, int nTop, int nW, int nH, int nID)
{
	return CreateWindow (_T("static"), pText, WS_VISIBLE | WS_CHILD, 
						 nLeft, nTop, nW, nH, m_hWnd, (HMENU)nID, m_hInst, NULL);
}

HWND CDemoUI::CreateSlider (RECT rcPos, int nID, bool bHorizon)
{
	DWORD dwStyle = WS_VISIBLE | WS_CHILD | TBS_BOTH | TBS_NOTICKS;
	if (!bHorizon)
		dwStyle |= TBS_VERT;

	return CreateWindow (_T("msctls_trackbar32"), _T(""), dwStyle, 
							rcPos.left, rcPos.top, rcPos.right - rcPos.left, rcPos.bottom - rcPos.top,
							m_hWnd, (HMENU)nID, m_hInst, NULL);
}

void CDemoUI::GetTimeText (long long llTime, TCHAR * szText)
{
	int nTextTime = (int)(llTime / 1000);
	int nHour = nTextTime / 3600;
	int nMins = (nTextTime % 3600) / 60;
	int nSecs = nTextTime % 60;

	_stprintf (szText, _T("%d:%02d:%02d"), nHour, nMins, nSecs);
}

int CDemoUI::yyMedeaSubCB (void * pUser, YY_BUFFER * pData)
{
//	YYLOGT ("DemoUI", "The Text is %s", (TCHAR *)pData->pBuff);
	return YY_ERR_NONE;
}
