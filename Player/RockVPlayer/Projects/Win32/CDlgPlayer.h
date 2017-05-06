/*******************************************************************************
	File:		CDlgPlayer.h

	Contains:	the demo UI header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-10-15		Fenger			Create file

*******************************************************************************/
#ifndef __CDlgPlayer_H__
#define __CDlgPlayer_H__

#include "CWndSlider.h"

#include "CLessonInfo.h"
#include "CMultiPlayer.h"

class CDlgPlayer
{
public:
	CDlgPlayer(void);
	virtual ~CDlgPlayer(void);

	virtual bool	Create (HINSTANCE hInst); 
	INT_PTR			MsgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	virtual int		OpenLessonFile (bool bDlgOpen);
	virtual int		UpdateLessonInfo (void);
	virtual int		UpdateLsnItemInfo (void);
	virtual void	GetTimeText (long long llTime, TCHAR * szText);

protected:
	HINSTANCE		m_hInst;
	HWND			m_hDlg;
	CWndSlider *	m_pSldPos;

	TCHAR			m_szFile[1024];
	CLessonInfo *	m_pLsnInfo;

	CMultiPlayer *	m_pPlayer;

};
#endif //__CDlgPlayer_H__
