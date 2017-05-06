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

#include "stdafx.h"
#include "OMXDecoder.h"

COMXCore	_Core;

COMXDecoder::COMXDecoder() :
	m_evtState(FALSE, FALSE),
	m_evtFlush(FALSE, FALSE),
	m_evtEnable(FALSE, FALSE)
{
	m_Callbacks.EventHandler = EventHandler;
	m_Callbacks.EmptyBufferDone = EmptyBufferDone;
	m_Callbacks.FillBufferDone = FillBufferDone;

	m_hComponent = NULL;
	m_pInPort = NULL;
	m_pOutPort = NULL;
    m_bFlushError = FALSE;

	m_nOMXState = OMX_StateInvalid;
	m_nOutBufBusy = 0;
}

COMXDecoder::~COMXDecoder()
{
	_ASSERTE(m_hComponent == NULL);
}

OMX_ERRORTYPE COMXDecoder::Create(LPCSTR pszComponent, LPCSTR pszRole, int nWidth, int nHeight, int nImageSize, OMX_EXT_SURFACE_ALLOCATOR* pExtAlloc)
{
	OMX_ERRORTYPE ret = _Core.GetHandle(&m_hComponent, const_cast<LPSTR>(pszComponent), this, &m_Callbacks);
	if (ret != OMX_ErrorNone)
		return ret;

	ret = OMX_GetState(m_hComponent, &m_nOMXState);
	if (ret != OMX_ErrorNone)
		return ret;

	OMX_PARAM_COMPONENTROLETYPE compType;
	INIT_OMX_PARAM(compType);
	strcpy_s((char*)compType.cRole, _countof(compType.cRole), pszRole);

	ret = OMX_SetParameter(m_hComponent, OMX_IndexParamStandardComponentRole, &compType);
	if (ret != OMX_ErrorNone)
		return ret;

	if (pExtAlloc != NULL)
	{
		ret = OMX_SetParameter(m_hComponent, OMX_IndexSetExtAllocator, pExtAlloc);
		if (ret != OMX_ErrorNone)
			return ret;
	}

	OMX_PORT_PARAM_TYPE portType;
	INIT_OMX_PARAM(portType);
	ret = OMX_GetParameter(m_hComponent, OMX_IndexParamVideoInit, &portType);
	if (ret != OMX_ErrorNone)
		return ret;

	for (UINT i = portType.nStartPortNumber; i < portType.nStartPortNumber + portType.nPorts; i++)
	{
		OMX_PARAM_PORTDEFINITIONTYPE param;
		INIT_OMX_PARAM(param);
		param.nPortIndex = i;

		ret = OMX_GetParameter(m_hComponent, OMX_IndexParamPortDefinition, &param);
		if (ret != OMX_ErrorNone)
			return ret;

		if (param.eDir == OMX_DirInput)
		{
			param.nBufferCountActual = NUM_INPUT_BUF;
			param.nBufferSize = SIZE_INPUT_BUF;
			ret = OMX_SetParameter(m_hComponent, OMX_IndexParamPortDefinition, &param);
			if (ret != OMX_ErrorNone)
				return ret;
			m_pInPort = new COMXPort(m_hComponent, param);
		}
		else if (param.eDir == OMX_DirOutput)
		{
			param.format.video.nFrameWidth = nWidth;
			param.format.video.nFrameHeight = nHeight;
			param.nBufferCountActual = NUM_OUTPUT_BUF;
			param.nBufferSize = nImageSize;
			ret = OMX_SetParameter(m_hComponent, OMX_IndexParamPortDefinition, &param);
			if (ret != OMX_ErrorNone)
				return ret;
			m_pOutPort = new COMXPort(m_hComponent, param);
		}
	}

	if (m_pInPort == NULL || m_pOutPort == NULL)
		return OMX_ErrorComponentNotFound;

	ret = OMX_SendCommand(m_hComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
	if (ret != OMX_ErrorNone)
		return ret;

	ret = m_pInPort->AllocateBuffers();
	if (ret != OMX_ErrorNone)
		return ret;

	ret = m_pOutPort->AllocateBuffers();
	if (ret != OMX_ErrorNone)
		return ret;

	if (! WaitState(OMX_StateIdle))
		return OMX_ErrorInvalidState;

	return ret;
}

OMX_ERRORTYPE COMXDecoder::Destroy()
{
	if (m_nOMXState > OMX_StateIdle)
	{
		OMX_ERRORTYPE ret = OMX_SendCommand(m_hComponent, OMX_CommandStateSet, OMX_StateIdle, NULL);
		if (ret == OMX_ErrorNone)
			WaitState(OMX_StateIdle);
	}

	WaitOutputBuffers();

	OMX_ERRORTYPE ret = OMX_SendCommand(m_hComponent, OMX_CommandStateSet, OMX_StateLoaded, NULL);

	m_pInPort->FreeBuffers();
	m_pOutPort->FreeBuffers();

	if (ret == OMX_ErrorNone)
		WaitState(OMX_StateLoaded);

	delete m_pInPort;
	m_pInPort = NULL;

	delete m_pOutPort;
	m_pOutPort = NULL;

	ret = _Core.FreeHandle(m_hComponent);
	m_hComponent = NULL;
	return ret;
}

OMX_ERRORTYPE COMXDecoder::Decode(BYTE* pbData, UINT cbData, OMX_TICKS nTimestamp, BOOL bEndOfFrame, IUnknown* pFrame)
{
	if (m_nOMXState != OMX_StateExecuting)
	{
		OMX_ERRORTYPE ret = OMX_SendCommand(m_hComponent, OMX_CommandStateSet, OMX_StateExecuting, NULL);
		if (ret != OMX_ErrorNone)
			return ret;
		if (! WaitState(OMX_StateExecuting))
			return OMX_ErrorInvalidState;

		ret = FillOutputBuffers();
		if (ret != OMX_ErrorNone)
			return ret;
	}

	m_pInPort->m_BufferList.Wait();

	OMX_BUFFERHEADERTYPE* pOMXBuf = m_pInPort->m_BufferList.Remove();
	if (pOMXBuf == NULL)
		return OMX_ErrorUnderflow;

	pOMXBuf->nFlags = bEndOfFrame ? OMX_BUFFERFLAG_ENDOFFRAME : 0;
	pOMXBuf->pBuffer = pbData;
	pOMXBuf->nFilledLen = cbData;
	pOMXBuf->nTimeStamp = nTimestamp;
	pOMXBuf->pAppPrivate = pFrame;

	if (pFrame != NULL)
		pFrame->AddRef();

	OMX_ERRORTYPE ret = OMX_EmptyThisBuffer(m_hComponent, pOMXBuf);
	if (ret != OMX_ErrorNone)
	{
		if (pFrame != NULL)
			pFrame->Release();
	}
	return ret;
}

OMX_ERRORTYPE COMXDecoder::BeginFlush()
{
	OMX_ERRORTYPE ret =	OMX_SendCommand(m_hComponent, OMX_CommandFlush, OMX_ALL, NULL);
	return ret;
}

OMX_ERRORTYPE COMXDecoder::EndFlush()
{
	DWORD dwRet = ::WaitForSingleObject(m_evtFlush, INFINITE);
	ASSERT(dwRet == WAIT_OBJECT_0);
    
    if(m_bFlushError == TRUE)
    {
        // Flush command resulted in error.
        return OMX_ErrorIncorrectStateOperation;
    }

	WaitOutputBuffers();

	if (m_nOMXState == OMX_StateExecuting)
		return FillOutputBuffers();
	return OMX_ErrorNone;
}

void COMXDecoder::FillBuffer(OMX_BUFFERHEADERTYPE* pOMXBuf, void* pParam)
{
	COMXDecoder* pThis = (COMXDecoder*)pParam;
	if (pOMXBuf != NULL)
		pThis->FillBuffer(pOMXBuf);
}

OMX_ERRORTYPE COMXDecoder::FillBuffer(OMX_BUFFERHEADERTYPE* pOMXBuf)
{
	pOMXBuf->nFlags = 0;
	pOMXBuf->nFilledLen = 0;

	OMX_ERRORTYPE ret = OMX_FillThisBuffer(m_hComponent, pOMXBuf);
//	ASSERT(ret == OMX_ErrorNone);
	if (ret == OMX_ErrorNone)
		::InterlockedIncrement(&m_nOutBufBusy);
	return ret;
}

OMX_ERRORTYPE COMXDecoder::FillOutputBuffers()
{
	m_pOutPort->m_BufferList.ForEach(FillBuffer, this);
	return OMX_ErrorNone;
}

BOOL COMXDecoder::CanInputData()
{
	if (m_pInPort != NULL)
		return m_pInPort->m_BufferList.GetCount() > 0;
	return TRUE;
}

BOOL COMXDecoder::WaitState(OMX_STATETYPE nState)
{
	while (m_nOMXState != nState && m_nOMXState != OMX_StateInvalid)
	{
		TRACE(_T("Begin wait OMX State for: %d -> %d\n"), m_nOMXState, nState);
		DWORD dwRet = ::WaitForSingleObject(m_evtState, INFINITE);
		TRACE(_T("End wait OMX State: %d\n"), m_nOMXState);
		if (dwRet != WAIT_OBJECT_0)
			break;
	}
    return m_nOMXState == nState;
}

void COMXDecoder::WaitOutputBuffers()
{
	while (m_nOutBufBusy > 0)
		::Sleep(10);
}

OMX_ERRORTYPE COMXDecoder::OnSettingsChanged(int nPortIndex, DWORD dwParam)
{
	OMX_ERRORTYPE ret = m_pOutPort->GetParameter();
	if (ret != OMX_ErrorNone)
		return ret;

	TRACE(_T("Output port video format: Width = %d, Height = %d, Stride = %d, Buffer Size = %d\n"),
		m_pOutPort->format.video.nFrameWidth,
		m_pOutPort->format.video.nFrameHeight,
		m_pOutPort->format.video.nStride,
		m_pOutPort->nBufferSize);

	ret = OMX_SendCommand(m_hComponent, OMX_CommandPortDisable, nPortIndex, NULL);
	if (ret != OMX_ErrorNone)
		return ret;

	WaitOutputBuffers();

	ret = m_pOutPort->FreeBuffers();
	if (ret != OMX_ErrorNone)
		return ret;

	DWORD dwRet = ::WaitForSingleObject(m_evtEnable, INFINITE);
	ASSERT(dwRet == WAIT_OBJECT_0);

	ret = OMX_SendCommand(m_hComponent, OMX_CommandPortEnable, nPortIndex, NULL);
	if (ret != OMX_ErrorNone)
		return ret;

	ret = m_pOutPort->AllocateBuffers();
	if (ret != OMX_ErrorNone)
		return ret;

	dwRet = ::WaitForSingleObject(m_evtEnable, INFINITE);
	ASSERT(dwRet == WAIT_OBJECT_0);

	ret = FillOutputBuffers();
	if (ret != OMX_ErrorNone)
		return ret;
	return ret;
}

OMX_ERRORTYPE COMXDecoder::OnEvent(OMX_EVENTTYPE eEvent, OMX_U32 ui32Data1, OMX_U32 ui32Data2, OMX_PTR pEventData)
{
	switch (eEvent)
	{
	case OMX_EventCmdComplete:
		switch (ui32Data1)
		{
		case OMX_CommandStateSet:
			TRACE(_T("OMX_EventCmdComplete: OMX_CommandStateSet, %d\n"), ui32Data2);
			m_nOMXState = (OMX_STATETYPE)ui32Data2;
			m_evtState.Set();
			break;

		case OMX_CommandPortDisable:
			TRACE(_T("OMX_EventCmdComplete: OMX_CommandPortDisable, %d\n"), ui32Data2);
			m_evtEnable.Set();
			break;

		case OMX_CommandPortEnable:
			TRACE(_T("OMX_EventCmdComplete: OMX_CommandPortEnable, %d\n"), ui32Data2);
			m_evtEnable.Set();
			break;

		case OMX_CommandFlush:
			TRACE(_T("OMX_EventCmdComplete: OMX_CommandFlush, %d\n"), ui32Data2);
			if (ui32Data2 == m_pOutPort->nPortIndex)
				m_evtFlush.Set();
			break;
		}
		break;

	case OMX_EventError:
	//	TRACE(_T("OMX_EventError: 0x%x, %d\n"), ui32Data1, ui32Data2);
        if((ui32Data1 == OMX_ErrorIncorrectStateOperation)
            && (ui32Data1 == OMX_ALL)) // OMX_CommandFlush fail for OMX_ALL ports.Could be a better check for Flush/Seek in Progress .
        {
            m_bFlushError = TRUE;
            m_evtFlush.Set();
        }
		break;

	case OMX_EventPortSettingsChanged:
		TRACE(_T("OMX_EventPortSettingsChanged: %d, %d\n"), ui32Data1, ui32Data2);
		return OnSettingsChanged(ui32Data1, ui32Data2);
	}

	return OMX_ErrorNone;
}

OMX_ERRORTYPE COMXDecoder::OnEmptyBufferDone(OMX_BUFFERHEADERTYPE* pOMXBuf)
{
	IUnknown* pFrame = (IUnknown*)pOMXBuf->pAppPrivate;
	if (pFrame != NULL)
		pFrame->Release();

	pOMXBuf->pBuffer = NULL;
	pOMXBuf->pAppPrivate = NULL;
	m_pInPort->m_BufferList.Add(pOMXBuf);
	return OMX_ErrorNone;
}

OMX_ERRORTYPE COMXDecoder::OnFillBufferDone(OMX_BUFFERHEADERTYPE* pOMXBuf)
{
	::InterlockedDecrement(&m_nOutBufBusy);
	return OMX_ErrorNone;
}

OMX_ERRORTYPE COMXDecoder::EventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
										OMX_EVENTTYPE eEvent, OMX_U32 ui32Data1, OMX_U32 ui32Data2, OMX_PTR pEventData)
{
	COMXDecoder* pThis = (COMXDecoder*)pAppData;
	return pThis->OnEvent(eEvent, ui32Data1, ui32Data2, pEventData);
}

OMX_ERRORTYPE COMXDecoder::EmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
										OMX_BUFFERHEADERTYPE* pOMXBuf)
{
	COMXDecoder* pThis = (COMXDecoder*)pAppData;
	return pThis->OnEmptyBufferDone(pOMXBuf);
}

OMX_ERRORTYPE COMXDecoder::FillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
										OMX_BUFFERHEADERTYPE* pOMXBuf)
{
	COMXDecoder* pThis = (COMXDecoder*)pAppData;
	return pThis->OnFillBufferDone(pOMXBuf);
}
