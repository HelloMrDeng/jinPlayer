/*******************************************************************************
	File:		CBoxVideoRnd.cpp

	Contains:	The video render box implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifdef _OS_WIN32
#include "windows.h"
#include "CVideoGDIRnd.h"
#ifdef _OS_WINPC	
#include "CVideoDDrawRnd.h"
#else
#include "CVideoDD60Rnd.h"
#endif // _OS_WINPC
#endif // _OS_WIN32

#ifdef _CPU_MSB2531
#include "CVideoMSB2531Rnd.h"
#endif // _CPU_MSB2531

#include "CBoxVideoRnd.h"
#include "CBoxMonitor.h"

#include "USystemFunc.h"
#include "UYYDataFunc.h"

#include "yyConfig.h"
#include "yyMediaPlayer.h"
#include "yyLog.h"

CBoxVideoRnd::CBoxVideoRnd(void * hInst)
	: CBoxRender (hInst)
#ifdef _OS_WINCE
	, m_nRndType (YY_VRND_DDCE6)
#else
	, m_nRndType (YY_VRND_DDRAW)
#endif // _OS_WINCE
	, m_hView (NULL)
	, m_ddMode (YY_DDM_Memory)
	, m_nRotateAngle (0)
	, m_nARW (1)
	, m_nARH (1)
	, m_pSubTTEng (NULL)
	, m_pDDExt (NULL)
	, m_pGetBuff (NULL)
	, m_pExtRnd (NULL)
	, m_pRnd (NULL)
	, m_bNotifyFirstFrame (false)
	, m_nDisableFlag (YY_PLAY_VideoEnable)
	, m_pRCC (NULL)
	, m_llDelayTime (0)
	, m_llVideoTime (0)
	, m_llSystemTime (0)
	, m_nDroppedFrames (0)
	, m_llFirstOffsetTime (0)
	, m_llAVOffsetTime (0)
	, m_llLastBuffTime (0)
	, m_bSetThreadPriority (true)
	, m_byyDemoPlayer (false)
	, m_nScreenX (0)
	, m_nScreenY (0)
	, m_nDbgStartTime (0)
{
	SetObjectName ("CBoxVideoRnd");
	m_nMediaType = YY_MEDIA_Video;
	m_nBoxType = OMB_TYPE_RENDER;
	strcpy (m_szBoxName, "Video Render Box");

	memset (&m_rcView, 0, sizeof (m_rcView));
	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
	m_pFmtVideo = &m_fmtVideo;

	m_nCPUNum = yyGetCPUNum ();

	memset (&m_buffRender, 0, sizeof (m_buffRender));
	memset (&m_buffSubTT, 0, sizeof (m_buffSubTT));
	
#ifdef _OS_WIN32
	m_nScreenX = GetSystemMetrics (SM_CXSCREEN);
	m_nScreenY = GetSystemMetrics (SM_CYSCREEN);
#endif // _OS_WIN32
}

CBoxVideoRnd::~CBoxVideoRnd(void)
{
	Stop ();
	if (m_pRnd != NULL)
		m_pRnd->Uninit ();
	YY_DEL_P (m_pRnd);
	if (m_buffSubTT.pBuff != NULL)
		delete []m_buffSubTT.pBuff;
	m_buffSubTT.pBuff = NULL;
	yyDataResetVideoBuff (m_pGetBuff, true);
	YY_DEL_A (m_fmtVideo.pHeadData);
	YY_DEL_P (m_pRCC);
}

int CBoxVideoRnd::SetRndType (YYRND_TYPE nRndType)
{
	if (m_nRndType == nRndType)
		return YY_ERR_NONE;
	m_nRndType = nRndType;
	return CreateRender ();
}

int CBoxVideoRnd::SetDisplay (void * hView, RECT * pRect)
{
	CAutoLock lock (&m_mtRnd);
	m_bSetThreadPriority = false;
	m_hView = hView;
	if (pRect != NULL)
		memcpy (&m_rcView, pRect, sizeof (m_rcView));
	else
	{
#ifdef _OS_WIN32
		GetClientRect ((HWND)hView, &m_rcView);
#endif // _OS_WIN32
	}
	if (m_pRnd != NULL)
		m_pRnd->SetDisplay (m_hView, pRect);

	return YY_ERR_NONE;
}

int CBoxVideoRnd::UpdateDisp (void)
{
	if (m_status == YYRND_RUN && !m_bEOS)
		return YY_ERR_NONE;
	if (m_pDataCB != NULL)
		return YY_ERR_STATUS;
	if (m_nDisableFlag != YY_PLAY_VideoEnable)
		return YY_ERR_FAILED;

	CAutoLock lock (&m_mtRnd);
	if (m_pRnd != NULL)
		m_pRnd->UpdateDisp ();
	return YY_ERR_NONE;
}

int CBoxVideoRnd::SetAspectRatio (int w, int h)
{
	CAutoLock lock (&m_mtRnd);
	if (m_nARW == w && m_nARH == h)
		return YY_ERR_NONE;
	m_nARW = w;
	m_nARH = h;
	if (m_pRnd != NULL)
		m_pRnd->SetAspectRatio (m_nARW, m_nARH);
	return YY_ERR_NONE;
}

int CBoxVideoRnd::SetDDMode (YY_PLAY_DDMode nMode)
{
	CAutoLock lock (&m_mtRnd);
	if (m_ddMode == nMode)
		return YY_ERR_NONE;
	m_ddMode = nMode;
	if (m_pRnd != NULL)
		m_pRnd->SetDDMode (m_ddMode);
	return YY_ERR_NONE;
}

int CBoxVideoRnd::SetRotate (int nAngle)
{
	if (m_nRotateAngle == nAngle)
		return YY_ERR_NONE;
	m_nRotateAngle = nAngle;
	return CreateRender ();
}

int CBoxVideoRnd::DisableVideo (int nFlag)
{
	if (m_pDataCB != NULL)
		return YY_ERR_STATUS;
	if (m_nDisableFlag == nFlag)
		return YY_ERR_NONE;
	m_nDisableFlag = nFlag;
	YYLOGI ("DisableVideo, Flag: %d, m_pRnd = %p", m_nDisableFlag, m_pRnd);
#ifdef _CPU_MSB2531
	if (m_pRnd != NULL)
	{
		if (m_nDisableFlag == YY_PLAY_VideoEnable)
			m_pRnd->EnableKeyColor (true);
		else
			m_pRnd->EnableKeyColor (false);
	}
	return YY_ERR_NONE;
#endif // _CPU_MSB2531
	if (m_ddMode != YY_DDM_Overlay)
		return YY_ERR_NONE;
	CAutoLock lock (&m_mtRnd);
	if (m_nDisableFlag == YY_PLAY_VideoEnable)
	{
		if (m_pRnd != NULL)
			return YY_ERR_NONE;
		if (m_pBoxSource == NULL)
			return YY_ERR_STATUS;
		return CreateRender ();
	}
	else
	{
		if (m_pRnd != NULL)
		{
			m_pRnd->Stop ();
			delete m_pRnd;
			m_pRnd = NULL;
		}
	}
	return YY_ERR_NONE;
}

int CBoxVideoRnd::SetSubTTEng (CSubtitleEngine * pSubTTEng)
{
	CAutoLock lock (&m_mtRnd);
	m_pSubTTEng = pSubTTEng;
	if (m_pRnd != NULL)
		m_pRnd->SetSubTTEng (m_pSubTTEng);
	return YY_ERR_NONE;
}

int CBoxVideoRnd::SetExtDDraw (void * pDDExt)
{
	CAutoLock lock (&m_mtRnd);
	m_pDDExt = pDDExt;
	if (m_pRnd != NULL)
		m_pRnd->SetExtDDraw (m_pDDExt);
	return YY_ERR_NONE;
}

int CBoxVideoRnd::SetSource (CBoxBase * pSource)
{
	CAutoLock lock (&m_mtRnd);
	memset (&m_buffRender, 0, sizeof (m_buffRender));
	if (pSource == NULL)
	{
		m_pBoxSource = NULL;
		ResetMembers ();
		return YY_ERR_ARG;
	}
	CBoxBase::SetSource (pSource);
	YY_VIDEO_FORMAT * pFmt = pSource->GetVideoFormat ();
	if (pFmt == NULL)
		return YY_ERR_VIDEO;
	yyDataCloneVideoFormat (&m_fmtVideo, pFmt);

	m_bNotifyFirstFrame = false;
	m_nDbgStartTime = 0;

	if (m_pDataCB != NULL)
		return YY_ERR_NONE;

	return CreateRender ();
}

int	CBoxVideoRnd::Start (CThreadWork * pWork)
{
	if (m_pRnd != NULL)
		m_pRnd->Start ();

	int nRC = CBoxRender::Start (pWork);

	return nRC;
}

int CBoxVideoRnd::Pause (void)
{
	int nRC = CBoxRender::Pause ();
	if (m_pRnd != NULL)
		m_pRnd->Pause ();
	return nRC;
}

int	CBoxVideoRnd::Stop (void)
{
#ifdef _YY_PLAYER
	if (m_status != YYRND_STOP && m_nRndCount > 0)
	{
		int nUsedTime = yyGetSysTime ()- m_nDbgStartTime;
		if (nUsedTime > 0)
		{
			YYLOGI ("Status: % 8d : % 8d : % 8d", m_nRndCount, nUsedTime, (m_nRndCount * 100000) / nUsedTime);
		}
	}
#endif //_YY_PLAYER
		
	int nRC = CBoxRender::Stop ();
	if (m_pRnd != NULL)
		m_pRnd->Stop ();
	yyDataResetVideoBuff (m_pGetBuff, true);
	m_pGetBuff = NULL;
	return nRC;
}

CBaseClock * CBoxVideoRnd::GetClock (void)
{
	CAutoLock lock (&m_mtRnd);
	if (m_pRnd != NULL)
		return m_pRnd->GetClock ();
	return NULL;
}

RECT * CBoxVideoRnd::GetRenderRect (void )
{
	if (m_pRnd != NULL)
		return m_pRnd->GetRenderRect ();
	return NULL;
}

int CBoxVideoRnd::SetPos (int nPos, bool bSeek)
{
	CAutoLock lock (&m_mtRnd);

	m_nDroppedFrames = 0;
	m_llFirstOffsetTime = 0;
	m_llAVOffsetTime = 0;
	m_llLastBuffTime = 0;

	m_llVideoTime = nPos;
	m_llDelayTime = 0;

//	YYLOGI ("SetPos % 8d ********************", nPos);
	return CBoxRender::SetPos (nPos, bSeek);
}

int CBoxVideoRnd::SetZoom (RECT * pRect)
{
	if (pRect == NULL)
	{
		if (m_buffRender.pBuff != NULL)
		{
			if ((m_buffRender.uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
			{
				AVFrame * pFrmVideo = (AVFrame *)m_buffRender.pBuff;
				if (pFrmVideo->format != AV_PIX_FMT_YUV420P)
					return YY_ERR_IMPLEMENT;
			}
		}
		return YY_ERR_NONE;
	}

	if (m_pRnd == NULL)
		return YY_ERR_STATUS;

	return m_pRnd->SetZoom (pRect);
}

int CBoxVideoRnd::GetVideoData (YY_BUFFER ** ppVideoData)
{
	if (ppVideoData == NULL)
		return YY_ERR_ARG;

	CAutoLock lock (&m_mtRnd);
	if (m_buffRender.pBuff == NULL)
		return YY_ERR_FAILED;

	if (*ppVideoData == NULL)
	{
		if (m_pGetBuff == NULL)
		{
			m_pGetBuff = new YY_BUFFER ();
			memset (m_pGetBuff, 0, sizeof (YY_BUFFER));
		}
		else if (m_pGetBuff->llTime == m_buffRender.llTime)
		{
			*ppVideoData = m_pGetBuff;
			return YY_ERR_NONE;
		}
		yyDataCloneVideoBuff (m_pGetBuff, &m_buffRender);
		*ppVideoData = m_pGetBuff;
	}
	else
	{
		if (m_pRCC == NULL)
			m_pRCC = new CFFMpegVideoRCC (m_hInst);
		YY_VIDEO_BUFF * pVidBuff = (YY_VIDEO_BUFF *)(*ppVideoData)->pBuff;
		return m_pRCC->ConvertBuff (&m_buffRender, pVidBuff, NULL);
	}

	return YY_ERR_NONE;
}

int CBoxVideoRnd::SetVideoExtRnd (YY_DATACB * pDataCB)
{
	if (m_pExtRnd == pDataCB)
		return YY_ERR_NONE;
	m_pExtRnd = pDataCB;
	return YY_ERR_NONE;
}

int CBoxVideoRnd::DisableEraseBG (void)
{
	if (m_pRnd == NULL)
		return YY_ERR_STATUS;
	m_pRnd->DisableEraseBG ();
	return YY_ERR_NONE;
}

int CBoxVideoRnd::RenderFrame (bool bInBox, bool bWait)
{
	if (m_pBoxSource == NULL)
		return YY_ERR_STATUS;

	m_mtRnd.Lock ();

	if (!m_bSetThreadPriority)
	{
		if (m_rcView.right == m_nScreenX && m_rcView.bottom == m_nScreenY)
		{
#ifdef _OS_WIN32
			if (m_byyDemoPlayer)
				yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_NORMAL);
			else
				yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_BELOW_NORMAL);
#else
			yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_NORMAL);
#endif // _OS_WIN32
		}
		else
		{
			yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_BELOW_NORMAL);
		}
#ifdef _OS_WINPC
		yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_NORMAL);
#endif // _OS_WINPC
		m_bSetThreadPriority = true;
	}

	if (bInBox)
		m_bInRender = true;

	if (m_pBaseBuffer == NULL)
	{
		m_mtRnd.Unlock ();
		return YY_ERR_STATUS;
	}
		
	m_pBaseBuffer->nType = YY_MEDIA_Video;
	m_pBaseBuffer->uFlag = 0;
	if (m_nDroppedFrames > 0)
	{
		m_pBaseBuffer->uFlag = YYBUFF_DEC_DISA_DEBLOCK;
		if (m_llDelayTime > 0 && m_pClock->GetTime () - m_llVideoTime < m_llDelayTime)
			m_pBaseBuffer->uFlag += YYBUFF_DEC_SKIP_BFRAME;
	}

	m_llDelayTime = m_pClock->GetTime () - m_llVideoTime;
	if (m_nRndCount <= 1)
		m_llDelayTime = 0;
//	YYLOGI ("Delay Time: % 8d Drop Frames % 6d", (int)m_llDelayTime, m_nDroppedFrames);
	m_pBaseBuffer->llDelay = m_llDelayTime;
	if (m_pClock != NULL)
		m_pBaseBuffer->llTime = m_pClock->GetTime ();
	else
		m_pBaseBuffer->llTime = 0;
	m_pBaseBuffer->pBuff = NULL;
	if (m_nDisableFlag == YY_PLAY_VideoDisable_Decoder)
		m_pBaseBuffer->uFlag |= YYBUFF_DEC_DISABLE;

	int nRC = m_pBoxSource->ReadBuffer (m_pBaseBuffer, false);
//	YYLOGI ("Return %08X, Time: % 8d   % 8d", nRC, (int)m_pBaseBuffer->llTime, (int)(m_pBaseBuffer->llTime - m_pClock->GetTime ()));
	YY_BUFFER * pBuffer = m_pBaseBuffer;
	BOX_READ_BUFFER_REC_VIDEORND
	
	if ((m_pBaseBuffer->uFlag & YYBUFF_EOS) == YYBUFF_EOS)
	{		
		if (m_bEOS)
		{
			yySleep (10000);
		}
		else
		{
			m_bEOS = true;
			if (m_pDataCB != NULL && m_pDataCB->funcCB != NULL)
				nRC = m_pDataCB->funcCB (m_pDataCB->userData, m_pBaseBuffer);

			if (m_pNotifyFunc != NULL)
				m_pNotifyFunc (m_pUserData, YY_EV_Play_Complete, this);
		}
	}
	if (nRC == YY_ERR_RETRY || nRC == YY_ERR_FINISH)
	{
		m_mtRnd.Unlock ();
		if (m_nRndCount == 0)
			yySleep (2000);
		return nRC;	
	}

	if ((m_pBaseBuffer->uFlag & YYBUFF_DROP_FRAME) == YYBUFF_DROP_FRAME)
	{
		m_nDroppedFrames = m_pBaseBuffer->nValue;
		// YYLOGI ("Drop Frames % 4d Time: % 8d, Delay % 8d", m_nDroppedFrames, (int)m_pBaseBuffer->llTime, (int)(m_pBaseBuffer->llTime - m_pClock->GetTime ()));
	}
	
	if (m_nSeekMode == YY_SEEK_AnyPosition)
	{
		if (m_pBaseBuffer->llTime < m_llSeekPos)
		{
			m_mtRnd.Unlock ();
			return YY_ERR_NONE;
		}
	}
	
	if (nRC == YY_ERR_NONE)
	{
		if ((m_pBaseBuffer->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
		{
			YY_VIDEO_FORMAT * pFmt = m_pBoxSource->GetVideoFormat ();
			yyDataCloneVideoFormat (&m_fmtVideo, pFmt);
			if (m_pNotifyFunc != NULL)
				m_pNotifyFunc (m_pUserData, YY_EV_Video_Changed, 0);
			if (m_pRnd != NULL)
				m_pRnd->Init (pFmt);
		}
		memcpy (&m_buffRender, m_pBaseBuffer, sizeof (m_buffRender));
		if (!m_bEOS && m_nDisableFlag == YY_PLAY_VideoEnable)
		{
			int nRet = YY_ERR_NONE;
#ifdef _OS_WINPC
			if (m_nRndCount == 1 && m_pOtherRnd != NULL)
			{
				if (m_pBaseBuffer->llTime < m_pClock->GetTime ())
				{
					m_mtRnd.Unlock ();
					return YY_ERR_NONE;
				}
			}
#endif // _OS_WINPC

			pBuffer->uFlag = pBuffer->uFlag & ~YYBUFF_DEC_RENDER;
			m_pBoxSource->RendBuffer (m_pBaseBuffer, true);
			if (m_pExtRnd != NULL)
			{
				nRet = m_pExtRnd->funcCB (m_pExtRnd->userData, m_pBaseBuffer);
			}
			else
			{
				if (m_pRnd != NULL)
				{
					nRet = m_pRnd->Render (m_pBaseBuffer);
				}
				if (m_pDataCB != NULL && m_pDataCB->funcCB != NULL)
				{
					nRet = m_pDataCB->funcCB (m_pDataCB->userData, m_pBaseBuffer);
					if (m_pSubTTEng != NULL)
					{
						if (m_buffSubTT.pBuff == NULL)
							m_buffSubTT.pBuff = new unsigned char[2048];
						m_buffSubTT.uSize = 2048;
						memset (m_buffSubTT.pBuff, 0, m_buffSubTT.uSize);
						m_buffSubTT.llTime = m_pClock->GetTime ();
						nRet = m_pSubTTEng->GetItemText (m_buffSubTT.llTime, &m_buffSubTT);
						if (nRet == YY_ERR_NONE)
						{
							//YYLOGI ("The SubTT: %s", (char *)m_buffSubTT.pBuff);
							nRet = m_pDataCB->funcCB (m_pDataCB->userData, &m_buffSubTT);
						}
					}
				}
			}
		}
		if (m_nRndCount == 0)
			m_nDbgStartTime = yyGetSysTime ();

		if (m_pOtherRnd == NULL)
		{
			if (m_nRndCount == 0 || abs ((int)(m_pBaseBuffer->llTime - m_llVideoTime)) > 1000)
				m_pClock->SetTime (m_pBaseBuffer->llTime == 0 ? 1 : m_pBaseBuffer->llTime);
		}

		m_nRndCount++;
		if (!m_bNotifyFirstFrame && m_pNotifyFunc != NULL)
		{
			m_bNotifyFirstFrame = true;
			m_pNotifyFunc (m_pUserData, YY_EV_Draw_FirstFrame, this);
		}
		
		if (m_nCPUNum > 1 && m_nRndCount > 5)
		{
			if (m_pBaseBuffer->llTime - m_llVideoTime >= 25)
			{
				int nDispTime = yyGetSysTime () - m_llSystemTime;
				if (nDispTime < 25)
					yySleep ((25 - nDispTime) * 1000);	
			}
		}

		m_llVideoTime = m_pBaseBuffer->llTime;
		m_llSystemTime = yyGetSysTime ();
	}
	m_mtRnd.Unlock ();

//	if (m_nRndCount > 50 && m_nRndCount < 100)
//		yySleep ((rand () % 500 + 500) * 1000);
//	yySleep ((rand () % 20 + 40) * 1000);
//	yySleep (100000);

	if (nRC == YY_ERR_NONE && bWait)
		WaitRenderTime (m_pBaseBuffer);

	return YY_ERR_NONE;
}

int CBoxVideoRnd::WaitRenderTime (YY_BUFFER * pBuff)
{
	if (m_bEOS || m_pClock == NULL)
		return YY_ERR_NONE;

	if (m_nDroppedFrames >= 1)
	{
		long long llOffsetTime = pBuff->llTime - m_pClock->GetTime ();
		if (m_llFirstOffsetTime == 0)
			m_llFirstOffsetTime = llOffsetTime;

		if (llOffsetTime > 0 && (llOffsetTime - m_llFirstOffsetTime) < 500)
		{
			if (llOffsetTime >= m_llAVOffsetTime)
			{
				if (pBuff->llTime - m_llLastBuffTime < 50)
				{
					int nSleepTime = (int)(llOffsetTime - m_llAVOffsetTime);
					int nStartTime = yyGetSysTime ();
					while (yyGetSysTime () - nSleepTime < nSleepTime)
					{
						yySleep (2000);
						if (m_status != YYRND_RUN)
							break;
					}
				}
			}
			
			m_llLastBuffTime = pBuff->llTime;
			
			if (m_llAVOffsetTime == 0 || m_llAVOffsetTime > llOffsetTime)
				m_llAVOffsetTime = llOffsetTime;
			
			return YY_ERR_NONE;
		}

		m_nDroppedFrames = 0;
		m_llFirstOffsetTime = 0;
		m_llLastBuffTime = 0;
		m_llAVOffsetTime = 0;
	}
	
//	long long llPlayTime = m_pClock->GetTime ();
//	if  (llPlayTime < pBuff->llTime)
//		YYLOGI ("Index  % 8d   Playing % 8d,  Buff % 8d, Diff % 8d", m_nRndCount, (int)llPlayTime, (int)pBuff->llTime, (int)(llPlayTime -pBuff->llTime));
//	YYLOGI ("BuffTime: % 8d  Clock Time: % 8d    Diff % 8d", 
//				(int)pBuff->llTime, (int)m_pClock->GetTime (), (int)(pBuff->llTime - m_pClock->GetTime ())); 
	while (m_pClock->GetTime () < pBuff->llTime)
	{
		if (m_status != YYRND_RUN)
			break;

		int nOffset = (int)(pBuff->llTime - m_pClock->GetTime ());
		if (abs (nOffset) > 50000 && m_pClock->GetTime () != 0)
		{
			yySleep (30000);
			return YY_ERR_NONE;
		}
#ifdef _OS_NDK
		yySleep (2000);
#else
		yySleep (2000);
#endif // _OS_NDK
	}

	return YY_ERR_NONE;
}

void CBoxVideoRnd::ResetMembers (void)
{
	m_llSeekPos = 0;
	m_bEOS = true;
	m_nRndCount = 0;

	m_llDelayTime = 0;
	m_llVideoTime = 0;
	m_llSystemTime = 0;
	m_nDroppedFrames = 0;
	m_llFirstOffsetTime = 0;
	m_llAVOffsetTime = 0;
	m_llLastBuffTime = 0;
	m_bSetThreadPriority = true;
}

int CBoxVideoRnd::CreateRender (void)
{
	CAutoLock lock (&m_mtRnd);
	YY_DEL_P (m_pRnd);
#ifdef _OS_WIN32
	if (m_nRndType == YY_VRND_GDI)
		m_pRnd = new CVideoGDIRnd (m_hInst);
	else 
#ifdef _OS_WINPC
		m_pRnd = new CVideoDDrawRnd (m_hInst);
#elif defined _CPU_MSB2531
		m_pRnd = new CVideoMSB2531Rnd (m_hInst);
#elif defined _OS_WINCE
		m_pRnd = new CVideoDD60Rnd (m_hInst);
#endif // _OS_WINPC
#endif // _OS_WIN32
	if (m_pRnd == NULL)
		return YY_ERR_MEMORY;

	m_pRnd->SetExtDDraw (m_pDDExt);
	if (m_rcView.right > 0 && m_rcView.bottom > 0)
		m_pRnd->SetDisplay (m_hView, &m_rcView);
	else
		m_pRnd->SetDisplay (m_hView, NULL);
	m_pRnd->SetAspectRatio (m_nARW, m_nARH);
	m_pRnd->SetRotate (m_nRotateAngle);
	if (m_pSubTTEng != NULL)
		m_pRnd->SetSubTTEng (m_pSubTTEng);
	int nRC = m_pRnd->Init (&m_fmtVideo);
	if (nRC != YY_ERR_NONE)
	{
		delete m_pRnd;
		m_pRnd = NULL;
		return nRC;
	}
	m_pRnd->SetDDMode (m_ddMode);

	if (m_status == YYRND_RUN)
	{
		m_pRnd->Start ();
	}
	else if (m_status == YYRND_PAUSE)
	{
		m_pRnd->Start ();
		m_pRnd->Pause ();
		nRC = m_pRnd->Render (&m_buffRender);
	}
	return YY_ERR_NONE;
}