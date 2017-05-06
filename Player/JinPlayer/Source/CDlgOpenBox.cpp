/*******************************************************************************
	File:		CDlgOpenBox.cpp

	Contains:	open box dialog implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "commctrl.h"
#include "shlobj.h"

#include "tchar.h"
#include "stdint.h"

#include "CDlgOpenBox.h"
#include "CRegMng.h"
#include "Resource.h"

#include "USystemFunc.h"

#define TIMER_SetFocus 501

CDlgOpenBox::CDlgOpenBox(HINSTANCE hInst, HWND hParent, CBaseKey * pKey)
	: m_hInst (hInst)
	, m_hParent (hParent)
	, m_pKey (pKey)
	, m_hDlg (NULL)
{
}

CDlgOpenBox::~CDlgOpenBox(void)
{
}

int CDlgOpenBox::OpenDlg (void)
{
	int nRC = DialogBoxParam (m_hInst, MAKEINTRESOURCE(IDD_DIALOG_OPENBOX), m_hParent, OpenBoxDlgProc, (LPARAM)this);
	return nRC;
}

LRESULT CDlgOpenBox::OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int	wmId, wmEvent;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		RECT rcDlg;
		SetWindowLong (hWnd, GWL_USERDATA, lParam);
		m_hDlg = hWnd;
		GetClientRect (m_hDlg, &rcDlg);

		SetWindowText (m_hDlg, yyLangGetText (YYTEXT_MyBox));
		SetWindowText (GetDlgItem (m_hDlg, IDC_STATIC_PWINFO), yyLangGetText (YYTEXT_PWLimit));
		SetWindowText (GetDlgItem (m_hDlg, IDOK), yyLangGetText (YYTEXT_OK));
		SetWindowText (GetDlgItem (m_hDlg, IDCANCEL), yyLangGetText (YYTEXT_Cancel));

		SetWindowPos (m_hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcDlg.right) / 2, 
						(GetSystemMetrics (SM_CYSCREEN) - rcDlg.bottom ) / 2, 0, 0, SWP_NOSIZE);

		EnableWindow (GetDlgItem (m_hDlg, IDC_BUTTON_STAR), FALSE);
		//EnableWindow (GetDlgItem (m_hDlg, IDC_BUTTON_CHA), FALSE);
		SetFocus (GetDlgItem (m_hDlg, IDC_EDIT_PW1));
		SetTimer (m_hDlg, TIMER_SetFocus, 100, NULL);
		return S_FALSE;
	}

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDC_BUTTON_0:
		case IDC_BUTTON_1:
		case IDC_BUTTON_2:
		case IDC_BUTTON_3:
		case IDC_BUTTON_4:
		case IDC_BUTTON_5:
		case IDC_BUTTON_6:
		case IDC_BUTTON_7:
		case IDC_BUTTON_8:
		case IDC_BUTTON_9:
			SendMessage (GetDlgItem (m_hDlg, IDC_EDIT_PW1), WM_CHAR, 0X30 + (wmId - IDC_BUTTON_0), 0);
			SetFocus (GetDlgItem (m_hDlg, IDC_EDIT_PW1));
			break;

		case IDC_BUTTON_DEL:
		{
			TCHAR szText[32];
			GetWindowText (GetDlgItem (m_hDlg, IDC_EDIT_PW1), szText, sizeof (szText));
			if (_tcslen (szText) <= 0)
				return S_OK;
			szText[_tcslen (szText) - 1] = 0;
			SetWindowText (GetDlgItem (m_hDlg, IDC_EDIT_PW1), szText);
			break;
		}

		case IDOK:
		{
			TCHAR szPW1[256];
			GetDlgItemText (m_hDlg, IDC_EDIT_PW1, szPW1, sizeof (szPW1) / sizeof (TCHAR));
			if (_tcslen (szPW1) != 6)
			{
				MessageBox (m_hDlg, yyLangGetText (YYTEXT_PW_6), yyLangGetText (YYTEXT_Error), MB_OK);
				return S_OK;
			}
			m_pKey->CreateKey (szPW1);

			TCHAR	szBoxPath[1024];
			_tcscpy (szBoxPath, CRegMng::g_pRegMng->GetTextValue (_T("BoxRootPath")));
			_tcscat (szBoxPath, _T("\\"));
			_tcscat (szBoxPath, CBaseKey::g_Key->GetWKey ());
			DWORD dwAttr = GetFileAttributes (szBoxPath);
			if (dwAttr == -1)
			{
				MessageBox (m_hDlg, yyLangGetText (YYTEXT_FindContent), yyLangGetText (YYTEXT_Error), MB_OK);
				return S_OK;
			}
			return EndDialog(m_hDlg, LOWORD(wParam));
		}
		case IDCANCEL:
			EndDialog(m_hDlg, LOWORD(wParam));
			break;

		default:
			break;
		}
		break;
	case WM_TIMER:
		KillTimer (m_hDlg, TIMER_SetFocus);
		SetFocus (GetDlgItem (m_hDlg, IDC_EDIT_PW1));
		return S_OK;
	default:
		break;
	}

	return S_OK;
}

INT_PTR CALLBACK CDlgOpenBox::OpenBoxDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CDlgOpenBox * pDlgBox = NULL;
	UNREFERENCED_PARAMETER(lParam);
	if (message == WM_INITDIALOG)
		pDlgBox = (CDlgOpenBox *)lParam;
	else if (hDlg != NULL)
		pDlgBox = (CDlgOpenBox *)GetWindowLong (hDlg, GWL_USERDATA);

	if (pDlgBox != NULL)
		return (INT_PTR) pDlgBox->OnMessage (hDlg, message, wParam, lParam);

	return (INT_PTR) FALSE;
}

