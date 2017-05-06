/*******************************************************************************
	File:		CVideoRender.cpp

	Contains:	New box dialog implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "commctrl.h"

#include "CDlgSubTT.h"
#include "CBaseKey.h"

#include "Resource.h"

#include "USystemFunc.h"
#include "UFileFunc.h"

#pragma warning (disable : 4996)

CDlgSubTT::CDlgSubTT(HINSTANCE hInst, HWND hParent)
	: m_hInst (hInst)
	, m_hParent (hParent)
	, m_hDlg (NULL)
	, m_pLangText (NULL)
	, m_pMedia (NULL)
{
}

CDlgSubTT::~CDlgSubTT(void)
{
}

int CDlgSubTT::OpenDlg (CMediaEngine * pMedia)
{
	int nRC = DialogBoxParam (m_hInst, MAKEINTRESOURCE(IDD_DIALOG_SUBTT), m_hParent, SunttSettingDlgProc, (LPARAM)this);
	return nRC;
}

LRESULT CDlgSubTT::OnMessage (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int	wmId, wmEvent;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		RECT rcDlg;
		SetWindowLong (hWnd, GWL_USERDATA, lParam);
		m_hDlg = hWnd;
		GetClientRect (m_hDlg, &rcDlg);
		SetWindowPos (m_hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcDlg.right) / 2, 
						(GetSystemMetrics (SM_CYSCREEN) - rcDlg.bottom ) / 2, 0, 0, SWP_NOSIZE);
		return S_FALSE;
	}

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDC_BUTTON_FONT:
		{
			LOGFONT lf;
			CHOOSEFONT cf;
			cf.lStructSize = sizeof(CHOOSEFONT);
			cf.hwndOwner = m_hDlg;
			cf.hDC = NULL;
			cf.lpLogFont = &lf;
			cf.iPointSize = 0;
			cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_SCREENFONTS;
			cf.rgbColors = 0;
			cf.lCustData = 0;
			cf.lpfnHook = NULL;
			cf.lpTemplateName = NULL;
			cf.hInstance = NULL;
			cf.lpszStyle = NULL;
			cf.nFontType = 0;
			cf.nSizeMin = 0;
			cf.nSizeMax = 0;

			ChooseFont (&cf);

			break;
		}

		case IDC_BUTTON_COLOR:
		{
			static CHOOSECOLOR cc;
			static COLORREF crCustCoLors[16];
			cc.lStructSize=sizeof(CHOOSECOLOR);
			cc.hwndOwner=m_hDlg;
			cc.hInstance=NULL;
			cc.rgbResult=RGB(0x80,0x80,0x80);
			cc.lpCustColors=crCustCoLors;
			cc.Flags=CC_RGBINIT|CC_FULLOPEN;
			cc.lCustData=0;
			cc.lpfnHook=NULL;
			cc.lpTemplateName=NULL;
			ChooseColor(&cc);
			break;
		}

		case IDCANCEL:
			EndDialog(m_hDlg, LOWORD(wParam));
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return S_OK;
}

INT_PTR CALLBACK CDlgSubTT::SunttSettingDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CDlgSubTT * pDlgCopy = NULL;
	UNREFERENCED_PARAMETER(lParam);
	if (message == WM_INITDIALOG)
		pDlgCopy = (CDlgSubTT *)lParam;
	else if (hDlg != NULL)
		pDlgCopy = (CDlgSubTT *)GetWindowLong (hDlg, GWL_USERDATA);

	if (pDlgCopy != NULL)
		return (INT_PTR) pDlgCopy->OnMessage (hDlg, message, wParam, lParam);

	return (INT_PTR) FALSE;
}

