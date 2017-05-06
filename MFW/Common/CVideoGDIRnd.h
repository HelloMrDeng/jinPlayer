/*******************************************************************************
	File:		CVideoGDIRnd.h

	Contains:	The Video GDI render header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CVideoGDIRnd_H__
#define __CVideoGDIRnd_H__
#include "windows.h"

#include "CBaseVideoRnd.h"
#include "CSubtitleEngine.H"

class CVideoGDIRnd : public CBaseVideoRnd
{
public:
	CVideoGDIRnd(void * hInst);
	virtual ~CVideoGDIRnd(void);

	virtual int		SetDisplay (void * hView, RECT * pRect);
	virtual int		UpdateDisp (void);
	virtual int		SetSubTTEng (void * pSubTTEng);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);
	virtual int		SetZoom (RECT * pRect);

	virtual int		Render (YY_BUFFER * pBuff);

protected:
	virtual bool	UpdateRenderSize (void);
	bool			CreateResBMP (void);
	bool			ReleaseResBMP (void);
	bool			ReleaseResDC (void);

protected:
	HWND				m_hWnd;
	HDC					m_hWinDC;
	HDC					m_hMemDC;
	HBITMAP				m_hBmpVideo;
	LPBYTE				m_pBmpBuff;
	LPBYTE				m_pBmpInfo;
	int					m_nPixelBits;
	int					m_nRndStride;
	HBITMAP				m_hBmpOld;

	YY_VIDEO_BUFF		m_buffRnd;
	LPBYTE				m_pRGBBuff;

	CSubtitleEngine *	m_pSubTT;
	RECT				m_rcBmp;
};

#endif // __CVideoGDIRnd_H__
