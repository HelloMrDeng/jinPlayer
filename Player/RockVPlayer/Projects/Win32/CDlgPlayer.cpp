/*******************************************************************************
	File:		CVideoRender.cpp

	Contains:	Demo UI implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-14		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"
#include "commctrl.h"
#include <Commdlg.h>
#include <winuser.h>
#include <shellapi.h>

#include "CDlgPlayer.h"

#include "yyLog.h"

#include "resource.h"

#pragma warning (disable : 4996)
#define		WM_TIMER_POS	101

CDlgPlayer::CDlgPlayer(void)
	: m_hInst (NULL)
	, m_hDlg (NULL)
	, m_pSldPos (NULL)
	, m_pLsnInfo (NULL)
	, m_pPlayer (NULL)
{
	memset (m_szFile, 0, sizeof (m_szFile));
}

CDlgPlayer::~CDlgPlayer(void)
{
	YY_DEL_P (m_pLsnInfo);
	YY_DEL_P (m_pPlayer);
}

// Message handler for about box.
INT_PTR CALLBACK DlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	if (message == WM_INITDIALOG)
		SetWindowLong (hDlg, GWL_USERDATA,lParam);
	if (hDlg == NULL)
		return (INT_PTR)FALSE;
	CDlgPlayer * pDlg = (CDlgPlayer *)GetWindowLong (hDlg, GWL_USERDATA);
	return pDlg->MsgProc (hDlg, message, wParam, lParam);
}

bool CDlgPlayer::Create (HINSTANCE hInst)
{
	m_hInst = hInst;
	DialogBoxParam (hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, DlgProc, (LPARAM)this);
	return true;
}

INT_PTR CDlgPlayer::MsgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT	rcWnd;
	switch (message)
	{
	case WM_INITDIALOG:
		m_hDlg = hDlg;
		GetClientRect (hDlg, &rcWnd);
		SetWindowPos (hDlg, HWND_TOPMOST, (GetSystemMetrics (SM_CXSCREEN) - rcWnd.right) / 2, 
					(GetSystemMetrics (SM_CYSCREEN) - rcWnd.bottom ) / 2 - 100, 0, 0, SWP_NOSIZE);
		for (int i = 0; i < 4; i++)
			SendMessage (GetDlgItem (m_hDlg, IDC_SLIDER_1 + i), TBM_SETRANGE, TRUE, MAKELONG (0, 100));
		{
			POINT ptRect;
			m_pSldPos = new CWndSlider (m_hInst);
			GetWindowRect (GetDlgItem (m_hDlg, IDC_STATIC_POS), &rcWnd);
			ptRect.x = rcWnd.left;
			ptRect.y = rcWnd.top;
			ScreenToClient (m_hDlg, &ptRect);
			SetRect (&rcWnd, ptRect.x, ptRect.y, ptRect.x + (rcWnd.right - rcWnd.left), ptRect.y + (rcWnd.bottom - rcWnd.top));
			m_pSldPos->CreateWnd (m_hDlg, rcWnd, RGB (100, 100, 100));
		}
		SetTimer (m_hDlg, WM_TIMER_POS, 500, NULL);
		UpdateLessonInfo ();
		_tcscpy (m_szFile, _T("O:\\Data\\Works\\Customers\\RockV\\Demo\\Class\\Òô½×Á·Ï°30\\Òô½×Á·Ï°-30.rkv"));
		OpenLessonFile (false);
		return (INT_PTR)TRUE;

	case WM_DESTROY:
		SendMessage (m_pSldPos->GetWnd (), WM_CLOSE, 0, 0);
		delete m_pSldPos;
		break;

	case WM_TIMER:
		if (m_pSldPos != NULL && m_pPlayer != NULL)
		{
			TCHAR szWinText[1024];
			GetTimeText (m_pPlayer->GetPos (), szWinText);
			SetWindowText (GetDlgItem (m_hDlg, IDC_STATIC_PLAYTIME), szWinText);
			m_pSldPos->SetPos (m_pPlayer->GetPos ());
		}
		break;

	case WM_YYSLD_NEWPOS:
		if (m_pPlayer != NULL)
			m_pPlayer->SetPos (m_pSldPos->GetPos ());
		return (INT_PTR)TRUE;

	case WM_HSCROLL:
	{	
		int nSld = -1;
		if (lParam == (LPARAM)GetDlgItem (m_hDlg, IDC_SLIDER_1))
			nSld = 0;
		else if (lParam == (LPARAM)GetDlgItem (m_hDlg, IDC_SLIDER_2))
			nSld = 1;
		else if (lParam == (LPARAM)GetDlgItem (m_hDlg, IDC_SLIDER_3))
			nSld = 2;
		else if (lParam == (LPARAM)GetDlgItem (m_hDlg, IDC_SLIDER_4))
			nSld = 3;
		if (nSld >= 0 && m_pLsnInfo != NULL)
		{
			int nVolume = SendMessage (GetDlgItem (m_hDlg, IDC_SLIDER_1 + nSld), TBM_GETPOS, 0, 0);
			m_pLsnInfo->GetItem ()->m_ppTrack[nSld]->m_nVolume = nVolume;
		}
		break;
	}	

	case WM_COMMAND:
	{
		int nCmd = LOWORD(wParam);
		switch (nCmd)
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;

		case IDC_BUTTON_OPEN:
			OpenLessonFile (true);
			return (INT_PTR)TRUE;

		case IDC_COMBO_LESSON:
			if (HIWORD (wParam) == CBN_SELCHANGE)
			{
				int nSel = SendMessage (GetDlgItem (m_hDlg, IDC_COMBO_LESSON), CB_GETCURSEL, 0, 0);
				m_pLsnInfo->SetItemSel (nSel);
				UpdateLsnItemInfo ();
				if (m_pPlayer != NULL)
				{
					m_pPlayer->Open (m_pLsnInfo);
					m_pSldPos->SetRange (0, m_pPlayer->GetDur ());
					TCHAR szWinText[1024];
					GetTimeText (m_pPlayer->GetDur (), szWinText);
					SetWindowText (GetDlgItem (m_hDlg, IDC_STATIC_DURATION), szWinText);
				}
			}
			break;
		
		case IDC_RADIO_0:
		case IDC_RADIO_A:
		case IDC_RADIO_B:
		case IDC_RADIO_C:
		case IDC_RADIO_D:
		{
			int nRptSel = nCmd - IDC_RADIO_0 - 1;
			if (m_pLsnInfo != NULL)
			{
				m_pLsnInfo->SetRepeatSel (nRptSel);
//				CLsnRepeat * pRpt = m_pLsnInfo->GetRepeat ();
//				if (m_pPlayer != NULL)
//					m_pPlayer->SetPos (pRpt->m_nStart * 1000);
			}
			break;
		}

		case IDC_CHECK_1:
		case IDC_CHECK_2:
		case IDC_CHECK_3:
		case IDC_CHECK_4:
		{
			int nItem = nCmd - IDC_CHECK_1;
			if (m_pLsnInfo != NULL)
			{
				CLsnTrack * pTrack = m_pLsnInfo->GetItem ()->m_ppTrack[nItem];
				pTrack->m_bEnable = SendMessage (GetDlgItem (m_hDlg, nCmd), BM_GETCHECK, 0, 0) == BST_CHECKED;
			}
			break;
		}

		case IDC_BUTTON_PLAY:
			if (m_pPlayer != NULL)
				m_pPlayer->Run ();
			break;

		case IDC_BUTTON_PAUSE:
			if (m_pPlayer != NULL)
				m_pPlayer->Pause ();
			break;

		case IDC_BUTTON_STOP:
			if (m_pPlayer != NULL)
			{
				m_pPlayer->Pause ();
				m_pPlayer->SetPos (0);
			}
			break;

		default:
			break;
		}
	}
		break;
	}
	return (INT_PTR)FALSE;
}

int CDlgPlayer::OpenLessonFile (bool bDlgOpen)
{
	if (bDlgOpen)
	{
		OPENFILENAME		ofn;
		memset( &(ofn), 0, sizeof(ofn));
		ofn.lStructSize	= sizeof(ofn);
		ofn.hwndOwner = m_hDlg;
		ofn.lpstrFilter = TEXT("RockV File (*.rkv)\0*.rkv\0");	
		if (_tcsstr (m_szFile, _T(":/")) != NULL)
			_tcscpy (m_szFile, _T("*.rkv"));
		ofn.lpstrFile = m_szFile;
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = TEXT("Open RockV File");
		ofn.Flags = OFN_EXPLORER;
					
		if (!GetOpenFileName(&ofn))
			return -1;
	}
	if (m_pLsnInfo == NULL)
		m_pLsnInfo = new CLessonInfo ();
	if (!m_pLsnInfo->Open (m_szFile))
		return -1;
	if (m_pPlayer == NULL)
		m_pPlayer = new CMultiPlayer ();
	m_pPlayer->Open (m_pLsnInfo);

	TCHAR * pPos = _tcsrchr (m_szFile, _T('\\'));
	if (pPos != NULL)
		SetWindowText (GetDlgItem (m_hDlg, IDC_EDIT_FILE), pPos + 1);

	UpdateLessonInfo ();

	return 0;
}

int CDlgPlayer::UpdateLessonInfo (void)
{
	int	i = 0;
	for (i = 0; i < 5; i++)
		EnableWindow (GetDlgItem (m_hDlg, IDC_RADIO_0 + i), FALSE);
	if (m_pLsnInfo == NULL)
	{
		UpdateLsnItemInfo ();
		return YY_ERR_NONE;
	}

	HWND		hCmbItem = GetDlgItem (m_hDlg, IDC_COMBO_LESSON);
	CLsnItem *	pLsnItem = NULL;
	for (i = 0; i < m_pLsnInfo->GetItemNum (); i++)
	{
		pLsnItem = m_pLsnInfo->GetItemInfo (i);
		SendMessage (hCmbItem, CB_ADDSTRING, 0, (LPARAM)pLsnItem->m_szName);
	}
	SendMessage (hCmbItem, CB_SETCURSEL, (WPARAM)m_pLsnInfo->GetItemSel (), 0);

	int nRptNum = m_pLsnInfo->GetRepeatNum ();
	if (nRptNum > 0)
	{
		for (i = 0; i <= nRptNum; i++)
			EnableWindow (GetDlgItem (m_hDlg, IDC_RADIO_0 + i), TRUE);
		SendMessage (GetDlgItem (m_hDlg, IDC_RADIO_0 + m_pLsnInfo->GetRepeatSel() + 1), BM_SETCHECK, BST_CHECKED, 0);
	}

	UpdateLsnItemInfo ();

	m_pSldPos->SetRange (0, m_pPlayer->GetDur ());
	TCHAR szWinText[1024];
	GetTimeText (m_pPlayer->GetDur (), szWinText);
	SetWindowText (GetDlgItem (m_hDlg, IDC_STATIC_DURATION), szWinText);

	return YY_ERR_NONE;
}

int CDlgPlayer::UpdateLsnItemInfo (void)
{
	int	i = 0;
	for (i = 0; i < 4; i++)
	{
		SetWindowText (GetDlgItem (m_hDlg, IDC_STATIC_1 + i), _T(""));
		EnableWindow (GetDlgItem (m_hDlg, IDC_SLIDER_1 + i), FALSE);
		EnableWindow (GetDlgItem (m_hDlg, IDC_CHECK_1 + i), FALSE);
		SendMessage (GetDlgItem (m_hDlg, IDC_CHECK_1 + i), BM_SETCHECK, BST_UNCHECKED, 0);
	}
	if (m_pLsnInfo == NULL)
		return YY_ERR_NONE;

	CLsnItem * pLsnItem = m_pLsnInfo->GetItem ();
	if (pLsnItem == NULL)
		return YY_ERR_NONE;
	CLsnTrack * pTrack = NULL;
	for (i = 0; i < pLsnItem->m_nTrackNum; i++)
	{
		pTrack = pLsnItem->m_ppTrack[i];
		if (pTrack == NULL)
			continue;
		SetWindowText (GetDlgItem (m_hDlg, IDC_STATIC_1 + i), pTrack->m_szName);
		SendMessage (GetDlgItem (m_hDlg, IDC_SLIDER_1 + i), TBM_SETPOS, TRUE, pTrack->m_nVolume);
		if (pTrack->m_bEnable)
		{
			EnableWindow (GetDlgItem (m_hDlg, IDC_SLIDER_1 + i), TRUE);
			EnableWindow (GetDlgItem (m_hDlg, IDC_CHECK_1 + i), TRUE);
			SendMessage (GetDlgItem (m_hDlg, IDC_CHECK_1 + i), BM_SETCHECK, BST_CHECKED, 0);
		}
	}

	return YY_ERR_NONE;
}

void CDlgPlayer::GetTimeText (long long llTime, TCHAR * szText)
{
	int nTextTime = (int)(llTime / 1000);
	int nHour = nTextTime / 3600;
	int nMins = (nTextTime % 3600) / 60;
	int nSecs = nTextTime % 60;

	_stprintf (szText, _T("%d:%02d:%02d"), nHour, nMins, nSecs);
}
