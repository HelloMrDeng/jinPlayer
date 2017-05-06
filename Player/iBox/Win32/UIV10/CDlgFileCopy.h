/*******************************************************************************
	File:		CDlgFileCopy.h

	Contains:	New box dialog header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#ifndef __CDlgFileCopy_H__
#define __CDlgFileCopy_H__
#include "Windows.h"
#include "TCHAR.h"

#include "yyType.h"

#include "CLangText.h"
#include "UThreadFunc.h"

class CDlgFileCopy
{
public:
	static INT_PTR CALLBACK OpenFileCopyDlgProc (HWND, UINT, WPARAM, LPARAM);

public:
	CDlgFileCopy(HINSTANCE hInst, HWND hParent);
	virtual ~CDlgFileCopy(void);

	void		SetLangText (CLangText * pLang) {m_pLangText = pLang;}
	int			OpenDlg (TCHAR * pSource, TCHAR * pTarget);

protected:
	LRESULT			OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static	int		CopyProc (void * pParam);
	virtual int		CopyLoop (void);

protected:
	HINSTANCE				m_hInst;
	HWND					m_hParent;
	HWND					m_hDlg;
	HWND					m_hProg;

	TCHAR					m_szSource[1024];
	TCHAR					m_szTarget[1024];
	bool					m_bExport;

	yyThreadHandle			m_hThread;
	bool					m_bCancel;

	CLangText *				m_pLangText;

};
#endif //__CDlgFileCopy_H__