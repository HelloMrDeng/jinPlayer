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

#pragma once

#include <OMX_Core.h>
#include <OMX_Types.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>
#include <OMX_Component.h>

#define NUM_INPUT_BUF	2
#define NUM_OUTPUT_BUF	4
#define SIZE_INPUT_BUF	(1024*1024)

#define INIT_OMX_PARAM(param)	ZeroMemory(&param, sizeof(param)); param.nSize = sizeof(param);

static LPCTSTR g_pszOMXCoreDll = _T("omx_core.dll");

//	{ "OMX.IMG.MSVDX.MPEG2.Decoder",	"video_decoder.mpeg2" },	//	CODEC_MPEG2,
//	{ "OMX.IMG.MSVDX.MPEG4.Decoder",	"video_decoder.mpeg4" },	//	CODEC_MPEG4,
//	{ "OMX.IMG.MSVDX.AVC.Decoder",		"video_decoder.avc" },		//	CODEC_H264,
//	{ "OMX.IMG.MSVDX.VC1.Decoder",		"video_decoder.wmv" },		//	CODEC_WMV,
//	{ "OMX.IMG.MSVDX.REAL.Decoder",		"video_decoder.rv" },		//	CODEC_RM
//	{ "OMX.IMG.MSVDX.MPEG4.Decoder",	"video_decoder.mpeg4" },	//	CODEC_H263,

class COMXCore
{
public:
	COMXCore()
	{
		m_hModule = NULL;
		m_bInited = FALSE;
		m_OMX_Init = NULL;
		m_OMX_Deinit = NULL;
		m_OMX_GetHandle = NULL;
		m_OMX_FreeHandle = NULL;
	}
	~COMXCore()
	{

	}

	BOOL Load(LPCTSTR pszPath)
	{
		TCHAR szOMXCorePath[MAX_PATH];
		_tcscpy_s(szOMXCorePath, MAX_PATH, pszPath);

		TCHAR* psz = _tcsrchr(szOMXCorePath, '\\');
		if (psz != NULL)
			_tcscpy_s(psz + 1, MAX_PATH - (psz - szOMXCorePath), g_pszOMXCoreDll);
		else
			_tcscpy_s(szOMXCorePath, MAX_PATH, g_pszOMXCoreDll);

		m_hModule = ::LoadLibrary(szOMXCorePath);
		if (m_hModule == NULL)
			m_hModule = ::LoadLibrary(g_pszOMXCoreDll);
		if (m_hModule == NULL)
			return FALSE;

		(PROC&)m_OMX_Init = ::GetProcAddress(m_hModule, _T("OMX_Init"));
		(PROC&)m_OMX_Deinit = ::GetProcAddress(m_hModule, _T("OMX_Deinit"));
		(PROC&)m_OMX_GetHandle = ::GetProcAddress(m_hModule, _T("OMX_GetHandle"));
		(PROC&)m_OMX_FreeHandle = ::GetProcAddress(m_hModule, _T("OMX_FreeHandle"));
		return m_OMX_Init != NULL
			&& m_OMX_Deinit != NULL
			&& m_OMX_GetHandle != NULL
			&& m_OMX_FreeHandle != NULL;
	}

	BOOL UnLoad()
	{
		if (m_hModule != NULL)
			::FreeLibrary(m_hModule);
		return TRUE;
	}

	OMX_ERRORTYPE Init()
	{
		if (m_bInited)
			return OMX_ErrorNone;
		if (m_OMX_Init != NULL)
		{
			OMX_ERRORTYPE ret = m_OMX_Init();
			m_bInited = ret == OMX_ErrorNone;
			return ret;
		}
		return OMX_ErrorUndefined;
	}

	OMX_ERRORTYPE Deinit()
	{
		if (! m_bInited)
			return OMX_ErrorNone;
		if (m_OMX_Deinit != NULL)
		{
			OMX_ERRORTYPE ret = m_OMX_Deinit();
			m_bInited = FALSE;
			return ret;
		}
		return OMX_ErrorUndefined;
	}

	OMX_ERRORTYPE GetHandle(OMX_HANDLETYPE* pHandle, OMX_STRING cComponentName, OMX_PTR pAppData, OMX_CALLBACKTYPE* pCallBacks)
	{
		if (m_OMX_GetHandle != NULL)
			return m_OMX_GetHandle(pHandle, cComponentName, pAppData, pCallBacks);
		return OMX_ErrorUndefined;
	}

	OMX_ERRORTYPE FreeHandle(OMX_HANDLETYPE hComponent)
	{
		if (m_OMX_FreeHandle != NULL)
			return m_OMX_FreeHandle(hComponent);
		return OMX_ErrorUndefined;
	}

protected:
	HMODULE m_hModule;
	BOOL	m_bInited;

	OMX_ERRORTYPE (*m_OMX_Init)(void);
	OMX_ERRORTYPE (*m_OMX_Deinit)(void);
	OMX_ERRORTYPE (*m_OMX_GetHandle)(
					OMX_OUT OMX_HANDLETYPE* pHandle, 
					OMX_IN  OMX_STRING cComponentName,
					OMX_IN  OMX_PTR pAppData,
					OMX_IN  OMX_CALLBACKTYPE* pCallBacks);
	OMX_ERRORTYPE (*m_OMX_FreeHandle)(
					OMX_IN  OMX_HANDLETYPE hComponent);
};

extern COMXCore _Core;
