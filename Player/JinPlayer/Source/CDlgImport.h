/*******************************************************************************
	File:		CDlgImport.h

	Contains:	New box dialog header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#ifndef __CDlgImport_H__
#define __CDlgImport_H__
#include "Windows.h"
#include "TCHAR.h"

#include "yyType.h"

#include "CLangText.h"
#include "CBaseKey.h"

#include "UThreadFunc.h"

class CDlgImport
{
public:
	static INT_PTR CALLBACK ImportFileDlgProc (HWND, UINT, WPARAM, LPARAM);

public:
	CDlgImport(HINSTANCE hInst, HWND hParent, CBaseKey * pKey);
	virtual ~CDlgImport(void);

	int			OpenDlg (TCHAR * pFolder);
	void		SetSource (TCHAR ** ppSource, int nCount) {m_ppSource = ppSource; m_nCount = nCount;}
	int			GetFiles (void) {return m_nAddFiles;}

protected:
	LRESULT			OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void	AddFiles (void);
	virtual void	DelFiles (void);

	static	int		CopyProc (void * pParam);
	virtual int		CopyLoop (void);

protected:
	HINSTANCE				m_hInst;
	HWND					m_hParent;
	CBaseKey *				m_pKey;
	HWND					m_hDlg;
	HWND					m_hProg;
	HWND					m_hList;
	int						m_nAddFiles;
	TCHAR					m_szFolder[1024];
	TCHAR **				m_ppSource;
	int						m_nCount;

	yyThreadHandle			m_hThread;
	bool					m_bCancel;
};
#endif //__CDlgImport_H__