/*******************************************************************************
	File:		CExtPlayer.h

	Contains:	The ext render player header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-21		Fenger			Create file

*******************************************************************************/
#ifndef __CExtPlayer_H__
#define __CExtPlayer_H__

#include "CBaseObject.h"
#include "COMBoxMng.h"
#include "CBoxBase.h"
#include "CWaveOutRnd.h"

#include "yyData.h"

class CExtPlayer : public CBaseObject
{
public:
	CExtPlayer(void * hInst, HWND hWnd);
	virtual ~CExtPlayer(void);

	virtual int		SetPlayer (COMBoxMng * pPlayer);

	virtual int		Start (void);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		SetPos (int nPos);

	static	int		yyMediaDataCB (void * pUser, YY_BUFFER * pData);
	virtual int		RenderData (YY_BUFFER * pData);

protected:
	bool			CreateResBMP (int nW, int nH);
	bool			ReleaseResBMP (void);
	bool			ReleaseAudio (void);

protected:
	void *				m_hInst;
	HWND				m_hWnd;

	COMBoxMng *			m_pPlayer;
	YY_DATACB			m_cbData;

	YY_BUFFER 			m_bufBoxAudio;
	YY_BUFFER 			m_bufBoxVideo;

protected:
	bool				m_bStop;
	bool				m_bPause;

	static	int			PlayAudioProc (void * pParam);
	virtual int			PlayAudioLoop (void);
	HANDLE				m_hThreadAudio;

	YY_AUDIO_FORMAT		m_fmtAudio;
	YY_BUFFER 			m_bufAudioRnd;
	YY_BUFFER_CONVERT	m_dataConvertAudio;

	CWaveOutRnd *		m_pRndAudio;
	unsigned char *		m_pPCMBuff;
	int					m_nPCMSize;

	static	int			PlayVideoProc (void * pParam);
	virtual int			PlayVideoLoop (void);
	HANDLE				m_hThreadVideo;

	HDC					m_hWinDC;
	HDC					m_hMemDC;
	HBITMAP				m_hBmpVideo;
	LPBYTE				m_pBmpBuff;
	LPBYTE				m_pBmpInfo;
	int					m_nBmpW;
	int					m_nBmpH;
	HBITMAP				m_hBmpOld;

	YY_VIDEO_BUFF		m_bufVideoData;
	YY_BUFFER 			m_bufVideoDraw;
	YY_BUFFER_CONVERT	m_dataConvertVideo;
};

#endif // __CExtPlayer_H__
