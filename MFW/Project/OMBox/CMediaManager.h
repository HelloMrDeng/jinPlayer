/*******************************************************************************
	File:		CMediaManager.h

	Contains:	the media engine header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-24		Fenger			Create file

*******************************************************************************/

#ifndef __CMediaManager_H__
#define __CMediaManager_H__
#include "yyMediaEng.h"

#include "COMBoxMng.h"

#define	 MAX_PLAYER_NUM			9
#define	 YYMM_PLAYER_VERSION	0X1000

class CMediaManager : public CBaseObject
{
public:
	struct YYMM_PLAYER {
		YYM_Player *	pInterface;
		COMBoxMng *		pPlayer;
	};

public:
	CMediaManager(void * hInst);
	virtual ~CMediaManager(void);

	int		CreateMedia (int nType, void ** ppMedia);
	int		DestroyMedia (int nType, void * pMedia);

protected:
	void *			m_hInst;

	YYMM_PLAYER		m_mmPlayer[MAX_PLAYER_NUM];
};

#endif // __CMediaManager_H__
