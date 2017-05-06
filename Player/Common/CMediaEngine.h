/*******************************************************************************
	File:		CMediaEngine.h

	Contains:	yy Media Engine wrap head file

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#ifndef __CMediaEngine_H__
#define __CMediaEngine_H__
#ifdef _OS_WIN32
#include "windows.h"
#include "tchar.h"
#endif // _OS_WIN32

#include "CBaseObject.h"
#include "yyMediaEng.h"

class CMediaEngine : public CBaseObject
{
public:
	CMediaEngine(void);
	virtual ~CMediaEngine(void);

	virtual int		Init (TCHAR * pPath);
	virtual int		Uninit (void);
	YYM_Player *	GetPlayer (void);

	virtual void	SetNotify (YYMediaNotifyEvent pFunc, void * pUserData);
	virtual void	SetView (void * hView, YYRND_TYPE nRndType);
	virtual int		UpdateView (RECT * rcView);

	virtual int		Open (const TCHAR * pSource, int nFlag);
	virtual int		Close (void);
	
	virtual int		Run (void);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		SetPos (int nPos);
	virtual int		GetPos (void);
	virtual int		GetDur (void);
	YYPLAY_STATUS	GetStatus (void);

	virtual int		SetVolume (int nVolume);
	virtual int		GetVolume (void);

	virtual void *	GetThumb (const TCHAR * pFile, YYINFO_Thumbnail * pThumbInfo);

	virtual int		MediaInfo (TCHAR * pInfo, int nSize);

	virtual int 	SetParam (int nID, void * pParam);
	virtual int		GetParam (int nID, void * pParam);

protected:
	YYM_Player *	m_pPlayer;

	void *			m_hDllBase;
	void *			m_hDllMedia;

	YYMEDIAINIT		m_fInit;
	YYMEDIACREATE	m_fCreate;
	YYMEDIADESTROY	m_fDestroy;
	YYMEDIAUNINIT	m_fUninit;
	void *			m_hMediaEng;

};
#endif // __CMediaEngine_H__