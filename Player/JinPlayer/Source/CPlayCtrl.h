/*******************************************************************************
	File:		CPlayCtrl.h

	Contains:	The play base control header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-09		Fenger			Create file

*******************************************************************************/
#ifndef __CPlayCtrl_H__
#define __CPlayCtrl_H__
#include "Windows.h"

#include "CBaseObject.h"
#include "CBaseConfig.h"
#include "CBmpFile.h"

#define	YYPOS_CENTER	20000
#define	YYPOS_RIGHTB	50000

class CPlayCtrl : public CBaseObject
{
public:
	CPlayCtrl(HINSTANCE hInst);
	virtual ~CPlayCtrl(void);

	virtual bool	Create (HWND hWnd, CBaseConfig * pCfg, char * pItemName); 
	virtual bool	Show (bool bShow);
	virtual bool	Enable (bool bEnable);
	virtual bool	NeedUpdate (bool bUpdate);

	virtual bool	OnDraw (HDC hDC, HBITMAP hBmp, LPBYTE pBuff, RECT * pRect);
	virtual RECT *	GetDrawRect (void) {return &m_rcDraw;}
	virtual LRESULT	MsgProc	(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	virtual LRESULT		OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT		OnSize (UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool		UpdateView (RECT * pRect, BOOL bEraseBG);
	virtual bool		UpdateRect (void);
	virtual bool		InRect (int nX, int nY);
	virtual bool		IsCenter (int nValue);
	virtual bool		IsRightB (int nValue);
	virtual CBmpFile *	CreateBmpFile (TCHAR * pPath, char * pFile, int nNum);

protected:
	HINSTANCE	m_hInst;
	HWND		m_hWnd;

	char		m_szName[32];
	char		m_szType[32];
	int			m_nID;
	RECT		m_rcFile;
	RECT		m_rcItem;
	RECT		m_rcDraw;
	int			m_nBmpNum;
	CBmpFile *	m_pBmpFile;
	int			m_nWidth;
	int			m_nHeight;

	bool		m_bShow;
	bool		m_bEnable;
	bool		m_bUpdate;
	bool		m_bBtnDown;
	bool		m_bMusOver;

};
#endif //__CPlayCtrl_H__