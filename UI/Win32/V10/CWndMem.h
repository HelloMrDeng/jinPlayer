/*******************************************************************************
	File:		CWndMem.h

	Contains:	the window mem header file

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#ifndef __CWndMem_H__
#define __CWndMem_H__

#include "CWndBase.h"

class CWndMem : public CWndBase
{
public:
	CWndMem(HINSTANCE hInst);
	virtual ~CWndMem(void);

	virtual bool	CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG);
	virtual LRESULT	OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	virtual bool	CreateBGBmp (int nType);
	virtual bool	UpdateBmpInfo (void);

protected:
	HPEN			m_hPenLine;
	HPEN			m_hPenInfo;
	HBRUSH			m_hBrushBG;

	MEMORYSTATUS	m_memInfo;

	HDC				m_hDCBmp;
	HBITMAP			m_hBmpMem;
	LPBYTE			m_pBmpMemBuff;
	HBITMAP			m_hBmpCPU;
	LPBYTE			m_pBmpCPUBuff;
	int				m_nPrevCPULoad;
	int				m_nBmpWidth;
	int				m_nBmpHeight;

	LPBYTE			m_pBmpBGData;

	int				m_nStep;

};
#endif //__CWndMem_H__