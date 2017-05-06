/*******************************************************************************
	File:		CDlgOpenBox.h

	Contains:	open box dialog header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#ifndef __CDlgOpenBox_H__
#define __CDlgOpenBox_H__
#include "Windows.h"
#include "TCHAR.h"

#include "CBaseKey.h"
#include "CLangText.h"

#include "yyType.h"

class CDlgOpenBox
{
public:
	static INT_PTR CALLBACK OpenBoxDlgProc (HWND, UINT, WPARAM, LPARAM);

public:
	CDlgOpenBox(HINSTANCE hInst, HWND hParent, CBaseKey * pKey);
	virtual ~CDlgOpenBox(void);

	int			OpenDlg (void);

protected:
	LRESULT		OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HINSTANCE				m_hInst;
	HWND					m_hParent;
	CBaseKey *				m_pKey;
	HWND					m_hDlg;
};
#endif //__CDlgOpenBox_H__