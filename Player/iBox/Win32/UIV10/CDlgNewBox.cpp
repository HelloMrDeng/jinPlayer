/*******************************************************************************
	File:		CVideoRender.cpp

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

#include "Resource.h"

#include "USystemFunc.h"

CDlgNewBox::CDlgNewBox(HINSTANCE hInst, HWND hParent)
	: m_hInst (hInst)
	, m_hParent (hParent)
	, m_hDlg (NULL)
	, m_bNewBox (true)
	, m_pLangText (NULL)
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

		SetWindowText (m_hDlg, m_pLangText->GetText (YYTEXT_MyBox));
		SetWindowText (GetDlgItem (m_hDlg, IDOK), m_pLangText->GetText (YYTEXT_OK));
		SetWindowText (GetDlgItem (m_hDlg, IDCANCEL), m_pLangText->GetText (YYTEXT_Cancel));
		SetWindowText (GetDlgItem (m_hDlg, IDC_STATIC_PW), m_pLangText->GetText (YYTEXT_Password));
		SetWindowText (GetDlgItem (m_hDlg, IDC_STATIC_PW2), m_pLangText->GetText (YYTEXT_Confirm));
		SetWindowText (GetDlgItem (m_hDlg, IDC_BUTTON_FOLDER), m_pLangText->GetText (YYTEXT_Folder));

		SetWindowPos (m_hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcDlg.right) / 2, 
						(GetSystemMetrics (SM_CYSCREEN) - rcDlg.bottom ) / 2, 0, 0, SWP_NOSIZE);

		if (_tcslen (m_szFolder) > 0)
		{
			if (CBaseKey::g_pBoxPW == NULL)
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
			yyGetAppPath (m_hInst, m_szFolder, sizeof (m_szFolder));
			_tcscat (m_szFolder, _T("\\"));
			_tcscat (m_szFolder, _T("myBox"));
			SetDlgItemText (m_hDlg, IDC_EDIT_PATH, m_szFolder);
		}
		if (!m_bNewBox)
		{
			ShowWindow (GetDlgItem (m_hDlg, IDC_STATIC_PW2), SW_HIDE);
			ShowWindow (GetDlgItem (m_hDlg, IDC_EDIT_PW2), SW_HIDE);

			GetClientRect (m_hDlg, &rcDlg);
			rcDlg.bottom = rcDlg.bottom - 60;
			SetWindowPos (m_hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcDlg.right) / 2, 
							(GetSystemMetrics (SM_CYSCREEN) - rcDlg.bottom ) / 2, rcDlg.right, rcDlg.bottom, 0);	
			SetWindowPos (GetDlgItem (m_hDlg, IDOK), NULL, 80, rcDlg.bottom - 70, 0, 0, SWP_NOSIZE);
			SetWindowPos (GetDlgItem (m_hDlg, IDCANCEL), NULL, 240, rcDlg.bottom - 70, 0, 0, SWP_NOSIZE);
		}
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
				_tcscat (m_szFolder, _T("myBox"));
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
					MessageBox (m_hDlg, m_pLangText->GetText (YYTEXT_PW_6), m_pLangText->GetText (YYTEXT_Error), MB_OK);
					return S_OK;
				}
				_tcscpy (m_szPW, szPW1);
				m_keyBase.SetPassWord (m_szPW);
				return EndDialog(m_hDlg, LOWORD(wParam));
			}

			GetDlgItemText (m_hDlg, IDC_EDIT_PW2, szPW2, sizeof (szPW2) / sizeof (TCHAR));
			if (_tcslen (szPW1) != 6 || _tcslen (szPW1) != 6)
			{
				MessageBox (m_hDlg, m_pLangText->GetText (YYTEXT_PW_6), m_pLangText->GetText (YYTEXT_Error), MB_OK);
				return S_OK;
			}
		
			if (_tcscmp (szPW1, szPW2))
			{
				MessageBox (m_hDlg, m_pLangText->GetText (YYTEXT_PW_Same), m_pLangText->GetText (YYTEXT_Error), MB_OK);
				return S_OK;
			}

			for (int i = 0; i < 6; i++)
			{
				if ((szPW1[i] > _T('z') || szPW1[i] < _T('a')) &&
					(szPW1[i] > _T('Z') || szPW1[i] < _T('A')) &&
					(szPW1[i] > _T('9') || szPW1[i] < _T('0')))
				{
					MessageBox (m_hDlg, m_pLangText->GetText (YYTEXT_CharNum), m_pLangText->GetText (YYTEXT_Error), MB_OK);
					return S_OK;
				}
			}

			BOOL bRC = CreateDirectory (m_szFolder, NULL);
			DWORD dwAttr = GetFileAttributes (m_szFolder);
			dwAttr |= FILE_ATTRIBUTE_HIDDEN;
			bRC = SetFileAttributes (m_szFolder, dwAttr);

			TCHAR		szFolder[1024];
			_tcscpy (szFolder, m_szFolder);
			_tcscat (szFolder, _T("\\"));

			TCHAR szNewName[256];
			_tcscpy (szNewName, szPW1);
			m_keyBase.CreateText (szNewName, sizeof (szNewName));
			_tcscat (szFolder, szNewName);
			bRC = CreateDirectory (szFolder, NULL);
			if (!bRC)
			{
				MessageBox (m_hDlg, m_pLangText->GetText (YYTEXT_PW_Exist), m_pLangText->GetText (YYTEXT_Error), MB_OK);
				break;
			}
			dwAttr = GetFileAttributes (szFolder);
			dwAttr |= FILE_ATTRIBUTE_HIDDEN;
			bRC = SetFileAttributes (szFolder, dwAttr);

			_tcscpy (m_szPW, szPW1);
			m_keyBase.SetPassWord (m_szPW);
		}
		case IDCANCEL:
			EndDialog(m_hDlg, LOWORD(wParam));
			break;

		default:
			break;
		}
		break;

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

