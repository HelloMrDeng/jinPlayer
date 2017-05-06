/*******************************************************************************
	File:		CWndBar.h

	Contains:	The control panel header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-22		Fenger			Create file

*******************************************************************************/
#ifndef __CWndBar_H__
#define __CWndBar_H__
#include "commctrl.h"

#include "CBaseObject.h"
#include "CMediaEngine.h"
#include "CBaseConfig.h"
#include "CCtrlBase.h"
#include "CNodeList.h"

class CWndBar : public CBaseObject
{
public:
	CWndBar(HINSTANCE hInst);
	virtual ~CWndBar(void);

	virtual void	SetEngine (CMediaEngine * pMedia) {m_pMedia = pMedia;}
	virtual bool	CreateWnd (HWND hParent, TCHAR * pCfgFile);

	virtual HWND	GetWnd (void) {return m_hWnd;}

protected:
	static LRESULT	CALLBACK WndBarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnMsg (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool	LoadConfig (TCHAR * pCfgFile);
	virtual bool	AddControl (char * pName);

protected:
	HINSTANCE		m_hInst;
	HWND			m_hParent;
	HWND			m_hWnd;
	TCHAR			m_szClass[32];

	CMediaEngine *	m_pMedia;
	CBaseConfig *	m_pConfig;
	TCHAR			m_szPath[1024];

	CObjectList<CCtrlBase>		m_lstCtrl;

};

#endif //__CWndBar_H__