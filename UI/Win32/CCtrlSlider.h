/*******************************************************************************
	File:		CCtrlSlider.h

	Contains:	The slider bar control header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-28		Fenger			Create file

*******************************************************************************/
#ifndef __CCtrlSlider_H__
#define __CCtrlSlider_H__
#include "windows.h"

#include "CCtrlBase.h"

class CCtrlSlider : public CCtrlBase
{
public:
	CCtrlSlider(TCHAR * pPath);
	virtual ~CCtrlSlider(void);

	virtual bool	Create (HWND hWnd, CBaseConfig * pCfg, char * pItemName);

	virtual bool	SetRange (int nMin, int nMax);
	virtual bool	SetPos (int nPos);
	virtual int		GetPos (void);

	virtual void	OnPaint (HDC hDC);

protected:
	virtual LRESULT	OnLButton (WPARAM wParam, LPARAM lParam, bool bDown);
	virtual LRESULT	OnMouseMove (WPARAM wParam, LPARAM lParam);

	virtual void	UpdateThumbRect (void);
	virtual bool	InThumb (int nX, int nY);

protected:
	int			m_nMin;
	int			m_nMax;
	int			m_nPos;

	CBmpFile *	m_pThumbBmp;
	int			m_nThumbNum;
	HBITMAP		m_hThumbMem;
	LPBYTE		m_pThumbBuf;
	RECT		m_rcThumb;
	int			m_nThumbW;
	int			m_nThumbH;
	int			m_nOffLeft;
	int			m_nOffRight;
};

#endif // __CCtrlSlider_H__
