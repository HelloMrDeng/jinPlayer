/*******************************************************************************
	File:		UJNIFunc.cpp

	Contains:	The utility for jni implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-01-08		Fenger			Create file

*******************************************************************************/
#include "UJNIFunc.h"
#include "dlfcn.h"
#include "yyLog.h"

int yyJNISetIntegerValue (JNIEnv * env, jobject obj, const char * pName, jint nValue) 
{
	if (obj == NULL || pName == NULL)
		return -1;
		
	jclass clsValue = env->GetObjectClass(obj);
	if (clsValue == NULL) 
	{
		YYLOGT ("yyJNISetIntegerValue", "It can't find the class %s in object!", pName);
		return -1;
	}
	jfieldID fldValue = env->GetFieldID (clsValue, pName, "I");
	if (fldValue == NULL)
	{
		YYLOGT ("yyJNISetIntegerValue", " It Can't find field %s!", pName);
		return -1;
	}
	env->SetIntField(obj, fldValue, nValue);	
	env->DeleteLocalRef(clsValue);
	return 0;
}

int yyJNIGetIntegerValue (JNIEnv * env, jobject obj, const char * pName) 
{
	int nValue = 0;
	if (obj == NULL || pName == NULL)
		return nValue;
		
	jclass clsValue = env->GetObjectClass(obj);
	if (clsValue == NULL) 
	{
		YYLOGT ("yyJNIGetIntegerValue", "It can't find the class %s in object!", pName);
		return nValue;
	}
	jfieldID fldValue = env->GetFieldID (clsValue, pName, "I");
	if (fldValue == NULL)
	{
		YYLOGT ("yyJNIGetIntegerValue", " It Can't find field %s!", pName);
		return nValue;
	}
	nValue = env->GetIntField(obj, fldValue);	
	env->DeleteLocalRef(clsValue);
	return nValue;
}


/*
float yyJNIGetFloatValue(JNIEnv * env, jobject value)
{
	if (value == NULL)
		return 0;
	
	jclass clazz = env->FindClass("java/lang/Float");

	jfieldID field = env->GetFieldID (clazz, "value" , "F");

	float nRC = env->GetFloatField (value, field);

	env->DeleteLocalRef(clazz);

	return nRC;
}
*/



jstring yyJNICharToString (JNIEnv* env,  const char* pChar) 
{  
	jclass 		strClass = env->FindClass("java/lang/String");   
	jmethodID 	ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V"); 
	jbyteArray 	bytes = env->NewByteArray(strlen(pChar));  
	
	env->SetByteArrayRegion(bytes, 0, strlen(pChar), (jbyte*)pChar);    
	jstring encoding = env->NewStringUTF("utf-8");  
	
	return (jstring)env->NewObject(strClass, ctorID, bytes, encoding);  
}

char* yyJNIStringToChar (JNIEnv* env, jstring strText)    
{    
	char* 		pChar = NULL;    
	jclass 		clsstring = env->FindClass("java/lang/String");    
	jstring 	strencode = env->NewStringUTF("utf-8");    
	jmethodID 	mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");    
	jbyteArray 	bytes = (jbyteArray)env->CallObjectMethod(strText, mid, strencode);    
	jsize 		len = env->GetArrayLength(bytes);    
	jbyte* 		ba = env->GetByteArrayElements(bytes, JNI_FALSE);   
	
	if (len > 0)    
	{    
		pChar = (char*)malloc(len + 1);    
		memcpy(pChar, ba, len);    
		pChar[len] = 0;    
	}   
	 
	env->ReleaseByteArrayElements(bytes, ba, 0);    
	
	return pChar;    
} 


jstring yyJNIWCharToString (JNIEnv* env, wchar_t * str) 
{  
	size_t 	len = wcslen(str);  
	jchar* 	str2 = (jchar*)malloc(sizeof(jchar)*(len+1));   
	  
	for (int i = 0; i < len; i++) 
		str2[i] = str[i];   
	str2[len] = 0;   
	jstring js = env->NewString(str2, len);   
	free(str2);  
	 
	return js;             
}

wchar_t * yyJNIStringToWChar (JNIEnv* env, jstring str) 
{  
	int 		len = env->GetStringLength(str);   
	wchar_t *	w_buffer = new wchar_t[len];  
	  
	memset(w_buffer,0,len+1);
	w_buffer[len]='\0';    
	wcsncpy(w_buffer,(wchar_t *)env->GetStringChars(str,0),len);     
	env->ReleaseStringChars(str,(const unsigned short *)w_buffer);
	
	return w_buffer;        
}

/*
static audio_io_handle_t getOutput(audio_stream_type_t stream,
                                    uint32_t samplingRate = 0,
                                    audio_format_t format = AUDIO_FORMAT_DEFAULT,
                                    audio_channel_mask_t channelMask = AUDIO_CHANNEL_OUT_STEREO,
                                    audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE);                                  
static status_t getLatency(audio_io_handle_t output,
                               audio_stream_type_t stream,
                               uint32_t* latency);
*/
     
#define		AUDIO_STREAM_MUSIC			3
#define 	AUDIO_FORMAT_PCM_16_BIT		1
#define 	AUDIO_CHANNEL_OUT_MONO		1
#define		AUDIO_CHANNEL_OUT_STEREO	3
#define 	AUDIO_OUTPUT_FLAG_NONE		0
    
typedef void * (* YYGETOUTPUTDEVICE) (int nType, int nSampleRate, int nFormat, int nChannels, int nFlag);                              
typedef int (* YYGETOUTPUTLATENCY) (int * latency, int nType);
typedef int (* YYGETDEVICELATENCY) (void * hDeivce, int nStream, int * pLatency);

int yyGetOutputLatency (int nSampleRate, int nChannels)
{
	FILE * hLibFile = fopen ("/system/lib/libmedia.so", "rb");
	if (hLibFile == NULL)
	{
		YYLOGT ("yyGetOutputLatency", "hLibFile = %p", hLibFile);
		return 0;		
	}
	fseeko (hLibFile, 0LL, SEEK_END);
	int nFileSize = ftello (hLibFile);
	fseeko (hLibFile, 0, SEEK_SET);		
	if (nFileSize <= 0)
		YYLOGT ("yyGetOutputLatency", "nFilesize = %d", nFileSize);		
	char * pFileBuff = new char[nFileSize];
	int nRead = fread (pFileBuff, 1, nFileSize, hLibFile);
	if (nRead != nFileSize)
		YYLOGT ("yyGetOutputLatency", "nRead = %d", nRead);		
	fclose (hLibFile);
	
	char *	pFind = NULL;
	char	szFunGetDevice[256];
	char	szFunGetDevLatency[256];
	char	szFunGetOutLatency[256];		
	char	szFunName[256];
	strcpy (szFunName, "getOutput");
	int		nNameLen = strlen (szFunName);
	char	szFunLatency[256];
	strcpy (szFunLatency, "getLatency");
	int 	nLatencyLen = strlen (szFunLatency);
	
	strcpy (szFunGetDevice, "");
	strcpy (szFunGetDevLatency, "");
	strcpy (szFunGetOutLatency, "");
	
	char * pNameBuff = pFileBuff;
	while (pNameBuff - pFileBuff < nFileSize - nNameLen)
	{
		if (!memcmp (pNameBuff, szFunLatency, nLatencyLen))
		{
			pFind = pNameBuff;
			while (*pFind != 0)
				pFind--;
			pFind++;
			if (strstr (pFind, "AudioSystem") != NULL)
			{
				strcpy (szFunGetDevLatency, pFind);
				//YYLOGT ("yyGetOutputLatency", "szFunGetDevLatency: %s", szFunGetDevLatency);	
			}							
			pNameBuff = pFind + strlen (pFind);
			continue;			
		}
		if (!memcmp (pNameBuff, szFunName, nNameLen))
		{
			pFind = pNameBuff;
			while (*pFind != 0)
				pFind--;
			pFind++;	
			if (strstr (pFind, "audio_output_flags") != NULL)
			{
				strcpy (szFunGetDevice, pFind);
				//YYLOGT ("yyGetOutputLatency", "szFunGetDevice: %s", szFunGetDevice);	
			}
			else if (strstr (pFind, "getOutputLatency") != NULL && strstr (pFind, "AudioSystem") != NULL)
			{
				strcpy (szFunGetOutLatency, pFind);
				//YYLOGT ("yyGetOutputLatency", "szFunGetOutLatency: %s", szFunGetOutLatency);	
			}			
			pNameBuff = pFind + strlen (pFind);
			continue;
		}
		pNameBuff++;
	}
	delete []pFileBuff;
//	if (strlen (szFunGetDevice) <= 0 || strlen (szFunGetDevLatency) <= 0)
//		return 0;	
	if (strlen (szFunGetOutLatency) <= 0)
	{
		YYLOGT ("yyGetOutputLatency", "szFunGetOutLatency: %s", szFunGetOutLatency);	
		return 0;
	}
	
	void * hDllMedia = dlopen("/system/lib/libmedia.so", RTLD_NOW);
	if (hDllMedia == NULL)
	{
		YYLOGT ("yyGetOutputLatency", "hDllMedia = %p", hDllMedia);
		return 0;
	}
		
	int nLatency = 0;		
/*	
	void * hDevice = NULL;
	YYGETOUTPUTDEVICE fGetDev = (YYGETOUTPUTDEVICE) dlsym (hDllMedia, szFunGetDevice);
	YYLOGT ("yyGetOutputLatency", "fGetDev = %p", fGetDev);
	if (fGetDev != NULL)
	{
		hDevice = fGetDev (AUDIO_STREAM_MUSIC, nSampleRate, AUDIO_FORMAT_PCM_16_BIT, AUDIO_CHANNEL_OUT_STEREO, AUDIO_OUTPUT_FLAG_NONE);
		YYLOGT ("yyGetOutputLatency", "hDevice = %p", hDevice);
	}
	YYGETDEVICELATENCY fGetDevLatency = (YYGETDEVICELATENCY) dlsym (hDllMedia, szFunGetDevLatency);
	YYLOGT ("yyGetOutputLatency", "szFunGetDevLatency = %p", szFunGetDevLatency);	
	if (fGetDevLatency != NULL && hDevice != NULL)
	{
		fGetDevLatency (hDevice, AUDIO_STREAM_MUSIC, &nLatency);
		YYLOGT ("yyGetOutputLatency", "nLatency = %d", nLatency);
	}
*/	
	
	YYGETOUTPUTLATENCY fGetOutLatency = (YYGETOUTPUTLATENCY) dlsym (hDllMedia, szFunGetOutLatency);
	if (fGetOutLatency != NULL)
	{
		fGetOutLatency (&nLatency, -1);
		YYLOGT ("yyGetOutputLatency", "nLatency = %d", nLatency);
	}
	else
	{
		YYLOGT ("yyGetOutputLatency", "fGetOutLatency = %p", fGetOutLatency);
	}
		
	dlclose (hDllMedia);	
	
	return nLatency;
}
