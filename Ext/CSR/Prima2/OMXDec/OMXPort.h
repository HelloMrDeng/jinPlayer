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

#include "AsyncQueue.h"
#include "OMXCore.h"

typedef CAsyncQueue<OMX_BUFFERHEADERTYPE*> COMXBufferQueue;

class COMXPort : public OMX_PARAM_PORTDEFINITIONTYPE
{
	friend class COMXDecoder;

public:
	COMXPort(OMX_HANDLETYPE hComponent, OMX_PARAM_PORTDEFINITIONTYPE& param)
		: m_BufferList(param.nBufferCountActual)
	{
		m_hComponent = hComponent;
		CopyMemory(this, &param, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
	}

	OMX_ERRORTYPE GetParameter()
	{
		return OMX_GetParameter(m_hComponent, OMX_IndexParamPortDefinition, this);
	}

	OMX_ERRORTYPE SetParameter()
	{
		return OMX_SetParameter(m_hComponent, OMX_IndexParamPortDefinition, this);
	}

	OMX_ERRORTYPE AllocateBuffers()
	{
		OMX_ERRORTYPE ret = OMX_ErrorUndefined;
		for (UINT i = 0; i < nBufferCountActual; i++)
		{
			OMX_BUFFERHEADERTYPE* pBuf = NULL;
			if (eDir == OMX_DirInput)
				ret = OMX_UseBuffer(m_hComponent, &pBuf, nPortIndex, NULL, nBufferSize, NULL);
			else if (eDir == OMX_DirOutput)
				ret = OMX_AllocateBuffer(m_hComponent, &pBuf, nPortIndex, NULL, nBufferSize);
			if (ret != OMX_ErrorNone)
				break;
			m_BufferList.Add(pBuf);
		}
		return ret;
	}

	OMX_ERRORTYPE FreeBuffers()
	{
		OMX_ERRORTYPE ret = OMX_ErrorUndefined;
		while (m_BufferList.GetCount() > 0)
		{
			OMX_BUFFERHEADERTYPE* pBuf = m_BufferList.Remove();
			if (pBuf != NULL)
				ret = OMX_FreeBuffer(m_hComponent, nPortIndex, pBuf);
		}
		return ret;
	}

protected:
	OMX_HANDLETYPE	m_hComponent;
	COMXBufferQueue	m_BufferList;
};
