/*******************************************************************************
	File:		CDlgExport.h

	Contains:	New box dialog header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#ifndef __CDlgExport_H__
#define __CDlgExport_H__
#include "Windows.h"
#include "TCHAR.h"

#include "yyType.h"

#include "CLangText.h"
#include "CBaseKey.h"

#include "UThreadFunc.h"

class CDlgExport
{
public:
	static INT_PTR CALLBACK ExportFileDlgProc (HWND, UINT, WPARAM, LPARAM);

public:
	CDlgExport(HINSTANCE hInst, HWND hParent, CBaseKey * pKey);
	virtual ~CDlgExport(void);

	int			OpenDlg (TCHAR * pSource);

protected:
	LRESULT			OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static	int		CopyProc (void * pParam);
	virtual int		CopyLoop (void);

protected:
	HINSTANCE				m_hInst;
	HWND					m_hParent;
	CBaseKey *				m_pKey;
	HWND					m_hDlg;
	HWND					m_hProg;

	TCHAR					m_szSource[1024];
	TCHAR					m_szTarget[1024];

	yyThreadHandle			m_hThread;
	bool					m_bCancel;
};
#endif //__CDlgExport_H__