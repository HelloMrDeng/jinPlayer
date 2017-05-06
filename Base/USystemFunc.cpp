/*******************************************************************************
	File:		USystemFunc.cpp

	Contains:	The base utility for system implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifdef _OS_WIN32
#include "windows.h"
#include "shlobj.h"
#elif defined _OS_LINUX
#include <stdio.h>
#include <unistd.h>
#include <time.h>      
#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h> 
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <assert.h>
#endif // _OS_WIN32

#include "USystemFunc.h"
#include "UStringFunc.h"

#include "yyLog.h"

#ifdef _OS_WIN32
#pragma warning (disable : 4996)
#endif // _OS_WIN32

TCHAR g_szWorkPath[1024];

int	yyGetSysTime (void)
{
	int nTime = 0;
#ifdef _OS_WIN32
	nTime = GetTickCount ();
#elif defined _OS_LINUX
    timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);

    static timespec stv = {0, 0};
    if ((0 == stv.tv_sec) && (0 == stv.tv_nsec))
	{
		stv.tv_sec = tv.tv_sec;
		stv.tv_nsec = tv.tv_nsec;
	}
    
    nTime = (int)((tv.tv_sec - stv.tv_sec) * 1000 + (tv.tv_nsec - stv.tv_nsec) / 1000000);
/*	
	struct timeval tval;
	gettimeofday(&tval, NULL);
	nTime = tval.tv_sec*1000 + tval.tv_usec/1000;
*/
#endif // _OS_WIN32

	return nTime;
}

void yySleep (int nTime)
{
#ifdef _OS_WIN32
	Sleep (nTime / 1000);
#elif defined _OS_LINUX
	usleep (nTime);
#endif // _OS_WIN32
}

int yyGetThreadTime (void * hThread)
{
	int nTime = -1;
#ifdef _OS_WIN32
	if(hThread == NULL)
		hThread = GetCurrentThread();

	FILETIME ftCreationTime;
	FILETIME ftExitTime;
	FILETIME ftKernelTime;
	FILETIME ftUserTime;

	BOOL bRC = GetThreadTimes(hThread, &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUserTime);
	if (!bRC)
		return nTime;

	LONGLONG llKernelTime = ftKernelTime.dwHighDateTime;
	llKernelTime = llKernelTime << 32;
	llKernelTime += ftKernelTime.dwLowDateTime;

	LONGLONG llUserTime = ftUserTime.dwHighDateTime;
	llUserTime = llUserTime << 32;
	llUserTime += ftUserTime.dwLowDateTime;

	nTime = int((llKernelTime + llUserTime) / 10000);

#elif defined _OS_LINUX
    timespec tv;
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tv);

    static timespec stvThread = {0, 0};
    if ((0 == stvThread.tv_sec) && (0 == stvThread.tv_nsec))
	{
		stvThread.tv_sec = tv.tv_sec;
		stvThread.tv_nsec = tv.tv_nsec;
	}
    
    nTime = (int)((tv.tv_sec - stvThread.tv_sec) * 1000 + (tv.tv_nsec - stvThread.tv_nsec) / 1000000);

#endif // _OS_WIN32

	return nTime;
}

int	yyGetCPUNum (void)
{
#ifdef _OS_NDK
	int nTemps[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21};
	char cCpuName[512];
	memset(cCpuName, 0, sizeof(cCpuName));

	for(int i = (sizeof(nTemps)/sizeof(nTemps[0])) - 1 ; i >= 0; i--)
	{
		sprintf(cCpuName, "/sys/devices/system/cpu/cpu%d", nTemps[i]);
		int nOk = access(cCpuName, F_OK);
		if( nOk == 0)
		{
			return nTemps[i]+1;
		}
	}
	return 1;
#elif defined(_WIN32)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
#endif

	return 1;
}

int	yyGetAppPath (void * hInst, TCHAR * pPath, int nSize)
{
#ifdef _OS_WIN32
	GetModuleFileName ((HMODULE)hInst, pPath, nSize);
    TCHAR * pPos = _tcsrchr(pPath, _T('/'));
	if (pPos == NULL)
		pPos = _tcsrchr(pPath, _T('\\'));
    int nPos = pPos - pPath;
    pPath[nPos+1] = _T('\0');	
#else
	int			size = 0;
	FILE *		hFile = NULL;
	char		szPkgName[256];
	memset (szPkgName, 0, sizeof (szPkgName));
	hFile = fopen("/proc/self/cmdline", "rb");
	if (hFile != NULL)
	{  
		fgets(szPkgName, 256, hFile);
		fclose(hFile);
		strcpy (pPath, "/data/data/");
		strcat (pPath, szPkgName);
		strcat (pPath, "/");
	}

#endif // _OS_WIN32

	return 0;
}

int yyGetDataPath (void * hWnd, TCHAR * pPath, int nSize)
{
#ifdef _OS_WINPC
	BOOL bRC = SHGetSpecialFolderPath ((HWND)hWnd, pPath, CSIDL_COMMON_APPDATA, TRUE);
	if (!bRC)
		return YY_ERR_FAILED;
	TCHAR szApp[1024];
	memset (szApp, 0, sizeof (szApp));
	GetModuleFileName (NULL, szApp, sizeof (szApp));
	TCHAR * pDir = _tcsrchr (szApp, _T('\\'));
	if (pDir == NULL)
		return YY_ERR_FAILED;
	TCHAR * pExt = _tcsrchr (szApp, _T('.'));
	if (pExt != NULL)
		*pExt = 0;
	_tcscat (pPath, pDir);
	DWORD dwAttr = GetFileAttributes (pPath);
	if (dwAttr == -1)
		bRC = CreateDirectory (pPath, NULL);
	_tcscat (pPath, _T("\\"));
	return 0;
#else
	return yyGetAppPath (NULL, pPath, nSize);
#endif // _OS_WINPC
}

bool yyDeleteFolder (TCHAR * pFolder)
{
#ifdef _OS_WIN32
	TCHAR	szFolder[1024];
	TCHAR	szFilter[1024];
	_tcscpy (szFilter, pFolder);
	_tcscat (szFilter, _T("\\*.*"));
	WIN32_FIND_DATA  data;
	HANDLE  hFind = FindFirstFile(szFilter,&data);
	if (hFind == INVALID_HANDLE_VALUE)
		return true;
	do
	{
		if (!_tcscmp (data.cFileName, _T(".")) || !_tcscmp (data.cFileName, _T("..")))
			continue;	
		
		_tcscpy (szFolder, pFolder);
		_tcscat (szFolder, _T("\\"));
		_tcscat (szFolder, data.cFileName);
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			yyDeleteFolder (szFolder);
		}

		DeleteFile (szFolder);
	}while(FindNextFile(hFind, &data));
	FindClose (hFind);

	BOOL bRC = RemoveDirectory (pFolder);

	return bRC == TRUE ? true : false;
#else
	return false;
#endif // _OS_WIN32
}
