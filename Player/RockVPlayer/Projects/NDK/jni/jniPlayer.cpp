/*******************************************************************************
	File:		jniPlayer.cpp

	Contains:	yy demo player jni implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <jni.h>

#include "CNDKPlayer.h"
#include "yyLog.h"


#define g_szRockVPlayerName "com/cansure/mediaEngine/MediaPlayer"


// Java Bindings
static jint native_init (JNIEnv* env, jobject obj, jobject player, jstring apkPath) 
{
	JavaVM * 	jvm = 0;
	jobject 	envobj;
	
	env->GetJavaVM(&jvm);
	
	jclass clazz = env->GetObjectClass(obj);
	jclass clsPlayer = (jclass)env->NewGlobalRef(clazz);
	jobject objPlayer  = env->NewGlobalRef(player);	
	
	char * strPath = (char *) env->GetStringUTFChars(apkPath, NULL);
	
	CNDKPlayer * pPlayer = new CNDKPlayer ();
	int nRC = pPlayer->Init(jvm, env, clsPlayer, objPlayer, strPath);
	env->ReleaseStringUTFChars(apkPath, strPath);	
	env->DeleteLocalRef(clazz);	
	if (nRC) 
	{
		delete pPlayer;
		return 0;
	}	
	return (int)pPlayer;
}

static jint native_uninit(JNIEnv* env, jobject obj, jint nNativeContext) 
{	
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	pPlayer->Uninit(env);

	delete pPlayer;

	YYLOGT ("jniPlayer", "It was Safe to exit ndk player!");
	
	return 0;
}

static jint native_open (JNIEnv* env, jobject obj, jint nNativeContext, jstring strUrl) 
{		
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	int nRC = 0;
	if (strUrl != NULL) 
	{
		const char* pSource = env->GetStringUTFChars(strUrl, NULL);
		YYLOGT ("jniPlayer", "Open source: %s", pSource);
		
		nRC = pPlayer->Open (pSource);
		env->ReleaseStringUTFChars(strUrl, pSource);
	} 
	
	return nRC;
}

static jint native_play (JNIEnv* env, jobject obj, jint nNativeContext) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	return pPlayer->Run ();
}

static jint native_pause(JNIEnv* env, jobject obj, jint nNativeContext) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	return pPlayer->Pause();
}

static jint native_stop (JNIEnv* env, jobject obj, jint nNativeContext) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	return pPlayer->Stop();
}

static jint native_getpos(JNIEnv* env, jobject obj, jint nNativeContext) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;
	int nCurPos = pPlayer->GetPos ();
	return nCurPos;
}

static jint native_setpos(JNIEnv* env, jobject obj, jint nNativeContext, jint pos) 
{		
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	int nPos = pos;
	
	return pPlayer->SetPos(nPos);
}

static jlong native_getduration(JNIEnv* env, jobject obj, jint nNativeContext) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	return pPlayer->GetDur ();
}

static jint native_getitemcount (JNIEnv* env, jobject obj, jint nNativeContext)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return 0;	
	CLessonInfo * pInfo = pPlayer->GetLessonInfo ();
	if (pInfo == NULL)
		return 0;
	return pInfo->GetItemNum ();
}

static jint native_selectitem (JNIEnv* env, jobject obj, jint nNativeContext, jint nIndex)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return 0;	
	CLessonInfo * pInfo = pPlayer->GetLessonInfo ();
	if (pInfo == NULL)
		return 0;
	return pInfo->SetItemSel (nIndex);
}

static jint native_gettrackcount (JNIEnv* env, jobject obj, jint nNativeContext)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return 0;	
	CLessonInfo * pInfo = pPlayer->GetLessonInfo ();
	if (pInfo == NULL)
		return 0;
	CLsnItem * pItem = pInfo->GetItem ();
	if (pItem == NULL)
		return 0;
	return pItem->m_nTrackNum;
}

static jint native_getTrackVolume (JNIEnv* env, jobject obj, jint nNativeContext, jint nIndex)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return 0;	
	CLessonInfo * pInfo = pPlayer->GetLessonInfo ();
	if (pInfo == NULL)
		return 0;
	CLsnItem * pItem = pInfo->GetItem ();
	if (pItem == NULL)
		return 0;
	if (nIndex < 0 || nIndex >= pItem->m_nTrackNum)
		return 0;	
		
	YYLOGT ("jinPlayer", "Get Track %d Volume = %d", nIndex, pItem->m_ppTrack[nIndex]->m_nVolume);
	
	return pItem->m_ppTrack[nIndex]->m_nVolume;
}

static jint native_SetTrackVolume (JNIEnv* env, jobject obj, jint nNativeContext, jint nIndex, jint nVolume)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;	
	CLessonInfo * pInfo = pPlayer->GetLessonInfo ();
	if (pInfo == NULL)
		return -1;
	CLsnItem * pItem = pInfo->GetItem ();
	if (pItem == NULL)
		return -1;
	if (nIndex < 0 || nIndex >= pItem->m_nTrackNum)
		return -1;
		
	YYLOGT ("jinPlayer", "Set track %d Volume = %d", nIndex, nVolume);
			
	pItem->m_ppTrack[nIndex]->m_nVolume = nVolume;	
	return 0;
}

static jint native_getTrackEnable (JNIEnv* env, jobject obj, jint nNativeContext, jint nIndex)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return 0;	
	CLessonInfo * pInfo = pPlayer->GetLessonInfo ();
	if (pInfo == NULL)
		return 0;
	CLsnItem * pItem = pInfo->GetItem ();
	if (pItem == NULL)
		return 0;
	if (nIndex < 0 || nIndex >= pItem->m_nTrackNum)
		return 0;	
		
	YYLOGT ("jinPlayer", "Get track %d Enable = %d", nIndex, pItem->m_ppTrack[nIndex]->m_bEnable);
			
	return (jint)pItem->m_ppTrack[nIndex]->m_bEnable;
}

static jint native_SetTrackEnable (JNIEnv* env, jobject obj, jint nNativeContext, jint nIndex, jint nEnale)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;	
	CLessonInfo * pInfo = pPlayer->GetLessonInfo ();
	if (pInfo == NULL)
		return -1;
	CLsnItem * pItem = pInfo->GetItem ();
	if (pItem == NULL)
		return -1;
	if (nIndex < 0 || nIndex >= pItem->m_nTrackNum)
		return -1;	
		
	YYLOGT ("jinPlayer", "Set Track %d  Enable = %d", nIndex, nEnale);
			
	pItem->m_ppTrack[nIndex]->m_bEnable = nEnale > 0;
	return 0;
}

static jint native_getrepeatcount (JNIEnv* env, jobject obj, jint nNativeContext)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return 0;	
	CLessonInfo * pInfo = pPlayer->GetLessonInfo ();
	if (pInfo == NULL)
		return 0;
	return pInfo->GetRepeatNum ();
}

static jint native_getrepeatselect (JNIEnv* env, jobject obj, jint nNativeContext)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return 0;	
	CLessonInfo * pInfo = pPlayer->GetLessonInfo ();
	if (pInfo == NULL)
		return 0;
		
	YYLOGT ("jinPlayer", "GetRepeatSel: %d", pInfo->GetRepeatSel ());
			
	return pInfo->GetRepeatSel ();
}

static jint native_Setrepeatselect (JNIEnv* env, jobject obj, jint nNativeContext, jint nSel)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;	
	CLessonInfo * pInfo = pPlayer->GetLessonInfo ();
	if (pInfo == NULL)
		return -1;
		
	YYLOGT ("jinPlayer", "SetRepeatSel: %d", nSel);
			
	return pInfo->SetRepeatSel (nSel);
}

static jint native_getparam(JNIEnv* env, jobject obj, jint nNativeContext, jint nId, jobject objParam) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	int nRC = pPlayer->GetParam(env, nId, objParam);

	return nRC;
}

static jint native_setparam(JNIEnv* env, jobject obj, jint nNativeContext, jint nId, jint nParam, jobject objParam)
{	
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	return pPlayer->SetParam(env, nId, nParam, objParam);

}

static JNINativeMethod native_methods[] =
{
{ "nativeInit","(Ljava/lang/Object;Ljava/lang/String;)I",(void *) native_init },
{ "nativeUninit","(I)I",(void *) native_uninit },
{ "nativeOpen","(ILjava/lang/String;)I", (void *) native_open },
{ "nativePlay", "(I)I", (void *) native_play },
{ "nativePause", "(I)I", (void *) native_pause },
{ "nativeStop", "(I)I", (void *) native_stop },
{ "nativeGetPos", "(I)I", (void *) native_getpos },
{ "nativeSetPos", "(II)I", (void *) native_setpos },
{ "nativeGetDuration", "(I)J", (void *) native_getduration },
{ "nativeGetItemCount", "(I)I", (void *) native_getitemcount },
{ "nativeSelectItem", "(II)I", (void *) native_selectitem },
{ "nativeGetTrackCount", "(I)I", (void *) native_gettrackcount },
{ "nativeGetTrackVolume", "(II)I", (void *) native_getTrackVolume },
{ "nativeSetTrackVolume", "(III)I", (void *) native_SetTrackVolume },
{ "nativeGetTrackEnable", "(II)I", (void *) native_getTrackEnable },
{ "nativeSetTrackEnable", "(III)I", (void *) native_SetTrackEnable },
{ "nativeGetRepeatCount", "(I)I", (void *) native_getrepeatcount },
{ "nativeGetRepeatSelect", "(I)I", (void *) native_getrepeatselect },
{ "nativeSetRepeatSelect", "(II)I", (void *) native_Setrepeatselect },
{ "nativeGetParam", "(IILjava/lang/Object;)I",(void *) native_getparam },
{ "nativeSetParam", "(IIILjava/lang/Object;)I", (void *) native_setparam },
};

jint JNI_OnLoad (JavaVM *vm, void *reserved) 
{
	JNIEnv *env = NULL;
	jint jniver = JNI_VERSION_1_4;
	
	if (vm->GetEnv((void**) &env, jniver) != JNI_OK) 
	{
		jniver = JNI_VERSION_1_6;
		if (vm->GetEnv((void**) &env, jniver) != JNI_OK) 
		{
			YYLOGT ("jniPlayer", "It can't get env pointer!!!");
			return 0;
		}
	}

	jclass klass = env->FindClass(g_szRockVPlayerName);
	env->RegisterNatives(klass, native_methods, sizeof(native_methods) / sizeof(native_methods[0]));

	return jniver;
}
