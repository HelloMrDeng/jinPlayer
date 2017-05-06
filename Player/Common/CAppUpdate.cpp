/*******************************************************************************
	File:		CAppUpdate.cpp

	Contains:	The ext Source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "commctrl.h"
#include "CAppUpdate.h"
#include "CLangText.h"
#include "CRegMng.h"

#include "USystemFunc.h"
#include "UFIleFunc.h"
#include "Resource.h"
#include "yyLog.h"

#define	TIMER_GetInfo	201
#define WM_DownLoad_Failed	WM_USER + 97
#define WM_DownLoad_Finish	WM_USER + 99

CAppUpdate::CAppUpdate(void * hInst, HWND hParent)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hParent (hParent)
	, m_hDlg (NULL)
	, m_hProg (NULL)
	, m_pIO (NULL)
	, m_pCFG (NULL)
	, m_hThread (NULL)
	, m_bCancel (false)
	, m_bMSIFile (false)
{
	SetObjectName ("AppUpdate");
	TCHAR * pVer = CRegMng::g_pRegMng->GetTextValue (_T("CurVersion"));
	if (_tcslen (pVer) <= 0)
		_tcscpy (m_szVer, _T("V10_B109"));
	else
		_tcscpy (m_szVer, pVer);
//	_tcscpy (m_szVer, _T("V10_B109"));
}

CAppUpdate::~CAppUpdate(void)
{
	YY_DEL_P (m_pIO);
	YY_DEL_P (m_pCFG);
}

int CAppUpdate::OpenDlg (void)
{	
	m_pIO = new CSourceIO ();
	m_pCFG = new CBaseConfig ();

	int nRC = DialogBoxParam ((HINSTANCE)m_hInst, MAKEINTRESOURCE(IDD_DIALOG_UPDATE), m_hParent, AppUpdateDlgProc, (LPARAM)this);
	return nRC;
}

LRESULT CAppUpdate::OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int	wmId, wmEvent;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		RECT rcDlg;
		SetWindowLong (hWnd, GWL_USERDATA, lParam);
		m_hDlg = hWnd;
		m_hProg = GetDlgItem (m_hDlg, IDC_PROGRESS_UPDATE);
		GetClientRect (m_hDlg, &rcDlg);
		SetWindowPos (m_hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcDlg.right) / 2, 
						(GetSystemMetrics (SM_CYSCREEN) - rcDlg.bottom ) / 2, 0, 0, SWP_NOSIZE);

		SendMessage (m_hProg, PBM_SETRANGE, 0, MAKELPARAM (0, 100));

		SetWindowText (m_hDlg, yyLangGetText (YYTEXT_CheckUpate));
		SetWindowText (GetDlgItem (m_hDlg, IDC_BUTTON_CANCEL), yyLangGetText (YYTEXT_Cancel));
		SetWindowText (GetDlgItem (m_hDlg, IDC_BUTTON_START), yyLangGetText (YYTEXT_Start));
		EnableWindow (GetDlgItem (m_hDlg, IDC_BUTTON_START), FALSE);
		SetTimer (m_hDlg, TIMER_GetInfo, 100, NULL);
		return S_FALSE;
	}

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDC_BUTTON_START:
		{
			EnableWindow (GetDlgItem (m_hDlg, IDC_BUTTON_START), FALSE);
			int nID = 0;
			yyThreadCreate (&m_hThread, &nID, UpdateProc, this, 0);
			break;
		}

		case IDCANCEL:
		case IDC_BUTTON_CANCEL:
			m_bCancel = true;
			while (m_hThread != NULL)
				Sleep (10);
			EndDialog(m_hDlg, IDCANCEL);
			break;

		default:
			break;
		}
		break;

	case WM_TIMER:
		KillTimer (m_hDlg, TIMER_GetInfo);
		if (!GetUpdateInfo ())
		{
			MessageBox (m_hDlg, yyLangGetText (YYTEXT_GetInfoFail), yyLangGetText (YYTEXT_Error), MB_OK);
			EndDialog(m_hDlg, IDCANCEL);
		}
		break;

	case WM_DownLoad_Failed:
		MessageBox (m_hDlg, yyLangGetText (YYTEXT_DL_Fail), yyLangGetText (YYTEXT_Error), MB_OK);
		EndDialog(m_hDlg, IDCANCEL);
		break;

	case WM_DownLoad_Finish:
		if (StartUpdate ())
			PostMessage (m_hParent, WM_CLOSE, 0, 0);
		EndDialog(m_hDlg, IDOK);
		break;

	default:
		break;
	}

	return S_OK;
}

INT_PTR CALLBACK CAppUpdate::AppUpdateDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CAppUpdate * pDlgUpdate = NULL;
	UNREFERENCED_PARAMETER(lParam);
	if (message == WM_INITDIALOG)
		pDlgUpdate = (CAppUpdate *)lParam;
	else if (hDlg != NULL)
		pDlgUpdate = (CAppUpdate *)GetWindowLong (hDlg, GWL_USERDATA);

	if (pDlgUpdate != NULL)
		return (INT_PTR) pDlgUpdate->OnMessage (hDlg, message, wParam, lParam);

	return (INT_PTR) FALSE;
}


int CAppUpdate::UpdateProc (void * pParam)
{
	CAppUpdate * pDlg = (CAppUpdate *)pParam;
	if (!pDlg->m_bCancel)
	{
		if (pDlg->UpdateLoop () != YY_ERR_NONE)
			PostMessage (pDlg->m_hDlg, WM_DownLoad_Failed, 0, 0);
		else
			PostMessage (pDlg->m_hDlg, WM_DownLoad_Finish, 0, 0);
	}
	pDlg->m_hThread = NULL;
	return 0;
}

int CAppUpdate::UpdateLoop (void)
{
	TCHAR szApp[1024];
	yyGetAppPath (m_hInst, szApp, sizeof (szApp));
	_tcscat (szApp, _T("Update"));
	DWORD dwAttr = GetFileAttributes (szApp);
	if (dwAttr == -1)
	{
		CreateDirectory (szApp, NULL);
		dwAttr = GetFileAttributes (szApp);
		m_bMSIFile = dwAttr == -1 ? true : false;
	}

	TCHAR szFile[1024];
	yyGetAppPath (m_hInst, szFile, sizeof (szFile));
	_tcscat (szFile, _T("jpres\\jpUpdate.cfg"));
	if (!m_pCFG->Open (szFile))
		return YY_ERR_FAILED;
	char * pFile = m_pCFG->GetItemText ("Update", "ConfigFile", NULL);
	if (pFile == NULL)
		return YY_ERR_FAILED;

	int nRC = m_pIO->Init (pFile, 0);
	if (nRC != VO_ERR_NONE)
		return YY_ERR_FAILED;
	nRC = m_pIO->Open ();
	if (nRC != VO_ERR_NONE)
	{
		m_pIO->Close ();
		return YY_ERR_FAILED;
	}
	int nRead = 0;
	int nSize = (int)m_pIO->GetSize ();
	VO_PBYTE pBuff = NULL;
	pBuff = new VO_BYTE[nSize + 4];
	memset (pBuff, 0, nSize + 4);
	nRC = m_pIO->Read (pBuff, nSize, &nRead);
	nRC = m_pIO->Close ();
	if (nRead != nSize)
	{
		delete []pBuff;
		return YY_ERR_FAILED;
	}
	if (m_bMSIFile)
		yyGetDataPath (m_hInst, szFile, sizeof (szFile));
	else
		yyGetAppPath (m_hInst, szFile, sizeof (szFile));
	_tcscat (szFile, _T("update"));
	dwAttr = GetFileAttributes (szFile);
	if (dwAttr == -1)
		nRC = CreateDirectory (szFile, NULL);
	_tcscat (szFile, _T("\\updatefiles.cfg"));
	yyFile hFile = yyFileOpen (szFile, YYFILE_WRITE);
	int nWrite = yyFileWrite (hFile, pBuff, nSize);
	yyFileClose (hFile);
	delete []pBuff;
	if (nWrite != nSize)
		return YY_ERR_FAILED;

	if (!m_pCFG->Open (szFile))
		return YY_ERR_FAILED;
	char * pPath = m_pCFG->GetItemText ("UpdateFiles", "Path", NULL);
	if (pPath == NULL)
		return YY_ERR_FAILED;
	char * pVer = m_pCFG->GetItemText ("UpdateFiles", "Version", NULL);
	if (pVer == NULL)
		return YY_ERR_FAILED;
	int nFileNum = m_pCFG->GetItemValue ("UpdateFiles", "FileNum", 0);
	if (nFileNum == 0)
		return YY_ERR_FAILED;
	if (m_bMSIFile)
		nFileNum = 1;

	TCHAR szTarg[1024];
	TCHAR szPath[1024];
	TCHAR szName[1024];
	TCHAR * pDir = NULL;
	if (m_bMSIFile)
		yyGetDataPath (m_hInst, szPath, sizeof (szPath));
	else
		yyGetAppPath (m_hInst, szPath, sizeof (szPath));
	_tcscat (szPath, _T("Update\\Files"));
	dwAttr = GetFileAttributes (szPath);
	if (dwAttr == -1)
		nRC = CreateDirectory (szPath, NULL);
	_tcscat (szPath, _T("\\"));

	char *	pFileName = NULL;
	char	szFileName[32];
	char	szSource[1024];
	int		nRestSize = 0;
	int		nReadSize = 1024 * 32;
	int		i = 0;
	int		nTotalSize = 0;
	int		nTotalRead = 0;
	if (!m_bMSIFile)
	{
		for (i = 0; i < nFileNum; i++)
		{
			sprintf (szFileName, "FileSize%d", i+1);
			nTotalSize = nTotalSize + m_pCFG->GetItemValue ("UpdateFiles", szFileName, NULL);
		}
	}
	pBuff = new VO_BYTE[nReadSize];
	for (i = 0; i < nFileNum; i++)
	{
		sprintf (szFileName, "FileName%d", i+1);
		if (m_bMSIFile)
			pFileName = m_pCFG->GetItemText ("UpdateFiles", "MSIFile", NULL);
		else
			pFileName = m_pCFG->GetItemText ("UpdateFiles", szFileName, NULL);
		if (pFileName == NULL)
			return YY_ERR_FAILED;

		strcpy (szSource, pPath);
		strcat (szSource, "/");
		strcat (szSource, pVer);
		strcat (szSource, "/");
		strcat (szSource, pFileName);

		nRC = m_pIO->Init (szSource, 0);
		if (nRC != VO_ERR_NONE)
			return YY_ERR_FAILED;
		nRC = m_pIO->Open ();
		if (nRC != VO_ERR_NONE)
			return YY_ERR_FAILED;
		
		memset (szName, 0, sizeof (szName));
		MultiByteToWideChar (CP_ACP, 0, pFileName, -1, szName, sizeof (szName));
		_tcscpy (szTarg, szPath);
		pDir = _tcsstr (szName, _T("/"));
		if (pDir == NULL)
		{
			_tcscat (szTarg, szName);
		}
		else
		{
			_tcsncat (szTarg, szName, pDir - szName);
			dwAttr = GetFileAttributes (szTarg);
			if (dwAttr == -1)
				nRC = CreateDirectory (szTarg, NULL);
			_tcscat (szTarg, pDir);
		}
		if (m_bMSIFile)
			_tcscpy (m_szMSIFile, szTarg);
		hFile = yyFileOpen (szTarg, YYFILE_WRITE);
		if (hFile == NULL)
		{
			nRC = m_pIO->Close ();
			return YY_ERR_FAILED;
		}

		nSize = (int)m_pIO->GetSize ();
		nRestSize = nSize;
		if (m_bMSIFile)
			nTotalSize = nSize;
		while (nRestSize > 0)
		{
			nRC = m_pIO->Read (pBuff, nReadSize, &nRead);
			nRestSize = nRestSize - nRead;
			nTotalRead += nRead;
			nWrite = yyFileWrite (hFile, pBuff, nRead);
			if (nWrite != nRead || m_bCancel)
			{
				delete []pBuff;
				nRC = m_pIO->Close ();
				yyFileClose (hFile);
				return YY_ERR_FAILED;
			}
			SendMessage (m_hProg, PBM_SETPOS, (WPARAM)(nTotalRead / (nTotalSize / 100)), 0);
		}
		nRC = m_pIO->Close ();
		yyFileClose (hFile);
		if (m_bCancel)
		{
			delete []pBuff;
			return YY_ERR_FAILED;
		}
	}
	delete []pBuff;

	CRegMng::g_pRegMng->SetTextValue (_T("CurVersion"), m_szVer);

	return YY_ERR_NONE;
}

bool CAppUpdate::GetUpdateInfo (void)
{
	TCHAR szFile[1024];
	yyGetAppPath (m_hInst, szFile, sizeof (szFile));
	_tcscat (szFile, _T("jpres\\jpUpdate.cfg"));
	if (!m_pCFG->Open (szFile))
		return false;
	char * pFile = m_pCFG->GetItemText ("Update", "VersionFile", NULL);
	if (pFile == NULL)
		return false;

	int nRC = m_pIO->Init (pFile, 0);
	if (nRC != VO_ERR_NONE)
		return false;
	nRC = m_pIO->Open ();
	if (nRC != VO_ERR_NONE)
	{
		m_pIO->Close ();
		return false;
	}
	int nRead = 0;
	int nSize = (int)m_pIO->GetSize ();
	VO_PBYTE pBuff = NULL;
	pBuff = new VO_BYTE[nSize + 4];
	memset (pBuff, 0, nSize + 4);
	nRC = m_pIO->Read (pBuff, nSize, &nRead);
	nRC = m_pIO->Close ();
	if (nRead != nSize)
	{
		delete []pBuff;
		return false;
	}

	TCHAR * pInfo = (TCHAR *)(pBuff+2);
	TCHAR * pVer = _tcsstr (pInfo, m_szVer);
	if (pVer == NULL)
	{
		pVer = _tcsstr (pInfo, _T("V"));
		if (pVer != NULL)
		{
			memset (m_szVer, 0, sizeof (m_szVer));
			int nIdx = 0;
			while (pVer - pInfo < nSize)
			{
				m_szVer[nIdx++] = *pVer++;
				if (*pVer == _T(' ') || *pVer == _T('\r') || *pVer == _T('\n'))
					break;
			}
		}
		SetDlgItemText (m_hDlg, IDC_EDIT_CHANGED, (TCHAR *)(pBuff+2));
		delete []pBuff;
		EnableWindow (GetDlgItem (m_hDlg, IDC_BUTTON_START), TRUE);
	}
	else
	{
		MessageBox (m_hDlg, yyLangGetText (YYTEXT_LastVer), yyLangGetText (YYTEXT_Info), MB_OK);
		EndDialog(m_hDlg, IDCANCEL);
	}

	return true;
}

bool CAppUpdate::StartUpdate (void)
{
	if (m_bMSIFile)
	{
		 ShellExecute(NULL, _T("open"), m_szMSIFile, NULL, NULL, SW_SHOWNORMAL);
	}
	else
	{
		STARTUPINFO			si = { sizeof(si) };   
		PROCESS_INFORMATION pi;  
		TCHAR				szApp[1024];
		yyGetAppPath (m_hInst, szApp, sizeof (szApp));
		_tcscat (szApp, _T("jpUpdate.exe"));

		si.dwFlags = STARTF_USESHOWWINDOW;   
		si.wShowWindow = TRUE;
		BOOL bRet = CreateProcess (NULL, szApp, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);   
		if(bRet)   
		{   
			CloseHandle (pi.hThread);   
			CloseHandle (pi.hProcess);   
		}   
		else  
		{  
			MessageBox (m_hDlg, yyLangGetText (YYTEXT_StartUPFail), yyLangGetText (YYTEXT_Error), MB_OK);
			return false;
		}
	}
		
	return true;
}

