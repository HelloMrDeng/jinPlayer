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
#include "UJNIFunc.h"
#include "yyLog.h"

#define YY_EV_AudioFormat_Change	0X22

CNDKPlayer::CNDKPlayer ()
	: CMultiPlayer ()
	, m_pjVM (NULL)
	, m_pjCls (NULL)
	, m_pjObj (NULL)
	, m_fPostEvent (NULL)
	, m_fPushAudio (NULL)
	, m_pEnvAudio (NULL)
	, m_pDataAudio (NULL)
	, m_nBuffSize (0)
	, m_nDataSize (0)
{
	SetObjectName ("CNDKPlayer");	
	memset (m_szPath, 0, sizeof (m_szPath));
}

CNDKPlayer::~CNDKPlayer ()
{
	Uninit (NULL);
}

int CNDKPlayer::Init (JavaVM * jvm, JNIEnv* env, jclass clsPlayer, jobject objPlayer, char * pPath)
{
	m_pjVM = jvm;
	m_pjCls = clsPlayer;
	m_pjObj = objPlayer;
	
	m_fPostEvent = env->GetStaticMethodID (m_pjCls, "postEventFromNative", "(Ljava/lang/Object;IIILjava/lang/Object;)V");
	m_fPushAudio = env->GetStaticMethodID (m_pjCls, "audioDataFromNative", "(Ljava/lang/Object;[BI)V");
	YYLOGI ("Audio Render method = %p", m_fPushAudio);
			
	strcpy (m_szPath, pPath);

	return YY_ERR_NONE;	
}

int CNDKPlayer::Uninit (JNIEnv* env)
{	
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

int CNDKPlayer::Open (const char* szUrl)
{			
	if (!m_lsnInfo.Open ((char *)szUrl))
		return YY_ERR_FAILED;			
	int nRC = CMultiPlayer::Open (&m_lsnInfo);	
	return nRC;
}

int CNDKPlayer::Run (void)
{
	int nRC = CMultiPlayer::Run ();
	if (m_pWorkAudio != NULL)
		m_pWorkAudio->SetStartStopProc (AudioPlayStart, AudioPlayStop);
	return nRC;
}

int CNDKPlayer::GetParam (JNIEnv* env, int nID, jobject pValue)
{	
	return 0;
}

int CNDKPlayer::SetParam (JNIEnv* env, int nID, int nParam, jobject pValue)
{	
	return 0;
}

int CNDKPlayer::HandleEvent (int nID, void * pV1)
{
	JNIEnv * env = NULL;	
	if (nID == YY_EV_Play_Complete)
	{
		if (m_bPrevPlay)
		{
			m_pPrevEng->Pause ();
			m_pPrevEng->SetPos (0);
			m_nPrevComp = 1;
			m_bPrevPlay = false;
			return YY_ERR_NONE;
		}
		else
		{	
			m_nPlayComp++;
			if (m_nPlayComp == m_nEngs)
			{
				m_nPlayComp = 0;
				Pause ();
				SetPos (0);
			}
		}
		m_pjVM->AttachCurrentThread (&env, NULL);
		env->CallStaticVoidMethod(m_pjCls, m_fPostEvent, m_pjObj, YY_EV_Play_Complete, 0, 0, NULL);	
		m_pjVM->DetachCurrentThread ();	
		return YY_ERR_NONE;
	}	
	else if (nID == YY_EV_Play_Duration)
	{
		YYLOGI ("Duration changed!");		
		m_pjVM->AttachCurrentThread (&env, NULL);
		env->CallStaticVoidMethod(m_pjCls, m_fPostEvent, m_pjObj, YY_EV_Play_Duration, (int)pV1, 0, NULL);		
		m_pjVM->DetachCurrentThread ();
	}
		
	
	return CMultiPlayer::HandleEvent (nID, pV1);
}

int CNDKPlayer::WriteAudio (void)
{	
	if (m_pDataAudio == NULL)
		UpdateAudioInfo (m_pEnvAudio, m_fmtAudio.nSampleRate, m_fmtAudio.nChannels);		

	jbyte* pBuff = m_pEnvAudio->GetByteArrayElements(m_pDataAudio, NULL) + m_nDataSize;
	memcpy (pBuff, m_pRndBuff, m_bufAudioRnd.uSize);
	m_pEnvAudio->CallStaticVoidMethod(m_pjCls, m_fPushAudio, m_pjObj, m_pDataAudio, m_bufAudioRnd.uSize);		
	m_pEnvAudio->ReleaseByteArrayElements(m_pDataAudio, pBuff, 0);
	
	return YY_ERR_NONE;	
}

int CNDKPlayer::AudioPlayStart (void * pParam)
{
	CNDKPlayer * pPlayer = (CNDKPlayer *)pParam;
	if (pPlayer->m_pEnvAudio == NULL)
		pPlayer->m_pjVM->AttachCurrentThread (&pPlayer->m_pEnvAudio, NULL);
			
	return YY_ERR_NONE;
}
	
int CNDKPlayer::AudioPlayStop (void * pParam)
{
	CNDKPlayer * pPlayer = (CNDKPlayer *)pParam;	
	if (pPlayer->m_pEnvAudio != NULL)
	{
		if (pPlayer->m_pDataAudio != NULL)
			pPlayer->m_pEnvAudio->DeleteLocalRef(pPlayer->m_pDataAudio);
		pPlayer->m_pDataAudio = NULL;		
				
		pPlayer->m_pjVM->DetachCurrentThread ();		
		pPlayer->m_pEnvAudio = NULL;
	}
	return YY_ERR_NONE;
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
	}	
}
