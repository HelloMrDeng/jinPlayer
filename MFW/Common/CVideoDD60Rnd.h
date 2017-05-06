/*******************************************************************************
	File:		CVideoDD60Rnd.h

	Contains:	The Video DDraw CE60 render header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CVideoDD60Rnd_H__
#define __CVideoDD60Rnd_H__
#include "windows.h"
#include <ddraw_ce60.h>
#include <mmsystem.h>

#include "CBaseVideoRnd.h"
#include "CSubtitleEngine.H"
#include "CMutexLock.h"

class CVideoDD60Rnd : public CBaseVideoRnd
{
public:
	CVideoDD60Rnd(void * hInst);
	virtual ~CVideoDD60Rnd(void);

	virtual int		SetDisplay (void * hView, RECT * pRect);
	virtual int		UpdateDisp (void);
	virtual int		SetDDMode (YY_PLAY_DDMode nMode);
	virtual int		SetSubTTEng (void * pSubTTEng);
	virtual int		SetExtDDraw (void * pDDExt);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Render (YY_BUFFER * pBuff);

protected:
	virtual bool	UpdateRenderSize (void);
	bool			CreateDD (void);
	bool			ReleaseDD (void);
	int				ShowOverlay (bool bShow);

protected:
	HWND								m_hWnd;
	_WINCE_60::IDirectDraw *			m_pDDExt;
	_WINCE_60::IDirectDraw *			m_pDD;
	_WINCE_60::IDirectDrawSurface *		m_pPrmSur;
	_WINCE_60::IDirectDrawSurface *		m_pOvlSur;
	_WINCE_60::IDirectDrawSurface *		m_pMemSur;
	_WINCE_60::DDSURFACEDESC			m_ddsd;			
	_WINCE_60::DDCAPS					m_DDCaps;
	_WINCE_60::DDBLTFX					m_ddBltFX;

	_WINCE_60::DDOVERLAYFX				m_ddOverlayFX;

	YY_PLAY_DDMode						m_nDDMode;
	bool								m_bOverride;
	bool								m_bOverlayUpdate;
	bool								m_bOverlayShow;

	RECT								m_rcDest;

	CSubtitleEngine *					m_pSubTT;
	RECT								m_rcSubTT;
};

#endif // __CVideoDD60Rnd_H__
