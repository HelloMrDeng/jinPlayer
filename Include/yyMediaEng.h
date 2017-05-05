/*******************************************************************************
	File:		yyMediaEng.h

	Contains:	yy media engine define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __yyMediaEng_h__
#define __yyMediaEng_h__

#ifdef _OS_WIN32
#include "windows.h"
#endif // _OS_WIN32

#include "yyType.h"
#include "yyMediaPlayer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define YYME_PLAYER			0X00000001
#define YYME_MTV			0X00000002
#define YYME_REC			0X00000003

/**
 * Initialize Media Engine SDK instance use default settings.
 * \param phMedia [out] Return the Media handle.
 * \param nFlag [in] It is not using now. It should be 0.
 * \param pParam [in] It is not using now. It should be NULL.
 * \retval YY_ERR_NONE Succeeded.
 * \retval YY_ERR_MEMORY Out of memory.
 */
int yyMediaInit (void ** phMedia, int nFlag, void * pParam);
typedef int (* YYMEDIAINIT) (void ** phMedia, int nFlag, void * pParam);

/**
 * Create the media in engine. It can create more. The max is 9. It supports Player now.
 * \param hMeda [in] The handle.which was created by yyMediaInit.
 * \param nType [in] What the media will be created. It supports player YYME_PLAYER now.
 * \param ppMeda [out] The interface of the created media.
 * \retval YY_ERR_NONE Succeeded.
 */
int yyMediaCreate (void * hMeda, int nType, void ** ppMeda);
typedef int (* YYMEDIACREATE) (void * hPlayer, int nType, void ** ppMeda);

/**
 * Destroy the media in engine to save the resource. It will destroy all in uninit function.
 * \param hMeda [in] The handle.which was created by yyMediaInit.
 * \param nType [in] What the media will be created. It supports player YYME_PLAYER now.
 * \param pMeda [int] Which media like to destroy.
 * \retval YY_ERR_NONE Succeeded.
 */
int yyMediaDestroy (void * hMeda, int nType, void * ppMeda);
typedef int (* YYMEDIADESTROY) (void * hPlayer, int nType, void * pMeda);


/**
 * Uninitialize Media Engine SDK and free the resource.
 * \param hMeda [in] The handle.which was created by yyMediaInit.
 * \retval YY_ERR_NONE Succeeded.
 */
int yyMediaUninit (void * hMeda);
typedef int (* YYMEDIAUNINIT) (void * hMeda);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif // __yyMediaEng_h__
