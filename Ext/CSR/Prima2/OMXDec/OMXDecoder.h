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

#include "OMXPort.h"
#include <OMX_VideoExtAlloc.h>

class COMXDecoder
{
public:
	COMXDecoder();
	~COMXDecoder();

	OMX_ERRORTYPE Create(LPCSTR pszComponent, LPCSTR pszRole, int nWidth, int nHeight, int nImageSize, OMX_EXT_SURFACE_ALLOCATOR* pExtAlloc = NULL);
	OMX_ERRORTYPE Destroy();

	OMX_ERRORTYPE Decode(BYTE* pbData, UINT cbData, OMX_TICKS nTimestamp, BOOL bEndOfFrame = FALSE, IUnknown* pFrame = NULL);

	OMX_ERRORTYPE BeginFlush();
	OMX_ERRORTYPE EndFlush();

protected:
	static void FillBuffer(OMX_BUFFERHEADERTYPE* pOMXBuf, void* pParam);
	OMX_ERRORTYPE FillBuffer(OMX_BUFFERHEADERTYPE* pOMXBuf);

	OMX_ERRORTYPE FillOutputBuffers();

	BOOL CanInputData();

	BOOL WaitState(OMX_STATETYPE nState);

	void WaitOutputBuffers();

	//	Overridable
	virtual OMX_ERRORTYPE OnSettingsChanged(int nPortIndex, DWORD dwParam);
	virtual OMX_ERRORTYPE OnEvent(OMX_EVENTTYPE eEvent, OMX_U32 ui32Data1, OMX_U32 ui32Data2, OMX_PTR pEventData);
	virtual OMX_ERRORTYPE OnEmptyBufferDone(OMX_BUFFERHEADERTYPE* pBuffer);
	virtual OMX_ERRORTYPE OnFillBufferDone(OMX_BUFFERHEADERTYPE* pBuffer);

	static OMX_ERRORTYPE EventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
				OMX_EVENTTYPE eEvent, OMX_U32 ui32Data1, OMX_U32 ui32Data2, OMX_PTR pEventData);
	static OMX_ERRORTYPE EmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
				OMX_BUFFERHEADERTYPE* pOMXBuf);
	static OMX_ERRORTYPE FillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
				OMX_BUFFERHEADERTYPE* pOMXBuf);

protected:
	CEvent		m_evtState;
	CEvent		m_evtFlush;
	CEvent		m_evtEnable;
	COMXPort*	m_pInPort;
	COMXPort*	m_pOutPort;
	CCriticalSection	m_lockBuffer;
	OMX_STATETYPE		m_nOMXState;
	LONG				m_nOutBufBusy;
	OMX_HANDLETYPE		m_hComponent;
	OMX_CALLBACKTYPE	m_Callbacks;
	BOOL				m_bFlushError;
};
