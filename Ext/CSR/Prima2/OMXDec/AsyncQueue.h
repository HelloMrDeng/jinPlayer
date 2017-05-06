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

template <class T>
class CAsyncQueue
{
public:
	CAsyncQueue(int nMaxCount) : m_Semaphore(0, nMaxCount)
	{
	}

	int GetCount()
	{
		m_Lock.Enter();
		int nCount = m_List.GetCount();
		m_Lock.Leave();
		return nCount;
	}

	void Add(T val)
	{
		m_Lock.Enter();
		m_List.AddTail(val);
		m_Lock.Leave();
		m_Semaphore.Release();
	}

	BOOL Wait(DWORD dwTimeout = INFINITE)
	{
		DWORD dwRet = ::WaitForSingleObject(m_Semaphore, dwTimeout);
		m_Lock.Enter();
		int nCount = m_List.GetCount();
		m_Lock.Leave();
		return nCount > 0;
	}

	T Remove()
	{
		T val = NULL;
		m_Lock.Enter();
		if (! m_List.IsEmpty())
			val = m_List.RemoveHead();
		m_Lock.Leave();
		return val;
	}

	void Clear()
	{
		m_Lock.Enter();
		m_List.RemoveAll();
		m_Lock.Leave();
	}

	void ForEach(void (* Func)(T val, void* pParam), void* pParam)
	{
		m_Lock.Enter();
		for (POSITION pos = m_List.GetHeadPosition(); pos != NULL;)
			Func(m_List.GetNext(pos), pParam);
		m_Lock.Leave();
	}

protected:
	CCriticalSection	m_Lock;
	CSemaphore			m_Semaphore;
	CAtlList<T>			m_List;
};
