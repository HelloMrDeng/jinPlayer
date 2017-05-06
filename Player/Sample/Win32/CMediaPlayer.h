/*******************************************************************************
	File:		CMediaPlayer.h

	Contains:	yy Media Engine wrap head file

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#ifndef __CMediaPlayer_H__
#define __CMediaPlayer_H__
#include "windows.h"
#include "tchar.h"

#include "yyMediaEng.h"

class CMediaPlayer
{
public:
	CMediaPlayer(void);
	virtual ~CMediaPlayer(void);

	int				Init (void);
	int				Uninit (void);
	YYM_Player *	GetPlayer (void);

protected:
	YYM_Player *	m_pPlayer;

	HMODULE			m_hDll;
	YYMEDIAINIT		m_fInit;
	YYMEDIACREATE	m_fCreate;
	YYMEDIADESTROY	m_fDestroy;
	YYMEDIAUNINIT	m_fUninit;
	void *			m_hMediaEng;

};
#endif // __CMediaPlayer_H__