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


#define g_szBasePlayerName "com/cansure/mediaEngine/BasePlayer"

#define PARAM_PID_SURFACE_CHANGED		0X100
#define	PARAM_PID_VIEW_ACTIVE			0X200
	
static JavaVM * java_vm;
static char g_szPathLib[256];

// Private methods
int getIntegerValue(JNIEnv * env, jobject value) 
{
	if (value == NULL)
		return 0;

	jclass clazz = env->FindClass("java/lang/Integer");
	jfieldID field = env->GetFieldID(clazz, "value", "I");

	int nRC = env->GetIntField(value, field);

	env->DeleteLocalRef(clazz);

	return nRC;
}

void setIntegerValue(JNIEnv * env, jobject obj, jint value) 
{
	if (obj == NULL)
		return;

	jclass clazz = env->FindClass("java/lang/Integer");
	jfieldID field = env->GetFieldID(clazz, "value", "I");

	env->SetIntField(obj, field, value);

	env->DeleteLocalRef(clazz);
}

void callbackEvent(JNIEnv* env, jobject obj, int nId, void * pParam) 
{	
	if (pParam == NULL)
	{
		YYLOGE ("The param can't set as NULL!");
		return;
	}
	
	jclass clazz = env->FindClass(g_szBasePlayerName);
	jmethodID callbackId = env->GetMethodID(clazz, "onEventCallback", "(ILjava/lang/Object;)I");
	if (callbackId == NULL)
	{
		YYLOGE ("It can't find the call back method!");
		return;
	}
	
	jint nRet = 0;
	
	jclass clazzInt = env->FindClass("java/lang/Integer");
	jobject objnew = env->AllocObject(clazzInt);
	setIntegerValue(env, objnew, *(int*) pParam);
	
	nRet = env->CallIntMethod (obj, callbackId, nId, objnew);

//	YYLOGI ("return value %d ", nRet);
}

int updateVideoSize(JNIEnv* env, jobject obj, int nWidth, int nHeight) 
{
	jclass clsVR = env->FindClass(g_szBasePlayerName);
	if (clsVR == NULL) 
		return -1;

	jfieldID cntWidth = env->GetFieldID(clsVR, "m_nVideoWidth", "I");
	if (cntWidth == NULL) 
		return -1;
	env->SetIntField(obj, cntWidth, nWidth);

	jfieldID cntHeight = env->GetFieldID(clsVR, "m_nVideoHeight", "I");
	if (cntHeight == NULL)
		return -1;
	env->SetIntField(obj, cntHeight, nHeight);

	env->DeleteLocalRef(clsVR);

	return 0;
}

int updateAudioFormat(JNIEnv* env, jobject thiz, int nSampleRate, int nChannels) 
{
	jclass clsVR = env->GetObjectClass(thiz);
	if (clsVR == NULL)
		return -1;

	jfieldID cntSampleRate = env->GetFieldID(clsVR, "m_nSampleRate", "I");
	if (cntSampleRate == NULL)
		return -1;
	env->SetIntField(thiz, cntSampleRate, nSampleRate);

	jfieldID cntChannels = env->GetFieldID(clsVR, "m_nChannels", "I");
	if (cntChannels == NULL)
		return -1;

	env->SetIntField(thiz, cntChannels, nChannels);
	env->DeleteLocalRef(clsVR);

	return 0;
}

static void SetSurface(JNIEnv* env, jobject obj, CNDKPlayer * pPlayer) 
{
	jclass clazz = env->FindClass(g_szBasePlayerName);
	if (clazz == NULL)
	{
		YYLOGE ("It can't find the BasePlayer!");
		return;
	}

	jfieldID surfaceID = env->GetFieldID(clazz, "m_NativeSurface", "Landroid/view/Surface;");
	if (surfaceID == NULL)
	{
		YYLOGE ("It can't find the native surface!");
		return;
	}

	jobject surfaceObj = env->GetObjectField(obj, surfaceID);
	if (surfaceObj == NULL)
	{
		YYLOGE ("It can't find the surface object!");
		return;
	}

	int nRC = 0;
	if (pPlayer) 
		nRC = pPlayer->SetView(env, surfaceObj);

	env->DeleteLocalRef(clazz);
	
	return;
}


// Java Bindings
static jint native_init (JNIEnv* env, jobject obj, jobject player, jstring apkPath) 
{
	char * str = (char *) env->GetStringUTFChars(apkPath, NULL);
	strcpy(g_szPathLib, str);
	env->ReleaseStringUTFChars(apkPath, str);

	CNDKPlayer * pPlayer = new CNDKPlayer ();
	int nRC = pPlayer->Init(g_szPathLib);
	if (nRC) 
	{
		delete pPlayer;
		return nRC;
	}

	JavaVM * jvm = 0;
	jobject envobj;

	env->GetJavaVM(&jvm);
	envobj = env->NewGlobalRef(obj);
	pPlayer->SetJavaVM(jvm, envobj);

	jclass clazz;
	clazz = env->GetObjectClass(obj);
	if (clazz == NULL) 
	{
		pPlayer->Uninit();
		env->DeleteGlobalRef(envobj);
		delete pPlayer;
		return -1;
	}
	
	jfieldID context = env->GetFieldID(clazz, "m_NativeContext", "I");
	if (context == NULL) 
	{
		pPlayer->Uninit();
		env->DeleteGlobalRef(envobj);
		delete pPlayer;
		return -1;
	}

	env->SetIntField(obj, context, (int) pPlayer);

	env->DeleteLocalRef(clazz);

	return 0;
}

static jint native_uninit(JNIEnv* env, jobject obj, jint nNativeContext) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	pPlayer->Uninit();
	void* objSaved = pPlayer->GetJavaObj();
	if (objSaved != NULL)
		env->DeleteGlobalRef((jobject) objSaved);

	delete pPlayer;

	return 0;
}

static jint native_open (JNIEnv* env, jobject obj, jint nNativeContext, jstring strUrl) 
{		
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	SetSurface(env, obj, pPlayer);

	int nRC = 0;
	if (strUrl != NULL) 
	{
		const char* pSource = env->GetStringUTFChars(strUrl, NULL);
		YYLOGI ("Open source: %s", pSource);
		
		nRC = pPlayer->Open (pSource);
		env->ReleaseStringUTFChars(strUrl, pSource);
	} 
	
	return nRC;
	
/*
	while (pPlayer->GetOpenStatus () == 0)
	{
		usleep (10000);
	}
	
	if (pPlayer->GetOpenStatus () > 0)
	{
		int nW, nH;
		pPlayer->GetVideoSize (&nW, &nH);
		updateVideoSize (env, obj, nW, nH);
		
		int nSR, nCh;
		pPlayer->GetAudioFormat (&nSR, &nCh);
		updateAudioFormat (env, obj, nSR, nCh);
	
		int nParam = 0;
		callbackEvent(env, obj, YY_EV_Open_Complete, &nParam);
		
		return nRC;
	}
	else
	{
//		int nParam = 0;
//		callbackEvent(env, obj, YY_EV_Open_Failed, &nParam);
		
		return -1;
	}
*/	
}

static jint native_openext (JNIEnv* env, jobject obj, jint nNativeContext, jint nExtData, jint nFlag) 
{		
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	SetSurface(env, obj, pPlayer);

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

static jlong native_getvideodata(JNIEnv* env, jobject obj, jint nNativeContext, jcharArray data) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	return pPlayer->GetVideoData(env, obj, NULL, 0);
}

static jlong native_getaudiodata(JNIEnv* env, jobject obj, jint nNativeContext, jcharArray data) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;
//	return pPlayer->GetAudioData(env, obj, NULL, 0);	
	
	if (data == NULL) 
	{
		int nSampleRate = 0;
		int nChannels = 0;		
		pPlayer->GetAudioFormat(&nSampleRate, &nChannels);
		updateAudioFormat(env, obj, nSampleRate, nChannels);
		return 0;
	}

	unsigned long lSize = env->GetArrayLength(data);
	jchar* pData = env->GetCharArrayElements(data, 0);
	if (pData == NULL)
		return -1;

	long  			nFill = 0;
#if 0		
	int				nRC = 0;
	long  			nSize = lSize;
	unsigned char * pBuff = (unsigned char *)pData;

	while (nFill < lSize / 2)
	{
		nRC = pPlayer->GetAudioData(env, obj, (unsigned char **) &pBuff, (int) nSize);	
		if (nRC <= 0)
			break;
				
		nFill += nRC;
		pBuff += nRC;
		nSize -= nRC;
	}
#else

	nFill = pPlayer->GetAudioData(env, obj, (unsigned char **) &pData, (int) lSize);	

#endif // 0
		
	env->ReleaseCharArrayElements(data, pData, 0);

	return nFill;
}

static jlong native_geteventstatus(JNIEnv* env, jobject obj, jint nNativeContext) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	return pPlayer->GetEventStatus(env, obj);
}

static jint native_getparam(JNIEnv* env, jobject obj, jint nNativeContext, jint nId, jobject objParam) 
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;

	int nRC = pPlayer->GetParam(env, nId, objParam);

	return nRC;
}

static jint native_setparam(JNIEnv* env, jobject obj, jint nNativeContext, jint nId, jobject objParam)
{
	CNDKPlayer* pPlayer = (CNDKPlayer*) nNativeContext;
	if (pPlayer == NULL)
		return -1;
			
//	if (nId == PARAM_PID_VIEW_ACTIVE || nId == PARAM_PID_SURFACE_CHANGED)
	if (nId == PARAM_PID_VIEW_ACTIVE)
	{
		
		SetSurface(env, obj, pPlayer);		
		return 0;
	}

	return pPlayer->SetParam(env, nId, objParam);
}

static JNINativeMethod native_methods[] =
{
{ "nativeInit","(Ljava/lang/Object;Ljava/lang/String;)I",(void *) native_init },
{ "nativeUninit","(I)I",(void *) native_uninit },
{ "nativeOpen","(ILjava/lang/String;)I", (void *) native_open },
{ "nativeOpenExt","(III)I", (void *) native_openext },
{ "nativePlay", "(I)I", (void *) native_play },
{ "nativePause", "(I)I", (void *) native_pause },
{ "nativeStop", "(I)I", (void *) native_stop },
{ "nativeGetPos", "(I)I", (void *) native_getpos },
{ "nativeSetPos", "(II)I", (void *) native_setpos },
{ "nativeGetDuration", "(I)J", (void *) native_getduration },
{ "nativeGetVideoData", "(I[B)J", (void *) native_getvideodata },
{ "nativeGetAudioData", "(I[B)J", (void *) native_getaudiodata },
{ "nativeGetEventStatus", "(I)J", (void *) native_geteventstatus },
{ "nativeGetParam", "(IILjava/lang/Object;)I",(void *) native_getparam },
{ "nativeSetParam", "(IILjava/lang/Object;)I", (void *) native_setparam }
};

jint JNI_OnLoad (JavaVM *vm, void *reserved) 
{
	JNIEnv *env = NULL;
	java_vm = vm;

	jint jniver = JNI_VERSION_1_4;
	if (vm->GetEnv((void**) &env, jniver) != JNI_OK) 
	{
		jniver = JNI_VERSION_1_6;
		if (vm->GetEnv((void**) &env, jniver) != JNI_OK) 
		{
			YYLOGE ("It can't get env pointer!!!");
			return 0;
		}
	}

	jclass klass = env->FindClass(g_szBasePlayerName);
	env->RegisterNatives(klass, native_methods, sizeof(native_methods) / sizeof(native_methods[0]));

	return jniver;
}
