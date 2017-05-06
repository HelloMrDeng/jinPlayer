/*******************************************************************************
	File:		CJinPlayer.h

	Contains:	Jin player header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CJinPlayer_H__
#define __CJinPlayer_H__

#include "CLangText.h"
#include "CRegMng.h"
#include "CBaseKey.h"

#include "CMediaEngine.h"
#include "CExtSource.h"

#include "CWndList.h"
#include "CWndPlay.h"

#define	PLAYER_WND_LIST		1
#define	PLAYER_WND_PLAY		2

typedef int (* yyVerifyLicenseText) (void * pUserData, char * pText, int nSize);
#define	WM_YYRR_CHKLCS	WM_USER + 765

class CJinPlayer : public CBaseObject
{
public:
	CJinPlayer(HINSTANCE hInst);
	virtual ~CJinPlayer(void);

	virtual bool	Create (HWND hWnd, TCHAR * pFile); 
	virtual LRESULT	MsgProc	(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	virtual int		OpenMediaFile (void);
	virtual bool	SwitchWnd (int nType, bool BAnmt);
	virtual bool	CreateVideoBmp (void);
	virtual bool	UpdateLang (int nLang);

	static int		CheckExtLicense (char * pText, int nSize, yyVerifyLicenseText fVerify, void * pUserData);

protected:
	HINSTANCE		m_hInst;
	HWND			m_hWnd;
	HMENU			m_hMenu;
	CLangText *		m_pLang;
	CRegMng *		m_pReg;
	CBaseKey *		m_pKey;

	CMediaEngine *	m_pMedia;
	CExtSource *	m_pExtSrc;

	int				m_nWndType;
	CWndList *		m_pWndList;
	CWndPlay *		m_pWndPlay;
	HBITMAP			m_hPlayBmp;
	bool			m_bResized;
};
#endif //__CJinPlayer_H__