/*******************************************************************************
	File:		CWndPlay.cpp

	Contains:	The play window implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CWndPlay.h"
#include "CFFMpegVideoEnc.h"

#include "CRegMng.H"
#include "resource.h"
#include "USystemFunc.h"
#include "UStringFunc.h"
#include "UBitmapFunc.h"
#include "UFileFunc.h"

#include "RPlayerDef.h"
#include "yyLog.h"

#pragma warning (disable : 4996)

#define	ZOOM_MAX_LEVEL		7
#define ZOOM_INIT			-2
#define ZOOM_UNSUPPORT		-1

float g_VideoZoom[] = {1, 1.5, 2, 2.5, 3, 4, 5, 6, 8, 12};
float g_PlaySpeed[] = {0.2, 0.4, 0.5, 0.8, 1.0, 1.5, 2.0, 4.0};

CWndPlay::CWndPlay(HINSTANCE hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hWnd (NULL)
	, m_bShow (false)
	, m_dwStyle (WS_VISIBLE)
	, m_bPopupMenu (false)
	, m_nClickCount (0)
	, m_nClickTimer (0)
	, m_nLastClkTime (0)
	, m_nMonitorTimer (0)
	, m_pMedia (NULL)
	, m_pExtSrc (NULL)
	, m_bPlayNextFile (false)
	, m_bMediaClose (false)
	, m_bOpening (false)
	, m_bAutoPlay (false)
	, m_nVRndType (YY_VRND_DDRAW)
	, m_nSeekMode (YY_SEEK_KeyFrame)
//	, m_nSeekMode (YY_SEEK_AnyPosition)
	, m_nRotateAngle (0)
	, m_nResizeY (1)
	, m_nZoomNum (ZOOM_INIT)
	, m_fZoomLevel (1.0)
	, m_nLastZoomX (0)
	, m_nLastZoomY (0)
	, m_bZoomSelect (false)
	, m_nTimerZoomSlt (0)
	, m_nAudioTrackNum (1)
	, m_nPlaySpeed (4)
	, m_nViewRatio (0)
	, m_hBrhBlack (NULL)
	, m_pWndBar (NULL)
	, m_nHideBarTimer (0)
	, m_nHideCursorTimer (0)
	, m_nLastMoveX (0)
	, m_nLastMoveY (0)
	, m_hCursor (NULL)
	, m_pWndSubTT (NULL)
	, m_pLstItem (NULL)
	, m_pPlayItem (NULL)
{
	SetObjectName ("CWndPlay");
	m_bScrActive = FALSE;
	SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &m_bScrActive, 0); 
	m_nScreenX = GetSystemMetrics (SM_CXSCREEN);
	m_nScreenY = GetSystemMetrics (SM_CYSCREEN);
	SetRectEmpty (&m_rcView);
	m_ptZoomSelect.x = -1;
	m_ptZoomSelect.y = -1;
	m_ptMouseDown.x = -1;
	m_ptMouseDown.y = -1;
	_tcscpy (m_szFile, _T(""));
	_tcscpy (m_szPath, _T(""));
	m_nVRndType = (YYRND_TYPE)CRegMng::g_pRegMng->GetIntValue (_T("VRndType"), m_nVRndType);
}	

CWndPlay::~CWndPlay(void)
{
	CRegMng::g_pRegMng->SetIntValue (_T("VRndType"), m_nVRndType);
	m_bMediaClose = true;
	if (m_hBrhBlack != NULL)
		DeleteObject (m_hBrhBlack);

	YY_DEL_P (m_pWndBar);
	YY_DEL_P (m_pWndSubTT);
}

bool CWndPlay::Create (HWND hWnd, CMediaEngine * pMedia, CExtSource * pExtSrc)
{
	m_hWnd = hWnd;
	m_hBrhBlack = CreateSolidBrush (RGB (0, 0, 0));

	m_pMedia = pMedia;
	m_pMedia->SetNotify (NotifyEvent, this);
	m_pMedia->SetParam (YYPLAY_PID_SeekMode, (void *)m_nSeekMode);
	m_pMedia->SetView (m_hWnd, m_nVRndType);
	m_pExtSrc = pExtSrc;

	m_pWndBar = new CPlayBar (m_hInst, this);
	m_pWndBar->Create (m_hWnd, m_pMedia);
	m_pWndBar->SetZoomRect (&m_rcZoom);

	OSVERSIONINFO verInfo;
	memset (&verInfo, 0, sizeof (verInfo));
	verInfo.dwOSVersionInfoSize = sizeof (verInfo);
	GetVersionEx (&verInfo);
	if (verInfo.dwMajorVersion >= 6)
	{
		m_pWndSubTT = new CWndSubTT (m_hInst, m_hWnd);
		m_pWndSubTT->SetMediaEngine (m_pMedia);
	}

	UpdateLang ();

	return true;
}

bool CWndPlay::Show (bool bShow)
{
	m_bShow = bShow;
	if (m_bShow)
		m_bPlayNextFile = false;
	return true;
}

bool CWndPlay::UpdateLang (void)
{
	HMENU hMenuWin = GetMenu (m_hWnd);
	HMENU hMenuPlay = GetSubMenu (hMenuWin, 1);
	HMENU hMenuVideo = GetSubMenu (hMenuPlay, 0);
	HMENU hMenuMode = GetSubMenu (hMenuVideo, 4);
	if (m_nVRndType == YY_VRND_GDI)
	{
		CheckMenuItem (hMenuMode, ID_VIDEO_GDI, MF_BYCOMMAND | MF_CHECKED);
		CheckMenuItem (hMenuMode, ID_VIDEO_DDRAW, MF_BYCOMMAND | MF_UNCHECKED);
	}
	else
	{
		CheckMenuItem (hMenuMode, ID_VIDEO_GDI, MF_BYCOMMAND | MF_UNCHECKED);
		CheckMenuItem (hMenuMode, ID_VIDEO_DDRAW, MF_BYCOMMAND | MF_CHECKED);
	}
	HMENU hMenuZoom = GetSubMenu (hMenuVideo, 0);
	if (m_bZoomSelect)
		CheckMenuRadioItem (hMenuZoom, ID_VIDEOZOOM_1, ID_ZOOM_SELECT, ID_ZOOM_SELECT, MF_BYCOMMAND | MF_CHECKED);
	else
		CheckMenuRadioItem (hMenuZoom, ID_VIDEOZOOM_1, ID_VIDEOZOOM_9, ID_VIDEOZOOM_1 + m_nZoomNum, MF_BYCOMMAND | MF_CHECKED);
	HMENU hMenuRotate = GetSubMenu (hMenuVideo, 2);
	CheckMenuRadioItem (hMenuRotate, ID_ROTATE_0, ID_ROTATE_270, ID_ROTATE_0 + m_nRotateAngle / 90, MF_BYCOMMAND | MF_CHECKED);

	HMENU hMenuSpeed = GetSubMenu (hMenuPlay, 6);
	CheckMenuRadioItem (hMenuSpeed, ID_SPEED_1, ID_SPEED_8, ID_SPEED_1 + m_nPlaySpeed, MF_BYCOMMAND | MF_CHECKED);

	HMENU hMenuRatio = GetSubMenu (hMenuPlay, 8);
	CheckMenuRadioItem (hMenuRatio, ID_RATIO_ORIGINAL, ID_RATIO_FITWINDOW, ID_RATIO_ORIGINAL + m_nViewRatio, MF_BYCOMMAND | MF_CHECKED);

	if (m_pWndSubTT != NULL)
		m_pWndSubTT->UpdateLang ();

	HMENU hMenuAudio = GetSubMenu (hMenuPlay, 2);
	int	i = 0;
	int nCount = GetMenuItemCount (hMenuAudio);
	for (i = nCount; i > 1; i--)
		RemoveMenu (hMenuAudio, i - 1, MF_BYPOSITION);
	m_nAudioTrackNum = 0;
	m_pMedia->GetParam (YYPLAY_PID_AudioTrackNum, &m_nAudioTrackNum);
	if (m_nAudioTrackNum <= 1)
	{
		CheckMenuItem (hMenuAudio, ID_AUDIO_TRACK1, MF_BYCOMMAND | MF_CHECKED);
		return true;
	}
	TCHAR szMenu[32];
	for (i = 1; i < m_nAudioTrackNum; i++)
	{
		_stprintf (szMenu, _T("%s %d"), yyLangGetText (YYTEXT_Track), i + 1);
		AppendMenu (hMenuAudio, MF_BYCOMMAND | MF_STRING, ID_AUDIO_TRACK1 + i, szMenu);
	}
	CheckMenuItem (hMenuAudio, ID_AUDIO_TRACK1, MF_BYCOMMAND | MF_UNCHECKED);
	int nTrackPlay = 0;
	m_pMedia->GetParam (YYPLAY_PID_AudioTrackPlay, &nTrackPlay);
	CheckMenuItem (hMenuAudio, ID_AUDIO_TRACK1 + nTrackPlay, MF_BYCOMMAND | MF_CHECKED);

	return true;
}

int CWndPlay::ShowFullScreen (void)
{
	if (IsFullScreen ())
	{
		SetWindowLong (m_hWnd, GWL_STYLE, m_dwStyle);
		SetMenu (m_hWnd, m_hMenu);
		SetWindowPos (m_hWnd, HWND_TOP, m_rcView.left, m_rcView.top, 
						m_rcView.right - m_rcView.left, m_rcView.bottom - m_rcView.top, 0);
	}
	else
	{
		m_dwStyle = GetWindowLong (m_hWnd, GWL_STYLE);
		m_hMenu = GetMenu (m_hWnd);
		SetWindowLong (m_hWnd, GWL_STYLE, WS_VISIBLE);
		SetMenu (m_hWnd, NULL);
		GetWindowRect (m_hWnd, &m_rcView);
		SetWindowPos (m_hWnd, HWND_TOP, 0, 0, m_nScreenX, m_nScreenY, 0);
	}
	return YY_ERR_NONE;
}

bool CWndPlay::IsFullScreen (void)
{
	RECT rcView;
	GetWindowRect (m_hWnd, &rcView);
	if (rcView.right == m_nScreenX && rcView.bottom == m_nScreenY)
		return true;
	else
		return false;
}

bool CWndPlay::GetZoomSelect (RECT * pZoomSelect)
{
	if (!m_bZoomSelect || pZoomSelect == NULL)
		return false;
	if (m_ptZoomSelect.x < 0 || m_ptZoomSelect.y < 0)
		return false;
	SetRect (pZoomSelect, m_ptZoomSelect.x, m_ptZoomSelect.y, m_nLastMoveX, m_nLastMoveY);
	return true;
}

int CWndPlay::PlaybackItem (CListItem * pItem, void * pList)
{
	if (pItem == NULL)
		return YY_ERR_FAILED;
	TCHAR szFile[1024];
	memset (szFile, 0, sizeof (szFile));
	if (pItem->GetFile (szFile, sizeof (szFile)))
	{
		if (PlaybackFile (szFile, pList) != YY_ERR_NONE)
			return YY_ERR_FAILED;
	}
	pItem->m_bModified = true;
	m_pPlayItem = pItem;
	return YY_ERR_NONE;
}

int CWndPlay::PlaybackFile (TCHAR * pFile, void * pList)
{
	if (pFile == NULL || _tcslen (pFile) < 4)
		return YY_ERR_FAILED;
	m_pPlayItem = NULL;
	m_pLstItem = (CObjectList<CListItem> *)pList;

	int		nRC = 0;
	TCHAR * pName = _tcsrchr (pFile, _T('\\'));
	if (pName == NULL)
		pName = _tcsrchr (pFile, _T('/'));
	pName++;
	SendMessage (m_hWnd, WM_PLAYER_SetWinText, (WPARAM)pName, 0);

	nRC = m_pMedia->SetParam (YYPLAY_PID_Prepare_Close, 0);
	m_pMedia->Close ();

	SetRectEmpty (&m_rcVideo);
	m_bOpening = true;
	m_nZoomNum = ZOOM_INIT;
	m_fZoomLevel = 1.0;
	if (pFile != m_szFile)
		_tcscpy (m_szFile, pFile);

	if (yyGetProtType (m_szFile) == YY_PROT_FILE)
	{
#if 1
		int nFlag = YY_OPEN_SRC_READ;
		TCHAR * pSource = (TCHAR *)m_pExtSrc->GetExtData (YY_EXTDATA_Mux, m_szFile);
		if (pSource == NULL)
			return YY_ERR_FAILED;
		nRC = m_pMedia->Open (pSource, nFlag);
#else
		nRC = m_pMedia->Open (m_szFile, 0);
#endif // 0
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
	m_pLstItem = (CObjectList<CListItem> *)pList;
	return 0;
}

int CWndPlay::PlayNextFile (bool bNext)
{
	if (m_pLstItem == NULL)
		return YY_ERR_FAILED;

	TCHAR *	pPlayName = _tcsrchr (m_szFile, _T('\\'));
	if (pPlayName == NULL)
		return YY_ERR_FAILED;
	pPlayName++;

	CListItem *				pItem = NULL;
	CListItem *				pPlayItem = NULL;
	CObjectList<CListItem>	lstFile;
	NODEPOS pos = m_pLstItem->GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_pLstItem->GetNext (pos);
		if (pItem->m_nType != ITEM_Audio && pItem->m_nType != ITEM_Video && pItem->m_nType != ITEM_Image)
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
				if (!_tcscmp (pItem->m_pName, pPlayName))
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
				if (!_tcscmp (pItem->m_pName, pPlayName))
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

	if (m_pWndSubTT != NULL)
		m_pWndSubTT->OnMediaClose ();

	int nRC = PlaybackItem (pPlayItem, m_pLstItem);
	m_bPlayNextFile = true;
	if (nRC < 0 && bNext)
		SetTimer (m_hWnd, WT_PLAY_PlayNextFile, 100, NULL);
	return YY_ERR_NONE;
}

int CWndPlay::Start (void)
{
	if (m_pMedia == NULL)
		return YY_ERR_NONE;
	if (m_pWndSubTT != NULL)
		m_pWndSubTT->OnMediaStart ();
	if (m_pPlayItem != NULL && m_pPlayItem->m_nRotate > 0)
	{
		m_nRotateAngle = m_pPlayItem->m_nRotate;
		m_pWndBar->SetRotate (m_nRotateAngle);
		m_pMedia->SetParam (YYPLAY_PID_Rotate, (void *)m_nRotateAngle);
		UpdateLang ();
	}
	if (m_bScrActive) 
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, 0); 

	return m_pMedia->Run ();
}

int CWndPlay::Close (void)
{
	if (m_pMedia == NULL)
		return YY_ERR_NONE;
	if (m_bScrActive) 
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, NULL, 0); 
	if (m_pWndSubTT != NULL)
		m_pWndSubTT->OnMediaClose ();
	return m_pMedia->Close ();
}

void CWndPlay::NotifyEvent (void * pUserData, int nID, void * pV1)
{
	((CWndPlay *)pUserData)->HandleEvent (nID, pV1);;
}

void CWndPlay::HandleEvent (int nID, void * pV1)
{
	int nRC = 0;
	if (m_bMediaClose)
		return;

	if (m_pWndBar != NULL)
		m_pWndBar->HandleEvent (nID, pV1);
	
	switch (nID)
	{
	case YY_EV_Open_Complete:
	{
		m_pMedia->SetParam (YYPLAY_PID_DisableEraseBG, 0);
		YY_VIDEO_FORMAT * pFmtVideo = NULL;
		m_pMedia->GetParam (YYPLAY_PID_Fmt_Video, &pFmtVideo);
		if (pFmtVideo != NULL)
		{
			SetRect (&m_rcVideo, 0, 0, pFmtVideo->nWidth, pFmtVideo->nHeight);
			SetRect (&m_rcZoom, 0, 0, pFmtVideo->nWidth, pFmtVideo->nHeight);
		}
		m_bOpening = false;	
		m_nRotateAngle = 0;
		m_nPlaySpeed = 4;
		m_nViewRatio = 0;
		if (m_pPlayItem != NULL && m_pPlayItem->m_nPlayPos > 0)
		{
			m_pMedia->SetPos (m_pPlayItem->m_nPlayPos);
			m_pPlayItem->m_nPlayPos = 0;
		}
		UpdateLang ();
		ShowBar (true);
		PostMessage (m_hWnd, WM_PLAY_Play, 0, 0);
		return;
	}

	case YY_EV_Open_Failed:
		m_bOpening = false;
		if (m_bAutoPlay)
			SetTimer (m_hWnd, WT_PLAY_PlayNextFile, 100, NULL);
		else
		{
			MessageBox (m_hWnd, yyLangGetText (YYTEXT_OpenFail), yyLangGetText (YYTEXT_Error), MB_OK);
			PostMessage (m_hWnd, WM_PLAY_Close, 0, 0);
		}
		break;

	case YY_EV_Play_Complete:
		if (m_pPlayItem != NULL)
			m_pPlayItem->m_nPlayPos = 0;
		if (m_pLstItem == NULL || _tcsstr (m_szFile, _T(":/")) != NULL)
		{
			PostMessage (m_hWnd, WM_PLAY_Close, 0, 0);
		}
		else
		{
			m_bAutoPlay = true;
			SetTimer (m_hWnd, WT_PLAY_PlayNextFile, 100, NULL);
		}		
		break;

	case YY_EV_Video_Changed:
	{
		YY_VIDEO_FORMAT * pFmtVideo = NULL;
		m_pMedia->GetParam (YYPLAY_PID_Fmt_Video, &pFmtVideo);
		if (pFmtVideo != NULL)
		{
			SetRect (&m_rcVideo, 0, 0, pFmtVideo->nWidth, pFmtVideo->nHeight);
			SetRect (&m_rcZoom, 0, 0, pFmtVideo->nWidth, pFmtVideo->nHeight);
		}
		OnZoomVideo (1.0, true);
		break;
	}

	case YY_EV_Play_Status:
		if (m_pMedia->GetStatus () == YY_PLAY_Run)
			SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
		else
			SetThreadExecutionState(ES_CONTINUOUS);
		break;

	case YY_EV_Draw_FirstFrame:
		if (m_bPlayNextFile)
			UpdateBackgroud ();
		break;

	default:
		break;
	}
}

LRESULT	CWndPlay::MsgProc (HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!m_bShow && uMsg != WM_COMMAND)
		return S_FALSE;

	LRESULT lRC = S_FALSE;
	if (m_pWndBar != NULL)
		lRC = m_pWndBar->MsgProc (hWnd, uMsg, wParam, lParam);
	if (lRC != S_FALSE)
	{
		if (uMsg == WM_COMMAND || uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP)
			ShowBar (true);
		return lRC;
	}

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
	case WM_SIZE:
		return OnSize (uMsg, wParam, lParam);
	case WM_MOVE:
		return OnMove (uMsg, wParam, lParam);
	case WM_KEYDOWN:
		return OnKeyDown (uMsg, wParam, lParam);
	case WM_KEYUP:
		return OnKeyUp (uMsg, wParam, lParam);
	case WM_TIMER:
		return OnTimer (uMsg, wParam, lParam);
	case WM_EXITMENULOOP:
		m_bPopupMenu = false;
		break;
	case WM_ERASEBKGND:
		return OnEraseBG (uMsg, wParam, lParam);
	case WM_PAINT:
		return OnPaint (uMsg, wParam, lParam);
	default:
		break;
	}
	return S_FALSE;
}

LRESULT	CWndPlay::OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRC = S_FALSE;
	int		wmId = LOWORD(wParam);
	if (wmId == ID_PLAY_NEXT)
	{
		PlayNextFile (true);
		return S_OK;
	}
	else if (wmId == ID_PLAY_PREV)
	{
		PlayNextFile (false);
		return S_OK;
	}
	else if (wmId == ID_PLAY_RETURN)
	{
		SendMessage (m_hWnd, WM_PLAY_Close, 0, 0);
		return S_OK;
	}
	else if (wmId == ID_VIDEO_GDI)
	{
		m_nVRndType = YY_VRND_GDI;
		m_pMedia->SetView (m_hWnd, m_nVRndType);
		UpdateLang ();
		return S_OK;
	}
	else if (wmId == ID_VIDEO_DDRAW)
	{
		m_nVRndType = YY_VRND_DDRAW;
		m_pMedia->SetView (m_hWnd, m_nVRndType);
		UpdateLang ();
		return S_OK;
	}
	else if (wmId == ID_VIDEO_FULLSCREEN)
	{
		ShowFullScreen ();
		return S_OK;
	}
	else if (wmId >= ID_AUDIO_TRACK1 && wmId <= ID_AUDIO_TRACK1 + 16)
	{
		int nTrack = wmId - ID_AUDIO_TRACK1;
		m_pMedia->SetParam (YYPLAY_PID_AudioTrackPlay, (void *)nTrack);
		UpdateLang ();
		return S_OK;
	}
	else if (wmId >= ID_VIDEOZOOM_1 && wmId <= ID_VIDEOZOOM_9)
	{
		OnZoomVideo (g_VideoZoom[wmId - ID_VIDEOZOOM_1], true);
		return S_OK;
	}
	else if (wmId == ID_ZOOM_SELECT)
	{
		m_bZoomSelect = true;
		m_nTimerZoomSlt = SetTimer (m_hWnd, WT_PLAY_ZoomSelect, 5000, NULL);
		return S_OK;
	}
	else if (wmId >= ID_ROTATE_0 && wmId <= ID_ROTATE_270)
	{
		m_nRotateAngle = 0;
		if (wmId == ID_ROTATE_90)
			m_nRotateAngle = 90;
		else if (wmId == ID_ROTATE_180)
			m_nRotateAngle = 180;
		else if (wmId == ID_ROTATE_270)
			m_nRotateAngle = 270;
		if (m_pPlayItem != NULL && m_pPlayItem->m_nRotate != m_nRotateAngle)
		{
			m_pPlayItem->m_nRotate = m_nRotateAngle;
			m_pPlayItem->m_bModified = true;
		}
		OnZoomVideo (1.0, true);
		m_pMedia->SetParam (YYPLAY_PID_Rotate, (void *)m_nRotateAngle);
		m_pWndBar->SetRotate (m_nRotateAngle);
		UpdateBackgroud ();
		UpdateLang ();
		return S_OK;
	}
	else if (wmId >= ID_SPEED_1 && wmId <= ID_SPEED_8)
	{
		m_nPlaySpeed = wmId - ID_SPEED_1;
		m_pMedia->SetParam (YYPLAY_PID_Speed, (void *)&g_PlaySpeed[m_nPlaySpeed]);
		return S_OK;
	}
	else if (wmId >= ID_RATIO_ORIGINAL && wmId <= ID_RATIO_FITWINDOW)
	{
		m_nViewRatio = wmId - ID_RATIO_ORIGINAL;
		YYPLAY_ARInfo arInfo;
		if (wmId == ID_RATIO_ORIGINAL)
		{
			arInfo.nWidth = 1;
			arInfo.nHeight = 1;
		}
		else if (wmId == ID_RATIO_43)
		{
			arInfo.nWidth = 4;
			arInfo.nHeight = 3;
		}
		else if (wmId == ID_RATIO_169)
		{
			arInfo.nWidth = 16;
			arInfo.nHeight = 9;
		}
		else
		{
			RECT rcView;
			GetClientRect (m_hWnd, &rcView);
			arInfo.nWidth = rcView.right;
			arInfo.nHeight = rcView.bottom;
		}
		m_pMedia->SetParam (YYPLAY_PID_AspectRatio, (void *)&arInfo);
		return S_OK;
	}
	else if (wmId == ID_VIDEO_CAPTURE)
	{
		CaptureVideo ();
		return S_OK;
	}

	if (m_pWndSubTT != NULL)
	{
		lRC = m_pWndSubTT->OnCommand (wmId);
		if (lRC != S_FALSE)
			return lRC;
	}

	return S_FALSE;
}

LRESULT	CWndPlay::OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ShowCursor (true);
	if (m_bZoomSelect)
	{
		m_ptZoomSelect.x = LOWORD (lParam);
		m_ptZoomSelect.y = HIWORD (lParam);
	}
	m_ptMouseDown.x = LOWORD (lParam);
	m_ptMouseDown.y = HIWORD (lParam);
	return S_FALSE;
}

LRESULT	CWndPlay::OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bZoomSelect)
	{
		OnZoomSelect (wParam, lParam);
		return S_OK;
	}
	if (m_ptMouseDown.x >= 0 && m_ptMouseDown.y >= 0)
	{
		if (abs (LOWORD (lParam) - m_ptMouseDown.x) > 16 || abs (HIWORD (lParam) - m_ptMouseDown.y) > 16)
		{
			m_ptMouseDown.x = -1;
			m_ptMouseDown.y = -1;
			return S_OK;
		}
	}
	m_ptMouseDown.x = -1;
	m_ptMouseDown.y = -1;
	
	m_nClickCount++;
	if (m_nClickTimer == 0)
		m_nClickTimer = SetTimer (m_hWnd, WT_PLAY_LButtonUP, WPT_LButtonUp_Delay, NULL);
	m_nLastZoomX = 0;
	m_nLastZoomY = 0;
	return S_OK;
}

LRESULT	CWndPlay::OnRButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_ptZoom.x = LOWORD (lParam);
	m_ptZoom.y = HIWORD (lParam);

	POINT pt;
	pt.x = LOWORD (lParam);
	pt.y = HIWORD (lParam);
	ClientToScreen (m_hWnd, &pt);

	HMENU hPopup = NULL;
	if (CLangText::g_pLang->GetLang () == YYLANG_ENG)
		hPopup = LoadMenu (m_hInst, MAKEINTRESOURCE(IDR_MENU_POPUP));
	else
		hPopup = LoadMenu (m_hInst, MAKEINTRESOURCE(IDR_MENU_POPUP_CHN));
	HMENU hMenu = GetSubMenu (hPopup, 3);
	if (m_nZoomNum == ZOOM_INIT)
	{
		int nRC = m_pMedia->SetParam (YYPLAY_PID_VideoZoomIn, NULL);
		if (nRC == YY_ERR_NONE)
			m_nZoomNum = 0;
		else
			m_nZoomNum = ZOOM_UNSUPPORT;
		m_fZoomLevel = 1.0;
	}
	if (m_nZoomNum >= 0)
	{
		HMENU hMenuZoom = GetSubMenu (hMenu, 0);
		if (m_bZoomSelect)
			CheckMenuRadioItem (hMenuZoom, ID_VIDEOZOOM_1, ID_ZOOM_SELECT, ID_ZOOM_SELECT, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuRadioItem (hMenuZoom, ID_VIDEOZOOM_1, ID_VIDEOZOOM_9, ID_VIDEOZOOM_1 + m_nZoomNum, MF_BYCOMMAND | MF_CHECKED);
	}
	else
	{
		EnableMenuItem (hMenu, 0, MF_BYPOSITION | MF_GRAYED);
	}

	HMENU hMenuRotate = GetSubMenu (hMenu, 2);
	CheckMenuRadioItem (hMenuRotate, ID_ROTATE_0, ID_ROTATE_270, ID_ROTATE_0 + m_nRotateAngle / 90, MF_BYCOMMAND | MF_CHECKED);

	HMENU hMenuSpeed = GetSubMenu (hMenu, 6);
	CheckMenuRadioItem (hMenuSpeed, ID_SPEED_1, ID_SPEED_8, ID_SPEED_1 + m_nPlaySpeed, MF_BYCOMMAND | MF_CHECKED);

	HMENU hMenuRatio = GetSubMenu (hMenu, 8);
	CheckMenuRadioItem (hMenuRatio, ID_RATIO_ORIGINAL, ID_RATIO_FITWINDOW, ID_RATIO_ORIGINAL + m_nViewRatio, MF_BYCOMMAND | MF_CHECKED);

	m_bPopupMenu = true;
	TrackPopupMenu (hMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, m_hWnd, NULL);
	DestroyMenu (hPopup);

	return S_FALSE;
}

LRESULT	CWndPlay::OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ShowCursor (true);

	if (abs (m_nLastMoveX - LOWORD (lParam)) > 4 || abs (m_nLastMoveY - HIWORD (lParam)) > 4)
	{
		if (yyGetSysTime () - m_nLastClkTime > 1500)
			ShowBar (true);
	}
	m_nLastMoveX = LOWORD (lParam);
	m_nLastMoveY = HIWORD (lParam);
	if (m_bZoomSelect)
	{
		if (m_nTimerZoomSlt != 0)
		{
			KillTimer (m_hWnd, WT_PLAY_ZoomSelect);
			m_nTimerZoomSlt = SetTimer (m_hWnd, WT_PLAY_ZoomSelect, 2000, NULL);
		}
		if (m_pMedia->GetStatus () != YY_PLAY_Run)
			InvalidateRect (m_hWnd, NULL, FALSE);
		return S_OK;
	}
	if (wParam == MK_LBUTTON && m_nZoomNum > 0)
	{
		if (m_nLastZoomX != 0 || m_nLastZoomY != 0)
		{
			int nX = LOWORD (lParam) - m_nLastZoomX;
			int nY = HIWORD (lParam) - m_nLastZoomY;
			nX = nX & ~3;
			nY = nY & ~1;
			OnZoomMove (nX, nY);
		}
		m_nLastZoomX = LOWORD (lParam);
		m_nLastZoomY = HIWORD (lParam);
	}

	return S_FALSE;
}

LRESULT CWndPlay::OnMouseWheel (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int nKey = LOWORD (wParam);
	if (HIWORD (wParam) == WHEEL_DELTA)
	{
		if (nKey == MK_CONTROL || nKey == MK_LBUTTON)
			OnZoomVideo (m_fZoomLevel * 1.05, false);
		else if (nKey == MK_SHIFT || nKey == MK_RBUTTON)
			OnZoomMove (-8, 0);
		else
			OnZoomMove (0, 8);
	}
	else
	{
		if (nKey == MK_CONTROL || nKey == MK_LBUTTON)
			OnZoomVideo (m_fZoomLevel * 0.95, false);
		else if (nKey == MK_SHIFT || nKey == MK_RBUTTON)
			OnZoomMove (8, 0);
		else		
			OnZoomMove (0, -8);
	}
	return S_FALSE;
}

LRESULT	CWndPlay::OnKeyDown (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_nZoomNum > 0)
	{
		if (wParam == VK_UP)
			OnZoomMove (0, 8);
		else if (wParam == VK_DOWN)
			OnZoomMove (0, -8);
		else if (wParam == VK_LEFT)
			OnZoomMove (8, 0);
		else if (wParam == VK_RIGHT)
			OnZoomMove (-8, 0);
	}
	else
	{
		if (wParam == VK_LEFT)
			m_pWndBar->SetPos (false, 0);
		else if (wParam == VK_RIGHT)
			m_pWndBar->SetPos (true, 0);
		else if (wParam == VK_UP)
			m_pWndBar->SetVolume (10);
		else if (wParam == VK_DOWN)
			m_pWndBar->SetVolume (-10);
	}
	return S_FALSE;
}

LRESULT	CWndPlay::OnKeyUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_ESCAPE)
	{
		if (IsFullScreen ())
		{
			ShowFullScreen ();
			return S_OK;
		}
	}
	else if (wParam == VK_F8)
	{
		CaptureVideo ();
		return S_OK;
	}
	else if (wParam == VK_SPACE)
	{
		if (m_pMedia->GetStatus () == YY_PLAY_Run)
			m_pMedia->Pause ();
		else if (m_pMedia->GetStatus () == YY_PLAY_Pause)
			m_pMedia->Run ();
		return S_OK;
	}
	return S_FALSE;
}

LRESULT	CWndPlay::OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WT_PLAY_LButtonUP)
	{
		KillTimer (m_hWnd, WT_PLAY_LButtonUP);
		m_nClickTimer = 0;
		if (m_nClickCount == 1)
		{
			if (m_pMedia->GetStatus () == YY_PLAY_Run)
			{
				if (m_pWndBar->IsShow ())
					ShowBar (false);
				else
				{
					m_pMedia->Pause ();
					ShowBar (true);
				}
			}
			else
			{
				m_pMedia->Run ();
				ShowBar (false);
			}
		}
		else if (m_nClickCount == 2)
			ShowFullScreen ();
		else if (m_nClickCount >= 3)
			SendMessage (m_hWnd, WM_PLAY_Close, 0, 0);
		m_nClickCount = 0;
		m_nLastClkTime = yyGetSysTime ();
	}
	else if (wParam == WT_PLAY_HideBar)
	{
		ShowBar (false);
	}
	else if (wParam == WT_PLAY_HideCursor)
	{
		ShowCursor (false);
	}
	else if (wParam == WT_PLAY_PlayNextFile)
	{
		KillTimer (m_hWnd, WT_PLAY_PlayNextFile);
		PlayNextFile (true);
	}
	else if (wParam == WT_PLAY_TurnOnMonitor)
	{
		PostMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, (LPARAM)YYMonitro_PowerOn);
		return S_OK;
	}
	else if (wParam == WT_PLAY_ZoomSelect)
	{
		if (m_nTimerZoomSlt != 0)
		{
			KillTimer (m_hWnd, WT_PLAY_ZoomSelect);
			m_nTimerZoomSlt = 0;
		}
		m_bZoomSelect = false;
		m_ptZoomSelect.x = -1;
		m_ptZoomSelect.y = -1;
	}
	return S_FALSE;
}

LRESULT	CWndPlay::OnSize (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CAutoLock lock (m_pWndBar->GetDrawLock ());
	if (m_pMedia != NULL)
	{
		if (m_nViewRatio == 3)
		{
			YYPLAY_ARInfo arInfo;
			RECT rcView;
			GetClientRect (m_hWnd, &rcView);
			arInfo.nWidth = rcView.right;
			arInfo.nHeight = rcView.bottom;
			m_pMedia->SetParam (YYPLAY_PID_AspectRatio, (void *)&arInfo);
		}
		m_pMedia->SetView (m_hWnd, m_nVRndType);
	}
	if (m_pWndSubTT != NULL)
		m_pWndSubTT->OnSizeMove ();
	return S_FALSE;
}

LRESULT	CWndPlay::OnMove (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pWndSubTT != NULL)
		m_pWndSubTT->OnSizeMove ();
	return S_FALSE;
}

LRESULT	CWndPlay::OnEraseBG (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pMedia->GetStatus () <= YY_PLAY_Open)
		return S_OK;
	if (m_pWndBar->IsShow ())
		return S_OK;

	UpdateBackgroud ();

	return S_OK;
}

LRESULT	CWndPlay::OnPaint (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(m_hWnd, &ps);
	EndPaint(m_hWnd, &ps);
	return S_OK;
}

bool CWndPlay::ShowBar (bool bShow)
{
	if (m_nHideBarTimer != 0)
		KillTimer (m_hWnd, WT_PLAY_HideBar);
	m_nHideBarTimer = 0;
	if (bShow)
	{
		m_pWndBar->Show (true);
		m_nHideBarTimer = SetTimer (m_hWnd, WT_PLAY_HideBar, WPT_BAR_ShowTime, NULL);
	}
	else
	{
		if (m_pMedia->GetStatus () == YY_PLAY_Run)
			m_pWndBar->Show (false);
	}
	OnEraseBG (0, 0, 0);
	return true;
}

bool CWndPlay::ShowCursor (bool bShow)
{
	if (m_nHideCursorTimer != 0)
		KillTimer (m_hWnd, WT_PLAY_HideCursor);
	m_nHideCursorTimer = 0;
	if (!IsFullScreen ())
	{
		if (m_hCursor != NULL)
			SetCursor (m_hCursor);
		return true;
	}

	if (bShow)
	{
		if (m_hCursor != NULL)
			SetCursor (m_hCursor);
		SetTimer (m_hWnd, WT_PLAY_HideCursor, WPT_BAR_ShowTime + 1000, NULL);
	}
	else
	{
		if (m_bPopupMenu)
		{
			SetTimer (m_hWnd, WT_PLAY_HideCursor, WPT_BAR_ShowTime + 1000, NULL);
			return true;
		}
		m_hCursor = SetCursor (NULL);
	}
	return true;
}

void CWndPlay::OnZoomVideo (float fLevel, bool bFix)
{
	if (m_fZoomLevel == fLevel)
		return;
	CAutoLock lock (m_pWndBar->GetDrawLock ());
	if (fLevel < 1.0)
		fLevel = 1.0;
	if (fLevel > g_VideoZoom[ZOOM_MAX_LEVEL-1])
		fLevel = g_VideoZoom[ZOOM_MAX_LEVEL-1];
	m_nZoomNum = 0;
	for (int i = 0; i < ZOOM_MAX_LEVEL; i++)
	{
		if (fLevel <= g_VideoZoom[i])
		{
			m_nZoomNum = i;
			break;
		}
	}
	m_fZoomLevel = fLevel;

	int nW = 0;
	int nH = 0;
	int nXC = 0;
	int nYC = 0;
	if (bFix)
	{
		RECT rcView;
		GetClientRect (m_hWnd, &rcView);
		nW = m_rcZoom.right - m_rcZoom.left;
		nH = m_rcZoom.bottom - m_rcZoom.top;
		nXC = m_rcZoom.left + m_ptZoom.x * nW / rcView.right;
		nYC = m_rcZoom.top + m_ptZoom.y * nH / rcView.bottom;
	}
	else
	{
		nXC = (m_rcZoom.right + m_rcZoom.left) / 2;
		nYC = (m_rcZoom.bottom + m_rcZoom.top) / 2;
	}

	nW = (int)(m_rcVideo.right / m_fZoomLevel);
	nH = (int)(m_rcVideo.bottom / m_fZoomLevel);

	int nL = nXC - nW / 2;
	if (nL < 0) nL = 0;
	if (nL + nW > m_rcVideo.right) nL = m_rcVideo.right - nW;

	int nT = nYC - nH / 2;
	if (nT < 0) nT = 0;
	if (nT + nH > m_rcVideo.bottom) nT = m_rcVideo.bottom - nH;

	nL = nL & ~3;
	nT = nT & ~1;
	nW = nW & ~3;
	nH = nH & ~1;
	SetRect (&m_rcZoom, nL, nT, nL + nW, nT + nH);

	m_pMedia->SetParam (YYPLAY_PID_VideoZoomIn, &m_rcZoom);
	m_pWndBar->SetZoomRect (&m_rcZoom);

	UpdateLang ();
	ShowBar (true);
}

void CWndPlay::OnZoomMove (int nX, int nY)
{
	if (m_nZoomNum <= 0)
		return;
	CAutoLock lock (m_pWndBar->GetDrawLock ());
	if (m_nRotateAngle == 90)
	{
		int x = nY;
		nY = -nX;
		nX = x;
	}
	else if (m_nRotateAngle == 180)
	{
		nX = -nX;
		nY = -nY;
	}
	else if (m_nRotateAngle == 270)
	{
		int x = nY;
		nY = nX;
		nX = -x;
	}
	int nW = m_rcZoom.right - m_rcZoom.left;
	int nH = m_rcZoom.bottom - m_rcZoom.top;
	nW = nW & ~3;
	nH = nH & ~1;

	m_rcZoom.left -= nX;
	if (m_rcZoom.left < 0) m_rcZoom.left = 0;
	if (m_rcZoom.left + nW > m_rcVideo.right) m_rcZoom.left = m_rcVideo.right - nW;
	m_rcZoom.right = m_rcZoom.left + nW;

	m_rcZoom.top -= nY;
	if (m_rcZoom.top < 0) m_rcZoom.top = 0;
	if (m_rcZoom.top + nH > m_rcVideo.bottom) m_rcZoom.top = m_rcVideo.bottom - nH;
	m_rcZoom.bottom = m_rcZoom.top + nH;

	m_pMedia->SetParam (YYPLAY_PID_VideoZoomIn, &m_rcZoom);
	m_pWndBar->SetZoomRect (&m_rcZoom);

	ShowBar (true);
}

void CWndPlay::OnZoomSelect (WPARAM wParam, LPARAM lParam)
{
	CAutoLock lock (m_pWndBar->GetDrawLock ());
	if (m_ptZoomSelect.x < 0 || m_ptZoomSelect.y < 0)
		return;
	if (m_nTimerZoomSlt != 0)
	{
		KillTimer (m_hWnd, WT_PLAY_ZoomSelect);
		m_nTimerZoomSlt = 0;
	}
	m_bZoomSelect = false;
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	RECT rcRnd;
	int nRC = m_pMedia->GetParam (YYPLAY_PID_RenderArea, &rcRnd);
	if (nRC != YY_ERR_NONE)
	{
		m_ptZoomSelect.x = -1;
		m_ptZoomSelect.y = -1;
		return;
	}
	int nRW = rcRnd.right - rcRnd.left;
	int nRH = rcRnd.bottom - rcRnd.top;
	RECT rcZoom;
	memcpy (&rcZoom, &m_rcZoom, sizeof (RECT));
	int nZW = m_rcZoom.right - m_rcZoom.left;
	int nZH = m_rcZoom.bottom - m_rcZoom.top;
	int	nLeft = m_ptZoomSelect.x - rcRnd.left;
	int nTop = m_ptZoomSelect.y - rcRnd.top;
	int nRight = m_nLastMoveX - rcRnd.left;
	int nBottom = m_nLastMoveY - rcRnd.top;

	if (m_nRotateAngle == 90 || m_nRotateAngle == 270)
	{
		nZW = m_rcZoom.bottom - m_rcZoom.top;
		nZH = m_rcZoom.right - m_rcZoom.left;
		if (m_nRotateAngle == 90)
		{
			rcZoom.left = m_rcVideo.bottom - m_rcZoom.bottom;
			rcZoom.top = m_rcZoom.left;
		}
		else
		{
			rcZoom.left = m_rcZoom.top;
			rcZoom.top = m_rcVideo.right - m_rcZoom.right;
		}
	}
	else if (m_nRotateAngle == 180)
	{
		rcZoom.left = m_rcVideo.right  - m_rcZoom.right;
		rcZoom.top = m_rcVideo.bottom - m_rcZoom.bottom;
	}

	rcZoom.left		= rcZoom.left + nZW * nLeft / nRW;
	rcZoom.top		= rcZoom.top + nZH * nTop / nRH;
	rcZoom.right	= rcZoom.left + nZW * nRight / nRW;
	rcZoom.bottom	= rcZoom.top + nZH * nBottom / nRH;

	RECT rcRotate;
	memcpy (&rcRotate, &rcZoom, sizeof (RECT));
	if (m_nRotateAngle == 90)
	{
		rcZoom.left = rcRotate.top;
		rcZoom.top = m_rcVideo.bottom - rcRotate.right;
		rcZoom.right = rcRotate.bottom;
		rcZoom.bottom = m_rcVideo.bottom - rcRotate.left;
	}
	else if (m_nRotateAngle == 270)
	{
		rcZoom.left = m_rcVideo.right - rcRotate.bottom;
		rcZoom.top = rcRotate.left;
		rcZoom.right = m_rcVideo.right - rcRotate.top;
		rcZoom.bottom = rcRotate.right;
	}
	else if (m_nRotateAngle == 180)
	{
		rcZoom.left = m_rcVideo.right - rcRotate.right;
		rcZoom.top = m_rcVideo.bottom - rcRotate.bottom;
		rcZoom.right = m_rcVideo.right - rcRotate.left;
		rcZoom.bottom = m_rcVideo.bottom - rcRotate.top;
	}

	m_ptZoomSelect.x = -1;
	m_ptZoomSelect.y = -1;
	if (rcZoom.left < 0) rcZoom.left = 0;
	if (rcZoom.top < 0) rcZoom.top = 0;
	if (rcZoom.right > m_rcVideo.right) rcZoom.right = m_rcVideo.right;
	if (rcZoom.bottom > m_rcVideo.bottom) rcZoom.bottom = m_rcVideo.bottom;
	rcZoom.left = rcZoom.left & ~7;
	rcZoom.right = rcZoom.right & ~7;
	rcZoom.top = rcZoom.top & ~3;
	rcZoom.bottom = rcZoom.bottom & ~3;
	if (rcZoom.right - rcZoom.left < 64 || rcZoom.bottom - rcZoom.top < 32)
		return;
	memcpy (&m_rcZoom, &rcZoom, sizeof (RECT));
	m_fZoomLevel = (float)m_rcVideo.right / (m_rcZoom.right - m_rcZoom.left);
	m_nZoomNum = ZOOM_MAX_LEVEL - 1;
	for (int i = 0; i < ZOOM_MAX_LEVEL; i++)
	{
		if (m_fZoomLevel <= g_VideoZoom[i])
		{
			m_nZoomNum = i;
			break;
		}
	}
	m_pMedia->SetParam (YYPLAY_PID_VideoZoomIn, &m_rcZoom);
	m_pWndBar->SetZoomRect (&m_rcZoom);
}

void CWndPlay::UpdateBackgroud (void)
{
	RECT rcView;
	GetClientRect (m_hWnd, &rcView);
	HDC hDC = GetDC(m_hWnd);
	if (m_pMedia == NULL)
	{
		FillRect (hDC, &rcView, m_hBrhBlack);
	}
	else
	{
		RECT rcDraw;
		RECT rcRnd;
		int nRC = m_pMedia->GetParam (YYPLAY_PID_RenderArea, &rcRnd);
		if (nRC == YY_ERR_NONE)
		{
			if (rcRnd.left > rcView.left)
			{
				SetRect (&rcDraw, rcView.left, rcView.top, rcRnd.left, rcView.bottom);
				FillRect (hDC, &rcDraw, m_hBrhBlack);
			}
			if (rcRnd.right < rcView.right)
			{
				SetRect (&rcDraw, rcRnd.right, rcView.top, rcView.right, rcView.bottom);
				FillRect (hDC, &rcDraw, m_hBrhBlack);
			}
			if (rcRnd.top > rcView.top)
			{
				SetRect (&rcDraw, rcView.left, rcView.top, rcView.right, rcRnd.top);
				FillRect (hDC, &rcDraw, m_hBrhBlack);
			}
			if (rcRnd.bottom < rcView.bottom)
			{
				SetRect (&rcDraw, rcView.left, rcRnd.bottom, rcView.right, rcView.bottom);
				FillRect (hDC, &rcDraw, m_hBrhBlack);
			}
		}
	}
	ReleaseDC (m_hWnd, hDC);
}

void CWndPlay::CaptureVideo (void)
{
	YYPLAY_STATUS sts = m_pMedia->GetStatus ();
	if (sts != YY_PLAY_Run && sts != YY_PLAY_Pause)
		return;
	if (sts == YY_PLAY_Run)
		m_pMedia->Pause ();

	YY_VIDEO_FORMAT * pFmtVideo = NULL;
	m_pMedia->GetParam (YYPLAY_PID_Fmt_Video, &pFmtVideo);
	if (pFmtVideo == NULL)
		return;
	LPBYTE	pBuff = NULL;
	HBITMAP hBmp = yyBmpCreate (NULL, pFmtVideo->nWidth, pFmtVideo->nHeight, &pBuff, 0);
	if (hBmp == NULL)
		return;

	YY_VIDEO_BUFF buffVideo;
	memset (&buffVideo, 0, sizeof (YY_VIDEO_BUFF));
	buffVideo.nType = YY_VDT_RGBA;
	buffVideo.nWidth = pFmtVideo->nWidth;
	buffVideo.nHeight = pFmtVideo->nHeight;
	buffVideo.pBuff[0] = (unsigned char *)pBuff;
	buffVideo.nStride[0] = pFmtVideo->nWidth * 4;

	YY_BUFFER yyBuff;
	memset (&yyBuff, 0, sizeof (yyBuff));
	yyBuff.pBuff = (unsigned char *)&buffVideo;
	yyBuff.uSize = sizeof (buffVideo);
	yyBuff.uFlag = YYBUFF_TYPE_VIDEO;
	YY_BUFFER * pVideoBuff = &yyBuff;
	m_pMedia->GetParam (YYPLAY_PID_VideoData, &pVideoBuff);

	TCHAR szPlay[1024];
	_tcscpy (szPlay, m_szFile);
	TCHAR * pName = _tcsrchr (szPlay, _T('\\'));
	if (pName == NULL)
		pName = _tcsrchr (szPlay, _T('/'));
	pName++;
	TCHAR * pExt = _tcsrchr (szPlay, _T('.'));
	if (pExt != NULL)
		*pExt = 0;
	TCHAR szPath[1024];
	TCHAR szFile[1024];
	yyGetDataPath (m_hInst, szPath, sizeof (szPath));
	_tcscat (szPath, _T("Capture"));
	DWORD dwAttr = GetFileAttributes (szPath);
	if (dwAttr == -1)
		CreateDirectory (szPath, NULL);
	_tcscat (szPath, _T("\\"));
	int nIndex = 0;
	while (true)
	{
		_stprintf (szFile, _T("%s%s_%d.bmp"), szPath, pName, nIndex);
		dwAttr = GetFileAttributes (szFile);
		if (dwAttr == -1)
			break;
		nIndex++;
	}
	
	yyBmpSave (hBmp, pBuff, szFile);

	DeleteObject (hBmp);

	if (sts == YY_PLAY_Run)
		m_pMedia->Run ();
}
