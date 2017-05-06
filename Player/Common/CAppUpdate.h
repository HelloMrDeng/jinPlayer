/*******************************************************************************
	File:		CAppUpdate.h

	Contains:	The ext source header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#ifndef __CAppUpdate_H__
#define __CAppUpdate_H__

#include "CBaseObject.h"
#include "CBaseConfig.h"
#include "CSourceIO.h"

#include "UFileFunc.h"
#include "UThreadFunc.h"
#include "yyData.h"

class CAppUpdate : public CBaseObject
{
public:
	static INT_PTR CALLBACK AppUpdateDlgProc (HWND, UINT, WPARAM, LPARAM);

public:
	CAppUpdate(void * hInst, HWND hParent);
	virtual ~CAppUpdate(void);

	int			OpenDlg (void);

protected:
	LRESULT			OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual bool	GetUpdateInfo (void);
	virtual bool	StartUpdate (void);

	static	int		UpdateProc (void * pParam);
	virtual int		UpdateLoop (void);

//	virtual bool	Update (void);

protected:
	void *			m_hInst;
	HWND			m_hParent;
	HWND			m_hDlg;
	HWND			m_hProg;
	CSourceIO *		m_pIO;
	CBaseConfig *	m_pCFG;

	TCHAR			m_szVer[64];

	yyThreadHandle	m_hThread;
	bool			m_bCancel;

	bool			m_bMSIFile;
	TCHAR			m_szMSIFile[1024];
};

#endif // __CAppUpdate_H__
