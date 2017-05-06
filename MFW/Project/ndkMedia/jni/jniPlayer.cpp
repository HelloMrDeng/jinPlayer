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


#define g_szMediaPlayerName "com/cansure/mediaEngine/MediaPlayer"


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
	if (nRC) 
	{
		delete pPlayer;
		return 0;
	}
	
	env->DeleteLocalRef(clazz);

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

static jint native_setView (JNIEnv* env, jobject obj, jint nNativeContext, jobject view) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;
			
	pPlayer->SetView(env, view);	
	
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

static jint native_openext (JNIEnv* env, jobject obj, jint nNativeContext, jint nExtData, jint nFlag) 
{		
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	int nRC = 0;

	nRC = pPlayer->OpenExt (nExtData, nFlag);
	
	return nRC;	
}

static jint native_play (JNIEnv* env, jobject obj, jint nNativeContext) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	return pPlayer->Play ();
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

	int nCurPos = 0;

	int nRC = pPlayer->GetPos(&nCurPos);
	if (nRC)
		return 0;

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

	return pPlayer->GetDuration();
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

static jint native_getThumb (JNIEnv* env, jobject obj, jint nNativeContext, jstring strFile, jint nWidth, jint nHeight, jobject objBitmap)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	const char* pSource = env->GetStringUTFChars(strFile, NULL);
	YYLOGT ("jniPlayer", "Open source: %s", pSource);
	
	int nRC = pPlayer->GetThumb(env, obj, pSource, nWidth, nHeight, objBitmap);	
	
	env->ReleaseStringUTFChars(strFile, pSource);
		
	return nRC;
}

static jint native_readvideo (JNIEnv* env, jobject obj, jint nNativeContext, jbyteArray data)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;		
	return pPlayer->ReadVideoBuff (env, obj, data);
}

static jint native_readaudio (JNIEnv* env, jobject obj, jint nNativeContext, jbyteArray data)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;		
	return pPlayer->ReadVideoBuff (env, obj, data);
}

static JNINativeMethod native_methods[] =
{
{ "nativeInit","(Ljava/lang/Object;Ljava/lang/String;)I",(void *) native_init },
{ "nativeUninit","(I)I",(void *) native_uninit },
{ "nativeSetView","(ILjava/lang/Object;)I",(void *) native_setView },
{ "nativeOpen","(ILjava/lang/String;)I", (void *) native_open },
{ "nativeOpenExt","(III)I", (void *) native_openext },
{ "nativePlay", "(I)I", (void *) native_play },
{ "nativePause", "(I)I", (void *) native_pause },
{ "nativeStop", "(I)I", (void *) native_stop },
{ "nativeGetPos", "(I)I", (void *) native_getpos },
{ "nativeSetPos", "(II)I", (void *) native_setpos },
{ "nativeGetDuration", "(I)J", (void *) native_getduration },
{ "nativeGetParam", "(IILjava/lang/Object;)I",(void *) native_getparam },
{ "nativeSetParam", "(IIILjava/lang/Object;)I", (void *) native_setparam },
{ "nativeGetThumb", "(ILjava/lang/String;IILjava/lang/Object;)I", (void *) native_getThumb },
{ "nativeReadVideo", "(I[B)I", (void *) native_readvideo },
{ "nativeReadAudio", "(I[B)I", (void *) native_readaudio }
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

	jclass klass = env->FindClass(g_szMediaPlayerName);
	env->RegisterNatives(klass, native_methods, sizeof(native_methods) / sizeof(native_methods[0]));

	return jniver;
}
