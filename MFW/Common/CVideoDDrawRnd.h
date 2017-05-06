/*******************************************************************************
	File:		CVideoDDrawRnd.h

	Contains:	The Video DDraw render header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CVideoDDrawRnd_H__
#define __CVideoDDrawRnd_H__
#include "windows.h"

#include "CBaseVideoRnd.h"
#include "CSubtitleEngine.H"

#include <ddraw.h>
#include <mmsystem.h>

class CVideoDDrawRnd : public CBaseVideoRnd
{
public:
	CVideoDDrawRnd(void * hInst);
	virtual ~CVideoDDrawRnd(void);

	virtual int		SetDisplay (void * hView, RECT * pRect);
	virtual int		UpdateDisp (void);
	virtual int		SetSubTTEng (void * pSubTTEng);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Render (YY_BUFFER * pBuff);

protected:
	virtual bool	RendVideo (YY_VIDEO_BUFF * pVideoBuff);
	virtual bool	UpdateRenderSize (void);
	bool			CreateDD (void);
	bool			ReleaseDD (void);

protected:
	HWND					m_hWnd;
	RECT					m_rcDest;	
	IDirectDraw7 *			m_pDD;			
	IDirectDrawSurface7 *	m_pDDSPrimary;  
	IDirectDrawSurface7 *	m_pDDSOffScr;
	DDSURFACEDESC2			m_ddsd;			
	DDCAPS					m_DDCaps;
	DDBLTFX					m_ddBltFX;
	DWORD *					m_pFourCC;
	DWORD					m_dwFourCC;

	LPBYTE					m_pYUVBuff;

	CSubtitleEngine *		m_pSubTT;
	RECT					m_rcDraw;
};

#endif // __CVideoDDrawRnd_H__
