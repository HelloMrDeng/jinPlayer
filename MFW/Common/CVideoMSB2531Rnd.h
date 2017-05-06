/*******************************************************************************
	File:		CVideoMSB2531Rnd.h

	Contains:	The Video DDraw CE60 render header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CVideoMSB2531Rnd_H__
#define __CVideoMSB2531Rnd_H__
#include "windows.h"
#include <mmsystem.h>
#include "ceddk.h"

#include "CBaseVideoRnd.h"
#include "CSubtitleEngine.H"

class CVideoMSB2531Rnd : public CBaseVideoRnd
{
public:
	typedef enum _BufferId
	{
		TYPE422BUF0,
		TYPE422BUF1,
		TYPE420BUF0,
		TYPE420BUF1,
		BUF_NUM
	}BUFFERID;

	typedef struct {
		void*    pvDestMem;
		DWORD    dwPhysAddr;
		DWORD    dwSize;
		DWORD    dwFlag;
	} VIRTUAL_COPY_EX_DATA;

public:
	CVideoMSB2531Rnd(void * hInst);
	virtual ~CVideoMSB2531Rnd(void);

	virtual int		SetDisplay (void * hView, RECT * pRect);
	virtual int		UpdateDisp (void);
	virtual int		SetSubTTEng (void * pSubTTEng);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Render (YY_BUFFER * pBuff);

	virtual int		EnableKeyColor (bool bEnable);

protected:
	virtual bool	UpdateRenderSize (void);
	bool			CreateDevice (void);
	bool			ReleaseDevice (void);

	BOOL			SetSrcSize (void);
	BOOL			SetOutPath (void);
	BOOL			SetBuffIndex (void);
	BOOL			SetColorKey (void);
	BOOL			SetDispSize (void);

	volatile LPVOID	GetVirtualAddr(DWORD dwPhyBaseAddress, DWORD dwSize);
	volatile LPVOID GetVirtualAddr(PHYSICAL_ADDRESS pa, DWORD size, BOOL cacheEnable);

protected:
	HWND					m_hWnd;
	RECT					m_rcDraw;
	RECT					m_rcDest;
	CSubtitleEngine *		m_pSubTT;

	HANDLE					m_hAtscTv;
	HANDLE					m_hAtscEvent;

	DWORD					m_dwColorKey;
	bool					m_bColorKeyEn;
	BUFFERID				m_eBufIdx;
	bool					m_bNeedUpdateDisp;

	void *					m_pYuv422Buf0;
	void *					m_pYuv422Buf1;
	void *					m_pYuv420Buf0;
	void *					m_pYuv420Buf1;

	int						m_nVideoWidth;
	int						m_nVideoHeight;

	int						m_nYUVWidth;
	int						m_nYUVHeight;
};

#endif // __CVideoMSB2531Rnd_H__
