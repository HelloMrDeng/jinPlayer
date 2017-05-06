/*******************************************************************************
	File:		ULibFunc.cpp

	Contains:	The utility for library implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "ULibFunc.h"
#include "UStringFunc.h"
#include "USystemFunc.h"
#ifdef _OS_NDK
#include "dlfcn.h"
#endif // _OS_NDK

#include "yyLog.h"

#ifdef _OS_WIN32
#pragma warning (disable : 4996)
#endif // _OS_WIN32

void * yyLibLoad (const TCHAR * pLibName, int nFlag)
{
	void * hDll = NULL;
	TCHAR szLib[256];
	
	_tcscpy (szLib, pLibName);
#ifdef _OS_WIN32
	_tcscat (szLib, _T(".dll"));
	hDll = LoadLibrary (szLib);
#else
	strcpy (szLib, "lib");
	strcat (szLib, pLibName);
	strcat (szLib, ".so");
	hDll = dlopen (szLib, RTLD_NOW);
#endif // _Os_WIN32
	if (hDll == NULL)
	{
#ifdef _OS_WIN32
		_tcscpy (szLib, g_szWorkPath);
		_tcscat (szLib, pLibName);
		_tcscat (szLib, _T(".dll"));
		hDll = LoadLibrary (szLib);
#else
		strcpy (szLib, g_szWorkPath);
		strcat (szLib, "lib/lib");
		strcat (szLib, pLibName);
		strcat (szLib, ".so");
		hDll = dlopen (szLib, RTLD_NOW);
#endif // _Os_WIN32
	}
	if (hDll == NULL)
		YYLOGT ("ULIBFunc", "Load %s failed! %s", pLibName, szLib);
	return hDll;
}

void * yyLibGetAddr (void * hLib, const char * pFuncName, int nFlag)
{
	void * hFunc = NULL;

#ifdef _OS_WIN32
#ifdef _OS_WINCE
	TCHAR szName[256];
	memset (szName, 0, sizeof (szName));
	MultiByteToWideChar (CP_ACP, 0, pFuncName, -1, szName, sizeof (szName));
	hFunc = GetProcAddress ((HMODULE)hLib, szName);
#else
	hFunc = GetProcAddress ((HMODULE)hLib, pFuncName);
#endif // _OS_WINCE
#else
	hFunc = dlsym (hLib, pFuncName);
#endif // _OS_WIN32
	if (hFunc == NULL)
		YYLOGT ("ULibFunc",  "GetAddr %s Failed! Hlib = %p ", pFuncName, hLib);
	return hFunc;
}

int yyLibFree (void * hLib, int nFlag)
{
#ifdef _OS_WIN32
	FreeLibrary ((HMODULE)hLib);
#else
	dlclose (hLib);
#endif // _OS_WIN32
	return 0;
}
