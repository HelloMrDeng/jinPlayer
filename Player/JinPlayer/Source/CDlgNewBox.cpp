/*******************************************************************************
	File:		CDlgNewBox.cpp

	Contains:	New box dialog implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "commctrl.h"
#include "shlobj.h"

#include "tchar.h"
#include "stdint.h"

#include "CDlgNewBox.h"
#include "CRegMng.h"
#include "Resource.h"

#include "USystemFunc.h"

#define TIMER_SetFocus 501

CDlgNewBox::CDlgNewBox(HINSTANCE hInst, HWND hParent, CBaseKey * pKey)
	: m_hInst (hInst)
	, m_hParent (hParent)
	, m_pKey (pKey)
	, m_hDlg (NULL)
	, m_bNewBox (true)
{
	_tcscpy (m_szFolder, _T(""));
	_tcscpy (m_szPW, _T(""));
}

CDlgNewBox::~CDlgNewBox(void)
{
}

int CDlgNewBox::OpenDlg (TCHAR * pFolder, bool bNewBox)
{
	if (pFolder != NULL)
		_tcscpy (m_szFolder, pFolder);
	else
		_tcscpy (m_szFolder, CRegMng::g_pRegMng->GetTextValue (_T("BoxRootPath")));

	m_bNewBox = bNewBox;

	int nRC = DialogBoxParam (m_hInst, MAKEINTRESOURCE(IDD_DIALOG_NEWBOX), m_hParent, OpenNewBoxDlgProc, (LPARAM)this);
	return nRC;
}

LRESULT CDlgNewBox::OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		if (m_bNewBox)
			SetWindowText (GetDlgItem (m_hDlg, IDOK), yyLangGetText (YYTEXT_Create));
		else
			SetWindowText (GetDlgItem (m_hDlg, IDOK), yyLangGetText (YYTEXT_OK));
		SetWindowText (GetDlgItem (m_hDlg, IDCANCEL), yyLangGetText (YYTEXT_Cancel));
		SetWindowText (GetDlgItem (m_hDlg, IDC_STATIC_PW), yyLangGetText (YYTEXT_Password));
		SetWindowText (GetDlgItem (m_hDlg, IDC_STATIC_PW2), yyLangGetText (YYTEXT_Confirm));
		SetWindowText (GetDlgItem (m_hDlg, IDC_BUTTON_FOLDER), yyLangGetText (YYTEXT_Folder));

		SetWindowPos (m_hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcDlg.right) / 2, 
						(GetSystemMetrics (SM_CYSCREEN) - rcDlg.bottom ) / 2, 0, 0, SWP_NOSIZE);

		if (_tcslen (m_szFolder) > 0)
		{
			if (m_pKey->IsUsed ())
			{
				ShowWindow (GetDlgItem (m_hDlg, IDC_BUTTON_FOLDER), SW_HIDE);
				ShowWindow (GetDlgItem (m_hDlg, IDC_EDIT_PATH), SW_HIDE);
			}
			else
			{
				EnableWindow (GetDlgItem (m_hDlg, IDC_BUTTON_FOLDER), FALSE);
				SetDlgItemText (m_hDlg, IDC_EDIT_PATH, m_szFolder);
			}
		}
		else
		{
			yyGetDataPath (m_hInst, m_szFolder, sizeof (m_szFolder));
			_tcscat (m_szFolder, _T("MyBox"));
			SetDlgItemText (m_hDlg, IDC_EDIT_PATH, m_szFolder);
		}
		if (!m_bNewBox)
		{
			ShowWindow (GetDlgItem (m_hDlg, IDC_STATIC_PW2), SW_HIDE);
			ShowWindow (GetDlgItem (m_hDlg, IDC_EDIT_PW2), SW_HIDE);

			GetClientRect (m_hDlg, &rcDlg);
			rcDlg.bottom = rcDlg.bottom - 55;
			SetWindowPos (m_hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcDlg.right) / 2, 
							(GetSystemMetrics (SM_CYSCREEN) - rcDlg.bottom ) / 2, rcDlg.right, rcDlg.bottom, 0);	
			SetWindowPos (GetDlgItem (m_hDlg, IDOK), NULL, 80, rcDlg.bottom - 64, 0, 0, SWP_NOSIZE);
			SetWindowPos (GetDlgItem (m_hDlg, IDCANCEL), NULL, 240, rcDlg.bottom - 64, 0, 0, SWP_NOSIZE);
		}

		SetFocus (GetDlgItem (m_hDlg, IDC_EDIT_PW1));
		SetTimer (m_hDlg, TIMER_SetFocus, 100, NULL);
		return S_FALSE;
	}

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDC_BUTTON_FOLDER:
		{
			TCHAR szPath[256];
			BROWSEINFO info;
			memset (&info, 0, sizeof (info));
			info.hwndOwner = m_hDlg;
			info.ulFlags = BIF_EDITBOX;
			info.pszDisplayName = szPath;

			LPITEMIDLIST pIDList = SHBrowseForFolder (&info);
			if(pIDList)
			{
				SHGetPathFromIDList(pIDList, szPath);
				_tcscpy (m_szFolder, szPath);
				_tcscat (m_szFolder, _T("\\"));
				_tcscat (m_szFolder, _T("yyMyBox"));
				SetDlgItemText (m_hDlg, IDC_EDIT_PATH, m_szFolder);
			}
			return S_OK;
		}

		case IDOK:
		{
			TCHAR szPW1[256];
			TCHAR szPW2[256];
			GetDlgItemText (m_hDlg, IDC_EDIT_PW1, szPW1, sizeof (szPW1) / sizeof (TCHAR));
			if (!m_bNewBox)
			{
				if (_tcslen (szPW1) != 6)
				{
					MessageBox (m_hDlg, yyLangGetText (YYTEXT_PW_6), yyLangGetText (YYTEXT_Error), MB_OK);
					return S_OK;
				}
				_tcscpy (m_szPW, szPW1);
				m_pKey->CreateKey (szPW1);

				TCHAR	szBoxPath[1024];
				_tcscpy (szBoxPath, m_szFolder);
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

			GetDlgItemText (m_hDlg, IDC_EDIT_PW2, szPW2, sizeof (szPW2) / sizeof (TCHAR));
			if (_tcslen (szPW1) != 6 || _tcslen (szPW1) != 6)
			{
				MessageBox (m_hDlg, yyLangGetText (YYTEXT_PW_6), yyLangGetText (YYTEXT_Error), MB_OK);
				return S_OK;
			}
		
			if (_tcscmp (szPW1, szPW2))
			{
				MessageBox (m_hDlg, yyLangGetText (YYTEXT_PW_Same), yyLangGetText (YYTEXT_Error), MB_OK);
				return S_OK;
			}

			for (int i = 0; i < 6; i++)
			{
				if ((szPW1[i] > _T('z') || szPW1[i] < _T('a')) &&
					(szPW1[i] > _T('Z') || szPW1[i] < _T('A')) &&
					(szPW1[i] > _T('9') || szPW1[i] < _T('0')))
				{
					MessageBox (m_hDlg, yyLangGetText (YYTEXT_CharNum), yyLangGetText (YYTEXT_Error), MB_OK);
					return S_OK;
				}
			}

			BOOL bRC = TRUE;
			DWORD dwAttr = GetFileAttributes (m_szFolder);
			if (dwAttr == -1)
			{
				bRC = CreateDirectory (m_szFolder, NULL);
				dwAttr = GetFileAttributes (m_szFolder);
				dwAttr |= FILE_ATTRIBUTE_HIDDEN;
				bRC = SetFileAttributes (m_szFolder, dwAttr);
			}
			CRegMng::g_pRegMng->SetTextValue (_T("BoxRootPath"), m_szFolder);

			TCHAR		szFolder[1024];
			_tcscpy (szFolder, m_szFolder);
			_tcscat (szFolder, _T("\\"));

			m_pKey->CreateKey (szPW1);
			_tcscat (szFolder, m_pKey->GetWKey ());
			bRC = CreateDirectory (szFolder, NULL);
			if (!bRC)
			{
				MessageBox (m_hDlg, yyLangGetText (YYTEXT_PW_Exist), yyLangGetText (YYTEXT_Error), MB_OK);
				break;
			}
			dwAttr = GetFileAttributes (szFolder);
			dwAttr |= FILE_ATTRIBUTE_HIDDEN;
			bRC = SetFileAttributes (szFolder, dwAttr);
			_tcscpy (m_szPW, szPW1);

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

INT_PTR CALLBACK CDlgNewBox::OpenNewBoxDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CDlgNewBox * pDlgBox = NULL;
	UNREFERENCED_PARAMETER(lParam);
	if (message == WM_INITDIALOG)
		pDlgBox = (CDlgNewBox *)lParam;
	else if (hDlg != NULL)
		pDlgBox = (CDlgNewBox *)GetWindowLong (hDlg, GWL_USERDATA);

	if (pDlgBox != NULL)
		return (INT_PTR) pDlgBox->OnMessage (hDlg, message, wParam, lParam);

	return (INT_PTR) FALSE;
}

