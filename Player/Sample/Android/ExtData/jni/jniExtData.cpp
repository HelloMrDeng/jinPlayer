/*******************************************************************************
	File:		jniExtData.cpp

	Contains:	yy ext data jni implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-12-24		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <jni.h>

#include "CExtData.h"

#define g_szBasePlayerName "com/cansure/ExtData/ExtData"

// Java Bindings
static jint native_getapi (JNIEnv* env, jobject obj, jobject player, jstring apkPath) 
{
	CExtData * pExt = new CExtData ();

	jclass clazz;

	clazz = env->GetObjectClass(obj);
	jfieldID context = env->GetFieldID(clazz, "m_NativeContext", "I");
	env->SetIntField(obj, context, (int) pExt);

	env->DeleteLocalRef(clazz);

	return (jint)pExt->GetExtData (YY_EXTDATA_Mux);
}

static jint native_free(JNIEnv* env, jobject obj, jint nNativeContext) 
{
	CExtData* pExt = (CExtData*) nNativeContext;
	if (pExt == NULL)
		return -1;

	delete pExt;

	return 0;
}

static JNINativeMethod native_methods[] =
{
{ "nativeGetAPI","(Ljava/lang/Object;Ljava/lang/String;)I",(void *) native_getapi },
{ "nativeFree","(I)I",(void *) native_free },
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
			return 0;
		}
	}

	jclass klass = env->FindClass(g_szBasePlayerName);
	env->RegisterNatives(klass, native_methods, sizeof(native_methods) / sizeof(native_methods[0]));

	return jniver;
}
