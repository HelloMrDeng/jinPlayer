/*******************************************************************************
	File:		CStreamDRM.h

	Contains:	The stream drm header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-07-17		Fenger			Create file

*******************************************************************************/
#ifndef __CStreamDRM_H__
#define __CStreamDRM_H__

#include "voDRM2.h"
#include "voAdaptiveStreamParser.h"

#include "CBaseObject.h"

#define	VO_PID_AS_DRM2_BASE				0x432A0000						/*!< the base param ID for adaptive streaming DRM2 modules */
#define VO_PID_AS_DRM2_NEW_API			(VO_PID_AS_DRM2_BASE | 0x0001)	/*!<g> get a new DRM wrapper's callback API for multi-thread*/
#define VO_PID_AS_DRM2_CONVERT_URL		(VO_PID_AS_DRM2_BASE | 0x0002)	/*!<g> transform the URL for customer*/
#define VO_PID_AS_DRM2_STREAMING_TYPE	(VO_PID_AS_DRM2_BASE | 0x0003)	/*!<s> set the type of adaptive streaming, refer to VO_ADAPTIVESTREAMPARSER_STREAMTYPE*/

class CStreamDRM : public CBaseObject
{
public:
    CStreamDRM (VO_SOURCE2_IO_API* pIO, VO_PTR pReserved);
    virtual ~CStreamDRM(void);

	VO_U32		PreprocessURL (const VO_CHAR* urlSrc, VO_CHAR* urlDes, VO_PTR pReserved);
	VO_U32		Info (VO_CHAR* szManifestURL, VO_BYTE* pManifest, VO_U32 uSizeManifest, VO_PTR pReserved);

	VO_U32		DataBegin(VO_U32 uIdentifer, VO_PTR pInfo);
	VO_U32		DataProcess_Chunk(VO_U32 uIdentifer, VO_U64 nOffset, VO_PBYTE pSrcData, VO_U32 nSrcSize, VO_PBYTE *ppDesData, VO_U32 *pnDesSize, VO_BOOL bChunkEnd, VO_PTR pAdditionalInfo);	//File Chunk DRM
	VO_U32		DataEnd(VO_U32 uIdentifer, VO_PTR pInfo);

	VO_U32		Info_FR(VO_PTR pInfo, VO_DRM2_INFO_TYPE eInfoType, VO_PTR pReserved);
	VO_U32		DataProcess_FR(VO_PBYTE pSrcData, VO_U32 nSrcSize, VO_PBYTE *ppDesData, VO_U32 *pnDesSize, VO_DRM2_DATATYPE eDataType, VO_DRM2_DATAINFO_TYPE eInfoType, VO_PTR pAdditionalInfo);		//Media Format DRM 
	VO_U32		PreprocessPlaylist(VO_BYTE* pPlaylist, VO_U32 uSizeBuffer, VO_U32* puSizePlaylist, VO_PTR pReserved);

	VO_U32		SetParameter(VO_U32 uID, VO_PTR pParam);
	VO_U32		GetParameter(VO_U32 uID, VO_PTR pParam);

protected:
	VO_U32			Init (void);
	VO_U32			Uninit (void);

	static VO_S32	OSDRMListener(VO_PTR pUserData, VO_U32 nID, VO_U32 nParam1, VO_U32 nParam2);
	int				HandleEvent(int nID, void * pParam1, void * pParam2);
	static VO_U32	VerifyCallBackFunc( VO_PTR hHandle , VO_U32 uID , VO_PTR pUserData );
	VO_U32			doVerifyCallBackFunc(VO_U32 nID, VO_PTR pParam1);

protected:
	VO_SOURCE2_IO_API *		m_pIO;
	VO_SOURCEDRM_CALLBACK2*	m_pDRMCB;
	VO_DRM2_SOURCE_FORMAT	m_eDRMSource;

	void *						m_hDRMLib;
	VO_DRM2_API					m_fAPI;
	VO_DRM_OPENPARAM			m_sOpenParam;
	void*						m_hDrmHandle;
	VO_SOURCE2_EVENTCALLBACK	m_cEventCallBack;
	VO_SOURCE2_IO_HTTP_VERIFYCALLBACK	m_sVerifyCallBack;

};

#endif
