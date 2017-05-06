/*******************************************************************************
	File:		CVideoRender.cpp

	Contains:	New box dialog implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "commctrl.h"

#include "CDlgImport.h"
#include "CBaseKey.h"

#include "Resource.h"

#include "USystemFunc.h"
#include "UFileFunc.h"

#pragma warning (disable : 4996)

CDlgImport::CDlgImport(HINSTANCE hInst, HWND hParent, CBaseKey * pKey)
	: m_hInst (hInst)
	, m_hParent (hParent)
	, m_pKey (pKey)
	, m_hDlg (NULL)
	, m_hProg (NULL)
	, m_hList (NULL)
	, m_nAddFiles (0)
	, m_ppSource (NULL)
	, m_nCount (0)
	, m_hThread (NULL)
	, m_bCancel (false)
{
}

CDlgImport::~CDlgImport(void)
{
}

int CDlgImport::OpenDlg (TCHAR * pFolder)
{
	m_nAddFiles = 0;
	_tcscpy (m_szFolder, pFolder);
	int nRC = DialogBoxParam (m_hInst, MAKEINTRESOURCE(IDD_DIALOG_IMPORT), m_hParent, ImportFileDlgProc, (LPARAM)this);
	return nRC;
}

LRESULT CDlgImport::OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int	wmId, wmEvent;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		RECT rcDlg;
		SetWindowLong (hWnd, GWL_USERDATA, lParam);
		m_hDlg = hWnd;
		m_hProg = GetDlgItem (m_hDlg, IDC_PROGRESS_COPY);
		m_hList = GetDlgItem (m_hDlg, IDC_LIST_FILE);
		GetClientRect (m_hDlg, &rcDlg);
		SetWindowPos (m_hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcDlg.right) / 2, 
						(GetSystemMetrics (SM_CYSCREEN) - rcDlg.bottom ) / 2, 0, 0, SWP_NOSIZE);

		SendMessage (m_hProg, PBM_SETRANGE, 0, MAKELPARAM (0, 100));

		SetWindowText (m_hDlg, yyLangGetText (YYTEXT_CopyFile));
		SetWindowText (GetDlgItem (m_hDlg, IDCANCEL), yyLangGetText (YYTEXT_Cancel));
		SetWindowText (GetDlgItem (m_hDlg, ID_BUTTON_ADD), yyLangGetText (YYTEXT_Add));
		SetWindowText (GetDlgItem (m_hDlg, ID_BUTTON_DELETE), yyLangGetText (YYTEXT_Delete));
		SetWindowText (GetDlgItem (m_hDlg, ID_BUTTON_START), yyLangGetText (YYTEXT_Start));
		SetWindowText (GetDlgItem (m_hDlg, IDC_CHECK_DELETE), yyLangGetText (YYTEXT_DelOrgFile));
		SetWindowText (GetDlgItem (m_hDlg, IDC_CHECK_CLOSE), yyLangGetText (YYTEXT_CloseFinish));

		if (m_nCount > 0 && m_ppSource != NULL)
		{
			for (int i = 0; i < m_nCount; i++)
				SendMessage (m_hList, LB_ADDSTRING, 0, (LPARAM)m_ppSource[i]);

			int nID = 0;
			yyThreadCreate (&m_hThread, &nID, CopyProc, this, 0);
		}

#if 0
		SendMessage (m_hList, LB_ADDSTRING, 0, (LPARAM)_T("11111"));
		SendMessage (m_hList, LB_ADDSTRING, 0, (LPARAM)_T("2222"));
		SendMessage (m_hList, LB_ADDSTRING, 0, (LPARAM)_T("333333"));
		SendMessage (m_hList, LB_ADDSTRING, 0, (LPARAM)_T("444444"));
#endif // 0
		return S_FALSE;
	}

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case ID_BUTTON_ADD:
			AddFiles ();
			break;

		case ID_BUTTON_DELETE:
			DelFiles ();
			break;

		case ID_BUTTON_START:
		{
			int nID = 0;
			yyThreadCreate (&m_hThread, &nID, CopyProc, this, 0);
			break;
		}

		case IDCANCEL:
			m_bCancel = true;
			while (m_hThread != NULL)
				Sleep (10);
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

void CDlgImport::AddFiles (void)
{
	TCHAR	szFiles[81920];
	_tcscpy (szFiles, _T("*.*"));
	OPENFILENAME	ofn;
	memset( &(ofn), 0, sizeof(ofn));
	ofn.lStructSize	= sizeof(ofn);
	ofn.hInstance = m_hInst;
	ofn.hwndOwner = m_hDlg;
	ofn.lpstrFilter = TEXT("Media File (*.*)\0*.*\0");	
	ofn.lpstrFile = szFiles;
	ofn.nMaxFile = sizeof (szFiles);
	ofn.lpstrTitle = TEXT("Import Media File");
	ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT;		
	if (!GetOpenFileName(&ofn))
		return;

	TCHAR		szFile[1024];
	TCHAR *		pName = szFiles + _tcslen (szFiles) + 1;
	if (*pName == 0)
	{
		pName = _tcsrchr (szFiles, _T('\\'));
		*pName++ = 0;
	}
	while (*pName != 0)
	{
		_tcscpy (szFile, szFiles);
		_tcscat (szFile, _T("\\"));
		_tcscat (szFile, pName);

		SendMessage (m_hList, LB_ADDSTRING, 0, (LPARAM)szFile);

		pName = pName + _tcslen (pName) + 1;
	}
}

void CDlgImport::DelFiles (void)
{
	int nSel = SendMessage (m_hList, LB_GETSELCOUNT, 0, 0);
	int * pSelItems = new int[nSel];
	nSel = SendMessage (m_hList, LB_GETSELITEMS, (WPARAM)nSel, (LPARAM)pSelItems);
	for (int i = nSel; i >= 0; i--)
		SendMessage (m_hList, LB_DELETESTRING, pSelItems[i], 0);
	delete []pSelItems;
}

INT_PTR CALLBACK CDlgImport::ImportFileDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CDlgImport * pDlgCopy = NULL;
	UNREFERENCED_PARAMETER(lParam);
	if (message == WM_INITDIALOG)
		pDlgCopy = (CDlgImport *)lParam;
	else if (hDlg != NULL)
		pDlgCopy = (CDlgImport *)GetWindowLong (hDlg, GWL_USERDATA);

	if (pDlgCopy != NULL)
		return (INT_PTR) pDlgCopy->OnMessage (hDlg, message, wParam, lParam);

	return (INT_PTR) FALSE;
}

int CDlgImport::CopyProc (void * pParam)
{
	CDlgImport * pDlg = (CDlgImport *)pParam;
	pDlg->CopyLoop ();
	pDlg->m_hThread = NULL;
	return 0;
}

int CDlgImport::CopyLoop (void)
{
	EnableWindow (GetDlgItem (m_hDlg, ID_BUTTON_ADD), FALSE);
	EnableWindow (GetDlgItem (m_hDlg, IDC_BUTTON_DELETE), FALSE);
	EnableWindow (GetDlgItem (m_hDlg, ID_BUTTON_START), FALSE);
	EnableWindow (GetDlgItem (m_hDlg, IDC_CHECK_DELETE), FALSE);
	EnableWindow (GetDlgItem (m_hDlg, IDC_CHECK_CLOSE), FALSE);
	int i = 0;
	int nCount = SendMessage (m_hList, LB_GETCOUNT, 0, 0);
	for (i = 0; i < nCount; i++)
		SendMessage (m_hList, LB_SETSEL, FALSE, i);

	bool bDelete = false;
	if (SendMessage (GetDlgItem (m_hDlg, IDC_CHECK_DELETE), BM_GETCHECK, 0, 0) == BST_CHECKED)
		bDelete = true;

	int nSize = 1024 * 64;
	unsigned char * pBuff = new unsigned char[nSize];

	TCHAR	szSource[1024];
	TCHAR	szTarget[1024];
	TCHAR *	pName = NULL;
	TCHAR * pExt = NULL;
	int		nIdx = 0;
	for (i = 0; i < nCount; i++)
	{
		SendMessage (m_hList, LB_SETSEL, TRUE, i);
		memset (szSource, 0, sizeof (szSource));
		SendMessage (m_hList, LB_GETTEXT, i, (LPARAM)szSource);
		_tcscpy (szTarget, m_szFolder);
		pName = _tcsrchr (szSource, _T('\\'));
		_tcscat (szTarget, pName);

		nIdx = 0;
		yyFile hFileTgt = yyFileOpen (szTarget, YYFILE_READ);
		while (hFileTgt != NULL)
		{
			yyFileClose (hFileTgt);
			_tcscpy (szTarget, m_szFolder);
			pName = _tcsrchr (szSource, _T('\\'));
			_tcscat (szTarget, pName);
			pExt = _tcsrchr (szTarget, _T('.'));
			*pExt = 0;
			_stprintf (szTarget, _T("%s_%d"), szTarget, nIdx);
			pExt = _tcsrchr (szSource, _T('.'));
			_tcscat (szTarget, pExt);
			hFileTgt = yyFileOpen (szTarget, YYFILE_READ);
			nIdx++;

		}
		hFileTgt = yyFileOpen (szTarget, YYFILE_WRITE);
		if (hFileTgt == NULL)
			continue;
		yyFile hFileSrc = yyFileOpen (szSource, YYFILE_READ);
		if (hFileSrc == NULL)
			continue;

		yyFileWrite (hFileTgt, (unsigned char *)YYKEY_TEXT, YYKEY_LEN);

		int nRead = 0;
		int	nWrite = 0;
		long long llFileSize = yyFileSize (hFileSrc);
		long long llFileRest = llFileSize;
		while (llFileRest > 0)
		{
			nRead = yyFileRead (hFileSrc, pBuff, nSize);
			llFileRest = llFileRest - nRead;
			m_pKey->EncryptData (pBuff, nRead);
			nWrite = yyFileWrite (hFileTgt, pBuff, nRead);

			PostMessage (m_hProg, PBM_SETPOS, (WPARAM)((llFileSize - llFileRest) * 100 / llFileSize), 0);
			if (m_bCancel)
				break;
		}
		yyFileClose (hFileSrc);
		yyFileClose (hFileTgt);
		if (m_bCancel)
		{
			DeleteFile (szTarget);
			delete []pBuff;
			return 0;
		}

		DWORD dwAttr = GetFileAttributes (szTarget);
		dwAttr |= FILE_ATTRIBUTE_HIDDEN;
		SetFileAttributes (szTarget, dwAttr);
		if (bDelete)
			DeleteFile (szSource);

		m_nAddFiles++;
		SendMessage (m_hList, LB_SETSEL, FALSE, i);
	}
	delete []pBuff;
	if (SendMessage (GetDlgItem (m_hDlg, IDC_CHECK_CLOSE), BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		m_nAddFiles = 0;
		PostMessage (m_hParent, WM_CLOSE, 0, 0);
	}

	EndDialog(m_hDlg, IDOK);

	return 0;
}
