/*******************************************************************************
	File:		CStreamDRM.cpp

	Contains:	stream drm implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-07-17		Fenger			Create file

*******************************************************************************/
#include "CStreamDRM.h"

#include "ULibFunc.h"
#include "yyLog.h"

typedef VO_S32 (VO_API * VOGETDRM2API)(VO_DRM2_API * pDRMHandle, VO_U32 uFlag);

CStreamDRM::CStreamDRM (VO_SOURCE2_IO_API* pIO, VO_PTR pReserved)
	: CBaseObject ()
	, m_pIO (pIO)
	, m_pDRMCB (NULL)
	, m_eDRMSource (VO_DRM2SRC_CHUNK)
	, m_hDRMLib (NULL)
	, m_hDrmHandle (NULL)
{
	SetObjectName ("CStreamDRM");
	memset (&m_fAPI, 0, sizeof (m_fAPI));
	memset (&m_cEventCallBack, 0, sizeof (m_cEventCallBack));
	memset (&m_sVerifyCallBack, 0, sizeof (m_sVerifyCallBack));
	Init ();
}

CStreamDRM::~CStreamDRM(void)
{
	Uninit ();
}

VO_U32 CStreamDRM::PreprocessURL(const VO_CHAR* urlSrc, VO_CHAR* urlDes, VO_PTR pReserved)
{
	VO_DRM2_CONVERT_URL s_Convert_URL = {0};
	s_Convert_URL.urlSrc	= urlSrc;
	s_Convert_URL.urlDes	= urlDes;
	s_Convert_URL.pReserved	= pReserved;

	return m_pDRMCB ? m_pDRMCB->fCallback(m_pDRMCB->pUserData, VO_DRM_FLAG_CONVERTURL, &s_Convert_URL, m_eDRMSource) : VO_ERR_DRM2_NO_DRM_API;
}

VO_U32 CStreamDRM::Info(VO_CHAR* szManifestURL, VO_BYTE* pManifest, VO_U32 uSizeManifest, VO_PTR pReserved)
{
	VO_DRM2_INFO_ADAPTIVESTREAMING s_infoDrm;
	memset( &s_infoDrm, 0, sizeof(VO_DRM2_INFO_ADAPTIVESTREAMING) );
	if (m_pDRMCB)
		strcpy(s_infoDrm.szDRMTYPE, m_pDRMCB->szDRMTYPE);
	s_infoDrm.pURL			= szManifestURL;
	s_infoDrm.pManifestData	= pManifest;
	s_infoDrm.uSizeManifest	= uSizeManifest;
	s_infoDrm.pAPI_IO		= m_pIO;
	s_infoDrm.pReserved		= pReserved;

	return m_pDRMCB ? m_pDRMCB->fCallback(m_pDRMCB->pUserData, VO_DRM_FLAG_DRMINFO, &s_infoDrm, m_eDRMSource) : VO_ERR_DRM2_NO_DRM_API;
}

VO_U32 CStreamDRM::DataBegin(VO_U32 uIdentifer, VO_PTR pInfo)
{
	VO_DRM2_DATA s_dataDrm;
	memset( &s_dataDrm, 0, sizeof(VO_DRM2_DATA) );
	s_dataDrm.sDataInfo.nDataType	= VO_DRM2DATATYPE_CHUNK_BEGIN;
	s_dataDrm.sDataInfo.pInfo		= pInfo;
	s_dataDrm.nReserved[2]			= uIdentifer;

	return m_pDRMCB ? m_pDRMCB->fCallback(m_pDRMCB->pUserData, VO_DRM_FLAG_DRMDATA, &s_dataDrm, m_eDRMSource) : VO_ERR_DRM2_NO_DRM_API;
}

VO_U32 CStreamDRM::DataProcess_Chunk(VO_U32 uIdentifer, VO_U64 nOffset, VO_PBYTE pSrcData, VO_U32 nSrcSize, VO_PBYTE *ppDesData, VO_U32 *pnDesSize, VO_BOOL bChunkEnd, VO_PTR pAdditionalInfo)
{
	VO_DRM2_DATA s_dataDrm;
	memset( &s_dataDrm, 0, sizeof(VO_DRM2_DATA) );
	s_dataDrm.sDataInfo.nDataType	= VO_DRM2DATATYPE_CHUNK_PROCESSING;
	s_dataDrm.sDataInfo.pInfo		= pAdditionalInfo;
	s_dataDrm.pData					= pSrcData;
	s_dataDrm.nSize					= nSrcSize;
	s_dataDrm.ppDstData				= ppDesData;
	s_dataDrm.pnDstSize				= pnDesSize;
	if (bChunkEnd)
		s_dataDrm.nReserved[0]		= 1;
	s_dataDrm.nReserved[1]			= (VO_U32)&nOffset;
	s_dataDrm.nReserved[2]			= uIdentifer;

	return m_pDRMCB ? m_pDRMCB->fCallback(m_pDRMCB->pUserData, VO_DRM_FLAG_DRMDATA, &s_dataDrm, m_eDRMSource) : VO_ERR_DRM2_NO_DRM_API;
}

VO_U32 CStreamDRM::DataEnd(VO_U32 uIdentifer, VO_PTR pInfo)
{
	VO_DRM2_DATA s_dataDrm;
	memset( &s_dataDrm, 0, sizeof(VO_DRM2_DATA) );
	s_dataDrm.sDataInfo.nDataType	= VO_DRM2DATATYPE_CHUNK_END;
	s_dataDrm.sDataInfo.pInfo		= pInfo;
	s_dataDrm.nReserved[2]			= uIdentifer;

	return m_pDRMCB ? m_pDRMCB->fCallback(m_pDRMCB->pUserData, VO_DRM_FLAG_DRMDATA, &s_dataDrm, m_eDRMSource) : VO_ERR_DRM2_NO_DRM_API;
}

VO_U32 CStreamDRM::Info_FR(VO_PTR pInfo, VO_DRM2_INFO_TYPE eInfoType, VO_PTR pReserved)
{
	VO_DRM2_INFO s_infoDrm;
	memset( &s_infoDrm, 0, sizeof(VO_DRM2_INFO) );

	s_infoDrm.nType			= VO_DRM2TYPE_UNKNOWN;
	s_infoDrm.pDrmInfo		= pInfo;
	s_infoDrm.nReserved[0]	= eInfoType;

	VO_DRM_CALLBACK_FLAG eflag = VO_DRM_FLAG_FLUSH;
	if (VO_DRM2_INFO_PROCECTION == eInfoType)
		eflag = VO_DRM_FLAG_DRM_PROCECTION;
	else
		eflag = VO_DRM_FLAG_DRM_TRACK_INFO;

	return m_pDRMCB ? m_pDRMCB->fCallback(m_pDRMCB->pUserData, eflag, &s_infoDrm, m_eDRMSource) : VO_ERR_DRM2_NO_DRM_API;
}

VO_U32 CStreamDRM::DataProcess_FR(VO_PBYTE pSrcData, VO_U32 nSrcSize, VO_PBYTE *ppDesData, VO_U32 *pnDesSize, VO_DRM2_DATATYPE eDataType, VO_DRM2_DATAINFO_TYPE eInfoType, VO_PTR pAdditionalInfo)
{
	VO_DRM2_DATA s_dataDrm;
	memset( &s_dataDrm, 0, sizeof(VO_DRM2_DATA) );

	s_dataDrm.sDataInfo.nDataType	= eDataType;
	s_dataDrm.sDataInfo.pInfo		= pAdditionalInfo;
	s_dataDrm.sDataInfo.nReserved[0]= eInfoType;
	s_dataDrm.pData					= pSrcData;
	s_dataDrm.nSize					= nSrcSize;
	s_dataDrm.ppDstData				= ppDesData;
	s_dataDrm.pnDstSize				= pnDesSize;

	return m_pDRMCB ? m_pDRMCB->fCallback(m_pDRMCB->pUserData, VO_DRM_FLAG_DRMDATA, &s_dataDrm, m_eDRMSource) : VO_ERR_DRM2_NO_DRM_API;
}

VO_U32 CStreamDRM::PreprocessPlaylist(VO_BYTE* pPlaylist, VO_U32 uSizeBuffer, VO_U32* puSizePlaylist, VO_PTR pReserved)
{
	if (NULL == m_pDRMCB)
		return VO_ERR_DRM2_NO_DRM_API;

	VO_DRM2_CONVERT_PLAYLIST s_ConvertPlaylist;
	memset( &s_ConvertPlaylist, 0, sizeof(VO_DRM2_CONVERT_PLAYLIST) );

	s_ConvertPlaylist.pPlaylist			= pPlaylist;
	s_ConvertPlaylist.uSizeBuffer		= uSizeBuffer;
	s_ConvertPlaylist.puSizePlaylist	= puSizePlaylist;
	s_ConvertPlaylist.pReserved			= pReserved;

	VO_U32 uRet = m_pDRMCB->fCallback(m_pDRMCB->pUserData, VO_DRM_FLAG_PLAYLIST, &s_ConvertPlaylist, m_eDRMSource);
	switch (uRet)
	{
	case VO_ERR_OUTPUT_BUFFER_SMALL:
		{
			return VO_RET_SOURCE2_OUTPUTDATASMALL;
		}
		break;

	case VO_ERR_NOT_IMPLEMENT:
		{
			return VO_RET_SOURCE2_NOIMPLEMENT;
		}
		break;

	default:
		return VO_RET_SOURCE2_FAIL;
	}
}

VO_U32 CStreamDRM::GetParameter(VO_U32 uID, VO_PTR pParam)
{
	if (NULL == pParam)
		return VO_RET_SOURCE2_EMPTYPOINTOR;

	switch (uID)
	{
	case VO_PID_AS_DRM2_NEW_API:
		{
			return m_pDRMCB ? m_pDRMCB->fCallback(m_pDRMCB->pUserData, VO_DRM_FLAG_NEWAPI, pParam, m_eDRMSource) : VO_ERR_DRM2_NO_DRM_API;
		}
		break;

	default:
		return VO_RET_SOURCE2_NOIMPLEMENT;
	}

	return VO_RET_SOURCE2_OK;
}

VO_U32 CStreamDRM::SetParameter(VO_U32 uID, VO_PTR pParam)
{
	if (NULL == pParam)
		return VO_RET_SOURCE2_EMPTYPOINTOR;

	switch (uID)
	{
	case VO_PID_AS_DRM2_STREAMING_TYPE:
		{
			switch (*(VO_ADAPTIVESTREAMPARSER_STREAMTYPE*)pParam)
			{
			case VO_ADAPTIVESTREAMPARSER_STREAMTYPE_HLS:
				{
					m_eDRMSource = VO_DRM2SRC_CHUNK_HLS;
				}
				break;

			case VO_ADAPTIVESTREAMPARSER_STREAMTYPE_ISS:
				{
					m_eDRMSource = VO_DRM2SRC_CHUNK_SSTR;
				}
				break;

			case VO_ADAPTIVESTREAMPARSER_STREAMTYPE_DASH:
				{
					m_eDRMSource = VO_DRM2SRC_CHUNK_DASH;
				}
				break;

			default:
				m_eDRMSource = VO_DRM2SRC_CHUNK;
			}
		}
		break;

	default:
		return VO_RET_SOURCE2_NOIMPLEMENT;
	}

	return VO_RET_SOURCE2_OK;
}

VO_U32 CStreamDRM::Init (void)
{
	m_hDRMLib = yyLibLoad (_T("yyDRMAES128"), 0);
	if (m_hDRMLib == NULL)
		return VO_ERR_FAILED;
	VOGETDRM2API pGetDRM2API = NULL;
	pGetDRM2API = (VOGETDRM2API)yyLibGetAddr (m_hDRMLib, "yyGetDRMAPI", 0);
	if (pGetDRM2API == NULL)
		return VO_ERR_FAILED;
	pGetDRM2API (&m_fAPI, 0);
	if (m_fAPI.Init == NULL)
		return VO_ERR_FAILED;
	m_sOpenParam.nFlag = 0;
	m_sOpenParam.pLibOP = NULL;
	int nRC = m_fAPI.Init(&m_hDrmHandle, &m_sOpenParam);
	if (m_hDrmHandle == NULL)
		return VO_ERR_FAILED;

	m_cEventCallBack.pUserData =  this;
	m_cEventCallBack.SendEvent = OSDRMListener;
	m_sVerifyCallBack.hHandle = this;
	m_sVerifyCallBack.HTTP_Callback = VerifyCallBackFunc;

	m_fAPI.GetInternalAPI(m_hDrmHandle, (VO_PTR *)&m_pDRMCB);
	m_fAPI.SetParameter(m_hDrmHandle,VO_PID_SOURCE2_EVENTCALLBACK, &m_cEventCallBack);
	m_fAPI.SetParameter(m_hDrmHandle,VO_PID_SOURCE2_HTTPVERIFICATIONCALLBACK, &m_sVerifyCallBack);
//	m_fAPI.SetParameter(m_hDrmHandle,VO_PID_DRM2_PackagePath, m_szPathLib);	
//	m_fAPI.SetParameter(m_hDrmHandle, VO_PID_DRM2_UNIQUE_IDENTIFIER, m_sDRMUniqueIden);
//	m_fAPI.SetParameter(m_hDrmHandle, VO_PID_DRM2_WRITABLE_PATH, m_pDRMWritablePath);

	return VO_ERR_NONE;
}

VO_U32 CStreamDRM::Uninit (void)
{
	if (m_hDrmHandle)
	{
		m_fAPI.Uninit(m_hDrmHandle);
		m_hDrmHandle = 0;
	}
	if (m_hDRMLib != NULL)
	{
		yyLibFree (m_hDRMLib, 0);
		m_hDRMLib = NULL;
	}		
	memset (&m_fAPI, 0, sizeof (VO_DRM2_API));
	memset(&m_sOpenParam, 0, sizeof(VO_DRM_OPENPARAM));
	return VO_ERR_NONE;
}

VO_S32 CStreamDRM::OSDRMListener(VO_PTR pUserData, VO_U32 nID, VO_U32 nParam1, VO_U32 nParam2)
{
	CStreamDRM *pDRM = (CStreamDRM *)pUserData;

	int nID_DRM = nID;
	if (nID == VO_EVENTID_SOURCE2_ERR_DRMFAIL)
	{
		YYLOGT ("CStreamDRM", "VO_EVENTID_SOURCE2_ERR_DRMFAIL");
	}
	else if (nID == VO_EVENTID_DRM2_ERR_NOT_SECURE)
	{
		YYLOGT ("CStreamDRM", "VO_EVENTID_DRM2_ERR_NOT_SECURE");
	}
	else if (nID == VO_EVENTID_DRM2_ERR_POLICY_FAIL)
	{
		YYLOGT ("CStreamDRM", "VO_EVENTID_DRM2_ERR_POLICY_FAIL");
	}
	else if (nID == VO_EVENTID_SOURCE2_OUTPUT_CONTROL_SETTINGS)
	{
		YYLOGT ("CStreamDRM", "VO_EVENTID_SOURCE2_OUTPUT_CONTROL_SETTINGS");
	}

	return pDRM->HandleEvent(nID_DRM, &nParam1, &nParam2);
}

int CStreamDRM::HandleEvent(int nID, void * pParam1, void * pParam2)
{
	return VO_ERR_NONE;
}

VO_U32 CStreamDRM::VerifyCallBackFunc(VO_PTR hHandle , VO_U32 uID, VO_PTR pUserData )
{
	CStreamDRM *pDRM = (CStreamDRM *)hHandle;
	return pDRM->doVerifyCallBackFunc(uID, pUserData);
}

VO_U32 CStreamDRM::doVerifyCallBackFunc(VO_U32 uID , VO_PTR pUserData)
{
	if (uID == VO_SOURCE2_CALLBACKIDBASE_DRM)
	{
		YYLOGI ("VO_SOURCE2_CALLBACKIDBASE_DRM");
	}
	else if (uID == VO_SOURCE2_IO_HTTP_BEGIN)
	{
		YYLOGI ("VO_SOURCE2_IO_HTTP_BEGIN");
	}
	else if (uID == VO_SOURCE2_IO_HTTP_SOCKETCONNECTED)
	{
		YYLOGI ("VO_SOURCE2_IO_HTTP_SOCKETCONNECTED");
	}
	else if (uID == VO_SOURCE2_IO_HTTP_REQUESTPREPARED)
	{
		YYLOGI ("VO_SOURCE2_IO_HTTP_REQUESTPREPARED");
	}
	else if (uID == VO_SOURCE2_IO_HTTP_REQUESTSEND)
	{
		YYLOGI ("VO_SOURCE2_IO_HTTP_REQUESTSEND");
	}
	else if (uID == VO_SOURCE2_IO_HTTP_RESPONSERECVED)
	{
		YYLOGI ("VO_SOURCE2_IO_HTTP_RESPONSERECVED");
	}
	else if (uID == VO_SOURCE2_IO_HTTP_RESPONSEANALYSED)
	{
		YYLOGI ("VO_SOURCE2_IO_HTTP_RESPONSEANALYSED");
	}
	else if (uID == VO_SOURCE2_CB_DRM_INITDATA )
	{
		YYLOGI ("VO_SOURCE2_CB_DRM_INITDATA");
	}

	return 0;
}
