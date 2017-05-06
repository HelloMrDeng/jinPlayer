/*******************************************************************************
	File:		CVideoRender.cpp

	Contains:	New box dialog implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "commctrl.h"

#include "CDlgExport.h"
#include "CBaseKey.h"

#include "Resource.h"

#include "USystemFunc.h"
#include "UFileFunc.h"

#pragma warning (disable : 4996)

CDlgExport::CDlgExport(HINSTANCE hInst, HWND hParent, CBaseKey * pKey)
	: m_hInst (hInst)
	, m_hParent (hParent)
	, m_pKey (pKey)
	, m_hDlg (NULL)
	, m_hProg (NULL)
	, m_hThread (NULL)
	, m_bCancel (false)
{
	_tcscpy (m_szSource, _T(""));
	_tcscpy (m_szTarget, _T(""));
}

CDlgExport::~CDlgExport(void)
{
}

int CDlgExport::OpenDlg (TCHAR * pSource)
{
	if (pSource == NULL)
		return -1;

	_tcscpy (m_szSource, pSource);


	TCHAR	szPath[1024];
	TCHAR * pName = _tcsrchr (pSource, _T('\\'));
	_tcscpy (szPath, pName);
	OPENFILENAME	ofn;
	memset( &(ofn), 0, sizeof(ofn));
	ofn.lStructSize	= sizeof(ofn);
	ofn.hInstance = m_hInst;
	ofn.hwndOwner = m_hDlg;
	ofn.lpstrFilter = TEXT("Media File (*.*)\0*.*\0");	
	ofn.lpstrFile = szPath;
	ofn.nMaxFile = sizeof (szPath);
	ofn.lpstrTitle = TEXT("Export Media File");
	ofn.Flags = OFN_EXPLORER;		
	if (!GetSaveFileName(&ofn))
		return -1;
	_tcscpy (m_szTarget, szPath);
	
	int nRC = DialogBoxParam (m_hInst, MAKEINTRESOURCE(IDD_DIALOG_COPY), m_hParent, ExportFileDlgProc, (LPARAM)this);
	return nRC;
}

int CDlgExport::CopyProc (void * pParam)
{
	CDlgExport * pDlg = (CDlgExport *)pParam;
	pDlg->CopyLoop ();
	pDlg->m_hThread = NULL;
	return 0;
}

int CDlgExport::CopyLoop (void)
{
	yyFile hFileSrc = yyFileOpen (m_szSource, YYFILE_READ);
	if (hFileSrc == NULL)
	{
		PostMessage (m_hDlg, WM_CLOSE, 0, 0);
		return false;
	}
	yyFile hFileTgt = yyFileOpen (m_szTarget, YYFILE_READ);
	if (hFileTgt != NULL)
	{
		yyFileClose (hFileTgt);
		if (MessageBox (m_hDlg, yyLangGetText (YYTEXT_FileExist), yyLangGetText (YYTEXT_Info), MB_YESNO) != IDYES)
		{
			yyFileClose (hFileSrc);
			PostMessage (m_hDlg, WM_CLOSE, 0, 0);
			return false;
		}
		DeleteFile (m_szTarget);
	}	
	hFileTgt = yyFileOpen (m_szTarget, YYFILE_WRITE);
	if (hFileTgt == NULL)
	{
		yyFileClose (hFileSrc);
		PostMessage (m_hDlg, WM_CLOSE, 0, 0);
		return false;
	}
	int nSize = 1024 * 64;
	unsigned char * pBuff = new unsigned char[nSize];
	yyFileRead (hFileSrc, pBuff, YYKEY_LEN);

	int nRead = 0;
	int	nWrite = 0;
	long long llFileSize = yyFileSize (hFileSrc);
	long long llFileRest = llFileSize;
	while (llFileRest > 0)
	{
		nRead = yyFileRead (hFileSrc, pBuff, nSize);
		if (nRead > 0 && nRead <= nSize)
		{
			llFileRest = llFileRest - nRead;
			m_pKey->DecryptData (pBuff, nRead);
			nWrite = yyFileWrite (hFileTgt, pBuff, nRead);
		}
		else
		{
			break;
		}
		if (m_bCancel)
			break;
		SendMessage (m_hProg, PBM_SETPOS, (WPARAM)((llFileSize - llFileRest) * 100 / llFileSize), 0);
	}
	delete []pBuff;

	yyFileClose (hFileSrc);
	yyFileClose (hFileTgt);

	if (m_bCancel)
	{
		DeleteFile (m_szTarget);
		return 0;
	}

	EndDialog(m_hDlg, IDOK);

	return 0;
}

LRESULT CDlgExport::OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		GetClientRect (m_hDlg, &rcDlg);
		SetWindowPos (m_hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcDlg.right) / 2, 
						(GetSystemMetrics (SM_CYSCREEN) - rcDlg.bottom ) / 2, 0, 0, SWP_NOSIZE);

		SendMessage (m_hProg, PBM_SETRANGE, 0, MAKELPARAM (0, 100));

		SetWindowText (m_hDlg, yyLangGetText (YYTEXT_ExportFile));
		SetWindowText (GetDlgItem (m_hDlg, IDCANCEL), yyLangGetText (YYTEXT_Cancel));
		
		int nID = 0;
		yyThreadCreate (&m_hThread, &nID, CopyProc, this, 0);
		return S_FALSE;
	}

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
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

INT_PTR CALLBACK CDlgExport::ExportFileDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CDlgExport * pDlgCopy = NULL;
	UNREFERENCED_PARAMETER(lParam);
	if (message == WM_INITDIALOG)
		pDlgCopy = (CDlgExport *)lParam;
	else if (hDlg != NULL)
		pDlgCopy = (CDlgExport *)GetWindowLong (hDlg, GWL_USERDATA);

	if (pDlgCopy != NULL)
		return (INT_PTR) pDlgCopy->OnMessage (hDlg, message, wParam, lParam);

	return (INT_PTR) FALSE;
}

