/*******************************************************************************
	File:		CUpdateCopy.cpp

	Contains:	The ext Source implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#include "string.h"
#include "tchar.h"
#include "CUpdateCopy.h"

#include "USystemFunc.h"
#include "UFileFunc.h"

#pragma warning (disable : 4996)

CUpdateCopy::CUpdateCopy(void * hInst)
	: m_hInst (hInst)
{
}

CUpdateCopy::~CUpdateCopy(void)
{
}

bool CUpdateCopy::UpdateFiles (void)
{
	TCHAR szSource[1024];
	TCHAR szTarget[1024];
	yyGetAppPath (m_hInst, szTarget, sizeof (szTarget));
	yyGetAppPath (m_hInst, szSource, sizeof (szSource));
	_tcscat (szSource, _T("Update\\Files\\"));

	bool bRC = CopyFiles (szSource, szTarget);
	if (bRC)
	{
		yyDeleteFolder (szSource);
		_tcscat (szTarget, _T("jinPlayer.exe"));
		STARTUPINFO			si = { sizeof(si) };   
		PROCESS_INFORMATION pi;  
		si.dwFlags = STARTF_USESHOWWINDOW;   
		si.wShowWindow = TRUE;
		BOOL bRet = CreateProcess (NULL, szTarget, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);   
		if(bRet)   
		{   
			CloseHandle (pi.hThread);   
			CloseHandle (pi.hProcess);   
		}   
	}
	return bRC;
}

bool CUpdateCopy::CopyFiles (TCHAR * pSource, TCHAR * pTarget)
{
	TCHAR	szSource[1024];
	TCHAR	szTarget[1024];
	TCHAR	szFilter[1024];
	TCHAR *	pExt = NULL;
	_tcscpy (szFilter, pSource);
	_tcscat (szFilter, _T("*.*"));
	WIN32_FIND_DATA  data;
	HANDLE  hFind = FindFirstFile(szFilter,&data);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	do
	{
		if (!_tcscmp (data.cFileName, _T(".")) || !_tcscmp (data.cFileName, _T("..")))
			continue;	
		
		TCHAR szName[256];
		_tcscpy (szName, data.cFileName);
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			TCHAR szSubSource[1024];
			TCHAR szSubTarget[1024];
			_tcscpy (szSubSource, pSource);
			_tcscat (szSubSource, data.cFileName);
			_tcscat (szSubSource, _T("\\"));
			_tcscpy (szSubTarget, pTarget);
			_tcscat (szSubTarget, data.cFileName);
			_tcscat (szSubTarget, _T("\\"));
			if (!CopyFiles (szSubSource, szSubTarget))
			{
				FindClose (hFind);
				return false;
			}
			continue;	
		}
		
		_tcscpy (szSource, pSource);
		_tcscat (szSource, data.cFileName);
		_tcscpy (szTarget, pTarget);
		_tcscat (szTarget, data.cFileName);
		pExt = _tcsrchr (szTarget, _T('.'));
		if (pExt != NULL)
		{
			if (!_tcscmp (pExt, _T(".exx")))
				_tcscpy (pExt, _T(".exe"));
			else if (!_tcscmp (pExt, _T(".Exx")))
				_tcscpy (pExt, _T(".Exe"));
			else if (!_tcscmp (pExt, _T(".Ddd")))
				_tcscpy (pExt, _T(".Dll"));
			else if (!_tcscmp (pExt, _T(".ddd")))
				_tcscpy (pExt, _T(".dll"));
		}

		yyFile hFileSrc = yyFileOpen (szSource, YYFILE_READ);
		if (hFileSrc == NULL)
		{
			FindClose (hFind);
			return false;
		}
		yyFile hFileTar = yyFileOpen (szTarget, YYFILE_WRITE);
		if (hFileTar == NULL)
		{
			int nTryTimes = 0;
			while (nTryTimes < 10)
			{
				Sleep (1000);
				nTryTimes++;
				hFileTar = yyFileOpen (szTarget, YYFILE_WRITE);
				if (hFileTar != NULL)
					break;
			}
			if (hFileTar == NULL)
			{
				yyFileClose (hFileSrc);
				FindClose (hFind);
				return false;
			}
		}
		
		int nRead = 0;
		int nWrite = 0;
		int nSize = 1024 * 32;
		unsigned char * pBuff = new unsigned char[nSize];
		while (nRead >= 0)
		{
			nRead = yyFileRead (hFileSrc, pBuff, nSize);
			if (nRead <= 0)
				break;
			nWrite = yyFileWrite (hFileTar, pBuff, nRead);
			if (nWrite != nRead)
				break;
		}
		yyFileClose (hFileSrc);
		yyFileClose (hFileTar);
		if (nWrite != nRead && nRead > 0)
		{
			FindClose (hFind);
			return false;
		}
	}while(FindNextFile(hFind, &data));
	FindClose (hFind);
	return true;
}
