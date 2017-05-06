/*******************************************************************************
	File:		UThreadFunc.h

	Contains:	The base utility for thread header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __UThreadFunc_H__
#define __UThreadFunc_H__

typedef void * yyThreadHandle;
typedef int (* yyThreadProc) (void * pParam);

typedef enum YYTHRD_Status
{
	YYTHRD_Stop			= 0,
	YYTHRD_Pause		= 1,
	YYTHRD_Run			= 2,
	YYTHRD_Work			= 3,
} YYTHRD_Status;

typedef enum yyThreadPriority
{
	YY_THREAD_PRIORITY_TIME_CRITICAL	= 0x0,
	YY_THREAD_PRIORITY_HIGHEST			= 0x1,
	YY_THREAD_PRIORITY_ABOVE_NORMAL		= 0x2,
	YY_THREAD_PRIORITY_NORMAL			= 0x3,
	YY_THREAD_PRIORITY_BELOW_NORMAL		= 0x4,
	YY_THREAD_PRIORITY_LOWEST			= 0x5,
	YY_THREAD_PRIORITY_IDLE				= 0x6,
	YY_THREAD_PRIORITY_MAX				= 0x7FFFFFFF,
} yyThreadPriority;

/**
 * Create the thread
 * \param pHandle [out] the thread handle
 * \param pID [out] the thread id
 * \param fProc [in] the thread start address
 * \param pParam [in] the thread call back parameter
 * \param uFlag [in] the thread was create with the flagr
 * \return value yy_ErrorNone
 */
int	yyThreadCreate (yyThreadHandle * pHandle, int * pID, 
						yyThreadProc fProc, void * pParam, int uFlag);


/**
 * close the thread
 * \param pHandle [in] the thread handle was created by yyThreadCreate
 * \param uExitCode [in] the return code after exit the thread
 * \return value yy_ErrorNone
 */
int	yyThreadClose (yyThreadHandle hHandle, int uExitCode);


/**
 * Get the thread priority
 * \param pHandle [in] the thread handle was created by yyThreadCreate
 * \param pPriority [out] the priority to get
 * \return value yy_ErrorNone
 */
int	yyThreadGetPriority (yyThreadHandle hHandle, yyThreadPriority * pPriority);


/**
 * Set the thread priority
 * \param pHandle [in] the thread handle was created by yyThreadCreate
 * \param uPriority [in] the priority to set
 * \return value yy_ErrorNone
 */
int	yyThreadSetPriority (yyThreadHandle hHandle, yyThreadPriority uPriority);


/**
 * Suspend the thread
 * \param pHandle [in] the thread handle was created by yyThreadCreate
 * \return value yy_ErrorNone
 */
int	yyThreadSuspend (yyThreadHandle hHandle);


/**
 * Resume the thread
 * \param pHandle [in] the thread handle was created by yyThreadCreate
 * \return value yy_ErrorNone
 */
int	yyThreadResume (yyThreadHandle hHandle);

/**
 * Protect the thread
 * \param pHandle [in] the thread handle was created by yyThreadCreate
 * \return value yy_ErrorNone
 */
int	yyThreadProtect (yyThreadHandle hHandle);

/**
 * set the thread name
 * \param uThreadID [in] the thread id was created by yyThreadCreate
 * \return value yy_ErrorNone
 */
void yyThreadSetName(int uThreadID, const char* threadName);

/**
 * convert the thread priority
 * \param uPriority [in] the thread priority
 * \return value 
 */
int	yyThreadConvertPriority (yyThreadPriority * pPriority, int * pPlatform, bool bPlatform);

/**
 * Get the current thread ID
 * \return current thread ID 
 */
int	yyThreadGetCurrentID (void);

/**
 * Get the current thread handle
 * \return current thread handle 
 */
yyThreadHandle	yyThreadGetCurHandle (void);

#endif // __UThreadFunc_H__
