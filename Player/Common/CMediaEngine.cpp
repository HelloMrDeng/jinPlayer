/*******************************************************************************
	File:		CMediaEngine.cpp

	Contains:	yy Media Engine wrap implement file

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#ifdef _OS_NDK
#include <dlfcn.h>
#endif // _OS_NDK

#include "CMediaEngine.h"

#include "USystemFunc.h"
#include "ULibFunc.h"
#include "yyLog.h"

CMediaEngine::CMediaEngine(void)
	: m_pPlayer (NULL)
	, m_hDllBase (NULL)		
	, m_hDllMedia (NULL)
	, m_fInit (NULL)
	, m_fCreate (NULL)
	, m_fDestroy (NULL)
	, m_fUninit (NULL)
	, m_hMediaEng (NULL)
{
	SetObjectName ("CMediaEngine");
}

CMediaEngine::~CMediaEngine(void)
{
	Uninit ();
}

YYM_Player * CMediaEngine::GetPlayer (void)
{
	if (m_pPlayer == NULL)
		Init (NULL);

	return m_pPlayer;
}

int CMediaEngine::Init (TCHAR * pPath)
{
	if (m_hDllMedia != NULL)
		return 0;

#ifdef _OS_WIN32
	m_hDllMedia = LoadLibrary (_T("yyMediaEng.Dll"));
	if (m_hDllMedia == NULL)
		return -1;
#ifdef WINCE
#define _FUNC _T
#else
#define _FUNC
#endif // WINCE
	m_fInit = 	(YYMEDIAINIT) GetProcAddress (m_hDllMedia, _FUNC("yyMediaInit"));
	m_fCreate = (YYMEDIACREATE) GetProcAddress (m_hDllMedia, _FUNC("yyMediaCreate"));
	m_fDestroy = (YYMEDIADESTROY) GetProcAddress (m_hDllMedia, _FUNC("yyMediaDestroy"));
	m_fUninit = (YYMEDIAUNINIT) GetProcAddress (m_hDllMedia, _FUNC("yyMediaUninit"));
	if (m_fInit == NULL || m_fCreate == NULL || m_fDestroy == NULL || m_fUninit == NULL)
		return -1;
#else
	char szPath[256];
	if (pPath == NULL)
	{
		memset (szPath, 0, sizeof (szPath));
		yyGetAppPath (NULL, szPath, sizeof (szPath));
		strcat (szPath, "lib/");
	}
	else
	{
		strcpy (szPath, pPath);
		strcat (szPath, "/");
	}
	
	m_hDllBase = dlopen("/data/local/tmp/yylib/libyyBaseEngn.so", RTLD_NOW);
	if (m_hDllBase == NULL)
	{
		strcat (szPath, "libyyBaseEngn.so");
		m_hDllBase = dlopen(szPath, RTLD_NOW);
		if (m_hDllBase == NULL)
		{
			YYLOGE ("The Base Dll could not be loaed!");
			return YY_ERR_FAILED;
		}
	}
	
	m_hDllMedia = dlopen("/data/local/tmp/yylib/libyyMediaEng.so", RTLD_NOW);
	if (m_hDllMedia == NULL)
	{
		if (pPath == NULL)
		{
			memset (szPath, 0, sizeof (szPath));
			yyGetAppPath (NULL, szPath, sizeof (szPath));
			strcat (szPath, "lib/");
		}
		else
		{
			strcpy (szPath, pPath);
			strcat (szPath, "/");
		}
		strcat (szPath, "libyyMediaEng.so");		
		m_hDllMedia = dlopen(szPath, RTLD_NOW);	
		if (m_hDllMedia == NULL)
		{
			YYLOGE ("The meida Dll could not be loaded! The error is %s", dlerror ());
			return YY_ERR_FAILED;
		}
	}
	m_fInit = (YYMEDIAINIT) dlsym (m_hDllMedia, ("yyMediaInit"));
	m_fCreate = (YYMEDIACREATE) dlsym (m_hDllMedia, ("yyMediaCreate"));	
	m_fDestroy = (YYMEDIADESTROY) dlsym (m_hDllMedia, ("yyMediaDestroy"));		
	m_fUninit = (YYMEDIAUNINIT) dlsym (m_hDllMedia, ("yyMediaUninit"));	
#endif // _OS_WIN32

	int nRC = m_fInit (&m_hMediaEng, 0, NULL);
	if (nRC < 0)
		return nRC;

	nRC = m_fCreate (m_hMediaEng, YYME_PLAYER, (void **)&m_pPlayer);
	if (nRC < 0)
		return nRC;

	return 0;
}

int CMediaEngine::Uninit (void)
{
	if (m_hMediaEng != NULL)
		m_fUninit (m_hMediaEng);
	m_hMediaEng = NULL;

	if (m_hDllMedia != NULL)
		yyLibFree (m_hDllMedia, 0);
	m_hDllMedia = NULL;

	if (m_hDllBase != NULL)
		yyLibFree (m_hDllBase, 0);
	m_hDllBase = NULL;
	
	return 0;
}

void CMediaEngine::SetNotify (YYMediaNotifyEvent pFunc, void * pUserData)
{
	if (m_pPlayer == NULL)
		return;
	m_pPlayer->SetNotify (m_pPlayer, pFunc, pUserData);
}

void CMediaEngine::SetView (void * hView, YYRND_TYPE nRndType)
{
	if (m_pPlayer == NULL)
		return;
	m_pPlayer->SetView (m_pPlayer, hView, nRndType);
}

int CMediaEngine::UpdateView (RECT * rcView)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	return m_pPlayer->UpdateView (m_pPlayer, rcView);
}

int CMediaEngine::Open (const TCHAR * pSource, int nFlag)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	return m_pPlayer->Open (m_pPlayer, pSource, nFlag);
}

int CMediaEngine::Close (void)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	return m_pPlayer->Close (m_pPlayer);
}

int CMediaEngine::Run (void)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	return m_pPlayer->Run (m_pPlayer);
}

int CMediaEngine::Pause (void)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	return m_pPlayer->Pause (m_pPlayer);
}

int CMediaEngine::Stop (void)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	return m_pPlayer->Stop (m_pPlayer);
}

int CMediaEngine::SetPos (int nPos)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	return m_pPlayer->SetPos (m_pPlayer, nPos);
}

int CMediaEngine::GetPos (void)
{
	if (m_pPlayer == NULL)
		return 0;
	return m_pPlayer->GetPos (m_pPlayer);
}

int CMediaEngine::GetDur (void)
{
	if (m_pPlayer == NULL)
		return 0;
	return m_pPlayer->GetDur (m_pPlayer);
}

YYPLAY_STATUS CMediaEngine::GetStatus (void)
{
	if (m_pPlayer == NULL)
		return YY_PLAY_Init;
	return m_pPlayer->GetStatus (m_pPlayer);
}

int CMediaEngine::SetVolume (int nVolume)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	return m_pPlayer->SetVolume (m_pPlayer, nVolume);
}

int CMediaEngine::GetVolume (void)
{
	if (m_pPlayer == NULL)
		return -1;
	return m_pPlayer->GetVolume (m_pPlayer);
}

void * CMediaEngine::GetThumb (const TCHAR * pFile, YYINFO_Thumbnail * pThumbInfo)
{
	if (m_pPlayer == NULL)
		return NULL;
	return m_pPlayer->GetThumb (m_pPlayer, pFile, pThumbInfo);
}

int CMediaEngine::MediaInfo (TCHAR * pInfo, int nSize)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	return m_pPlayer->MediaInfo (m_pPlayer, pInfo, nSize);
}

int CMediaEngine::SetParam (int nID, void * pParam)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	return m_pPlayer->SetParam (m_pPlayer, nID, pParam);
}

int CMediaEngine::GetParam (int nID, void * pParam)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	return m_pPlayer->GetParam (m_pPlayer, nID, pParam);
}
