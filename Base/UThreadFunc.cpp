/*******************************************************************************
	File:		UThreadFunc.cpp

	Contains:	The base utility for thread implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifdef _OS_WIN32
#include <windows.h>
#elif defined _OS_LINUX
#include <sys/mman.h>
#include <sys/prctl.h>
#include <pthread.h>
#endif

#include "yyType.h"
#include "UThreadFunc.h"

#define YY_DEFAULT_STACKSIZE (128 * 1024)

void yyThreadSetName(int uThreadID, const char* threadName)
{
#ifdef _OS_NDK
	prctl(PR_SET_NAME, (unsigned long)threadName , 0 , 0 , 0);
#endif // _OS_WIN32
}

int yyThreadCreate (yyThreadHandle * pHandle, int * pID,
						yyThreadProc fProc, void * pParam, int uFlag)
{
	if (pHandle == NULL)
		return YY_ERR_ARG;

	*pHandle = NULL;
	int nID = 0;
	if (pID != NULL)
		*pID = 0;
	else
		pID = &nID;
#ifdef _OS_WIN32
	*pHandle = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) fProc, pParam, 0, (LPDWORD)pID);
	if (*pHandle == NULL)
		return YY_ERR_MEMORY;
#elif defined _OS_LINUX
	pthread_t 		tt;
	pthread_attr_t	attr;
	pthread_attr_init(&attr);

	pthread_attr_setstacksize(&attr, YY_DEFAULT_STACKSIZE);
	pthread_attr_setguardsize(&attr, PAGE_SIZE);
//	pthread_attr_setschedpolicy(&attr, SCHED_NORMAL);
	pthread_attr_setschedpolicy(&attr, SCHED_RR);
	
	attr.sched_priority = 5;

	int rs = pthread_create(&tt, &attr, (void*(*)(void*))fProc ,pParam);
//	int rs = pthread_create(&tt, NULL, (void*(*)(void*))fProc ,pParam);
	pthread_attr_destroy(&attr);
	if (rs != 0)
		return YY_ERR_MEMORY;

	rs = pthread_detach(tt);

	*pHandle = (yyThreadHandle)tt;
	*pID = (int)tt;	
#endif // _OS_WIN32

	return YY_ERR_NONE;
}

int yyThreadClose (yyThreadHandle hHandle, int uExitCode)
{
	if (hHandle == NULL)
		return YY_ERR_ARG;

#ifdef _OS_WIN32
	CloseHandle (hHandle);
#endif //_OS_WIN32

	return YY_ERR_NONE;
}

int yyThreadGetPriority (yyThreadHandle hHandle, yyThreadPriority * pPriority)
{
	if (hHandle == NULL)
		return YY_ERR_ARG;

#ifdef _OS_WIN32
	int nPriority = 0;
	nPriority = GetThreadPriority (hHandle);
	yyThreadConvertPriority (pPriority, &nPriority, false);
#endif //_OS_WIN32

#ifdef _OS_IOS
	int policy = 0;
	struct sched_param param;
    int nPriority = 0;
	
	int iRet = pthread_getschedparam((pthread_t)hHandle, &policy, &param);
	if (0 != iRet) {
		VOLOGE("pthread_getschedparam error :%d", iRet);
		return YY_ERR_FAILED;
	}
	nPriority = param.sched_priority;
	VOLOGI("get succ hHandle:%ld, policy:%d, param.sched_priority:%d", (int)hHandle, policy, param.sched_priority);
	yyThreadConvertPriority (pPriority, &nPriority, YY_FALSE);
#endif

	return YY_ERR_NONE;
}


int yyThreadSetPriority (yyThreadHandle hHandle, yyThreadPriority uPriority)
{
	if (hHandle == NULL)
		return YY_ERR_ARG;

#ifdef _OS_WIN32
	int nPriority = 0;
	yyThreadConvertPriority (&uPriority, &nPriority, true);
	SetThreadPriority (hHandle, nPriority);
#endif //_OS_WIN32

#ifdef _OS_IOS
	int policy = 0;
	struct sched_param param;
    int nPriority = 0;
	
	int iRet = pthread_getschedparam((pthread_t)hHandle, &policy, &param);
	if (0 != iRet) {
		VOLOGE("pthread_getschedparam hHandle:%ld, error :%d", (int)hHandle, iRet);
		return YY_ERR_FAILED;
	}

	VOLOGI("get succ hHandle:%ld, policy:%d, param.sched_priority:%d", (int)hHandle, policy, param.sched_priority);
	
	yyThreadConvertPriority (&uPriority, &nPriority, YY_TRUE);
	param.sched_priority = nPriority;
	iRet = pthread_setschedparam((pthread_t)hHandle, policy, &param);
	
	if (0 != iRet) {
		VOLOGE("pthread_attr_setschedparam hHandle:%ld, error :%d, param.sched_priority:%d", (int)hHandle, iRet, param.sched_priority);
		return YY_ERR_FAILED;
	}

	VOLOGI("set succ hHandle:%ld, policy:%d, param.sched_priority:%d", (int)hHandle, policy, param.sched_priority);
#endif
	
	return YY_ERR_NONE;
}

int yyThreadSuspend (yyThreadHandle hHandle)
{
	if (hHandle == NULL)
		return YY_ERR_ARG;

#ifdef _OS_WIN32
	SuspendThread (hHandle);
#endif //_OS_WIN32

	return YY_ERR_NONE;
}

int yyThreadResume (yyThreadHandle hHandle)
{
	if (hHandle == NULL)
		return YY_ERR_ARG;

#ifdef _OS_WIN32
	ResumeThread (hHandle);
#endif //_OS_WIN32

	return YY_ERR_NONE;
}

int yyThreadProtect (yyThreadHandle hHandle)
{
	return YY_ERR_NONE;
}

int yyThreadConvertPriority (yyThreadPriority * pPriority, int * pPlatform, bool bPlatform)
{
	if (bPlatform)
	{
		switch (*pPriority)
		{
#if defined _OS_WIN32 || defined _OS_IOS
		case YY_THREAD_PRIORITY_TIME_CRITICAL:
			*pPlatform = THREAD_PRIORITY_TIME_CRITICAL;
			break;

		case YY_THREAD_PRIORITY_HIGHEST:
			*pPlatform = THREAD_PRIORITY_HIGHEST;
			break;

		case YY_THREAD_PRIORITY_ABOVE_NORMAL:
			*pPlatform = THREAD_PRIORITY_ABOVE_NORMAL;
			break;

		case YY_THREAD_PRIORITY_NORMAL:
			*pPlatform = THREAD_PRIORITY_NORMAL;
			break;

		case YY_THREAD_PRIORITY_BELOW_NORMAL:
			*pPlatform = THREAD_PRIORITY_BELOW_NORMAL;
			break;

		case YY_THREAD_PRIORITY_LOWEST:
			*pPlatform = THREAD_PRIORITY_LOWEST;
			break;

		case YY_THREAD_PRIORITY_IDLE:
			*pPlatform = THREAD_PRIORITY_IDLE;
			break;
#endif // _OS_WIN32
		default:
			break;
		}
	}
	else
	{
		switch (*pPlatform)
		{
#if defined _OS_WIN32 || defined _OS_IOS
		case THREAD_PRIORITY_TIME_CRITICAL:
			*pPriority = YY_THREAD_PRIORITY_TIME_CRITICAL;
			break;

		case THREAD_PRIORITY_HIGHEST:
			*pPriority = YY_THREAD_PRIORITY_HIGHEST;
			break;

		case THREAD_PRIORITY_ABOVE_NORMAL:
			*pPriority = YY_THREAD_PRIORITY_ABOVE_NORMAL;
			break;

		case THREAD_PRIORITY_NORMAL:
			*pPriority = YY_THREAD_PRIORITY_NORMAL;
			break;

		case THREAD_PRIORITY_BELOW_NORMAL:
			*pPriority = YY_THREAD_PRIORITY_BELOW_NORMAL;
			break;

		case THREAD_PRIORITY_LOWEST:
			*pPriority = YY_THREAD_PRIORITY_LOWEST;
			break;

		case THREAD_PRIORITY_IDLE:
			*pPriority = YY_THREAD_PRIORITY_IDLE;
			break;
#endif // _OS_WIN32
		default:
			break;
		}
	}

	return YY_ERR_NONE;
}

int	yyThreadGetCurrentID (void)
{
#ifdef _OS_WIN32
	return (int) GetCurrentThreadId ();
#elif defined _OS_LINUX
	return (int) pthread_self ();
#elif defined _OS_IOS || defined _OS_MACOS
	return (signed long) pthread_self ();
#endif // _OS_WIN32
}

yyThreadHandle yyThreadGetCurHandle (void)
{
#ifdef _OS_WIN32
	return GetCurrentThread ();
#elif defined _OS_LINUX
	return (yyThreadHandle) pthread_self ();
#elif defined _OS_IOS || defined _OS_MACOS
	return (yyThreadHandle) pthread_self ();
#endif // _OS_WIN32
}
