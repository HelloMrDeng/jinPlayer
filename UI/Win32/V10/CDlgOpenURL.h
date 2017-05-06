/*******************************************************************************
	File:		CDlgOpenURL.h

	Contains:	Open URL dialog header file

	Written by:	Fenger King

	Change History (most recent first):
	2013-10-21		Fenger			Create file

*******************************************************************************/
#ifndef __CDlgOpenURL_H__
#define __CDlgOpenURL_H__

#include "yyType.h"

class CDlgOpenURL
{
public:
	static INT_PTR CALLBACK OpenURLDlgProc (HWND, UINT, WPARAM, LPARAM);

public:
	CDlgOpenURL(HINSTANCE hInst, HWND hParent);
	virtual ~CDlgOpenURL(void);

	int			OpenDlg (void);

	TCHAR *		GetURL (void) {return m_szURL;}

protected:
	void		FillList (void);
	void		SaveList (void);

protected:
	HINSTANCE				m_hInst;
	HWND					m_hParent;
	HWND					m_hDlg;

	HWND					m_hEditURL;
	HWND					m_hListURL;

	TCHAR					m_szURL[2048];
};
#endif //__CDlgOpenURL_H__