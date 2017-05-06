/*******************************************************************************
	File:		CCtrlBase.h

	Contains:	The base control header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-27		Fenger			Create file

*******************************************************************************/
#ifndef __CCtrlBase_H__
#define __CCtrlBase_H__
#include "windows.h"

#include "CBaseObject.h"
#include "CBaseConfig.h"
#include "CBmpFile.h"

class CCtrlBase : public CBaseObject
{
public:
	CCtrlBase(TCHAR * pPath);
	virtual ~CCtrlBase(void);

	virtual bool	Create (HWND hWnd, CBaseConfig * pCfg, char * pItemName);
	
	virtual bool	Show (bool bShow);
	virtual bool	Enable (bool bEnable);

	virtual LRESULT OnMsg (UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void	OnPaint (HDC hDC);

	virtual char *	GetName (void) {return m_szName;}
	virtual char *	GetType (void) {return m_szType;}
	virtual int		GetID (void) {return m_nID;}
	virtual int		GetWidth (void) {return m_nWidth;}
	virtual int		GetHeight (void) {return m_nHeight;}
	virtual RECT *	GetRect (void) {return &m_rcPos;}

protected:
	virtual LRESULT	OnLButton (WPARAM wParam, LPARAM lParam, bool bDown);
	virtual LRESULT	OnMouseMove (WPARAM wParam, LPARAM lParam);

	virtual bool	InRect (int nX, int nY);

protected:
	HWND		m_hWnd;
	char		m_szName[32];
	char		m_szType[32];
	int			m_nID;
	RECT		m_rcPos;
	TCHAR		m_szBmpFile[1024];
	int			m_nBmpNum;
	COLORREF	m_clrTP;

	TCHAR *		m_pPath;
	CBmpFile *	m_pBmpFile;
	int			m_nWidth;
	int			m_nHeight;

	bool		m_bShow;
	bool		m_bEnable;
	bool		m_bBtnDown;
	bool		m_bMusOver;

	HDC			m_hMemDC;
	HBITMAP		m_hMemBmp;
	LPBYTE		m_pMemBuf;
};

#endif // __CCtrlBase_H__
