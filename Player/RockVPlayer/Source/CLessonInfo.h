/*******************************************************************************
	File:		CLessonInfo.h

	Contains:	the mutex lock header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-10-24		Fenger			Create file

*******************************************************************************/
#ifndef __CLessonInfo_H__
#define __CLessonInfo_H__

#include "CBaseObject.h"
#ifdef _OS_LINUX
#include <pthread.h>
#endif // _OS_LINUX

#include "CBaseConfig.h"
#include "CNodeList.h"

class CLsnTrack
{
public:
	CLsnTrack (void) 
	{
		memset (m_szName, 0, sizeof (m_szName));
		memset (m_szFile, 0, sizeof (m_szFile));
		m_bEnable = true;
		m_nVolume = 100;
		m_nType = 0;
	}
	virtual ~CLsnTrack (void){}

public:
	TCHAR		m_szName[64];
	int			m_nType;
	TCHAR		m_szFile[256];
	bool		m_bEnable;
	int			m_nVolume;
};

class CLsnItem
{
public:
	CLsnItem (void) 
	{
		memset (m_szName, 0, sizeof (m_szName));
		m_nTrackNum = 0;
		m_ppTrack = NULL;
	}
	virtual ~CLsnItem (void)
	{
		if (m_ppTrack != NULL)
		{
			for (int i = 0; i < m_nTrackNum; i++)
				YY_DEL_P (m_ppTrack[i]);
			delete []m_ppTrack;
		}
	}

public:
	TCHAR 			m_szName[256];
	int				m_nTrackNum;
	CLsnTrack **	m_ppTrack;
};

class CLsnRepeat
{
public:
	CLsnRepeat (void) 
	{
		m_nStart = 0;
		m_nEnd = 0;
	}
	virtual ~CLsnRepeat (void){}

public:
	int		m_nStart;
	int		m_nEnd;

};

class CLessonInfo : public CBaseObject
{
public:
    CLessonInfo(void);
    virtual ~CLessonInfo(void);

	virtual bool	Open (TCHAR * pFile);
	virtual void	Close (void);

	virtual int				GetRepeatNum (void) {return m_nRptNum;}
	virtual CLsnRepeat *	GetRepeatInfo (int nIndex);
	virtual CLsnRepeat *	GetRepeat (void);
	virtual int				GetRepeatSel (void) {return m_nRptSel;}
	virtual bool			SetRepeatSel (int nSel);

	virtual int				GetItemNum (void) {return m_nItemNum;}
	virtual CLsnItem *		GetItemInfo (int nIndex);
	virtual CLsnItem *		GetItem (void);
	virtual int				GetItemSel (void) {return m_nItemSel;}
	virtual bool			SetItemSel (int nSel);

	virtual int				GetChapTime (void) {return m_nChapTime;}
	virtual TCHAR *			GetPrevFile (void) {return m_szPrevFile;}
	virtual TCHAR *			GetPath (void) {return m_szPath;}

protected:
	CBaseConfig				m_cfgFile;
	TCHAR					m_szPath[1024];
	TCHAR					m_szPrevFile[256];

	int						m_nRptNum;
	CLsnRepeat **			m_ppRepeat;
	int						m_nRptSel;

	int						m_nBeatPM;
	int						m_nBeatPC;
	int						m_nChapTime;
	int						m_nItemNum;
	CLsnItem **				m_ppItem;
	int						m_nItemSel;
};

#endif //__CLessonInfo_H__
