/*******************************************************************************
	File:		CWndPanel.cpp

	Contains:	The control panel implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-13		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"
#include <commctrl.h>
#include <Commdlg.h>

#include "CWndPanel.h"
#include "resource.h"

#define WM_TIMER_UpdatePos	101
#define BTN_WIDTH			56
#define	BTN_HEIGHT			32

//#define YYBG_TRANSPARENT	

CWndPanel::CWndPanel(HINSTANCE hInst)
	: CWndBase (hInst)
	, m_pMedia (NULL)
	, m_nAudioVolume (100)
	, m_bMute (false)
	, m_pWndBG (NULL)
	, m_pSldPos (NULL)
	, m_dwTimerPos (0)
	, m_btnMute (NULL)
	, m_sldVolume (NULL)
	, m_btnPrev (NULL)
	, m_btnPlay (NULL)
	, m_btnPause (NULL)
	, m_btnNext (NULL)
	, m_btnList (NULL)
	, m_btnOpen (NULL)
	, m_btnFullSrn (NULL)
{
}

CWndPanel::~CWndPanel(void)
{
}

bool CWndPanel::CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG, bool bPopup)
{
	if (!CWndBase::CreateWnd (hParent, rcView, clrBG, bPopup))
		return false;

	int		nRC = 0;
	RECT	rcPanel;
	int		nLeft = 8;
	int		nTop = 8;

#ifdef YYBG_TRANSPARENT
	m_pWndBG = new CWndBase (m_hInst);
	m_pWndBG->CreateWnd (m_hWnd, rcView, clrBG, true);
	GetWindowRect (m_hParent, &rcPanel);
	SetWindowPos (m_pWndBG->GetWnd (), NULL, rcPanel.left, 500, 0, 0, SWP_NOSIZE);
	DWORD dwExStyle = GetWindowLong (m_pWndBG->GetWnd (), GWL_EXSTYLE);
	dwExStyle |= WS_EX_LAYERED;
	nRC = SetWindowLong (m_pWndBG->GetWnd (), GWL_EXSTYLE, dwExStyle);
	nRC = SetLayeredWindowAttributes  (m_pWndBG->GetWnd (), clrBG, 150, LWA_ALPHA);
#endif // YYBG_TRANSPARENT

	m_btnMute = CreateButton (_T("Mute"), nLeft, nTop, IDC_BTN_MUTE);
	nLeft += BTN_WIDTH;
	RECT	rcVolume;
	SetRect (&rcVolume, nLeft + 4, nTop, nLeft + 100, nTop + BTN_HEIGHT);
	m_sldVolume = CreateSlider (rcVolume, IDC_SLD_VOLUME, true);

	GetClientRect (m_hWnd, &rcPanel);
	nLeft = rcPanel.right / 2 - BTN_WIDTH - 4 - BTN_WIDTH / 2 - (BTN_WIDTH + 4);
	m_btnPrev = CreateButton (_T("|<"), nLeft, nTop, IDC_BTN_PREV);
	nLeft = rcPanel.right / 2 - BTN_WIDTH - 4 - BTN_WIDTH / 2;
	m_btnSeekPrev = CreateButton (_T("<<"), nLeft, nTop, IDC_BTN_SEEKPREV);

	nLeft = nLeft + BTN_WIDTH + 4;
	m_btnPlay = CreateButton (_T(">"), nLeft, nTop, IDC_BTN_PLAY);
	m_btnPause = CreateButton (_T("||"), nLeft, nTop, IDC_BTN_PAUSE);
	ShowWindow (m_btnPause, SW_HIDE);

	nLeft = nLeft + BTN_WIDTH + 4;
	m_btnSeekNext = CreateButton (_T(">>"), nLeft, nTop, IDC_BTN_SEEKNEXT);
	nLeft = nLeft + BTN_WIDTH + 4;
	m_btnNext = CreateButton (_T(">|"), nLeft, nTop, IDC_BTN_NEXT);

	nLeft = rcPanel.right - (BTN_WIDTH + 4) * 3;
	m_btnOpen = CreateButton (_T("Open"), nLeft, nTop, IDC_BTN_OPEN);
	nLeft = nLeft + BTN_WIDTH + 4;
	m_btnList = CreateButton (_T("List"), nLeft, nTop, IDC_BTN_LIST);
	nLeft = nLeft + BTN_WIDTH + 4;
	m_btnFullSrn = CreateButton (_T("Full"), nLeft, nTop, IDC_BTN_FULLSRN);

	rcPanel.top = rcPanel.bottom * 3 / 5;
	m_pSldPos = new CWndSlider (m_hInst);
	m_pSldPos->CreateWnd (m_hWnd, rcPanel, RGB (30, 30, 30));

	// Add the transparent effect.
	DWORD dwStyle = GetWindowLong (m_hWnd, GWL_EXSTYLE);
	dwStyle |= WS_EX_LAYERED;
	nRC = SetWindowLong (m_hWnd, GWL_EXSTYLE, dwStyle);
//	nRC = SetLayeredWindowAttributes  (m_hWnd, clrBG, 150, LWA_COLORKEY);
	nRC = SetLayeredWindowAttributes  (m_hWnd, clrBG, 150, LWA_COLORKEY | LWA_ALPHA);

	SendMessage (m_sldVolume, TBM_SETRANGE, TRUE, MAKELONG (0, 100));

	return true;
}

HWND CWndPanel::CreateButton (TCHAR * pText, int nLeft, int nTop, int nID)
{
	return CreateWindow (_T("button"), pText, WS_VISIBLE | WS_CHILD | BS_PUSHLIKE, 
						 nLeft, nTop, BTN_WIDTH, BTN_HEIGHT, m_hWnd, (HMENU)nID, m_hInst, NULL);
}

HWND CWndPanel::CreateSlider (RECT rcPos, int nID, bool bHorizon)
{
	DWORD dwStyle = WS_VISIBLE | WS_CHILD | TBS_BOTH | TBS_NOTICKS;
	if (!bHorizon)
		dwStyle |= TBS_VERT;

	return CreateWindow (_T("msctls_trackbar32"), _T(""), dwStyle, 
							rcPos.left, rcPos.top, rcPos.right - rcPos.left, rcPos.bottom - rcPos.top,
							m_hWnd, (HMENU)nID, m_hInst, NULL);
}

LRESULT CWndPanel::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int		wmId, wmEvent;

	switch (uMsg)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDC_BTN_PREV:
		case IDC_BTN_NEXT:
		case IDC_BTN_OPEN:
		case IDC_BTN_LIST:
		case IDC_BTN_FULLSRN:
			SendMessage (m_hParent, WM_COMMAND, wParam, lParam);
			break;

		case IDC_BTN_MUTE:
			if (m_bMute)
			{
				m_pMedia->SetVolume (m_nAudioVolume);
				SendMessage (m_sldVolume, TBM_SETPOS, TRUE, m_nAudioVolume);
			}
			else
			{
				m_pMedia->SetVolume (0);
				SendMessage (m_sldVolume, TBM_SETPOS, TRUE, 0);
			}
			m_bMute = !m_bMute;
			break;

		case IDC_BTN_PLAY:
			if (m_pMedia != NULL)
				m_pMedia->Run ();
			break;

		case IDC_BTN_PAUSE:
			if (m_pMedia != NULL)
				m_pMedia->Pause ();
			break;

		case IDC_BTN_SEEKPREV:
			if (m_pMedia != NULL)
			{
				int nPos = m_pMedia->GetPos () - 10000;
				if (nPos < 0)
					nPos = 0;
				m_pMedia->SetPos (nPos);
			}
			SendMessage (m_hParent, WM_YYPANEL_COMMAND, 0, 0);
			break;

		case IDC_BTN_SEEKNEXT:
			if (m_pMedia != NULL)
			{
				int nPos = m_pMedia->GetPos () + 10000;
				m_pMedia->SetPos (nPos);
			}
			SendMessage (m_hParent, WM_YYPANEL_COMMAND, 0, 0);
			break;

		default:
			break;
		}
		if (wmId != IDC_BTN_LIST)
			SendMessage (m_hParent, WM_YYPANEL_COMMAND, 0, 0);
		break;

	case WM_YYSLD_NEWPOS:
		if (m_pMedia != NULL)
			m_pMedia->SetPos ((int)wParam);
		SendMessage (m_hParent, WM_YYPANEL_COMMAND, 0, 0);
		break;

	case WM_KEYDOWN:
		SendMessage (m_hParent, uMsg, wParam, lParam);
		break;
	
	case WM_TIMER:
		m_pSldPos->SetPos (m_pMedia->GetPos ());
		break;

	case WM_HSCROLL:
		if (m_pMedia != NULL)
		{
			int nPos = 0;
			if (lParam = (LPARAM)m_sldVolume)
			{
				nPos = SendMessage (m_sldVolume, TBM_GETPOS, 0, 0);
				m_pMedia->SetVolume (nPos);
			}
		}
		break;

	case WM_CLOSE:
		if (m_dwTimerPos != 0)
			KillTimer (m_hWnd, WM_TIMER_UpdatePos);

		if (m_pSldPos != NULL)
		{
			SendMessage (m_pSldPos->GetWnd (), WM_CLOSE, 0, 0);
			delete m_pSldPos;
			m_pSldPos = NULL;
		}
		break;

	default:
		break;
	}

	return	CWndBase::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

void CWndPanel::HandleEvent (int nID, void * pV1)
{
	int nRC = 0;
	switch (nID)
	{
	case YY_EV_Open_Complete:
	{
		int nVolume = m_pMedia->GetVolume ();
		if (nVolume >= 0)
			m_nAudioVolume = nVolume;
		SendMessage (m_sldVolume, TBM_SETPOS, TRUE, m_nAudioVolume);

		int nDuration = m_pMedia->GetDur ();
		if (nDuration > 0)
		{
			if (m_pSldPos != NULL)
				m_pSldPos->SetRange (0, nDuration);
		}
		if (m_dwTimerPos == 0)
			m_dwTimerPos = SetTimer (m_hWnd, WM_TIMER_UpdatePos, 500, NULL);
		break;
	}

	case YY_EV_Play_Status:
	{
		YYPLAY_STATUS nStatus = (YYPLAY_STATUS)(int)pV1;
		if (nStatus == YY_PLAY_Run)
		{
			ShowWindow (m_btnPause, SW_SHOW);
			ShowWindow (m_btnPlay, SW_HIDE);
		}
		else
		{
			ShowWindow (m_btnPlay, SW_SHOW);
			ShowWindow (m_btnPause, SW_HIDE);
		}
		break;
	}

	default:
		break;
	}
}

