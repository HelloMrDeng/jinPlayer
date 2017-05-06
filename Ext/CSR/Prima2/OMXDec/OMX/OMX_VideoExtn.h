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
 * @file OMX_VideoExtension.h - OpenMax IL version 1.1.2
 * The structures needed by Video components to exchange vendor specified (MSVDX Specific) parameters and 
 * configuration data with the components.
 */
#ifndef OMX_VideoExtn_h
#define OMX_VideoExtn_h


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Each OMX header must include all required header files to allow the 
 * header to compile without errors.  The includes below are required  
 * for this header file to compile successfully 
 */
#include <OMX_IVCommon.h>

#define OMX_VIDEO_EXTENDED_PARAMS_FLAGS_MMU              (0x001) //!<Flag to configure MMU. Corresponding enum is OMX_EXTENDED_PARAM_MMU 
#define OMX_VIDEO_EXTENDED_PARAMS_FLAGS_SIMULATOR        (0x002) //!<Flag to configure simulator.
#define OMX_VIDEO_EXTENDED_PARAMS_FLAGS_LOAD_FIRMWARE    (0x004) //!<Flag to cofigure firmware.Corresponding enum is OMX_EXTENDED_PARAM_FIRMWARE_TYPE
#define OMX_VIDEO_EXTENDED_PARAMS_FLAGS_ERROR_HANDLING   (0x008) //!<Flag to configure error handling. Corresponding enum is OMX_EXTENDED_PARAM_ERROR_HANDLING
#define OMX_VIDEO_EXTENDED_PARAMS_FLAGS_TILING           (0x010) //!<Flag to configure MMU Tiling.Corresponding enum is OMX_EXTENDED_PARAM_TILING
#define OMX_VIDEO_EXTENDED_PARAMS_FLAG_ERROR_CONCEALMENT (0x020) //!<Flag to configure Error concealment options,enum is OMX_EXTENDED_PARAM_ERROR_CONCEALMENT
#define OMX_VIDEO_EXTENDED_PARAMS_FLAG_DELAY_IF_B_FRAME	 (0x040) //!<Flag to configure the logic of delay if B frame ,enum is OMX_EXTENDED_PARAM_DELAY_IF_B_FRAME
#define OMX_VIDEO_EXTENDED_PARAMS_FLAG_EXTENDED_STRIDE	 (0x080) //!<Flag to configure the extended stride


/*!enum corresponding to OMX_VIDEO_EXTENDED_PARAMS_FLAGS_MMU flag*/
typedef enum
{
    eExtParamMMUEnable,   /*MMU is enabled by default*/
    eExtParamMMUDisable
}OMX_EXTENDED_PARAM_MMU; 

/*!enum corresponding to OMX_VIDEO_EXTENDED_PARAMS_FLAGS_LOAD_FIRMWARE flag*/
typedef enum
{
    eExtParamHeterogeneousFW,
    eExtParamHomogeneousFW,
    eExtParamSingleStreamFW
}OMX_EXTENDED_PARAM_FIRMWARE_TYPE;

/*!enum corresponding to OMX_VIDEO_EXTENDED_PARAMS_FLAGS_ERROR_HANDLING flag*/
typedef enum
{
    eExtParamErrorHandlingEnable,  /*Error Handling is enabled by default*/
    eExtParamErrorHandlingDisable
}OMX_EXTENDED_PARAM_ERROR_HANDLING;

/*!enum corresponding to OMX_VIDEO_EXTENDED_PARAMS_FLAGS_TILING flag */
typedef enum
{
    eExtParamTilingDisable, /*Tiling is disabled by default*/
    eExtParamTilingEnable
}OMX_EXTENDED_PARAM_TILING;

/*!enum corresponding to OMX_VIDEO_EXTENDED_PARAMS_FLAGS_SIMULATOR flag */ 
typedef enum
{
    eExtParamSimulatorDisable, /*simulator is disabled by default*/
    eExtParamSimulatorEnable
}OMX_EXTENDED_PARAM_SIMULATOR;

/*! structure corresponding to OMX_VIDEO_EXTENDED_PARAMS_ERROR_CONCEALMENT flag */ 
typedef struct _OMX_EXTENDED_PARAM_ERROR_CONCEALMENT
{
	OMX_BOOL eExtParamErrorsConcealFE;
	OMX_BOOL eExtParamErrorsConcealBE;
	OMX_BOOL eExtParamErrorsConcealGrey;
	OMX_BOOL eExtParamErrorsProcessEntdec;
	OMX_BOOL eExtParamErrorsProcessSR;
	OMX_BOOL eExtParamErrorsProcessWDT;
}OMX_EXTENDED_PARAM_ERROR_CONCEALMENT;

/*!enum corresponding to OMX_VIDEO_EXTENDED_PARAMS_FLAG_DELAY_IF_B_FRAME flag */ 
typedef enum
{
    eExtParamDelayIfBFrameLogicDisable, /*DelayifBframe logic is disabled by default*/
    eExtParamDelayIfBFrameLogicEnable
}OMX_EXTENDED_PARAM_DELAY_IF_B_FRAME;



/*!enum corresponding to OMX_VIDEO_EXTENDED_PARAMS_FLAG_EXTENDED_STRIDE */ 
typedef enum
{
    eExtParamExtendedStrideEnable, /*Extended Stride is true by default*/
    eExtParamExtendedStrideDisable
}OMX_EXTENDED_PARAM_EXTENDED_STRIDE;

/** 
 * Extended Parameter Structure for the OpenMax Video component.
 * Optionally used to pass extra vendor specific command line options directly into the component for testing.
 *
 */
typedef struct _OMX_VIDEO_EXTENDED_PARAMS {
    OMX_U32 ui32Flags; 
	OMX_STRING pacSocketInfo[OMX_MAX_STRINGNAME_SIZE];
    OMX_EXTENDED_PARAM_FIRMWARE_TYPE eExtFirmware_type; //!< Type of Firmware
    OMX_EXTENDED_PARAM_ERROR_HANDLING eErrorHandling; //!< enable/disable error handling
    OMX_EXTENDED_PARAM_TILING eTiling; //!<enable/disable MMU tiling     
    OMX_EXTENDED_PARAM_MMU eMMU; //!<enable/disable MMU 
    OMX_EXTENDED_PARAM_SIMULATOR eSimulator; //!enables/disables simulator
    OMX_EXTENDED_PARAM_DELAY_IF_B_FRAME eDelayIfBFrame;//!enables/disables the logic of delaying the scaling/rotation till an I- or P-frame arrives
	OMX_EXTENDED_PARAM_ERROR_CONCEALMENT eErrorConcealment;
    OMX_EXTENDED_PARAM_EXTENDED_STRIDE eExtendedStride;//!enables/disables the use of Extended Stride
	OMX_BOOL bDWR;
    OMX_BOOL bSetFEWDT;
    OMX_BOOL bSetBEWDT;
	OMX_U32 ui32DWRFrameCount;
	OMX_U32 ui32DWRFrameCount1;
	OMX_U32 ui32DeviceWatchDuration;
}OMX_VIDEO_EXTENDED_PARAMS;





/** 
 * AVS profile types, each profile indicates support for various 
 * performance bounds and different annexes.
 */
typedef enum OMX_VIDEO_AVSPROFILETYPE {
    OMX_VIDEO_AVSProfileJizhun = 0,  /**< Jizhun Profile */
    OMX_VIDEO_AVSProfileMax = 0x7FFFFFFF  
} OMX_VIDEO_AVSPROFILETYPE;


/** 
 * AVS level types, each level indicates support for various frame 
 * sizes, bit rates, decoder frame rates.  No need 
 */
typedef enum OMX_VIDEO_AVSLEVELTYPE {
    OMX_VIDEO_AVSLevel20 = 0,   /**< 2.0 Level */ 
    OMX_VIDEO_AVSLevel40,       /**< 4.0 Level */ 
    OMX_VIDEO_AVSLevel42,       /**< 4.2 Level */ 
    OMX_VIDEO_AVSLevel60,       /**< 6.0 Level */   
    OMX_VIDEO_AVSLevel62,       /**< 6.2 Level */ 
    OMX_VIDEO_AVSLevelMax = 0x7FFFFFFF 
}OMX_VIDEO_AVSLEVELTYPE;

/** 
 *  VC1/WMV profile types, each profile indicates support for various 
 * performance bounds and different annexes.
 */
typedef enum OMX_VIDEO_VC1_WMV9PROFILETYPE {
    OMX_VIDEO_VC1_WMVProfileSimple = 0  ,  /**<  VC1/WMV Simple Profile */
    OMX_VIDEO_VC1_WMVProfileMain        ,  /**<  VC1/WMV Main Profile */
    OMX_VIDEO_VC1_WMVProfileAdvanced    ,  /**<  VC1/WMV Advanced Profile */
    OMX_VIDEO_VC1_WMVProfileMax = 0x7FFFFFFF  
} OMX_VIDEO_VC1_WMVPROFILETYPE;


/** 
 * VC1/WMV level types, each level indicates support for various frame 
 * sizes, bit rates, decoder frame rates.  No need 
 */
typedef enum OMX_VIDEO_VC1_WMV9LEVELTYPE {
    OMX_VIDEO_VC1_WMVLevelLow = 0,      /**< Low Level      */ 
    OMX_VIDEO_VC1_WMVLevelMedium,       /**< Medium Level   */ 
    OMX_VIDEO_VC1_WMVLevelHigh,         /**< High Level     */ 
    OMX_VIDEO_VC1_WMVLevelL0,           /**< L0 Level       */   
    OMX_VIDEO_VC1_WMVLevelL1,           /**< L1 Level       */ 
    OMX_VIDEO_VC1_WMVLevelL2,           /**< L2 Level       */   
    OMX_VIDEO_VC1_WMVLevelL3,           /**< L3 Level       */ 
    OMX_VIDEO_VC1_WMVLevelL4,           /**< L4 Level       */ 
    OMX_VIDEO_VC1_WMVLevelSMPTEReserved, /**< SMPTEReserved        */ 
    OMX_VIDEO_VC1_WMV9LevelMax = 0x7FFFFFFF 
}OMX_VIDEO_VC1_WMV9LEVELTYPE;

/*! 
 *  \enum OMX_VIDEO_VP6PROFILETYPE
 *  \brief VP6 profile types, each profile indicates support for various
    performance bounds and different annexes.
 */
typedef enum OMX_VIDEO_VP6PROFILETYPE {
    OMX_VIDEO_VP6ProfileSimple = 0  ,  /**< Simple Profile */
    OMX_VIDEO_VP6ProfileAdvanced    ,  /**< Advanced Profile */
    OMX_VIDEO_VP6ProfileMax = 0x7FFFFFFF  
} OMX_VIDEO_VP6PROFILETYPE;

/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
/* File EOF */
