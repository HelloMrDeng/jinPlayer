/*******************************************************************************
	File:		COMBoxMng.cpp

	Contains:	The media engine implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifdef _OS_WINPC
#include "windows.h"
#include "psapi.h"
#elif defined _OS_NDK
#include <sys/system_properties.h>
#endif // _OS_WINPC

#include "COMBoxMng.h"
#include "CBoxExtData.h"
#include "CBoxAudioDec.h"
#include "CBoxVideoDec.h"
#include "CBoxVDBASmt.h"
#include "CBoxAudioRndExt.h"
#include "CBoxVideoRndExt.h"

#include "CBasePDP.h"

#include "yyMetaData.h"
#include "yyBox.h"
#include "UThreadFunc.h"
#include "USystemFunc.h"
#include "UStringFunc.h"
#include "USocketFunc.h"
#include "UFileFormat.h"
#include "yyLog.h"

#ifdef _OS_WINCE
typedef HRESULT (WINAPI * DIRECTDRAWCREATE) (LPGUID lpGUID, _WINCE_60::LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter);
#endif // _OS_WICE

COMBoxMng::COMBoxMng(void * hInst, YYM_Player * pMPlayer)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_pMPlayer (pMPlayer)
	, m_byyDemoPlayer (false)
	, m_pLcsChk (NULL)
	, m_stsPlay (YY_PLAY_Init)
	// Source open
	, m_nOpenFlag (0)
	, m_nDur (0)
	, m_bOpening (false)
	, m_bOpenCancel (false)
	, m_bForceClosed (false)
	, m_bClosed (false)
	// render parameters
	, m_hView (NULL)
	, m_vdMode (YY_VDM_Auto)
#ifdef _OS_WINCE
	, m_nRndType (YY_VRND_DDCE6)
#else
	, m_nRndType (YY_VRND_DDRAW)
#endif // _OS_WINCE
	, m_bForceGDI (false)
#ifdef _CPU_MSB2531
	, m_arType (YY_ART_WAVE_FORMAT_QUERY)
#else
	, m_arType (YY_ART_WAVE_MAPPER)
#endif // 
	, m_ddMode (YY_DDM_Memory)
	, m_nDisVideoLevel (YY_PLAY_VideoEnable)
	, m_nAudioVolume (100)
	// seek parameters
	, m_nSeekPos (0)
	, m_bSeeking (false)
	, m_nLastSeekTime (0)
	, m_nSeekMode (YY_SEEK_KeyFrame)
//	, m_nSeekMode (YY_SEEK_AnyPosition)
	// box paramters
	, m_pBoxSource (NULL)
	, m_pRndAudio (NULL)
	, m_pRndVideo (NULL)
	, m_pDecVideo (NULL)
	, m_pClock (NULL)
	, m_pClockMng (NULL)
	, m_pBoxMonitor (NULL)
	, m_pThumb (NULL)
	, m_pSubTT (NULL)
	, m_nSubTTEnable (0)
	, m_nSubTTColor (0XFFFFFF)
	, m_nSubTTSize (28)
	, m_hSubTTFont (NULL)
	, m_hSubTTView (NULL)
	, m_pSubExtRnd (NULL)
	, m_pSubExtDraw (NULL)
	, m_fNotifyEvent (NULL)
	, m_pUserData (NULL)
	, m_hThreadNotifyEvent (NULL)
	, m_bStopNotifyEvent (false)
	, m_nCurEventID (0)
	, m_pWorkAudio (NULL)
	, m_pWorkVideo (NULL)
	, m_nDbgMemSize (0)
	, m_nDbgMemStep (0)
#ifdef _OS_WINCE
	, m_pDDExt (NULL)
	, m_hDDDll (NULL)
#endif // _OS_WINCE
{
	SetObjectName ("COMBoxMng");
	m_rcView.left = 0;
	m_rcView.top = 0;
	m_rcView.right = 0;
	m_rcView.bottom = 0;
	_tcscpy (m_szSource, _T(""));
	m_cbData.funcCB = NULL;
	m_cbData.userData = NULL;
	m_pLcsChk = new CLicenseCheck ();
	m_pBoxMonitor = new CBoxMonitor ();

#ifdef _OS_WINCE
	m_hDDDll = LoadLibrary (_T("ddraw.dll"));
	DIRECTDRAWCREATE  pCreate = (DIRECTDRAWCREATE)GetProcAddress (m_hDDDll, _T("DirectDrawCreate"));
	pCreate (NULL, &m_pDDExt, NULL);
#endif // _OS_WINCE
	int nID = 0;
	yyThreadCreate (&m_hThreadNotifyEvent, &nID, NotifyProc, this, 0);
}

COMBoxMng::~COMBoxMng(void)
{
	//make sure it can exit application.
	m_bOpening = false;
	m_bOpenCancel = true;

	Close ();
	CloseEvent ();

	YY_DEL_P (m_pThumb);
	YY_DEL_P (m_pSubTT);
	YY_DEL_P (m_pBoxMonitor);
	YY_DEL_P (m_pLcsChk);
	YY_DEL_P (m_pClockMng);
	YY_DEL_P (m_pWorkAudio);
	YY_DEL_P (m_pWorkVideo);

#ifdef _OS_WINCE
	YY_REL_P (m_pDDExt);
	if (m_hDDDll != NULL)
		FreeLibrary (m_hDDDll);
#endif // _OS_WINCE
}

void COMBoxMng::SetNotifyFunc (YYMediaNotifyEvent pFunc, void * pUserData)
{
	m_fNotifyEvent = pFunc;
	m_pUserData = pUserData;
}

void COMBoxMng::SetDisplay (void * hView, YYRND_TYPE nRndType)
{
	CAutoLock lock (&m_mtFunc);
#ifdef _OS_WIN32
	if (m_hView != hView)
	{
		TCHAR szWinName[256];
		GetWindowText ((HWND)hView, szWinName, sizeof (szWinName) / sizeof (TCHAR));
		if (!_tcscmp (szWinName, _T("yyDemoPlayerVideoWindow")))
			m_byyDemoPlayer = true;
		else
			m_byyDemoPlayer = false;
	}
#endif // _OS_WIN32
	m_hView = hView;
	if (!m_bForceGDI)
		m_nRndType = nRndType;
	if (m_pLcsChk != NULL && m_hView != NULL)
	{
		m_pLcsChk->SetView (m_hView);
		m_pLcsChk->CheckLicense ();
	}
	if (m_pRndVideo != NULL)
	{
		m_pRndVideo->SetYYDemoPlayer (m_byyDemoPlayer);
		m_pRndVideo->SetRndType (m_nRndType);
		if (m_rcView.right > 0 && m_rcView.bottom > 0)
			m_pRndVideo->SetDisplay (m_hView, &m_rcView);
		else
			m_pRndVideo->SetDisplay (m_hView, NULL);
	}
	if (m_pDecVideo != NULL)
		m_pDecVideo->SetDisplay (m_hView, &m_rcView);
}

int COMBoxMng::UpdateView (RECT * rcView)
{
	CAutoLock lock (&m_mtFunc);
	if (rcView != NULL)
		memcpy (&m_rcView, rcView, sizeof (RECT));
	if (m_pRndVideo != NULL)
	{
		if (rcView == NULL)
			return m_pRndVideo->UpdateDisp ();
		else
			return m_pRndVideo->SetDisplay (m_hView, rcView);
	}
	return YY_ERR_FAILED;
}

int COMBoxMng::Open (const TCHAR * pSource, int nFlag)
{
	if (CheckOpenStatus (200) != YY_ERR_NONE)
	{
		if (m_bOpening || m_bSeeking)
			m_bOpenCancel = true;
		YYLOGI ("Open failed for error status!");
		return YY_ERR_STATUS;
	}
	if (Close () < 0)
		return YY_ERR_STATUS;

	CAutoLock lock (&m_mtFunc);
	m_bOpenCancel = false;
	m_bClosed = false;

//	int nRC = DoOpen (pSource, nFlag);
//	PushTask (YY_EV_Open_Complete, NULL);
//	return nRC;

	CAutoLock lockEvent (&m_mtEvent);
	YY_EVENT * pTask = NULL;
	NODEPOS pos = m_lstFullEvent.GetHeadPosition ();
	pTask = m_lstFullEvent.GetNext (pos);
	while (pTask != NULL)
	{
		if (pTask->nID == YY_TASK_OPEN)
		{
			YYLOGI ("The previous open task was not finished!");
			return YY_ERR_STATUS;
		}
		else if (pTask->nID == YY_TASK_SEEK)
		{
			YYLOGI ("The previous seek task was not finished!");
			return YY_ERR_STATUS;
		}
		pTask = m_lstFullEvent.GetNext (pos);
	}

	m_nOpenFlag = nFlag;
	m_bOpening = true;
	if ((m_nOpenFlag & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ)
	{
		YY_READ_EXT_DATA * pExt = (YY_READ_EXT_DATA *)pSource;
		if (pExt != NULL && pExt->pSource != NULL)
			_tcscpy (m_szSource, pExt->pSource);
	}
	else
	{
		_tcscpy (m_szSource, pSource);
	}

	TCHAR * pLast = NULL;
	while (_tcslen (m_szSource) > 0)
	{
		pLast = m_szSource + _tcslen (m_szSource) - 1;
		if (*pLast == _T(' ') || *pLast == _T('\r') ||  *pLast == _T('\n'))
			*pLast = 0;
		else
			break;
	}
	PushTask (YY_TASK_OPEN, (void *)pSource);
	return YY_ERR_NONE;
}

int COMBoxMng::Close (void)
{	
	PrepareClose ();
	if (CheckOpenStatus (500) != YY_ERR_NONE)
	{
		YYLOGI ("Try to close failed for the status was error!");
		return YY_ERR_STATUS;
	}

	CAutoLock lock (&m_mtFunc);
	Stop ();
	m_bClosed = true;
	CBoxBase * pBoxIn = NULL;
	CBoxBase * pBox = NULL;
	pBox = m_pRndVideo;
	while (pBox != NULL)
	{
		pBoxIn = pBox->GetSource ();
		if (pBox != m_pBoxSource)
			delete pBox;
		pBox = pBoxIn;
	}
	m_pRndVideo = NULL;
	pBox = m_pRndAudio;
	while (pBox != NULL)
	{
		pBoxIn = pBox->GetSource ();
		if (pBox != m_pBoxSource)
			delete pBox;
		pBox = pBoxIn;
	}
	m_pRndAudio = NULL;
#ifndef _OS_WINCE
	if (m_pBoxSource != NULL)
		_tcscpy (CBasePDP::g_szPDPFile, _T(""));
#endif _OS_WINCE
	YY_DEL_P (m_pBoxSource);
	m_lstBox.RemoveAll ();
	m_pDecVideo = NULL;
	m_pClock = NULL;

	if (m_pSubTT != NULL)
		m_pSubTT->Close ();
	if (m_pBoxMonitor != NULL)
		m_pBoxMonitor->ReleaseItems ();

	m_nSeekPos = 0;
	m_stsPlay = YY_PLAY_Init;
	return YY_ERR_NONE;
}

void COMBoxMng::PrepareClose (void)
{
	if (m_pThumb != NULL)
		m_pThumb->Cancel ();
	if (m_pBoxSource != NULL)
	{
		if (m_bOpening || m_bSeeking)
		{
			if (!m_bForceClosed)
				YYLOGW ("Try to close when OPEN %d or SEEK %d !", m_bOpening, m_bSeeking);
			m_pBoxSource->Close ();
			m_bForceClosed = true;
		}
	}
}

int COMBoxMng::CheckOpenStatus (int nWaitTime)
{
	int nStartTime = yyGetSysTime ();
	while (m_bOpening || (m_nCurEventID > 0 && m_nCurEventID != YY_EV_Play_Status))
	{
		yySleep (2000);
		if (yyGetSysTime () > nWaitTime)
			break;
	}
	if (m_bOpening || (m_nCurEventID > 0 && m_nCurEventID != YY_EV_Play_Status))
	{
		YYLOGW ("CheckOpenStatus failed!");
		return YY_ERR_STATUS;
	}
	return YY_ERR_NONE;
}

int COMBoxMng::DoOpen (const TCHAR * pSource, int nFlag)
{
	CAutoLock lock (&m_mtFunc);
	m_bForceClosed = false;
	m_nDur = 0;

	if (nFlag == 0)
	{
#ifdef _OS_WIN32
		char szSource[2048];
		memset (szSource, 0, sizeof (szSource));
		WideCharToMultiByte (CP_ACP, 0, pSource, -1, szSource, sizeof (szSource), NULL, NULL);
		YYLOGI ("Source: %s", szSource);
#else
		YYLOGI ("Source: %s", pSource);
#endif // _OS_WIN32
	}
	int nRC = YY_ERR_NONE;
	if ((nFlag & YY_OPEN_SRC_BOX) == YY_OPEN_SRC_BOX)
		m_pBoxSource = new CBoxExtData (m_hInst);
	else
		m_pBoxSource = new CBoxSource (m_hInst);
	if (m_pBoxSource == NULL)
	{
		YYLOGE ("m_pBoxSource is NULL!");
		return YY_ERR_MEMORY;
	}
	m_pBoxSource->EnableSubTT (m_nSubTTEnable > 0);
	m_pBoxSource->SetNotifyFunc (NotifyEvent, this);
	m_lstBox.AddTail (m_pBoxSource);
	nRC = m_pBoxSource->OpenSource (pSource, nFlag);
	if (nRC != YY_ERR_NONE)
	{
		YYLOGE ("Open Source failed!");
		return nRC;
	}
	if (m_pBoxSource->GetStreamCount (YY_MEDIA_Audio) <= 0 &&
		m_pBoxSource->GetStreamCount (YY_MEDIA_Video) <= 0)
	{
		YYLOGE ("It was no audio and video!");
		return YY_ERR_SOURCE;
	}
	if (m_bOpenCancel)
		return YY_ERR_FAILED;

	int nVideo = YY_ERR_NONE;
	if (m_pBoxSource->GetStreamCount (YY_MEDIA_Video) > 0)
	{
#ifdef _CPU_MSB2531
		YY_VIDEO_FORMAT * pVideoFmt = m_pBoxSource->GetVideoFormat ();
		if (pVideoFmt != NULL)
		{
			if (pVideoFmt->nWidth > 1280 || pVideoFmt->nHeight > 720)
				return YY_ERR_SOURCE;
		}
#endif // _CPU_MSB2531
#ifdef _OS_NDK
		if ((m_vdMode & YY_VDM_MediaCodec) == YY_VDM_MediaCodec)	
		{
			YY_VIDEO_FORMAT * pFmt = m_pBoxSource->GetVideoFormat ();
			if (pFmt == NULL)
				return YY_ERR_VIDEO;
			if (pFmt->nCodecID == AV_CODEC_ID_H264)
			{
				char	szVer[64];	
				__system_property_get ("ro.build.version.release", szVer);	
				if (szVer[0] >= '4' && szVer[2] >= '1')
				{
					if (m_fNotifyEvent != NULL)
						m_fNotifyEvent (m_pUserData, YY_EV_Create_ExtVD, NULL);
					m_pRndVideo = new CBoxExtRnd (m_hInst);
					if (m_pRndVideo != NULL)
					{
						m_pRndVideo->SetNotifyFunc (NotifyEvent, this);
						m_lstBox.AddTail (m_pRndVideo);
						m_pRndVideo->SetYYDemoPlayer (m_byyDemoPlayer);
						m_pRndVideo->SetSource (m_pBoxSource);
					}
				}
			}
		}
#endif // _OS_NDK

		if (m_pRndVideo == NULL)
		{
			CBoxVideoDec * pVideoDec = NULL;
#ifdef _OS_NDK
			if (0)//yyffIsStreaming (pSource, nFlag))
				pVideoDec = new CBoxVDBASmt (m_hInst);
			else
#endif // _OS_NDK
				pVideoDec = new CBoxVideoDec (m_hInst);
			if (pVideoDec == NULL)
				return YY_ERR_MEMORY;
			m_pDecVideo = pVideoDec;
				m_pDecVideo->SetDecMode (m_vdMode);
			if (m_rcView.right > 0 && m_rcView.bottom > 0)
				m_pDecVideo->SetDisplay (m_hView, &m_rcView);
			else
				m_pDecVideo->SetDisplay (m_hView, NULL);
			pVideoDec->SetNotifyFunc (NotifyEvent, this);
			m_lstBox.AddTail (pVideoDec);
			nVideo = pVideoDec->SetSource (m_pBoxSource);

			if (m_bOpenCancel)
				return YY_ERR_FAILED;
			if (nVideo == YY_ERR_NONE)
			{
				if ((m_nOpenFlag & YY_OPEN_RNDV_EXT) == YY_OPEN_RNDV_EXT)
					m_pRndVideo = new CBoxVideoRndExt (m_hInst);
				else
					m_pRndVideo = new CBoxVideoRnd (m_hInst);
				if (m_pRndVideo == NULL)
					return YY_ERR_MEMORY;
				if ((m_nOpenFlag & YY_OPEN_RNDV_CB) == YY_OPEN_RNDV_CB)
					m_pRndVideo->SetDataCB (&m_cbData);
				m_pRndVideo->SetNotifyFunc (NotifyEvent, this);
				m_lstBox.AddTail (m_pRndVideo);
				m_pRndVideo->SetYYDemoPlayer (m_byyDemoPlayer);
#ifdef _OS_WINCE
				m_pRndVideo->SetExtDDraw (m_pDDExt);
#endif // _OS_WINCE
				m_pRndVideo->SetRndType (m_nRndType);
				if (m_rcView.right > 0 && m_rcView.bottom > 0)
					m_pRndVideo->SetDisplay (m_hView, &m_rcView);
				else
					m_pRndVideo->SetDisplay (m_hView, NULL);
				m_pRndVideo->SetDDMode (m_ddMode);
				m_pRndVideo->SetSeekMode (m_nSeekMode);
				m_pRndVideo->DisableVideo (m_nDisVideoLevel);
				nVideo = m_pRndVideo->SetSource (pVideoDec);
				if (nVideo != YY_ERR_NONE && m_nRndType == YY_VRND_DDRAW)
				{
					m_nRndType = YY_VRND_GDI;
					m_bForceGDI = true;
					m_pRndVideo->SetRndType (m_nRndType);
				}
			}
		}
	}

	if (m_bOpenCancel)
		return YY_ERR_FAILED;

	int nAudio = YY_ERR_NONE;
	if (m_pBoxSource->GetStreamCount (YY_MEDIA_Audio) > 0)
	{
		CBoxAudioDec * pAudioDec = new CBoxAudioDec (m_hInst);
		if (pAudioDec == NULL)
			return YY_ERR_MEMORY;
		pAudioDec->SetNotifyFunc (NotifyEvent, this);
		m_lstBox.AddTail (pAudioDec);
		nAudio = pAudioDec->SetSource (m_pBoxSource);

		if (m_bOpenCancel)
			return YY_ERR_FAILED;

		if (nAudio == YY_ERR_NONE)
		{
			if ((m_nOpenFlag & YY_OPEN_RNDA_EXT) == YY_OPEN_RNDA_EXT)
				m_pRndAudio = new CBoxAudioRndExt (m_hInst);
			else
				m_pRndAudio = new CBoxAudioRnd (m_hInst);
			if (m_pRndAudio == NULL)
				return YY_ERR_MEMORY;		
			if ((m_nOpenFlag & YY_OPEN_RNDA_CB) == YY_OPEN_RNDA_CB)
				m_pRndAudio->SetDataCB (&m_cbData);
			m_pRndAudio->SetNotifyFunc (NotifyEvent, this);
			m_lstBox.AddTail (m_pRndAudio);
			m_pRndAudio->SetOtherRender (m_pRndVideo);
			if (m_pRndVideo != NULL)
				m_pRndVideo->SetOtherRender (m_pRndAudio);
			m_pRndAudio->SetAudioRndType (m_arType);
			nAudio = m_pRndAudio->SetSource (pAudioDec);
			m_pRndAudio->SetVolume (m_nAudioVolume);
			m_pRndAudio->SetSeekMode (m_nSeekMode);
		}
	}

	if (m_pRndAudio == NULL && m_pRndVideo == NULL)
	{
		YYLOGE ("it was error both audio and video dec!");
		return YY_ERR_FAILED;
	}

	YY_DEL_P (m_pClockMng);
	if (m_pRndAudio != NULL)
		m_pClock = m_pRndAudio->GetClock ();
	else if (m_pRndVideo != NULL)
		m_pClock = m_pRndVideo->GetClock ();
	if (m_pClock == NULL)
	{
		m_pClockMng = new CBaseClock ();
		m_pClock = m_pClockMng;
	}

	CBoxBase * pBox = NULL;
	NODEPOS pos = m_lstBox.GetHeadPosition ();
	while (pos != NULL)
	{
		pBox = m_lstBox.GetNext (pos);
		if (pBox != NULL)
			pBox->SetClock (m_pClock);
	}
	if (m_pBoxMonitor != NULL)
		m_pBoxMonitor->SetClock (m_pClock);

	if (m_bOpenCancel)
		return YY_ERR_FAILED;

	if (m_nSubTTEnable > 0)
		OpenSubTitle ();

	m_nDur = m_pBoxSource->GetDuration ();
	if (m_nDur == 0)
		m_nDur = -1;
	m_stsPlay = YY_PLAY_Open;

//	YYLOGI ("Open source successed!");
	if (m_pMPlayer != NULL)
	{
		TCHAR	szInfo[2048];
		TCHAR * pInfo = NULL;
		TCHAR * pNewLine = NULL;
		memset (szInfo, 0, sizeof (szInfo));
		GetMediaInfo (szInfo, sizeof (szInfo));
		pInfo = _tcsstr (szInfo, _T("Video Codec:"));
		if (pInfo != NULL)
		{
			pNewLine = _tcsstr (pInfo, _T("\r\n"));
			pInfo = pNewLine;
			while (*pInfo != _T(' '))
				pInfo--;
			memset (m_pMPlayer->sInfo.szVideoCodec, 0, sizeof (m_pMPlayer->sInfo.szVideoCodec));
			int nLen = sizeof (m_pMPlayer->sInfo.szVideoCodec) / sizeof (TCHAR) - 1;
			if (nLen > pNewLine - pInfo)
				nLen = pNewLine - pInfo;
			_tcsncpy (m_pMPlayer->sInfo.szVideoCodec, pInfo, nLen);
		}
		pInfo = _tcsstr (szInfo, _T("Video Size:"));
		if (pInfo != NULL)
			_stscanf (pInfo, _T("Video Size: %d X %d"), &m_pMPlayer->sInfo.nWidth, &m_pMPlayer->sInfo.nHeight);
	
		pInfo = _tcsstr (szInfo, _T("Audio Codec:"));
		if (pInfo != NULL)
		{
			pNewLine = _tcsstr (pInfo, _T("\r\n"));
			pInfo = pNewLine;
			while (*pInfo != _T(' '))
				pInfo--;
			memset (m_pMPlayer->sInfo.szAudioCodec, 0, sizeof (m_pMPlayer->sInfo.szAudioCodec));
			int nLen = sizeof (m_pMPlayer->sInfo.szAudioCodec) / sizeof (TCHAR) - 1;
			if (nLen > pNewLine - pInfo)
				nLen = pNewLine - pInfo;
			_tcsncpy (m_pMPlayer->sInfo.szAudioCodec, pInfo, nLen);
		}
		pInfo = _tcsstr (szInfo, _T("Audio Format:"));
		if (pInfo != NULL)
			_stscanf (pInfo, _T("Audio Format: %d  %d"), &m_pMPlayer->sInfo.nSampleRate, &m_pMPlayer->sInfo.nChannels);
	}

	return YY_ERR_NONE;
}

int COMBoxMng::OpenSubTitle (void)
{
	int nRC = -1;
	if (_tcslen (m_szSource) <= 0)
		return -1;

	CAutoLock lock (&m_mtFunc);
	if (m_pSubTT != NULL)
	{
		m_pSubTT->Close ();
		delete m_pSubTT;
	}
	m_pSubTT = new CSubtitleEngine (m_hInst);
	if (m_pSubTT == NULL)
		return YY_ERR_MEMORY;
	m_pSubTT->SetSource (m_pBoxSource->GetMediaSource ());
	nRC = m_pSubTT->Open (m_szSource);
	if (nRC == YY_ERR_NONE)
	{
#ifdef _OS_WINCE
		SetParam (YYPLAY_PID_DDMode, (void *)YY_DDM_Overlay);
#endif // WINCE
		m_pSubTT->SetClock (m_pClock);
		m_pSubTT->Enable (m_nSubTTEnable);
		m_pSubTT->SetFontColor (m_nSubTTColor);
		m_pSubTT->SetFontSize (m_nSubTTSize);
		m_pSubTT->SetFontHandle (m_hSubTTFont);
		m_pSubTT->SetView (m_hSubTTView);
		m_pSubTT->SetExtRnd (m_pSubExtRnd);
		m_pSubTT->SetExtDraw (m_pSubExtDraw);
	}
	else
	{
		m_pSubTT->Close ();
		delete m_pSubTT;
		m_pSubTT = NULL;
	}
	if (m_pSubTT != NULL && m_pRndVideo != NULL && m_hSubTTView == NULL)
		m_pRndVideo->SetSubTTEng (m_pSubTT);

	return nRC;
}

int	COMBoxMng::Start (void)
{
	if (m_stsPlay <= YY_PLAY_Init)
		return YY_ERR_STATUS;

	if (m_bOpening || m_bOpenCancel)
		return YY_ERR_STATUS;

	CAutoLock lock (&m_mtFunc);
	if (m_pRndAudio != NULL)
	{
#ifdef _OS_WIN32
		if (m_pWorkAudio == NULL && ((m_nOpenFlag & YY_OPEN_RNDA_EXT) == 0))
		{
			m_pWorkAudio = new CThreadWork (AudioPlayProc, this);
			m_pWorkAudio->SetPriority (YY_THREAD_PRIORITY_ABOVE_NORMAL);
		}
#endif // _OS_WIN32
		m_pRndAudio->Start (m_pWorkAudio);
		if (m_pWorkAudio != NULL)
			m_pWorkAudio->Start ();
	}

	if (m_pRndVideo != NULL)
	{
		if (m_pRndVideo->GetRndCount () >= 1)
			WaitAudioRender (3000, false);
#ifdef _OS_WIN32
		if (m_pWorkVideo == NULL && ((m_nOpenFlag & YY_OPEN_RNDV_EXT) == 0))
			m_pWorkVideo = new CThreadWork (VideoPlayProc, this);
#endif // _OS_WIN32
		m_pRndVideo->Start (m_pWorkVideo);
		if (m_pWorkVideo != NULL)
			m_pWorkVideo->Start ();
	}

	if (m_pSubTT != NULL)
		m_pSubTT->Start ();
	if (m_pClockMng != NULL)
		m_pClockMng->Start ();

	m_stsPlay = YY_PLAY_Run;
	PushTask (YY_EV_Play_Status, (void *)m_stsPlay);

	return YY_ERR_NONE;
}

int COMBoxMng::Pause (void)
{
//	if (CheckOpenStatus (200) != YY_ERR_NONE)
//		return YY_ERR_STATUS;
	if (m_stsPlay <= YY_PLAY_Open)
		return YY_ERR_STATUS;

	CAutoLock lock (&m_mtFunc);
	if (m_pSubTT != NULL)
		m_pSubTT->Pause ();

	if (m_pRndVideo != NULL)
		m_pRndVideo->Pause ();

	if (m_pWorkVideo != NULL)
		m_pWorkVideo->Pause ();
	if (m_pWorkAudio != NULL)
		m_pWorkAudio->Pause ();

	if (m_pRndAudio != NULL)
		m_pRndAudio->Pause ();

	if (m_pClockMng != NULL)
		m_pClockMng->Pause ();

	m_stsPlay = YY_PLAY_Pause;
	PushTask (YY_EV_Play_Status, (void *)m_stsPlay);

	return YY_ERR_NONE;
}

int	COMBoxMng::Stop (void)
{
	if (m_stsPlay == YY_PLAY_Init)
		return YY_ERR_NONE;

	PrepareClose ();
	if (CheckOpenStatus (500) != YY_ERR_NONE)
		return YY_ERR_STATUS;

	CAutoLock lock (&m_mtFunc);
	if (m_pSubTT != NULL)
		m_pSubTT->Stop ();

	if (m_pRndVideo != NULL)
		m_pRndVideo->Stop ();

	if (m_pWorkAudio != NULL)
		m_pWorkAudio->Pause ();
	if (m_pWorkVideo != NULL)
		m_pWorkVideo->Pause ();

	if (m_pRndAudio != NULL)
		m_pRndAudio->Stop ();

	if (m_stsPlay == YY_PLAY_Run || m_stsPlay == YY_PLAY_Pause)
	{
		if (m_pBoxMonitor != NULL)
			m_pBoxMonitor->ShowResult ();
	}

	m_stsPlay = YY_PLAY_Stop;
	PushTask (YY_EV_Play_Status, (void *)m_stsPlay);

	return YY_ERR_NONE;
}

int COMBoxMng::SetPos (int nPos)
{
	if (m_stsPlay <= YY_PLAY_Init)
		return YY_ERR_STATUS;
	if (m_nDur <= 0)
		return YY_ERR_STATUS;
	if (nPos + 1000 > GetDuration ())
		nPos = GetDuration () - 1000;
	if (nPos < 0)
		nPos = 0;
	if (m_nSeekPos != 0 && nPos != 0)
	{
		if (m_pClock != NULL && m_pClock->GetTime () > m_nSeekPos + 1000)
			m_nSeekPos = 0;
		if (abs (m_nSeekPos - nPos) < 1000)
			return YY_ERR_IMPLEMENT;
	}
	if (yyGetSysTime () - m_nLastSeekTime < 200)
		return YY_ERR_IMPLEMENT;
	m_nLastSeekTime = yyGetSysTime ();

	if (m_stsPlay == YY_PLAY_Open)
	{
		m_nSeekPos = nPos;
		m_pBoxSource->SetPos (nPos, true);
		return YY_ERR_NONE;
	}

	CAutoLock lockEvent (&m_mtEvent);
	YY_EVENT * pTask = NULL;
	NODEPOS pos = m_lstFullEvent.GetHeadPosition ();
	pTask = m_lstFullEvent.GetNext (pos);
	while (pTask != NULL)
	{
		if (pTask->nID == YY_TASK_SEEK)
			return YY_ERR_STATUS;
		pTask = m_lstFullEvent.GetNext (pos);
	}

	PushTask (YY_TASK_SEEK, (void *)nPos);
	
	m_nSeekPos = nPos;
	m_bSeeking = true;

	return YY_ERR_NONE;
}

int COMBoxMng::DoSeek (const int nPos)
{
	CAutoLock lock (&m_mtFunc);
	if (m_stsPlay < YY_PLAY_Open)
		return YY_ERR_STATUS;
	if (m_pBoxSource == NULL)
		return YY_ERR_STATUS;
	YYLOGI ("Set Pos % 8d", nPos);

	if (m_stsPlay == YY_PLAY_Run)
	{
		if (m_pRndVideo != NULL)
			m_pRndVideo->Pause ();

		if (m_pWorkVideo != NULL)
			m_pWorkVideo->Pause ();
		if (m_pWorkAudio != NULL)
			m_pWorkAudio->Pause ();

		if (m_pRndAudio != NULL)
			m_pRndAudio->Pause ();
		if (m_pSubTT != NULL)
			m_pSubTT->Pause ();
	}

	if (m_pRndAudio != NULL)
		m_pRndAudio->SetPos (nPos, false);
	if (m_pRndVideo != NULL)
		m_pRndVideo->SetPos (nPos, false);
	if (m_pSubTT != NULL)
		m_pSubTT->SetPos (nPos);
	if (m_pClock != NULL)
		m_pClock->SetTime (nPos);
	m_pBoxSource->SetPos (nPos, true);

	int		nStartTime = yyGetSysTime ();
	bool	bSeekFailed = false;
	if (m_stsPlay == YY_PLAY_Run)
	{
		if (m_pRndAudio != NULL)
		{
			if (m_pWorkAudio != NULL)
				m_pWorkAudio->Start ();
			m_pRndAudio->Start (m_pWorkAudio);
			if (m_pRndVideo != NULL)
				DrawFirstVideoFrame (1000);
		}
		
		if (m_pRndVideo != NULL)
		{
			if (WaitAudioRender (3000, true)== YY_ERR_TIMEOUT)
				bSeekFailed = true;
			if (m_pWorkVideo != NULL)
				m_pWorkVideo->Start ();
			m_pRndVideo->Start (m_pWorkVideo);
		}
		
		if (m_pSubTT != NULL)
			m_pSubTT->Start ();
			
	}
	else if (m_stsPlay == YY_PLAY_Pause)
	{
		if (m_pRndVideo != NULL && m_pRndVideo->GetType () == OMB_TYPE_RENDER)
		{
			m_pBoxSource->Start ();
			if (DrawFirstVideoFrame (1000) == YY_ERR_FAILED)
				bSeekFailed = true;
			m_pBoxSource->Pause ();
		}
	}
	
	if (bSeekFailed)
		return YY_ERR_FAILED;

	return YY_ERR_NONE;
}

int COMBoxMng::GetPos (void)
{
	if (m_bOpening || m_bOpenCancel)
		return 0;

	if (m_stsPlay == YY_PLAY_Run || m_stsPlay == YY_PLAY_Pause)
	{
		if (m_bSeeking)
			return m_nSeekPos;

		if (m_pClock != NULL)
			return (int)m_pClock->GetTime ();
	}

	return 0;
}

int COMBoxMng::GetDuration (void)
{
	if (m_bOpening || m_bOpenCancel)
		return 0;

//	CAutoLock lock (&m_mtFunc);
	int nDur = 0;
	if (m_pBoxSource != NULL)
		nDur = m_pBoxSource->GetDuration ();

//	YYLOGI ("Duration is % 8d", nDur);
	
	return nDur;
}

int COMBoxMng::SetVolume (int nVolume)
{
	m_nAudioVolume = nVolume;

	CAutoLock lock (&m_mtFunc);
	if (m_pRndAudio == NULL)
		return YY_ERR_STATUS;

	m_pRndAudio->SetVolume (nVolume);

	return YY_ERR_NONE;
}

int COMBoxMng::GetVolume (void)
{
	CAutoLock lock (&m_mtFunc);
	if (m_pRndAudio != NULL)
		return m_pRndAudio->GetVolume ();

	return m_nAudioVolume;
}

void * COMBoxMng::GetThumb (const TCHAR * pFile, YYINFO_Thumbnail * pThumbInfo)
{
	CAutoLock lock (&m_mtFunc);
	
	if (m_pThumb == NULL)
		m_pThumb = new CMediaThumb (m_hInst);
	if (m_pThumb == NULL)
		return NULL;

	return m_pThumb->GetThumb (pFile, pThumbInfo);
}

int COMBoxMng::GetMediaInfo (TCHAR * pInfo, int nSize)
{
	CAutoLock lock (&m_mtFunc);

	if (m_pBoxSource != NULL)
		 return m_pBoxSource->GetMediaInfo (pInfo, nSize);

	return YY_ERR_STATUS;
}

int COMBoxMng::GetBoxCount (void)
{
	return m_lstBox.GetCount ();
}

CBoxBase * COMBoxMng::GetBox (int nIndex)
{
	CAutoLock lock (&m_mtFunc);
	if (nIndex < 0 || nIndex >= m_lstBox.GetCount ())
		return NULL;

	CBoxBase *	pBox = NULL;
	int			nListIndex = 0;
	NODEPOS pos = m_lstBox.GetHeadPosition ();
	while (pos != NULL)
	{
		pBox = m_lstBox.GetNext (pos);
		if (nIndex == nListIndex)
			break;
		nListIndex++;
	}

	return pBox;
}

int COMBoxMng::SetParam (int nID, void * pParam)
{
	if (nID == YYPLAY_PID_Cancel_GetThumb)
	{
		if (m_pThumb != NULL)
			m_pThumb->Cancel ();
		return YY_ERR_NONE;
	}
#ifndef _OS_WINCE
	else if (nID == YYPLAY_PID_PDPFile)
	{
		_tcscpy (CBasePDP::g_szPDPFile, (TCHAR *)pParam);
		return YY_ERR_NONE;
	}
#endif // _OS_WINCE
	else if (nID == YYPLAY_PID_Prepare_Close)
	{
		PrepareClose ();
		return YY_ERR_NONE;
	}


	CAutoLock lock (&m_mtFunc);
	YYPLAY_STATUS stsOld = m_stsPlay;

	switch (nID)
	{
	case YYPLAY_PID_AspectRatio:
	{
		if (pParam == NULL)
			return YY_ERR_ARG;
		YYPLAY_ARInfo * pAR = (YYPLAY_ARInfo *)pParam;
		if (m_pRndVideo != NULL)
			m_pRndVideo->SetAspectRatio (pAR->nWidth, pAR->nHeight);
		return YY_ERR_NONE;
	}

	case YYPLAY_PID_VideoZoomIn:
		if (m_pRndVideo == NULL)
			return YY_ERR_STATUS;
		return m_pRndVideo->SetZoom ((RECT *)pParam);

	case YYPLAY_PID_Disable_Video:
	{
		m_nDisVideoLevel = (int)pParam;
		if (m_pRndVideo != NULL)
			m_pRndVideo->DisableVideo (m_nDisVideoLevel);

		return YY_ERR_NONE;
	}

	case YYPLAY_PID_Speed:
	{
		if (pParam == NULL)
			return YY_ERR_ARG;
		float fSpeed = *(float *)pParam;
		if (m_pRndAudio != NULL)
			m_pRndAudio->SetSpeed (fSpeed);
		if (m_pClock != NULL)
			m_pClock->SetSpeed (fSpeed);
		return YY_ERR_NONE;
	}

	case YYPLAY_PID_Aduio_RndType:
		m_arType = (YY_PLAY_ARType)(int) pParam;
		if (m_pRndAudio != NULL)
			m_pRndAudio->SetAudioRndType (m_arType);
		return YY_ERR_NONE;

	case YYPLAY_PID_DDMode:
		m_ddMode = (YY_PLAY_DDMode)(int) pParam;
		if (m_ddMode == YY_DDM_Memory && m_pSubTT != NULL)
			return YY_ERR_NONE;
		if (m_pRndVideo != NULL)
			m_pRndVideo->SetDDMode (m_ddMode);
		return YY_ERR_NONE;

	case YYPLAY_PID_Rotate:
		if (m_pRndVideo != NULL)
			m_pRndVideo->SetRotate ((int)pParam);
		return YY_ERR_NONE;

	case YYPLAY_PID_SeekMode:
		m_nSeekMode = (YY_PLAY_SeekMode)(int) pParam;
		if (m_pRndAudio != NULL)
			m_pRndAudio->SetSeekMode (m_nSeekMode);
		if (m_pRndVideo != NULL)
			m_pRndVideo->SetSeekMode (m_nSeekMode);
		return YY_ERR_NONE;

	case YYPLAY_PID_SubTitle:
		m_nSubTTEnable = (int) pParam;
		if (m_nSubTTEnable > 0)
		{
			if (m_stsPlay < YY_PLAY_Open)
				return YY_ERR_NONE;

			if (m_pSubTT == NULL)
			{
				if (OpenSubTitle () != YY_ERR_NONE)
					return YY_ERR_IMPLEMENT;
				if (m_pSubTT != NULL)
				{
					if (m_stsPlay == YY_PLAY_Run)
						m_pSubTT->Start ();
				}
			}
		}
		if (m_pSubTT != NULL)
			m_pSubTT->Enable (m_nSubTTEnable);

		return YY_ERR_NONE;

	case YYPLAY_PID_SubTT_Size:
		m_nSubTTSize = (int)pParam;
		if (m_pSubTT != NULL)
			m_pSubTT->SetFontSize (m_nSubTTSize);
		return YY_ERR_NONE;

	case YYPLAY_PID_SubTT_Color:
		m_nSubTTColor = (int)pParam;
		if (m_pSubTT != NULL)
			m_pSubTT->SetFontColor (m_nSubTTColor);
		return YY_ERR_NONE;

	case YYPLAY_PID_SubTT_Font:
		m_hSubTTFont = (HFONT)pParam;
		if (m_pSubTT != NULL)
			m_pSubTT->SetFontHandle (m_hSubTTFont);
		return YY_ERR_NONE;

	case YYPLAY_PID_SubTT_View:
		m_hSubTTView = (HFONT)pParam;
		if (m_pSubTT != NULL)
			m_pSubTT->SetView (m_hSubTTView);
		if (m_pRndVideo != NULL)
		{
			if (m_hSubTTView != NULL)
				m_pRndVideo->SetSubTTEng (NULL);
			else
				m_pRndVideo->SetSubTTEng (m_pSubTT);
		}
		return YY_ERR_NONE;

	case YYPLAY_PID_Sub_CallBack:
		m_pSubExtRnd = (YY_DATACB *)pParam;
		if (m_pSubTT != NULL)
			m_pSubTT->SetExtRnd (m_pSubExtRnd);
		return YY_ERR_NONE;

	case YYPLAY_PID_Sub_ExtDraw:
		m_pSubExtDraw = (YYSUB_ExtDraw *)pParam;
		if (m_pSubTT != NULL)
			m_pSubTT->SetExtDraw (m_pSubExtDraw);
		return YY_ERR_NONE;

	case YYPLAY_PID_AudioTrackPlay:
		if (m_pBoxSource != NULL)
			return m_pBoxSource->SetStreamPlay (YY_MEDIA_Audio, (int)pParam);
		else
			return YY_ERR_STATUS;

	case YYPLAY_PID_Clock_OffTime:
		if (m_pClock != NULL)
			m_pClock->SetOffset ((int)pParam);
		return YY_ERR_NONE;

	case YYPLAY_PID_DataCB:
		if (pParam == NULL)
			return YY_ERR_ARG;
		m_cbData.funcCB =  ((YY_DATACB *)pParam)->funcCB;
		m_cbData.userData =  ((YY_DATACB *)pParam)->userData;
		return YY_ERR_NONE;

	case YYPLAY_PID_VideoExtRnd:
		if (m_pRndVideo == NULL)
			return YY_ERR_STATUS;	
		return m_pRndVideo->SetVideoExtRnd ((YY_DATACB *)pParam);

	case YYPLAY_PID_DisableEraseBG:
		if (m_pRndVideo == NULL)
			return YY_ERR_STATUS;	
		return m_pRndVideo->DisableEraseBG ();

	case YYPLAY_PID_VDMode:
		m_vdMode = (YY_PLAY_VDecMode)((int)pParam);
		if (m_pDecVideo != NULL)
			m_pDecVideo->SetDecMode (m_vdMode);
		return YY_ERR_NONE;

	default:
		break;
	}

	return YY_ERR_PARAMID;
}

int COMBoxMng::GetParam (int nID, void * pParam)
{
	if (nID == YYPLAY_PID_ReadData)
	{
		return ReadData ((YY_BUFFER *) pParam);
	}
	else if (nID == YYPLAY_PID_ConvertData)
	{
		if (pParam == NULL)
			return YY_ERR_ARG;
		return ConvertData (((YY_BUFFER_CONVERT *) pParam)->pSource, 
							((YY_BUFFER_CONVERT *) pParam)->pTarget, 
							((YY_BUFFER_CONVERT *) pParam)->pZoom);
	}
	else if (nID == YYPLAY_PID_RenderArea)
	{
		if (m_pRndVideo != NULL)
		{
			RECT * pRCVideo = m_pRndVideo->GetRenderRect ();
			if (pRCVideo != NULL)
			{
				memcpy (pParam, pRCVideo, sizeof (RECT));
				return YY_ERR_NONE;
			}
		}
		return YY_ERR_FAILED;
	}
	else if (nID == YYPLAY_PID_Sub_Charset)
	{
		if (m_pSubTT == NULL)
			return YY_ERR_FAILED;
		
		*((int *)pParam) = m_pSubTT->GetCharset ();
		return YY_ERR_NONE;
	}
	
	CAutoLock lock (&m_mtFunc);
	switch (nID)
	{
	case YYPLAY_PID_Speed:
		if (pParam == NULL)
			return YY_ERR_ARG;
		if (m_pClock != NULL)
			*(float *)pParam = m_pClock->GetSpeed ();
		return YY_ERR_NONE;

	case YYPLAY_PID_SubTitle:
		if (m_pSubTT == NULL)
			*(int *)pParam = 0;
		else
			*(int *)pParam = m_nSubTTEnable;
		return YY_ERR_NONE;

	case YYPLAY_PID_SubTT_Size:
		*(int *)pParam = m_nSubTTSize;
		return YY_ERR_NONE;

	case YYPLAY_PID_SubTT_Font:
		if (m_hSubTTFont != NULL)	
			*(void **)pParam = m_hSubTTFont;
		else if (m_pSubTT != NULL)
			*(void **)pParam = m_pSubTT->GetInFont ();
		return YY_ERR_NONE;

	case YYPLAY_PID_SubTT_View:
		*(void **)pParam = m_hSubTTView;
		return YY_ERR_NONE;

	case YYPLAY_PID_SubTT_Color:
		*(int *)pParam = m_nSubTTColor;
		return YY_ERR_NONE;

	case YYPLAY_PID_DDMode:
		*(int *)pParam = m_ddMode;
		return YY_ERR_NONE;

	case YYPLAY_PID_AudioTrackNum:
		if (m_pBoxSource != NULL)
			*(int *)pParam = m_pBoxSource->GetStreamCount (YY_MEDIA_Audio);
		return YY_ERR_NONE;

	case YYPLAY_PID_AudioTrackPlay:
		if (m_pBoxSource != NULL)
			*(int *)pParam = m_pBoxSource->GetStreamPlay (YY_MEDIA_Audio);
		return YY_ERR_STATUS;

	case YYPLAY_PID_BOX:
	{
		CBoxBase * pBox = GetBox ((int)pParam);
		return (int)pBox;
	}

	case YY_PLAY_BASE_META:
		if (m_pBoxSource == NULL)
			return YY_ERR_SOURCE;
		return m_pBoxSource->GetParam (nID, pParam);

	case YYPLAY_PID_VideoData:
		if (m_pRndVideo == NULL)
			return YY_ERR_STATUS;
		return m_pRndVideo->GetVideoData ((YY_BUFFER **)pParam);

	case YYPLAY_PID_Fmt_Audio:
		if (pParam == NULL)
			return YY_ERR_ARG;
		if (m_pRndAudio == NULL)
			return YY_ERR_STATUS;
		*(void **)pParam = m_pRndAudio->GetAudioFormat ();
		return YY_ERR_NONE;

	case YYPLAY_PID_Fmt_Video:
		if (pParam == NULL)
			return YY_ERR_ARG;
		if (m_pRndVideo == NULL)
			return YY_ERR_STATUS;
		*(void **)pParam = m_pRndVideo->GetVideoFormat ();
		return YY_ERR_NONE;
		
	case YYPLAY_PID_VDMode:
		*(int *)pParam = m_vdMode;
		return YY_ERR_NONE;
		
	default:
		break;
	}

	return YY_ERR_PARAMID;
}

int COMBoxMng::ReadData (YY_BUFFER * pData)
{
	CBoxBase * pBox = NULL;
	if (pData->nType == YY_MEDIA_Video)
		pBox = m_pRndVideo;
	else if (pData->nType == YY_MEDIA_Audio)
		pBox = m_pRndAudio;

	if (pBox == NULL)
		return YY_ERR_FAILED;

	if (pBox->GetType () != OMB_TYPE_RND_EXT)
		return YY_ERR_STATUS;

	int nRC = YY_ERR_NONE;
	if (m_stsPlay == YY_PLAY_Run)
		nRC = pBox->ReadBuffer (pData, true);
	else
		nRC = pBox->ReadBuffer (pData, false);

	return nRC;
}

int COMBoxMng::ConvertData (YY_BUFFER * pSource, YY_BUFFER * pTarget, RECT * pZoom)
{
	CBoxBase * pBox = NULL;
	if (pSource->nType == YY_MEDIA_Video)
		pBox = m_pRndVideo;
	else if (pSource->nType == YY_MEDIA_Audio)
		pBox = m_pRndAudio;

	if (pBox == NULL)
		return YY_ERR_FAILED;

	int nRC = pBox->Convert (pSource, pTarget, pZoom);

	return nRC;
}

int COMBoxMng::WaitAudioRender (int nWaitTime, bool bCheckStatus)
{
	if (m_pRndAudio == NULL)
		return YY_ERR_NONE;

	int nStartTime = yyGetSysTime ();
	while (m_pRndAudio->GetRndCount () <= 0)
	{
		if (bCheckStatus)
		{
			if (m_stsPlay != YY_PLAY_Run)
				break;
		}
		if (m_pRndAudio->IsEOS ())
			break;

		if (m_nSeekMode == YY_SEEK_KeyFrame)
		{
			if (yyGetSysTime () - nStartTime > nWaitTime)
			{
				PrepareClose ();
				return YY_ERR_TIMEOUT;
			}
		}
		yySleep (5000);
	}
	
	return YY_ERR_NONE;
}

int COMBoxMng::DrawFirstVideoFrame (int nWaitTime)
{
#ifdef _OS_WIN32
	int nStartTime = yyGetSysTime ();
	while (m_pRndVideo->GetRndCount () <= 0)
	{
		m_pRndVideo->RenderFrame (false, false);
		if (yyGetSysTime () - nStartTime > nWaitTime)
		{
			// PrepareClose ();
			return YY_ERR_FAILED;
		}
		if (m_pRndVideo->IsEOS ())
			return YY_ERR_FINISH;
	}
#endif // _OS_WIN32
	return YY_ERR_NONE;
}

void COMBoxMng::PushTask (int nID, void * pV1)
{
	YY_EVENT * pTask = m_lstFreeEvent.RemoveHead ();
	if (pTask == NULL)
		 pTask = new YY_EVENT ();
	pTask->nID = nID;
	pTask->pV1 = pV1;
	m_lstFullEvent.AddTail (pTask);
}

void COMBoxMng::NotifyEvent (void * pUserData, int nID, void * pV1)
{
	COMBoxMng * pEngine = (COMBoxMng *)pUserData;
	pEngine->HandleEvent (nID, pV1);
}

void COMBoxMng::HandleEvent (int nID, void * pV1)
{
	CAutoLock lockEvent (&m_mtEvent);

	if (nID == YY_EV_Play_Complete)
	{
		if (pV1 == m_pRndAudio)
		{
			if (!CBaseFile::g_bFileError)
				PushTask (YY_EV_Audio_EOS, NULL);
			if (m_pRndVideo != NULL)
			{
				YYLOGI ("Audio complete. Video is %d", m_pRndVideo->IsEOS ());
			}
			else
			{
				YYLOGI ("Audio complete.");
			}

			if (m_pRndVideo != NULL && !m_pRndVideo->IsEOS ())
				return;
		}
		else if (pV1 == m_pRndVideo)
		{
			if (!CBaseFile::g_bFileError)
				PushTask (YY_EV_Video_EOS, NULL);
			if (m_pRndAudio != NULL)
			{
				YYLOGI ("Video complete. audio is %d", m_pRndAudio->IsEOS ());
			}
			else
			{
				YYLOGI ("Video complete.");
			}

			if (m_pRndAudio != NULL && !m_pRndAudio->IsEOS ())
				return;
		}
	}
	if (nID == YY_EV_Play_Complete)
	{
		if (CBaseFile::g_bFileError)
			return;
	}
	PushTask (nID, pV1);
}

int COMBoxMng::NotifyProc (void * pParam)
{
	COMBoxMng * pEngine = (COMBoxMng *)pParam;
	pEngine->m_hThreadNotifyEvent = yyThreadGetCurHandle ();
	pEngine->NotifyLoop ();
	pEngine->m_hThreadNotifyEvent = NULL;
	return 0;
}

int COMBoxMng::NotifyLoop (void)
{
	yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_BELOW_NORMAL);
	YY_EVENT *	pEvent = NULL;
	int			nRC = 0;
	while (!m_bStopNotifyEvent)
	{
		if (m_fNotifyEvent != NULL)
		{
			{
				CAutoLock lockEvent (&m_mtEvent);
				pEvent = m_lstFullEvent.GetHead ();
			}
			if (pEvent != NULL)
			{
				if (pEvent->nID == YY_TASK_OPEN)
				{
					if ((m_nOpenFlag & YY_OPEN_SRC_READ) == YY_OPEN_SRC_READ || 
						(m_nOpenFlag & YY_OPEN_SRC_BOX) == YY_OPEN_SRC_BOX )
						nRC = DoOpen ((const TCHAR *)pEvent->pV1, m_nOpenFlag);
					else
						nRC = DoOpen (m_szSource, m_nOpenFlag);

					if (nRC < 0)
					{
						YYLOGI ("Open source failed! err = %08X", nRC);
						pEvent->nID = YY_EV_Open_Failed;
						if (m_bForceClosed)
							pEvent->pV1 = (void *)YY_ERR_FORCECLOSE;
						else
							pEvent->pV1 = (void *)nRC;
					}
					else
					{
						YYLOGI ("Open source finished!");
						pEvent->nID = YY_EV_Open_Complete;
						pEvent->pV1 = YY_ERR_NONE;
					}
				}
				else if (pEvent->nID == YY_TASK_SEEK)
				{
					nRC = DoSeek ((int)pEvent->pV1);
					m_bSeeking = false;
					if (nRC < 0)
					{
						pEvent->nID = YY_EV_Seek_Complete;
						pEvent->pV1 = (void *)nRC;
					}
					else
					{
						pEvent->nID = YY_EV_Seek_Complete;
						pEvent->pV1 = YY_ERR_NONE;
					}
				}

				if (pEvent->nID == YY_EV_Open_Failed || pEvent->nID == YY_EV_Open_Complete)
					m_bOpening = false;

				m_nCurEventID = pEvent->nID;
				if (!m_bClosed)
					m_fNotifyEvent (m_pUserData, pEvent->nID, pEvent->pV1);
				m_nCurEventID = 0;

				CAutoLock lockEvent (&m_mtEvent);
				pEvent = m_lstFullEvent.RemoveHead ();
				m_lstFreeEvent.AddTail (pEvent);
			}
			else
			{
				if (m_nDur == -1)
				{
					m_nDur = GetDuration ();
					if (m_nDur <= 0)
						m_nDur = -1;
					else
						PushTask (YY_EV_Play_Duration, (void *)m_nDur);
				}
				yySleep (10000);
			}
		}
		else
		{
			yySleep (20000);
		}
	}

	return 0;
}

void COMBoxMng::CloseEvent (void)
{
	m_bStopNotifyEvent = true;
	int nTryTimes = 0;
	while (m_hThreadNotifyEvent != NULL)
	{
		nTryTimes++;
		if (nTryTimes > 1000)
			break;
		yySleep (5000);
	}

	YY_EVENT * pEvent = m_lstFreeEvent.RemoveHead ();
	while (pEvent != NULL)
	{
		delete pEvent;
		pEvent = m_lstFreeEvent.RemoveHead ();
	}
	pEvent = m_lstFullEvent.RemoveHead ();
	while (pEvent != NULL)
	{
		delete pEvent;
		pEvent = m_lstFullEvent.RemoveHead ();
	}
}

int COMBoxMng::AudioPlayProc (void * pParam)
{
	COMBoxMng * pEngine = (COMBoxMng *)pParam;
	if (pEngine->m_pRndAudio != NULL)
		pEngine->m_pRndAudio->RenderFrame (false, true);
	return 0;
}

int COMBoxMng::VideoPlayProc (void * pParam)
{
	COMBoxMng * pEngine = (COMBoxMng *)pParam;
	if (pEngine->m_pRndVideo != NULL)
		pEngine->m_pRndVideo->RenderFrame (false, true);
	return 0;
}
