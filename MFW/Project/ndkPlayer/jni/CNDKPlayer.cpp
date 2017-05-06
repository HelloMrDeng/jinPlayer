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

#include "USystemFunc.h"
#include "yyLog.h"
#include "yyBox.h"

#define PARAM_PID_AUDIO_OFFSEST		0X300
#define PARAM_PID_PREPARE_CLOSE		0X500

CNDKPlayer::CNDKPlayer ()
	: m_pjVM (NULL)
	, m_pjObj (NULL)
	, m_pView (NULL)
	, m_pRndWnd (NULL)
	, m_hDllBase (NULL)		
	, m_hDllMedia (NULL)
	, m_fInit (NULL)
	, m_fCreate (NULL)
	, m_fDestroy (NULL)
	, m_fUninit (NULL)
	, m_hMediaEng (NULL)
	, m_pPlayer (NULL)	
	, m_pBoxVideo (NULL)
	, m_nVideoRndCount (0)
	, m_nVideoRndATime (0)
	, m_nVideoLastSysTime (0)
	, m_nVideoLastBufTime (0)
	, m_pBoxAudio (NULL)	
	, m_nAudioOffsetTime (0)
	, m_nOpenStatus (0)
	, m_bEOS (false)
	, m_bAudioEOS (true)
	, m_bVideoEOS (true)
{
	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));	
	
	memset (&m_bufVideo, 0, sizeof (m_bufVideo));
	memset (&m_bufVideoRnd, 0, sizeof (m_bufVideoRnd));
	memset (&m_bufVideoCvt, 0, sizeof (m_bufVideoCvt));	
	
	memset (&m_bufAudio, 0, sizeof (m_bufAudio));
	memset (&m_fmtAudio, 0, sizeof (m_fmtAudio));		
}

CNDKPlayer::~CNDKPlayer ()
{
	Uninit ();
}

int CNDKPlayer::Init (char * pPath)
{
	if (m_hDllMedia != NULL)
		return 0;
		
	char szPath[256];
	strcpy (szPath, pPath);
	YYLOGI ("%s", pPath);
		
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
			YYLOGE ("The meida Dll could not be loaded!");
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

	return YY_ERR_NONE;	
}

int CNDKPlayer::Uninit ()
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
	{
		dlclose (m_hDllBase);
		YYLOGI ("Exit ndk Player!");
		
	}
	m_hDllBase = NULL;
	
	if (m_pRndWnd != NULL)
		delete m_pRndWnd;	
	m_pRndWnd = NULL;
	
	YYNDK_EVENT * pEvent = m_lstFullEvent.RemoveHead ();
	while (pEvent != NULL)
	{
		delete pEvent;
		pEvent = m_lstFullEvent.RemoveHead ();
	}
	pEvent = m_lstFreeEvent.RemoveHead ();
	while (pEvent != NULL)
	{
		delete pEvent;
		pEvent = m_lstFreeEvent.RemoveHead ();
	}	
		
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
		
	m_pRndWnd->Init (env, pView);
		
	return YY_ERR_NONE;
}

jobject CNDKPlayer::GetView ()
{
	return m_pView;
}

int CNDKPlayer::Open (const char* szUrl)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	
	m_nOpenStatus = 0;
	m_bEOS = false;
	m_nVideoRndCount = 0;
	m_nVideoRndATime = 0;
	m_nVideoLastSysTime = 0;
	m_nVideoLastBufTime = 0;
	
	int nFlag = YY_OPEN_RNDA_EXT | YY_OPEN_RNDV_EXT;
	
	int nRC = m_pPlayer->Open (m_pPlayer, szUrl, nFlag);
	
	return nRC;
}


int CNDKPlayer::OpenExt (int nExtData, int nFlag)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	
	m_nOpenStatus = 0;
	m_bEOS = false;
	m_nVideoRndCount = 0;
	m_nVideoRndATime = 0;
	m_nVideoLastSysTime = 0;
	m_nVideoLastBufTime = 0;
	
	int nOpenFlag = YY_OPEN_SRC_READ | YY_OPEN_RNDA_EXT | YY_OPEN_RNDV_EXT;
	
	int nRC = m_pPlayer->Open (m_pPlayer, (char *)nExtData, nOpenFlag);
	
	return nRC;
}

int CNDKPlayer::Play (void)
{
	if (m_pPlayer == NULL)
		return YY_ERR_STATUS;
	
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
	
	m_nVideoRndCount = 0;
		
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

	m_bEOS = false;
	if (m_pBoxAudio != NULL)
		m_bAudioEOS = false;
	if (m_pBoxVideo != NULL)
		m_bVideoEOS = false;

	int nRC = m_pPlayer->SetPos (m_pPlayer, nCurPos);
	
	if (m_pPlayer->GetStatus (m_pPlayer) == YY_PLAY_Pause)
	{
		if (m_pBoxVideo == NULL || m_pRndWnd == NULL)
			return nRC;
			
		m_pBoxVideo->Start ();
		
		m_bufVideo.nType = YY_MEDIA_Video;
		int nRC = 0;
		int nRndNum = 0;
		while (true)
		{	
			nRC = m_pBoxVideo->ReadBuffer (&m_bufVideo, false);		
			if (nRC == YY_ERR_NONE)
			{
				nRndNum++;
				if (nRndNum <= 3)
					continue;
					
				nRC = m_pRndWnd->Lock (m_fmtVideo.nWidth, m_fmtVideo.nHeight, &m_bufVideoRnd);
				if (nRC == 0)
				{
					nRC = m_pBoxVideo->Convert (&m_bufVideo, &m_bufVideoCvt);	
					m_pRndWnd->Unlock ();
					break;
				}
			}
			
			if (m_pPlayer->GetStatus (m_pPlayer) != YY_PLAY_Pause)
				break;		
		}
		
		if (m_pPlayer->GetStatus (m_pPlayer) == YY_PLAY_Pause)
			m_pBoxVideo->Pause ();

	}
	
	return nRC;
}

int CNDKPlayer::GetVideoData (JNIEnv* env, jobject obj, unsigned char * pData, int nStride)
{
//	YYLOGI ("GetVideoData  m_bVideoEOS %d", m_bVideoEOS);
	if (m_bVideoEOS)
	{
//		m_bEOS = false;
//		int nParam = 1;
//		callbackEvent (env, obj, YY_EV_Play_Complete, &nParam);
		usleep (100000);
		return YY_ERR_FINISH;
	}
	
	if (m_pBoxVideo == NULL || m_pRndWnd == NULL)
		return YY_ERR_STATUS;
	
		
	m_bufVideo.nType = YY_MEDIA_Video;
			
	int nRC = m_pBoxVideo->ReadBuffer (&m_bufVideo, true);
//	YYLOGI ("Video Time % 08d, nRC = % 8d", (int)m_bufVideo.llTime, nRC);
	if (nRC == YY_ERR_FINISH)
	{
		m_bVideoEOS = true;
		YYLOGI ("Video is EOS!");
	}
	
	if (nRC == YY_ERR_NONE)
	{
		m_nVideoRndCount++;
					
		if (m_nVideoRndCount > 50 && m_nVideoRndATime > 300)
		{ 		
			if (m_bufVideo.llDelay > 30)
			{
				if (yyGetSysTime () - m_nVideoLastSysTime < 100 && m_bufVideo.llTime - m_nVideoLastBufTime < 90)
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
					
		nRC = m_pRndWnd->Lock (m_fmtVideo.nWidth, m_fmtVideo.nHeight, &m_bufVideoRnd);
		if (nRC == 0)
		{
			nRC = m_pBoxVideo->Convert (&m_bufVideo, &m_bufVideoCvt);					
			m_pRndWnd->Unlock ();
						
			m_nVideoLastSysTime = yyGetSysTime ();
			m_nVideoLastBufTime = m_bufVideo.llTime;
		}
		
		if (m_nVideoRndCount <= 50)
		{
			nRndTime = yyGetSysTime () - nRndTime;
			m_nVideoRndATime += nRndTime;
		}
		
		if (m_nVideoRndCount == 50)
			YYLOGI ("Video Render Time: %d", m_nVideoRndATime);
	}
	
	return YY_ERR_NONE;
}


int CNDKPlayer::GetVideoSize (int* nWidth, int* nHeight)
{	
	*nWidth = m_fmtVideo.nWidth;
	*nHeight = m_fmtVideo.nHeight;
	
	return YY_ERR_NONE;
}

int CNDKPlayer::GetAudioData (JNIEnv* env, jobject obj, unsigned char ** ppData,int nSize)
{			
	if (m_bAudioEOS)
	{
//		m_bEOS = false;
//		int nParam = 1;
//		callbackEvent (env, obj, YY_EV_Play_Complete, &nParam);
		usleep (100000);
		return -1;
	}

	if (m_pBoxAudio == NULL)
	{
		usleep (100000);
		return -1;
	}
		
	m_bufAudio.nType = YY_MEDIA_Audio;		
	int nRC = m_pBoxAudio->ReadBuffer (&m_bufAudio, false);	
	if (nRC == YY_ERR_FINISH)
		m_bAudioEOS = true;
	if (nRC != YY_ERR_NONE)
		return -1;
		
	if ((m_bufAudio.uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
	{
		YY_AUDIO_FORMAT * pFmtAudio = m_pBoxAudio->GetAudioFormat ();
		memcpy (&m_fmtAudio, pFmtAudio, sizeof (m_fmtAudio));
		updateAudioFormat (env, m_pjObj, m_fmtAudio.nSampleRate, m_fmtAudio.nChannels);
		return 0;
	}
		
		
	if (ppData == NULL)
		return -1;	
		
	m_bufAudioCvt.uFlag = YYBUFF_TYPE_PPOINTER;
	m_bufAudioCvt.pBuff = (unsigned char *)ppData;
	m_bufAudioCvt.uSize = nSize;
	m_bufAudioCvt.pFormat = &m_fmtAudio;
	nRC = m_pBoxAudio->Convert (&m_bufAudio, &m_bufAudioCvt);
	if (nRC != YY_ERR_NONE)
	{
		return -1;
	}

	return m_bufAudioCvt.uSize;
}

int CNDKPlayer::GetAudioFormat (int* nSampleRate,int* nChannels)
{
	*nSampleRate = m_fmtAudio.nSampleRate;
	*nChannels = m_fmtAudio.nChannels;
	
	return YY_ERR_NONE;
}

int CNDKPlayer::GetEventStatus (JNIEnv* env, jobject obj)
{	
	if (m_lstFullEvent.GetCount () <= 0)
	{
		usleep (10000);
		return YY_ERR_NONE;
	}
		
	CAutoLock lock (&m_mtEvent);
	YYNDK_EVENT * pEvent = m_lstFullEvent.RemoveHead ();

//	YYLOGI ("Event ID %d, value %p", pEvent->nID, pEvent->pV1);
	
	if (pEvent->nID == YY_EV_Open_Complete)  	
	{
		if (m_pBoxVideo != NULL)
		{
			if ((m_fmtVideo.nNum == 0 || m_fmtVideo.nNum == 1) && m_fmtVideo.nDen == 1)		
			{	
				updateVideoSize (env, m_pjObj, m_fmtVideo.nWidth, m_fmtVideo.nHeight);
			}
			else
			{
				if (m_fmtVideo.nDen == 0)
					m_fmtVideo.nDen = 1;
				int nWidth = m_fmtVideo.nWidth * m_fmtVideo.nNum / m_fmtVideo.nDen;
				updateVideoSize (env, m_pjObj, nWidth, m_fmtVideo.nHeight);
			}
			YYLOGI ("Update the video size %d X %d", m_fmtVideo.nWidth, m_fmtVideo.nHeight);
		}
		
		updateAudioFormat (env, m_pjObj, m_fmtAudio.nSampleRate, m_fmtAudio.nChannels);
		YYLOGI ("Update the audio format %d X %d", m_fmtAudio.nSampleRate, m_fmtAudio.nChannels);
	}
  			
  	int nTmpParam = 1;	
	callbackEvent (env, obj, pEvent->nID, &nTmpParam);
	
	m_lstFreeEvent.AddTail (pEvent);
	
	return YY_ERR_NONE;	
}

int CNDKPlayer::FillInfo (void)
{
	if (m_pBoxVideo == NULL)
	{
		int nIndex = 0;
		CBoxBase * pBox = (CBoxBase *)m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_BOX, (void *)nIndex);
		while (pBox != NULL)
		{
			if (!strcmp (pBox->GetName (), "Video Render Box Ext"))
			{
				m_pBoxVideo = pBox;
				//YYLOGI ("Find the video box! %p", m_pBoxVideo);
				break;
			}
			
			nIndex++;
			pBox = (CBoxBase *)m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_BOX, (void *)nIndex);
		}
		
		if (m_pBoxVideo != NULL)
		{
			YY_VIDEO_FORMAT * pFmtVideo = m_pBoxVideo->GetVideoFormat ();
			memcpy (&m_fmtVideo, pFmtVideo, sizeof (m_fmtVideo));	
			
			m_bVideoEOS = false;			
		}		
		
		m_bufVideoRnd.nType = YY_VDT_ARGB;
		m_bufVideoRnd.nWidth = m_fmtVideo.nWidth;
		m_bufVideoRnd.nHeight = m_fmtVideo.nHeight;
	
		m_bufVideoCvt.uFlag = YYBUFF_TYPE_VIDEO;
		m_bufVideoCvt.pBuff = (unsigned char *)&m_bufVideoRnd;
		m_bufVideoCvt.uSize = sizeof (m_bufVideoRnd);
	}
	
	if (m_pBoxAudio == NULL)
	{
		int nIndex = 0;
		CBoxBase * pBox = (CBoxBase *)m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_BOX, (void *)nIndex);
		while (pBox != NULL)
		{
			if (!strcmp (pBox->GetName (), "Audio Render Box Ext"))
			{
				m_pBoxAudio = pBox;
				//YYLOGI ("Find the Audio box! %p", m_pBoxAudio);
				break;
			}
			
			nIndex++;
			pBox = (CBoxBase *)m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_BOX, (void *)nIndex);
		}
		
		if (m_pBoxAudio != NULL)
		{
			YY_AUDIO_FORMAT * pFmtAudio = m_pBoxAudio->GetAudioFormat ();
			memcpy (&m_fmtAudio, pFmtAudio, sizeof (m_fmtAudio));
			
			if (m_nAudioOffsetTime != 0)
			{
				CBaseClock * pClock = m_pBoxAudio->GetClock ();
				if (pClock != NULL)
					pClock->SetOffset (m_nAudioOffsetTime);
			}
				
			m_bAudioEOS = false;
		}
	}	
		
	return YY_ERR_NONE;
}

int CNDKPlayer::GetParam (JNIEnv* env, int nID, jobject pValue)
{	
	return 0;
}

int CNDKPlayer::SetParam (JNIEnv* env, int nID, jobject pValue)
{
//	YYLOGI ("SetParam ID  %d", nID);
	
	if (nID == PARAM_PID_AUDIO_OFFSEST)
	{
		m_nAudioOffsetTime = getIntegerValue (env, pValue);	
		YYLOGI ("The offset time is %d", m_nAudioOffsetTime);
		if (m_pBoxAudio != NULL)
		{
			CBaseClock * pClock = m_pBoxAudio->GetClock ();
			if (pClock != NULL)
				pClock->SetOffset (m_nAudioOffsetTime);
		}
		
		return YY_ERR_NONE;
	}
	else if (nID == PARAM_PID_PREPARE_CLOSE)
	{
		m_pPlayer->SetParam (m_pPlayer, YYPLAY_PID_Prepare_Close, 0);
		return YY_ERR_NONE;	
	}
	
	return 0;
}

void CNDKPlayer::SetJavaVM (JavaVM * jvm, jobject envobj)
{	
	m_pjVM = jvm;
	m_pjObj = envobj;
}

void* CNDKPlayer::GetJavaObj ()
{
	return m_pjObj;
}

void CNDKPlayer::NotifyEvent (void * pUserData, int nID, void * pV1)
{
	CNDKPlayer * pPlayer = (CNDKPlayer *)pUserData;
	if (nID == YY_EV_Play_Complete)
	{
		YYLOGI ("Playback completed!");	
		pPlayer->m_bEOS = true;
	}
	else if (nID == YY_EV_Open_Complete)
	{
		YYLOGI ("Open completed!");	
	
		pPlayer->FillInfo ();			
		pPlayer->m_nOpenStatus = 1;	
//		pPlayer->jniEventCallBack (nID, pV1);
	}
	else if (nID == YY_EV_Open_Failed)
	{
		YYLOGI ("Open Failed!");
		pPlayer->m_nOpenStatus = -1;			
	}
	
	CAutoLock lock (&pPlayer->m_mtEvent);
	YYNDK_EVENT * pEvent = pPlayer->m_lstFreeEvent.RemoveHead ();
	if (pEvent == NULL)
		pEvent = new YYNDK_EVENT ();
	pEvent->nID = nID;
	pEvent->pV1 = pV1;
	pPlayer->m_lstFullEvent.AddTail (pEvent);
}

int CNDKPlayer::jniEventCallBack (int nId, void * pParam)
{
	if (m_pjVM == NULL || m_pjObj == NULL)
	{
		YYLOGE ("The JVM or Object is NULL!");
		return YY_ERR_FAILED;
	}
		
	bool		bAttached = false;
	int 		status;
	JNIEnv *	env = NULL;		
	jint jniver = JNI_VERSION_1_4;
	if (m_pjVM->GetEnv((void**) &env, jniver) != JNI_OK) 
	{
		jniver = JNI_VERSION_1_6;
		if (m_pjVM->GetEnv((void**) &env, jniver) != JNI_OK) 
		{
			YYLOGI ("It can't get env pointer in call thread!");
		}
	}
	
	if (env == NULL)
	{
	  	status = m_pjVM->AttachCurrentThread (&env, NULL);	
	  	if (status < 0)
	  	{
	  		YYLOGE ("Attach thread failed!");
	  		return YY_ERR_FAILED;
	  	}
	  	bAttached = true;
		YYLOGI ("The Env from attach is %p", env);  	
	}
	
	if (nId == YY_EV_Open_Complete)  	
	{
//		updateVideoSize (env, m_pjObj, m_fmtVideo.nWidth, m_fmtVideo.nHeight);
		YYLOGI ("Update the video size %d X %d", m_fmtVideo.nWidth, m_fmtVideo.nHeight);
		
//		updateAudioFormat (env, m_pjObj, m_fmtAudio.nSampleRate, m_fmtAudio.nChannels);
		YYLOGI ("Update the audio format %d X %d", m_fmtAudio.nSampleRate, m_fmtAudio.nChannels);
	}
  			
  	int nTmpParam = 1;
  	if (pParam == NULL)
  		pParam = &nTmpParam;
  		
	callbackEvent (env, m_pjObj, nId, &nTmpParam);
	
	if (bAttached)
		m_pjVM->DetachCurrentThread ();
	
	return YY_ERR_NONE;
} 