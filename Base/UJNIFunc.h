/*******************************************************************************
	File:		UJNIFunc.h

	Contains:	The base utility for jni header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-01-08		Fenger			Create file

*******************************************************************************/
#ifndef __UJNIFunc_H__
#define __UJNIFunc_H__

#include "stdio.h"
#include "wchar.h"
#include "string.h"

#include "jni.h"

#include "yyType.h"

int 		yyJNISetIntegerValue (JNIEnv * env, jobject obj, const char * pName, jint nValue);
int 		yyJNIGetIntegerValue (JNIEnv * env, jobject obj, const char * pName);

jstring 	yyJNICharToString (JNIEnv* env,  const char* pChar);
char* 		yyJNIStringToChar (JNIEnv* env, jstring strText); 


jstring 	yyJNIWCharToString (JNIEnv* env, wchar_t* str);
wchar_t* 	yyJNIStringToWChar (JNIEnv* env, jstring str);

int 		yyGetOutputLatency (int nSampleRate, int nChannels);

#endif // __UJNIFunc_H__
