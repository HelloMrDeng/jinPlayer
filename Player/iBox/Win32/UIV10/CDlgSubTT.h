/*******************************************************************************
	File:		CDlgSubTT.h

	Contains:	New box dialog header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#ifndef __CDlgSubTT_H__
#define __CDlgSubTT_H__
#include "Windows.h"
#include "TCHAR.h"

#include "yyType.h"

#include "CLangText.h"
#include "CMediaEngine.h"

class CDlgSubTT
{
public:
	static INT_PTR CALLBACK SunttSettingDlgProc (HWND, UINT, WPARAM, LPARAM);

public:
	CDlgSubTT(HINSTANCE hInst, HWND hParent);
	virtual ~CDlgSubTT(void);

	void		SetLangText (CLangText * pLang) {m_pLangText = pLang;}
	int			OpenDlg (CMediaEngine * pMedia);

protected:
	LRESULT			OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HINSTANCE				m_hInst;
	HWND					m_hParent;
	HWND					m_hDlg;

	CLangText *				m_pLangText;
	CMediaEngine *			m_pMedia;
};
#endif //__CDlgSubTT_H__