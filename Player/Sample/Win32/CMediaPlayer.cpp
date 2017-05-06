/*******************************************************************************
	File:		CMediaPlayer.cpp

	Contains:	yy Media Engine wrap implement file

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#include "CMediaPlayer.h"

CMediaPlayer::CMediaPlayer(void)
	: m_pPlayer (NULL)
	, m_hDll (NULL)
	, m_fInit (NULL)
	, m_fCreate (NULL)
	, m_fDestroy (NULL)
	, m_fUninit (NULL)
	, m_hMediaEng (NULL)
{

}

CMediaPlayer::~CMediaPlayer(void)
{
	Uninit ();
}

YYM_Player * CMediaPlayer::GetPlayer (void)
{
	if (m_pPlayer == NULL)
		Init ();

	return m_pPlayer;
}

int CMediaPlayer::Init (void)
{
	if (m_hDll != NULL)
		return 0;

	m_hDll = LoadLibrary (_T("yyMediaEng.Dll"));
	if (m_hDll == NULL)
		return -1;

#ifdef WINCE
#define _FUNC _T
#else
#define _FUNC
#endif // WINCE

	m_fInit = (YYMEDIAINIT) GetProcAddress (m_hDll, _FUNC("yyMediaInit"));
	m_fCreate = (YYMEDIACREATE) GetProcAddress (m_hDll, _FUNC("yyMediaCreate"));
	m_fDestroy = (YYMEDIADESTROY) GetProcAddress (m_hDll, _FUNC("yyMediaDestroy"));
	m_fUninit = (YYMEDIAUNINIT) GetProcAddress (m_hDll, _FUNC("yyMediaUninit"));
	if (m_fInit == NULL || m_fCreate == NULL || m_fDestroy == NULL || m_fUninit == NULL)
		return -1;

	int nRC = m_fInit (&m_hMediaEng, 0, NULL);
	if (nRC < 0)
		return nRC;

	nRC = m_fCreate (m_hMediaEng, YYME_PLAYER, (void **)&m_pPlayer);
	if (nRC < 0)
		return nRC;

	return 0;
}


int CMediaPlayer::Uninit (void)
{
	if (m_hMediaEng != NULL)
		m_fUninit (m_hMediaEng);
	m_hMediaEng = NULL;

	if (m_hDll != NULL)
		FreeLibrary (m_hDll);
	m_hDll = NULL;

	return 0;
}
