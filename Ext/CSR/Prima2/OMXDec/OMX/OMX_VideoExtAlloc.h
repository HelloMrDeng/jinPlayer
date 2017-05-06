/***************************************************************************
 *                                                                         *
 *                   SiRF Technology, Inc. Platform Software               *
 *                                                                         *
 *    Copyright (c) 2010 by SiRF Technology, Inc.  All rights reserved.    *
 *                                                                         *
 *    This Software is protected by United States copyright laws and       *
 *    international treaties.  You may not reverse engineer, decompile     *
 *    or disassemble this Software.                                        *
 *                                                                         *
 *    WARNING:                                                             *
 *    This Software contains SiRF Technology, Inc.'s confidential and      *
 *    proprietary information. UNAUTHORIZED COPYING, USE, DISTRIBUTION,    *
 *    PUBLICATION, TRANSFER, SALE, RENTAL OR DISCLOSURE IS PROHIBITED      *
 *    AND MAY RESULT IN SERIOUS LEGAL CONSEQUENCES.  Do not copy this      *
 *    Software without SiRF Technology, Inc.'s  express written            *
 *    permission.   Use of any portion of the contents of this Software    *
 *    is subject to and restricted by your written agreement with          *
 *    SiRF Technology, Inc.                                                *
 *                                                                         *
 ***************************************************************************/

/** 
 * @file OMX_VideoExtAlloc.h - OpenMax IL version 1.1.2 Allocator extension
 * The structures needed by Video components to exchange vendor specified (PrimaII)
 * parameters and configuration data with the components.
 */
#ifndef OMX_VideoExtAlloc_h
#define OMX_VideoExtAlloc_h


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Each OMX header must include all required header files to allow the 
 * header to compile without errors.  The includes below are required  
 * for this header file to compile successfully 
 */
#include <OMX_IVCommon.h>

#define OMX_EXT_SURFACE_VERSION		0x01

/**
 * Name space for vendor specific: "OMX.PRIMAII.index.param.extended_video.allocator"
 * Example:
 *   OMX_GetExtensionIndex(&component, "OMX.PRIMAII.index.param.extended_video.allocator", &eExtendedIndex);
 *   eErr = OMX_SetParameter(psAppPriv->hComponent, eExtendedIndex, pExtParamAlloc));
 *   ...
 */

/** The OMX_EXT_INTERLACE_TYPE enumeration is used to specify the type
 *  of interlace flag of a frame
 *  @ingroup extension
 */
typedef enum OMX_EXT_SURFACE_FLAG
{
    OMX_EXT_SURFACE_FLAG_NONE           = 0,
    OMX_EXT_SURFACE_FLAG_TOP_FIRST      = 1,
    OMX_EXT_SURFACE_FLAG_BOTTOM_FIRST   = 2,
} OMX_EXT_SURFACE_FLAG;


typedef enum OMX_EXT_SURFACE_LOCK_PARAM
{
    OMX_EXT_SURFACE_LOCK_NONE           = 0,
    OMX_EXT_SURFACE_LOCK_WAIT           = 1,
} OMX_EXT_SURFACE_LOCK_PARAM;


typedef struct _OMX_EXT_SURFACE_CREATE_PARAM
{
    OMX_VERSIONTYPE             nVersion;   /**< OMX specification version information */
    OMX_COLOR_FORMATTYPE        eColorFormat;
    OMX_U32                     nWidth;
    OMX_U32                     nHeight;
	OMX_U32                     nSize;
	OMX_U32                     nHStride;
    OMX_U32                     nVStride;
    OMX_U32                     nAlignedByteWished;
} OMX_EXT_SURFACE_CREATE_PARAM;


typedef struct _OMX_EXT_SURFACE
{

    /** nVersion is the version of the OMX specification that the structure 
        is built against.  It is the responsibility of the creator of this 
        structure to initialize this value and every user of this structure 
        should verify that it knows how to use the exact version of 
        this structure found herein. */
    OMX_VERSIONTYPE             nVersion;

    OMX_PTR                     pExtPrivate;        /**< pointer to any data the external
                                     wants to associate with this buffer */

    OMX_COLOR_FORMATTYPE        eColorFormat;       /**< color space, OMX_COLOR_FormatYUV420PackedSemiPlanar e.g. OMX_IVCommon.h */

	OMX_U32                     nWidth;             /**< surface width */
	OMX_U32                     nHeight;            /**< surface height */
    OMX_U32                     nPitch;             /**< bytes to next line */
    OMX_U32                     nSize;              /**< surface buffer size occupied in bytes */
	OMX_HANDLETYPE              plSurfAddr;         /**< surface flag, read-only to allocator and app */

	OMX_U32                     eFlag;              /**< surface flag, read-only to allocator and app */

    OMX_ERRORTYPE (*Lock)(
        OMX_IN OMX_HANDLETYPE   hSurface,
        OMX_IN OMX_U32          eFlag,              /**< defined OMX_EXT_SURFACE_LOCK_PARAM */
        OMX_INOUT OMX_PTR       *ppBuffer           /**< pointer to buffer address */
        );

    OMX_ERRORTYPE (*Unlock)(
        OMX_IN  OMX_HANDLETYPE  hSurface
        );

    OMX_U32 (*AddRef)(
        OMX_IN  OMX_HANDLETYPE  hSurface
        );

    OMX_U32 (*Release)(
        OMX_IN  OMX_HANDLETYPE  hSurface
        );

} OMX_EXT_SURFACE;


/** structure corresponding to OMX_SetParameter */ 
typedef struct _OMX_EXT_SURFACE_ALLOCATOR
{
    OMX_VERSIONTYPE             nVersion;           /**< OMX specification version information */

    OMX_PTR                     pExtPrivate;        /**< pointer to any data the external
														wants to associate with this buffer */

    OMX_ERRORTYPE (*CreateSurface)(
            OMX_IN  OMX_HANDLETYPE				hAllocator,
            OMX_IN OMX_EXT_SURFACE_CREATE_PARAM *pParam,	/**< OMX_EXT_SURFACE_CREATE_PARAM */
            OMX_OUT OMX_EXT_SURFACE				**ppSurface
            );

} OMX_EXT_SURFACE_ALLOCATOR;


/** The OMX_EXT_CreateSurface method will request the allocator to create
    surfaces. This method will allocate surfaces and return the surfaces
    handlers. The method will success and return valid surface handlers 
    only if all requested surfaces are created. This is a blocking call.
    
    The core should return after allocate returns.
    
    @param [in] hAlloc
        handler of the allocator.
    @param [out] phSurface
        handler array to the surface to be returned, the size must be 
        at least nNum.
    @param [in] nNum
        number of surface to be created.
    @param [in] pParam
        pointer to a OMX_EXT_SURFACE_CREATE_PARAM structure to be used
        as a surface create parameter.
    @return OMX_ERRORTYPE
        If the command successfully executes, the return code will be
        OMX_ErrorNone.  When the value of nIndex exceeds the number of 
        components in the system minus 1, OMX_ErrorNoMore will be
        returned. Otherwise the appropriate OMX error will be returned.
    @ingroup extension
 */
#define OMX_EXT_Surface_Create(                                     \
        hAllocator,                                                 \
        pParam,                                                     \
        ppSurface)                                                  \
    ((OMX_EXT_PARAM_ALLOCATOR *)hAlloc)->CreateSurface(             \
        hAllocator,                                                 \
        pParam,                                                     \
        ppSurface)                            /* Macro End */


#define OMX_EXT_Surface_Lock(                                       \
        hSurface,                                                   \
        eFlag,                                                      \
        ppBuffer)                                                   \
    ((OMX_EXT_SURFACE *)hSurface)->Lock(                            \
        hSurface,                                                   \
        eFlag,                                                      \
        ppBuffer)                          /* Macro End */


#define OMX_EXT_Surface_Unlock(                                     \
        hSurface)                                                   \
    ((OMX_EXT_SURFACE *)hSurface)->Unlock(                          \
        hSurface)                          /* Macro End */


#define OMX_EXT_Surface_AddRef(                                     \
        hSurface)                                                   \
    ((OMX_EXT_SURFACE *)hSurface)->AddRef(                          \
        hSurface)                          /* Macro End */


#define OMX_EXT_Surface_Release(                                    \
        hSurface)                                                   \
    ((OMX_EXT_SURFACE *)hSurface)->Release(                         \
        hSurface)                          /* Macro End */


/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
/* File EOF */
