/*******************************************************************************
	File:		CPlaySlider.h

	Contains:	The play slider header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-09		Fenger			Create file

*******************************************************************************/
#ifndef __CPlaySlider_H__
#define __CPlaySlider_H__

#include "CPlayCtrl.h"

class CPlaySlider : public CPlayCtrl
{
public:
	CPlaySlider(HINSTANCE hInst);
	virtual ~CPlaySlider(void);

	virtual bool	Create (HWND hWnd, CBaseConfig * pCfg, char * pItemName); 
	virtual bool	OnDraw (HDC hDC, HBITMAP hBmp, LPBYTE pBuff, RECT * pRect);

	virtual bool	SetRange (int nMin, int nMax);
	virtual bool	SetPos (int nPos);
	virtual int		GetPos (void);

protected:
	virtual bool	UpdateRect (void);
	virtual void	UpdateThumbPos (void);
	virtual bool	InThumb (int nX, int nY);

	virtual LRESULT	OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT	OnMouseMove (UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	int			m_nMin;
	int			m_nMax;
	int			m_nPos;
	int			m_nTmbLeft;
	int			m_nTmbTop;
	int			m_nTmbWidth;
	int			m_nTmbHeight;

	CBmpFile *	m_pBmpPrev;
	CBmpFile *	m_pBmpNext;
	CBmpFile *	m_pBmpThumb;
	int			m_nThumbNum;
};
#endif //__CPlaySlider_H__