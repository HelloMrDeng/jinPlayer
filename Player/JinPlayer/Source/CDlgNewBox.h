/*******************************************************************************
	File:		CDlgNewBox.h

	Contains:	New box dialog header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#ifndef __CDlgNewBox_H__
#define __CDlgNewBox_H__
#include "Windows.h"
#include "TCHAR.h"

#include "CBaseKey.h"
#include "CLangText.h"

#include "yyType.h"

class CDlgNewBox
{
public:
	static INT_PTR CALLBACK OpenNewBoxDlgProc (HWND, UINT, WPARAM, LPARAM);

public:
	CDlgNewBox(HINSTANCE hInst, HWND hParent, CBaseKey * pKey);
	virtual ~CDlgNewBox(void);

	int			OpenDlg (TCHAR * pFolder, bool bNewBox);
	TCHAR *		GetFolder (void) {return m_szFolder;}
	TCHAR *		GetPW (void) {return m_szPW;}
protected:
	LRESULT		OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HINSTANCE				m_hInst;
	HWND					m_hParent;
	CBaseKey *				m_pKey;
	HWND					m_hDlg;

	bool					m_bNewBox;
	TCHAR					m_szFolder[1024];
	TCHAR					m_szPW[256];
};
#endif //__CDlgNewBox_H__