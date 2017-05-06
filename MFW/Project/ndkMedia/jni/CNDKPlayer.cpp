/*******************************************************************************
	File:		CNDKPlayer.cpp

	Contains:	yy NDK player implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

#include "CNDKPlayer.h"

#include "libavformat/avformat.h"

#include "USystemFunc.h"
#include "UJNIFunc.h"

#include "yySubtitle.h"

#include "yyLog.h"

#define YY_EV_VideoFormat_Change	0X21
#define YY_EV_AudioFormat_Change	0X22

#define PARAM_PID_EVENT_DONE		0X100

#define PARAM_PID_AUDIO_OFFSEST		0X300
#define PARAM_PID_COLOR_FORMAT		0X400
#define PARAM_PID_VIDEODEC_MODE		0X405
#define PARAM_PID_BITMAP_VIEW		0X500
#define PARAM_PID_BITMAP_DRAW		0X501
	
#define PARAM_PID_SunTT_Enable		0X410
#define PARAM_PID_SunTT_Charset		0X411

#define PARAM_PID_PDP_FILE			0X600

CNDKPlayer::CNDKPlayer ()
	: CBaseObject ()
	, m_pjVM (NULL)
	, m_pjCls (NULL)
	, m_pjObj (NULL)
	, m_fPostEvent (NULL)
	, m_fPushAudio (NULL)
	, m_fPushVideo (NULL)
	, m_fPushSubTT (NULL)
	, m_bEventDone (true)
	, m_pView (NULL)
	, m_pRndWnd (NULL)
	, m_RndCodec (NULL)
	, m_hDllBase (NULL)		
	, m_hDllMedia (NULL)
	, m_fInit (NULL)
	, m_fCreate (NULL)
	, m_fDestroy (NULL)
	, m_fUninit (NULL)
	, m_hMediaEng (NULL)
	, m_pPlayer (NULL)
	, m_pEnvAudio (NULL)
	, m_pDataAudio (NULL)
	, m_nBuffSize (0)
	, m_nDataSize (0)
	, m_nAudioRndCount (0)
	, m_nLatency (-1)
	, m_jBmpVideo (NULL)
	, m_bDrawBmp (true)
	, m_bBmpShow (false)
	, m_nBmpTime (0)
	, m_nVideoRndCount (0)
	, m_nVideoRndATime (0)
	, m_nVideoLastSysTime (0)
	, m_nVideoLastBufTime (0)
	, m_nVideoDispColor (0)
	, m_nEnbSubTT (1)
	, m_pEnvSubTT (NULL)
	, m_hGraphics (NULL) 
	, m_fGetInfo (NULL)
	, m_fLockPixels (NULL)
	, m_fUnlockPixels (NULL)

{
	SetObjectName ("CNDKPlayer");
		
	m_cbData.userData = this;
	m_cbData.funcCB = MediaDataCB;
	
	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));	
	memset (&m_bufAudioRnd, 0, sizeof (m_bufAudioRnd));
	m_bufAudioRnd.uFlag = YYBUFF_TYPE_PPOINTER;	
	m_bufAudioRnd.pFormat = &m_fmtAudio;	
	m_dataConvertAudio.pTarget = &m_bufAudioRnd;	
	
	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));	
	memset (&m_bufVideoWnd, 0, sizeof (m_bufVideoWnd));		
	memset (&m_bufVideoRnd, 0, sizeof (m_bufVideoRnd));	
	m_bufVideoRnd.uFlag = YYBUFF_TYPE_VIDEO;	
	m_bufVideoRnd.pBuff = (unsigned char *)&m_bufVideoWnd;
	m_bufVideoRnd.uSize = sizeof (m_bufVideoWnd);
	m_bufVideoRnd.pFormat = &m_fmtVideo;
	m_dataConvertVideo.pTarget = &m_bufVideoRnd;	
	m_dataConvertVideo.pZoom = NULL;	
}

CNDKPlayer::~CNDKPlayer ()
{
	if (m_hGraphics != NULL)
		dlclose (m_hGraphics);
}

int CNDKPlayer::Init (JavaVM * jvm, JNIEnv* env, jclass clsPlayer, jobject objPlayer, char * pPath)
{
	m_pjVM = jvm;
	m_pjCls = clsPlayer;
	m_pjObj = objPlayer;
	
	m_fPostEvent = env->GetStaticMethodID (m_pjCls, "postEventFromNative", "(Ljava/lang/Object;IIILjava/lang/Object;)V");
	m_fPushAudio = env->GetStaticMethodID (m_pjCls, "audioDataFromNative", "(Ljava/lang/Object;[BI)V");
	m_fPushVideo = env->GetStaticMethodID (m_pjCls, "videoDataFromNative", "(Ljava/lang/Object;Ljava/lang/Object;)I");
	m_fPushTTByte = env->GetStaticMethodID (m_pjCls, "textBytesFromNative", "(Ljava/lang/Object;[BII)V");
	YYLOGI ("SubTT method = %p", m_fPushVideo);
			
	if (m_hDllMedia != NULL)
		return 0;
		
	char szPath[256];
	strcpy (szPath, pPath);
	m_hDllBase = dlopen("/data/local/tmp/yylib/libyyBaseEngn.so", RTLD_NOW);
	if (m_hDllBase == NULL)
	{
		strcat (szPath, "libyyBaseEngn.so");
		m_hDllBase = dlopen(szPath, RTLD_NOW);
		if (m_hDllBase == NULL)
		{
			YYLOGE ("The Base Dll could not be loaed!");
			return YY_ERR_FAILED;
		}
	}
	
	m_hDllMedia = dlopen("/data/local/tmp/yylib/libyyMediaEng.so", RTLD_NOW);
	if (m_hDllMedia == NULL)
	{
		strcpy (szPath, pPath);
		strcat (szPath, "libyyMediaEng.so");		
		m_hDllMedia = dlopen(szPath, RTLD_NOW);	
		if (m_hDllMedia == NULL)
		{
			YYLOGE ("The meida Dll could not be loaded! The error is %s", dlerror ());
			return YY_ERR_FAILED;
		}
	}
	
	m_fInit = (YYMEDIAINIT) dlsym (m_hDllMedia, ("yyMediaInit"));
	m_fCreate = (YYMEDIACREATE) dlsym (m_hDllMedia, ("yyMediaCreate"));	
	m_fDestroy = (YYMEDIADESTROY) dlsym (m_hDllMedia, ("yyMediaDestroy"));		
	m_fUninit = (YYMEDIAUNINIT) dlsym (m_hDllMedia, ("yyMediaUninit"));	

	if (m_fInit == NULL || m_fCreate == NULL || m_fDestroy == NULL || m_fUninit == NULL)
	{
		YYLOGE ("It can't get interface from media dll!");
		return -1;
	}

	int nRC = m_fInit (&m_hMediaEng, 0, NULL);
	if (nRC < 0)
	{	
		YYLOGE ("Init failed %08X", nRC);
		return nRC;
	}

	nRC = m_fCreate (m_hMediaEng, YYME_PLAYER, (void **)&m_pPlayer);
	if (nRC < 0)
	{	
		YYLOGE ("Create failed %08X", nRC);
		return nRC;
	}
	
	m_pPlayer->SetNotify (m_pPlayer, NotifyEvent, this);
	m_pPlayer->SetParam (m_pPlayer, YYPLAY_PID_DataCB, &m_cbData);
	
	m_pPlayer->SetParam (m_pPlayer, YYPLAY_PID_SubTitle, (void *)m_nEnbSubTT);
	m_pPlayer->SetParam (m_pPlayer, YYPLAY_PID_SubTT_View, m_pView);
	m_pPlayer->SetParam (m_pPlayer, YYPLAY_PID_Sub_CallBack, &m_cbData);

	if (m_pRndWnd != NULL)
		m_pPlayer->SetView (m_pPlayer, m_pRndWnd->GetWindow (), YY_VRND_GDI);

	memset (&m_bmpInfo, 0, sizeof (m_bmpInfo));		
				
	return YY_ERR_NONE;	
}

int CNDKPlayer::Uninit (JNIEnv* env)
{	
	if (m_pPlayer != NULL)
	{
		m_pPlayer->Close (m_pPlayer);
		m_fDestroy (m_hMediaEng, YYME_PLAYER, m_pPlayer);
		m_pPlayer = NULL;
	}
		
	if (m_hMediaEng != NULL)
		m_fUninit (m_hMediaEng);
	m_hMediaEng = NULL;

	if (m_hDllMedia != NULL)
		dlclose (m_hDllMedia);
	m_hDllMedia = NULL;

	if (m_hDllBase != NULL)
		dlclose (m_hDllBase);
	m_hDllBase = NULL;

	if (m_pRndWnd != NULL)
		delete m_pRndWnd;	
	m_pRndWnd = NULL;
	
	if (m_RndCodec != NULL)
		delete m_RndCodec;	
	m_RndCodec = NULL;

	if (m_pjObj != NULL)
    	env->DeleteGlobalRef(m_pjObj);
    m_pjObj = NULL;

	if (m_pjCls != NULL)
    	env->DeleteGlobalRef(m_pjCls);
    m_pjCls = NULL;  	
		
	if (m_pDataAudio != NULL)
		env->DeleteLocalRef(m_pDataAudio);
	m_pDataAudio = NULL;		
	
	return YY_ERR_NONE;
}

int	CNDKPlayer::SetView (JNIEnv* env, jobject pView)
{
	if (m_pView == pView)
		return YY_ERR_NONE;
		
	m_pView = pView;
	
	if (m_pRndWnd != NULL)
	{
		delete m_pRndWnd;
		m_pRndWnd = NULL;
	}
	
	if (m_pView == NULL)
		return YY_ERR_NONE;
		
	m_pRndWnd = new CNativeWndRender ();
	
	m_pRndWnd->SetColor (m_nVideoDispColor);		
	m_pRndWnd->Init (env, pView);
		
	if (m_pPlayer != NULL)
	{
		m_pPlayer->SetParam (m_pPlayer, YYPLAY_PID_SubTT_View, m_pView);	
		m_pPlayer->SetView (m_pPlayer, m_pRndWnd->GetWindow (), YY_VRND_GDI);
	}	
		
	return YY_ERR_NONE;
}

int CNDKPlayer::Open (const char* szUrl)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	
	m_nVideoRndCount = 0;
	m_nVideoRndATime = 0;
	m_nVideoLastSysTime = 0;
	m_nVideoLastBufTime = 0;
	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));	
	m_nBmpTime = 0;
		
	int nFlag = YY_OPEN_RNDA_CB | YY_OPEN_RNDV_CB;
	
	int nRC = m_pPlayer->Open (m_pPlayer, szUrl, nFlag);
	
	return nRC;
}

int CNDKPlayer::OpenExt (int nExtData, int nFlag)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
		
	m_nVideoRndCount = 0;
	m_nVideoRndATime = 0;
	m_nVideoLastSysTime = 0;
	m_nVideoLastBufTime = 0;		
	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
	m_nBmpTime = 0;
		
	int nOpenFlag = YY_OPEN_SRC_READ | YY_OPEN_RNDA_CB | YY_OPEN_RNDV_CB;
	
	int nRC = m_pPlayer->Open (m_pPlayer, (char *)nExtData, nOpenFlag);
	
	return nRC;
}

int CNDKPlayer::Play (void)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	m_nAudioRndCount = 0;
	return m_pPlayer->Run (m_pPlayer);
}

int CNDKPlayer::Pause (void)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	
	return m_pPlayer->Pause (m_pPlayer);
}

int CNDKPlayer::Stop (void)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
		
	m_bDrawBmp = false;
	return m_pPlayer->Stop (m_pPlayer);
}

long CNDKPlayer::GetDuration ()
{
	if (m_pPlayer == NULL)
		return 0;	
		
	return m_pPlayer->GetDur (m_pPlayer);
}

int CNDKPlayer::GetPos (int * pCurPos)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;	
		
	*pCurPos = m_pPlayer->GetPos (m_pPlayer);
		
	return YY_ERR_NONE;
}

int CNDKPlayer::SetPos (int nCurPos)
{
	if (m_pPlayer == NULL)
		return 0;	

	int nRC = m_pPlayer->SetPos (m_pPlayer, nCurPos);
	
	m_nAudioRndCount = 0;
	
	return nRC;
}

int CNDKPlayer::GetParam (JNIEnv* env, int nID, jobject pValue)
{	
	if (nID == PARAM_PID_SunTT_Charset)
	{
		int nCharset = 0;
		m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_Sub_Charset, (void *)&nCharset);
		return nCharset;
	}
	else if (nID == PARAM_PID_SunTT_Enable)
	{
		int nEnable = 0;
		m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_SubTitle, (void *)&nEnable);
		return nEnable;
	}
	else if (nID == PARAM_PID_VIDEODEC_MODE)
	{
		int nVDMode = 0;
		m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_VDMode, (void *)&nVDMode);
		return nVDMode;
	}	
	return 0;
}

int CNDKPlayer::SetParam (JNIEnv* env, int nID, int nParam, jobject pValue)
{	
	if (nID == PARAM_PID_AUDIO_OFFSEST)
	{
		int nOffset = 0;
		if (m_nLatency < 0)
			m_nLatency = yyGetOutputLatency (m_fmtAudio.nSampleRate, m_fmtAudio.nChannels);
		if (nParam & 0X80000000)
		{
			nParam = nParam & 0X7FFFFFFF;			
			if ((nParam + m_nLatency) < 200)
				nOffset = 220;
		}			
		YYLOGI ("The offset time Java %d, Output: %d, Offset: %d, Total: %d", nParam, m_nLatency, nOffset,  nParam + m_nLatency + nOffset);			
		nParam = nParam + m_nLatency + nOffset;
		m_pPlayer->SetParam (m_pPlayer, QCPLAY_PID_Clock_OffTime, (void *)nParam);
		
		return YY_ERR_NONE;
	}
	else if (nID == PARAM_PID_COLOR_FORMAT)
	{
		YYLOGI ("The color format is %d", nParam);
				
		m_nVideoDispColor = nParam;
		
		if (m_pRndWnd != NULL)
			m_pRndWnd->SetColor (m_nVideoDispColor);
	}
	else if (nID == PARAM_PID_SunTT_Enable)
	{
		m_pPlayer->SetParam (m_pPlayer, YYPLAY_PID_SubTitle, (void *)nParam);
	}	
	else if (nID == PARAM_PID_VIDEODEC_MODE)
	{
		m_pPlayer->SetParam (m_pPlayer, YYPLAY_PID_VDMode, (void *)nParam);
	}
	else if (nID == PARAM_PID_PDP_FILE)
	{
		jstring strPDP = (jstring)pValue;
		const char* pPDPFile = env->GetStringUTFChars(strPDP, NULL);
		m_pPlayer->SetParam (m_pPlayer, YYPLAY_PID_PDPFile, (void *)pPDPFile);
		env->ReleaseStringUTFChars(strPDP, pPDPFile);			
	}
	else if (nID == PARAM_PID_EVENT_DONE)
	{
		m_bEventDone = true;
		YYLOGI ("Event had handled! m_bEventDone = %d", m_bEventDone);
	}
	else if (nID == PARAM_PID_BITMAP_VIEW)
	{
		memset (&m_bmpInfo, 0, sizeof (m_bmpInfo));		
		if( m_fGetInfo == NULL)
			LoadBmpFunc ();		
		if( m_jBmpVideo != NULL) 
		{
			env->DeleteGlobalRef (m_jBmpVideo);
			m_jBmpVideo = NULL;
		}
		if (pValue != NULL)
		{		
			m_jBmpVideo = env->NewGlobalRef (pValue);
			m_fGetInfo (env, m_jBmpVideo, &m_bmpInfo);	
			YYLOGI ("BmpInfo: %d X %d", m_bmpInfo.width, m_bmpInfo.height);	
		}
	}
	else if (nID == PARAM_PID_BITMAP_DRAW)
	{
		//YYLOGI ("Bitmap had draw!");
		m_bDrawBmp = true;
	}
		
	return 0;
}

int CNDKPlayer::MediaDataCB (void * pUser, YY_BUFFER * pData)
{
	CNDKPlayer * pPlayer = (CNDKPlayer *)pUser;
	if (pData->nType == YY_MEDIA_Audio)	
		return pPlayer->RenderAudio (pData);	
	else if (pData->nType == YY_MEDIA_Video)	
		return pPlayer->RenderVideo (pData);	
	else if (pData->nType == YY_MEDIA_SubTT)	
		return pPlayer->RenderSubTT (pData);			
	else
		return YY_ERR_FAILED;	
}

int CNDKPlayer::RenderAudio (YY_BUFFER * pData)
{
	if (m_pEnvAudio == NULL)
		m_pjVM->AttachCurrentThread (&m_pEnvAudio, NULL);
	
	if ((pData->uFlag & YYBUFF_EOS) == YYBUFF_EOS)
	{			
		if (m_pEnvAudio != NULL)
		{
			if (m_pDataAudio != NULL)
				m_pEnvAudio->DeleteLocalRef(m_pDataAudio);
			m_pDataAudio = NULL;		
					
			m_pjVM->DetachCurrentThread ();		
			m_pEnvAudio = NULL;
			
			return 0;
		}
	}
	else if ((pData->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT || m_fmtAudio.nSampleRate == 0)
	{			
		YY_AUDIO_FORMAT * pFmtAudio = NULL;
		m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_Fmt_Audio, &pFmtAudio);	
		memcpy (&m_fmtAudio, pFmtAudio, sizeof (m_fmtAudio));		

		if (m_pDataAudio != NULL)
			m_pEnvAudio->DeleteLocalRef(m_pDataAudio);
		m_pDataAudio = NULL;
		m_nBuffSize = 0;	
	}
	
	if (m_pDataAudio == NULL)
		UpdateAudioInfo (m_pEnvAudio, m_fmtAudio.nSampleRate, m_fmtAudio.nChannels);		
	
	jbyte* pBuff = m_pEnvAudio->GetByteArrayElements(m_pDataAudio, NULL) + m_nDataSize;
	
	m_bufAudioRnd.pBuff = (unsigned char *)&pBuff;
	m_bufAudioRnd.uSize = m_nBuffSize - m_nDataSize;
	
	m_dataConvertAudio.pSource = pData;
	m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_ConvertData, &m_dataConvertAudio);
	
	m_nDataSize += m_bufAudioRnd.uSize;
								 
//	if (m_nDataSize > m_nBuffSize / 4) 
	{
		FadeIn (pBuff, m_nDataSize);
		m_pEnvAudio->CallStaticVoidMethod(m_pjCls, m_fPushAudio, m_pjObj, m_pDataAudio, m_nDataSize);	
		m_nDataSize = 0;
	}
	
	m_pEnvAudio->ReleaseByteArrayElements(m_pDataAudio, pBuff, 0);
		
	return 0;
}

int CNDKPlayer::RenderVideo (YY_BUFFER * pData)
{
	if (m_pRndWnd == NULL)
		return YY_ERR_FAILED;
	if ((pData->uFlag & YYBUFF_EOS) == YYBUFF_EOS)
		return YY_ERR_NONE;	
		
	int nRC = 0;
	if ((pData->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
	{
		YY_VIDEO_FORMAT * pFmtVideo = (YY_VIDEO_FORMAT *)pData->pFormat;			
		if (m_fmtVideo.nWidth != pFmtVideo->nWidth || m_fmtVideo.nHeight != pFmtVideo->nHeight ||
			m_fmtVideo.nNum != pFmtVideo->nNum || m_fmtVideo.nDen != pFmtVideo->nDen)
		{			
			JNIEnv* env = NULL;
			m_pjVM->AttachCurrentThread (&env, NULL);						
			UpdateVideoSize (env, pFmtVideo);	
			m_pjVM->DetachCurrentThread ();
		}
		memcpy (&m_fmtVideo, pFmtVideo, sizeof (m_fmtVideo));		
	}
	
	m_nVideoRndCount++;
	if (pData->uFlag & YYBUFF_DEC_RENDER) // It used hardware dec.
	{
		if (m_bBmpShow)
		{
			m_bBmpShow = false;
			JNIEnv* env = NULL;
			m_pjVM->AttachCurrentThread (&env, NULL);						
			env->CallStaticVoidMethod(m_pjCls, m_fPushVideo, m_pjObj, NULL);			
			m_pjVM->DetachCurrentThread ();
		}
		return YY_ERR_NONE;
	}
	if (pData->uFlag & YYBUFF_DEC_SOFT) // It used hardware dec.
	{
		if (m_jBmpVideo == NULL)
			return 0;
		
		while (!m_bDrawBmp)
			usleep (1000);
/*			
		if (yyGetSysTime () - m_nBmpTime < 60)
		{
			int nSleep = (60 - (yyGetSysTime () - m_nBmpTime)) * 1000;
			yySleep (nSleep);
		}
		m_nBmpTime = yyGetSysTime ();
*/					
		JNIEnv* env = NULL;
		m_pjVM->AttachCurrentThread (&env, NULL);
			
		m_bDrawBmp = false;
		unsigned char * pBmpBuff = NULL;
		m_fLockPixels (env, m_jBmpVideo, (void**)&pBmpBuff);		
		//memset (pBmpBuff, m_nVideoRndCount % 255, m_bmpInfo.width * m_bmpInfo.height);
	
		m_bufVideoWnd.nWidth = m_fmtVideo.nWidth;
		m_bufVideoWnd.nHeight = m_fmtVideo.nHeight;	
		m_bufVideoWnd.nType = YY_VDT_ARGB;
		m_bufVideoWnd.pBuff[0] = pBmpBuff;
		m_bufVideoWnd.nStride[0] = m_bmpInfo.stride;	
		
		m_dataConvertVideo.pSource = pData;
		nRC = m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_ConvertData, &m_dataConvertVideo);	
				
		m_fUnlockPixels (env, m_jBmpVideo);			
		
		if (nRC == YY_ERR_NONE)
		{				
			env->CallStaticVoidMethod(m_pjCls, m_fPushVideo, m_pjObj, m_jBmpVideo);			
			m_bBmpShow = true;
		}
		m_pjVM->DetachCurrentThread ();
					
		return 0;
	}	
			
	if (m_nVideoRndCount > 50 && m_nVideoRndATime > 300)
	{ 
		if (pData->llDelay > 50)
		{
			if (yyGetSysTime () - m_nVideoLastSysTime < 100 && pData->llTime - m_nVideoLastBufTime < 90)
			{
				if (m_nVideoRndCount % 2 == 1)
				{
					//YYLOGI ("Drop frame******");
					return YY_ERR_NONE;
				}
			}
		}
	}
		

	int nRndTime = 0;
	if (m_nVideoRndCount <= 50)
		nRndTime = yyGetSysTime ();		
	
	if (m_nVideoRndCount <= 1)
	{
		if ((pData->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
		{
			AVFrame * pFrmVideo = (AVFrame *)pData->pBuff;;
			if (pFrmVideo->format != AV_PIX_FMT_YUV420P)
				m_pRndWnd->SetColor (1);
		}
	}
		
	m_bufVideoWnd.nWidth = m_fmtVideo.nWidth;
	m_bufVideoWnd.nHeight = m_fmtVideo.nHeight;	
	nRC = m_pRndWnd->Lock (m_fmtVideo.nWidth, m_fmtVideo.nHeight, &m_bufVideoWnd);	
	if (nRC == 0)
	{
		m_dataConvertVideo.pSource = pData;
		nRC = m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_ConvertData, &m_dataConvertVideo);	
		m_pRndWnd->Unlock ();
		
		m_nVideoLastSysTime = yyGetSysTime ();
		m_nVideoLastBufTime = pData->llTime;			
	}
	
	if (m_nVideoRndCount <= 50)
	{
		nRndTime = yyGetSysTime () - nRndTime;
		m_nVideoRndATime += nRndTime;
	}
	
	if (m_nVideoRndCount == 50)
		YYLOGI ("Video Render Time: %d", m_nVideoRndATime);	
			
	return 0;
}

int CNDKPlayer::RenderSubTT (YY_BUFFER * pData)
{	
	if (m_pEnvSubTT == NULL)
		m_pjVM->AttachCurrentThread (&m_pEnvSubTT, NULL);
	
	if ((pData->uFlag & YYBUFF_EOS) == YYBUFF_EOS)
	{		
		if (m_pEnvSubTT != NULL)
		{	
			m_pjVM->DetachCurrentThread ();		
			m_pEnvSubTT = NULL;	
			return 0;
		}
	}
	
	if (pData->uSize > 0)
	{	
		jbyteArray txtBytes = m_pEnvSubTT->NewByteArray(pData->uSize);		
		jbyte* pBuff = m_pEnvSubTT->GetByteArrayElements(txtBytes, NULL);
		memcpy (pBuff, pData->pBuff, pData->uSize);
		m_pEnvSubTT->CallStaticVoidMethod(m_pjCls, m_fPushTTByte, m_pjObj, txtBytes, pData->uSize, pData->nValue);
		m_pEnvSubTT->ReleaseByteArrayElements(txtBytes, pBuff, 0);
		m_pEnvSubTT->DeleteLocalRef (txtBytes);
	}
	else
	{
		m_pEnvSubTT->CallStaticVoidMethod(m_pjCls, m_fPushTTByte, m_pjObj, NULL, pData->uSize, pData->nValue);
	}
	return 0;
}

void CNDKPlayer::FadeIn (void * pData, int nSize)
{
//	YYLOGI ("Data = %p, size = %d", pData, nSize);
	if (pData == NULL)
		return;
		
	int nStep = 10;
	if (m_nAudioRndCount < nStep)
	{
		long long	nAudio = 0;
		short * pAudio = (short *)pData;
		int nBlocks = nSize / 2;
		for (int i = 0; i < nBlocks; i++)
		{
			nAudio = *pAudio;
			nAudio = (nAudio * (i +  nBlocks * m_nAudioRndCount)) / (nBlocks * nStep);
			*pAudio++ = (short)nAudio;
		}
		
		//YYLOGI ("Fade in audio after seek! Time: %d", m_nAudioRndCount);
	}
	m_nAudioRndCount++;
	//memset (pData, 0, nSize);
}

int CNDKPlayer::LoadBmpFunc (void)
{
	if (m_hGraphics == NULL)
		m_hGraphics = dlopen("libjnigraphics.so" , RTLD_NOW );
	if (m_hGraphics == NULL)
		return -1;
	if( m_fGetInfo == NULL)
	{
		m_fGetInfo = (bitmap_getInfo)dlsym (m_hGraphics , "AndroidBitmap_getInfo" );
		m_fLockPixels = (bitmap_lockPixels) dlsym (m_hGraphics , "AndroidBitmap_lockPixels");
		m_fUnlockPixels = (bitmap_unlockPixels) dlsym (m_hGraphics , "AndroidBitmap_unlockPixels");
	}
	return 0;
}

void CNDKPlayer::NotifyEvent (void * pUserData, int nID, void * pV1)
{
	CNDKPlayer * pPlayer = (CNDKPlayer *)pUserData;
	pPlayer->HandleEvent (nID, pV1);	
}

void CNDKPlayer::HandleEvent (int nID, void * pV1)
{
	if (m_fPostEvent == NULL)
		return;
			
	JNIEnv * env = NULL;
	m_pjVM->AttachCurrentThread (&env, NULL);	
	
	if (nID == YY_EV_Play_Complete)
	{
		//YYLOGI ("Playback completed!");	
		env->CallStaticVoidMethod(m_pjCls, m_fPostEvent, m_pjObj, YY_EV_Play_Complete, 0, 0, NULL);		
	}
	else if (nID == YY_EV_Open_Complete)
	{	
		YY_VIDEO_FORMAT * pFmtVideo = NULL;
		m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_Fmt_Video, &pFmtVideo);
		if (pFmtVideo != NULL)
		{		
			memcpy (&m_fmtVideo, pFmtVideo, sizeof (m_fmtVideo));				
			UpdateVideoSize (env, &m_fmtVideo);
		}					
		env->CallStaticVoidMethod(m_pjCls, m_fPostEvent, m_pjObj, YY_EV_Open_Complete, 0, 0, NULL);		
	}
	else if (nID == YY_EV_Open_Failed)
	{
		YYLOGI ("Open Failed!");		
		env->CallStaticVoidMethod(m_pjCls, m_fPostEvent, m_pjObj, YY_EV_Open_Failed, 0, 0, NULL);		
	}
	else if (nID == YY_EV_Play_Duration)
	{
		YYLOGI ("Duration changed!");		
		env->CallStaticVoidMethod(m_pjCls, m_fPostEvent, m_pjObj, YY_EV_Play_Duration, (int)pV1, 0, NULL);		
	}
	else if (nID == YY_EV_Play_Status)
	{
		YYLOGI ("Status Changed to %d", (int)pV1);		
		env->CallStaticVoidMethod(m_pjCls, m_fPostEvent, m_pjObj, YY_EV_Play_Status, (int)pV1, 0, NULL);		
	}
	else if (nID == YY_EV_Create_ExtVD)
	{
		YYLOGI ("Create ext video dec and rnd!");		
		env->CallStaticVoidMethod(m_pjCls, m_fPostEvent, m_pjObj, YY_EV_Create_ExtVD, 0, 0, NULL);		
	}			
	else if (nID == YY_EV_Create_ExtAD)
	{
		YYLOGI ("Create ext audio dec and rnd!");		
		env->CallStaticVoidMethod(m_pjCls, m_fPostEvent, m_pjObj, YY_EV_Create_ExtAD, 0, 0, NULL);		
	}
	m_pjVM->DetachCurrentThread ();		
}

void CNDKPlayer::UpdateVideoSize (JNIEnv* env, YY_VIDEO_FORMAT * pFmtVideo)
{
	if (m_fPostEvent == NULL || pFmtVideo == NULL)
		return;
		
	YYLOGI ("Update Video Size: %d X %d  Ratio: %d, %d", pFmtVideo->nWidth, pFmtVideo->nHeight, pFmtVideo->nNum, pFmtVideo->nDen);
	int nWidth = pFmtVideo->nWidth;
	int nHeight = pFmtVideo->nHeight;
	if ((pFmtVideo->nNum == 0 || pFmtVideo->nNum == 1) &&
		(pFmtVideo->nDen == 1 || pFmtVideo->nDen == 0))
	{
		nWidth = pFmtVideo->nWidth;
		nHeight = pFmtVideo->nHeight;
	}
	else
	{
		if (pFmtVideo->nDen == 0)
			pFmtVideo->nDen = 1;
		if (pFmtVideo->nNum == 0)
			pFmtVideo->nNum == 1;
		if (pFmtVideo->nNum  < pFmtVideo->nDen)
			nWidth = nWidth * pFmtVideo->nNum / pFmtVideo->nDen;
		else
			nHeight = nHeight * pFmtVideo->nDen / pFmtVideo->nNum;
	}	
			
	m_bEventDone = false;
	env->CallStaticVoidMethod(m_pjCls, m_fPostEvent, m_pjObj, 
								YY_EV_VideoFormat_Change, nWidth, nHeight, NULL);									
	while (!m_bEventDone)
		usleep (1000);	
}
	
void CNDKPlayer::UpdateAudioInfo (JNIEnv* env, int nSampleRate, int nChannels)
{	
	YYLOGI ("New audio format % 8d X % 8d", nSampleRate, nChannels);
	if (m_fPostEvent == NULL)
		return;
			
	env->CallStaticVoidMethod(m_pjCls, m_fPostEvent, m_pjObj, 
								YY_EV_AudioFormat_Change, nSampleRate , nChannels, NULL);	
								
	if (nSampleRate * nChannels > m_nBuffSize || m_pDataAudio == NULL)
	{
		if (m_pDataAudio != NULL)
			env->DeleteLocalRef(m_pDataAudio);
		m_pDataAudio = NULL;
		
		m_nBuffSize = nSampleRate * nChannels;
		m_nDataSize = 0;
		m_pDataAudio = env->NewByteArray(m_nBuffSize);	
		
//		YYLOGI ("Func %p, Update Audio info %p, %d", m_fPostEvent, m_pDataAudio, m_nDataSize);	
	}	
}

int CNDKPlayer::GetThumb (JNIEnv* env, jobject obj, const char * pFile, int nWidth, int nHeight, jobject objBitmap)
{
	YYLOGI ("Getthumb %d X %d", nWidth, nHeight);
	
	if( m_fGetInfo == NULL)
		LoadBmpFunc ();
	
	memset (&m_sThumb, 0, sizeof (m_sThumb));
	m_sThumb.nInfoType = YYINFO_Get_Thumbnail;
	m_sThumb.nThumbWidth = nWidth;
	m_sThumb.nThumbHeight = nHeight;
	m_sThumb.nPos = 10000;
	
	void * pThumbImg = m_pPlayer->GetThumb (m_pPlayer, pFile, &m_sThumb);
	YYLOGI ("BMP %p, Buffer %p", pThumbImg, m_sThumb.pBmpBuff);
	if (pThumbImg == NULL)
		return -1;
	
	
	AndroidBitmapInfo info;
	m_fGetInfo (env, objBitmap , &info );	
	YYLOGI ("Info %d, %d, %d %d %d", info.width, info.height, info.stride, info.format, info.flags);
	
	unsigned char * pBmpBuff = NULL;
	m_fLockPixels (env, objBitmap, (void**)&pBmpBuff);
	YYLOGI ("Buffer %p 111", pBmpBuff);
	
	memcpy (pBmpBuff, m_sThumb.pBmpBuff, nWidth * nHeight * 4);
		
	m_fUnlockPixels (env, objBitmap);
				
	return YY_ERR_NONE;
}

int CNDKPlayer::ReadVideoBuff (JNIEnv* env, jobject obj, jbyteArray data)
{
	if (m_RndCodec == NULL)
		m_RndCodec = new CMediaCodecRnd (m_pPlayer);
	return m_RndCodec->ReadVideoBuff (env, obj, data);
}

int CNDKPlayer::ReadAudioBuff (JNIEnv* env, jobject obj, jbyteArray data)
{
	if (m_RndCodec == NULL)
		m_RndCodec = new CMediaCodecRnd (m_pPlayer);	
	return m_RndCodec->ReadAudioBuff (env, obj, data);

}
