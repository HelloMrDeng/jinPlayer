/*******************************************************************************
	File:		CProgInfo.cpp

	Contains:	Program Info implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "CProgInfo.h"
#include "UFileFunc.h"

static VO_U32 VOS2PRG_SetProgType (VO_HANDLE hHandle, VO_SOURCE2_PROGRAM_TYPE nType) {
	return ((CProgInfo *)hHandle)->SetProgType (nType); }

static VO_U32 VOS2PRG_RegNotify (VO_HANDLE hHandle, VOS2_ProgInfo_Notify * pNotify, VO_BOOL bReg) {
	return ((CProgInfo *)hHandle)->RegNotify (pNotify, bReg); }

static VO_U32 VOS2PRG_SetStream (VO_HANDLE hHandle, VO_SOURCE2_STREAM_INFO * pStream, VOS2_PROGINFO_COMMAND nCmd) {
	return ((CProgInfo *)hHandle)->SetStream (pStream, nCmd); }

static VO_U32 VOS2PRG_SetTrack (VO_HANDLE hHandle, VO_U32 uStrmID, VO_SOURCE2_TRACK_INFO * pTrack, VOS2_PROGINFO_COMMAND nCmd) {
	return ((CProgInfo *)hHandle)->SetTrack (uStrmID, pTrack, nCmd); }

static VO_U32 VOS2PRG_GetStream (VO_HANDLE hHandle, VOS2_PROGINFO_GETTYPE nGetType, VO_U32 uValue, VO_SOURCE2_STREAM_INFO * pStream) {
	return ((CProgInfo *)hHandle)->GetStream (nGetType, uValue, pStream); }

static VO_U32 VOS2PRG_GetTrack (VO_HANDLE hHandle, VOS2_PROGINFO_GETTYPE nGetType, VO_U32 uStrmID, VO_U32 uValue, VO_SOURCE2_TRACK_INFO * pTrack, VO_BOOL bCopyHead) {
	return ((CProgInfo *)hHandle)->GetTrack (nGetType, uStrmID, uValue, pTrack, bCopyHead); }

static VO_U32 VOS2PRG_Reset (VO_HANDLE hHandle) {
	return ((CProgInfo *)hHandle)->Reset (); }

static VO_U32 VOS2PRG_GetParam (VO_HANDLE hHandle, VO_U32 uID, VO_PTR pValue) {
	return ((CProgInfo *)hHandle)->GetParam (uID, pValue); }

static VO_U32 VOS2PRG_DumpInfo (VO_HANDLE hHandle, VO_TCHAR * pFile) {
	return ((CProgInfo *)hHandle)->DumpInfo (pFile); }


CProgInfo::CProgInfo (void)
{
	memset (&m_progInfo, 0, sizeof (m_progInfo));
	m_progInfo.sProgramType = VO_SOURCE2_STREAM_TYPE_VOD;

	memset (&m_func, 0, sizeof (m_func));
	m_func.hHandle			= this;
	m_func.SetProgType		= VOS2PRG_SetProgType;
	m_func.RegNotify		= VOS2PRG_RegNotify;
	m_func.SetStream		= VOS2PRG_SetStream;
	m_func.SetTrack			= VOS2PRG_SetTrack;
	m_func.GetStream		= VOS2PRG_GetStream;
	m_func.GetTrack			= VOS2PRG_GetTrack;
	m_func.Reset			= VOS2PRG_Reset;
	m_func.GetParam			= VOS2PRG_GetParam;
	m_func.DumpInfo			= VOS2PRG_DumpInfo;
}

CProgInfo::~CProgInfo(void)
{
	Reset ();

	VOS2_ProgInfo_Notify * pNotify = m_lstNotify.RemoveHead ();
	while (pNotify != NULL)
	{
		delete pNotify;
		pNotify = m_lstNotify.RemoveHead ();
	}
}

VO_U32 CProgInfo::SetProgType (VO_SOURCE2_PROGRAM_TYPE nType)
{
	m_progInfo.sProgramType = nType;

	Notify (VOCB_PROGINFO_UPDATE_PROGTYPE, 0, NULL);

	return VO_ERR_NONE;
}

VO_U32 CProgInfo::RegNotify (VOS2_ProgInfo_Notify * pNotify, VO_BOOL bReg)
{
	CAutoLock lock (&m_lock);

	if (pNotify == NULL)
		return VO_ERR_FAILED;

	if (bReg)
	{
		VOS2_ProgInfo_Notify * pNewNofity = new VOS2_ProgInfo_Notify ();

		pNewNofity->pUserData = pNotify->pUserData;
		pNewNofity->fNotify = pNotify->fNotify;

		m_lstNotify.AddTail (pNewNofity);

		return VO_ERR_NONE;
	}
	else
	{
		VOS2_ProgInfo_Notify * pDelNofity = NULL;
		NODEPOS pos = m_lstNotify.GetHeadPosition ();
		while (pos != NULL)
		{
			pDelNofity = m_lstNotify.GetNext (pos);
			if (pDelNofity != NULL)
			{
				if (pDelNofity->fNotify == pNotify->fNotify)
				{
					m_lstNotify.Remove (pDelNofity);
					delete pDelNofity;
					return VO_ERR_NONE;
				}
			}
		}
	}

	return VO_ERR_FAILED;
}

VO_U32 CProgInfo::SetStream (VO_SOURCE2_STREAM_INFO * pStream, VOS2_PROGINFO_COMMAND nCmd)
{
	CAutoLock lock (&m_lock);
	if (pStream == NULL)
		return VO_ERR_INVALID_ARG;

	VO_SOURCE2_STREAM_INFO **	ppStreamInfo = NULL;
	VO_SOURCE2_STREAM_INFO *	pStreamInfo = NULL;

	if (nCmd == VOS2_PROGINFO_NEW)
	{
		if (FindStream (pStream->uStreamID, &pStreamInfo) == VO_ERR_NONE)
			return VO_ERR_INVALID_ARG;

		if (pStream->ppTrackInfo != NULL || pStream->uTrackCount != 0)
			return VO_ERR_INVALID_ARG;
		
		ppStreamInfo = new VO_SOURCE2_STREAM_INFO *[m_progInfo.uStreamCount + 1];
		for (VO_U32 j = 0; j < m_progInfo.uStreamCount; j++)
			ppStreamInfo[j] = m_progInfo.ppStreamInfo[j];

		VO_SOURCE2_STREAM_INFO * pNewSteam = new VO_SOURCE2_STREAM_INFO ();
		memcpy (pNewSteam, pStream, sizeof (VO_SOURCE2_STREAM_INFO));
		ppStreamInfo[m_progInfo.uStreamCount] = pNewSteam;

		if (m_progInfo.ppStreamInfo != NULL)
			delete []m_progInfo.ppStreamInfo;
		
		m_progInfo.uStreamCount++;
		m_progInfo.ppStreamInfo = ppStreamInfo;

		Notify (VOCB_PROGINFO_UPDATE_STREAM, nCmd, pNewSteam);
		return VO_ERR_NONE;
	}
	else if (nCmd == VOS2_PROGINFO_DELETE)
	{
		if (FindStream (pStream->uStreamID, &pStreamInfo) != VO_ERR_NONE)
			return VO_ERR_INVALID_ARG;

		int	nIdx = 0;
		if (m_progInfo.uStreamCount > 1)
		{
			ppStreamInfo = new VO_SOURCE2_STREAM_INFO *[m_progInfo.uStreamCount - 1];
			for (VO_U32 j = 0; j < m_progInfo.uStreamCount; j++)
			{
				if (m_progInfo.ppStreamInfo[j]->uStreamID != pStream->uStreamID)
					ppStreamInfo[nIdx++] = m_progInfo.ppStreamInfo[j];
			}
		}
		delete []m_progInfo.ppStreamInfo;
		m_progInfo.uStreamCount--;
		m_progInfo.ppStreamInfo = ppStreamInfo;

		Notify (VOCB_PROGINFO_UPDATE_STREAM, nCmd, pStreamInfo);

		if (pStreamInfo->ppTrackInfo != NULL)
			delete []pStreamInfo->ppTrackInfo;
		delete pStreamInfo;

		return VO_ERR_NONE;
	}
	else if (nCmd == VOS2_PROGINFO_SELECT)
	{
		if (FindStream (pStream->uStreamID, &pStreamInfo) != VO_ERR_NONE)
			return VO_ERR_INVALID_ARG;

		for (VO_U32 i = 0; i < m_progInfo.uStreamCount; i++)
		{
			if (m_progInfo.ppStreamInfo[i] == NULL)
				continue;

			m_progInfo.ppStreamInfo[i]->uSelInfo = VO_SOURCE2_SELECT_SELECTABLE;
			if (m_progInfo.ppStreamInfo[i]->uStreamID == pStream->uStreamID)
			{
				pStreamInfo = m_progInfo.ppStreamInfo[i];
				pStreamInfo->uSelInfo = VO_SOURCE2_SELECT_SELECTED;
			}
		}

		Notify (VOCB_PROGINFO_UPDATE_STREAM, nCmd, pStreamInfo);

		return VO_ERR_NONE;
	}
	else if (nCmd == VOS2_PROGINFO_UPDATE)
	{
		if (FindStream (pStream->uStreamID, &pStreamInfo) != VO_ERR_NONE)
			return VO_ERR_INVALID_ARG;

		pStreamInfo->uStreamID = pStream->uStreamID;
		pStreamInfo->uBitrate = pStream->uBitrate;
		pStreamInfo->uSelInfo = pStream->uSelInfo;
//		memcpy (pStreamInfo, pStream, sizeof (VO_SOURCE2_STREAM_INFO));

		Notify (VOCB_PROGINFO_UPDATE_STREAM, nCmd, pStreamInfo);

		return VO_ERR_NONE;
	}

	return VO_ERR_FAILED;
}

VO_U32 CProgInfo::SetTrack (VO_U32 uStrmID, VO_SOURCE2_TRACK_INFO * pTrack, VOS2_PROGINFO_COMMAND nCmd)
{
	CAutoLock lock (&m_lock);
	if (pTrack == NULL)
		return VO_ERR_FAILED;

	VO_SOURCE2_STREAM_INFO * pStreamInfo = NULL;
	if (FindStream (uStrmID, &pStreamInfo) != VO_ERR_NONE)
		return VO_ERR_INVALID_ARG;

	VO_SOURCE2_TRACK_INFO **	ppTrackInfo = NULL;
	VO_SOURCE2_TRACK_INFO *		pTrackInfo = NULL;
	if (nCmd == VOS2_PROGINFO_NEW)
	{
		VO_SOURCE2_TRACK_INFO * pNewTrack = NULL;
		if (FindTrack (VODEF_PROGINFO_FINDALL, pTrack->uTrackID, &pTrackInfo) != VO_ERR_NONE)
		{
			pNewTrack = new VO_SOURCE2_TRACK_INFO ();
			memcpy (pNewTrack, pTrack, sizeof (VO_SOURCE2_TRACK_INFO));
			pNewTrack->pHeadData = NULL;
			if (pTrack->uHeadSize > 0)
			{
				pNewTrack->pHeadData = new VO_BYTE[pTrack->uHeadSize];
				memcpy (pNewTrack->pHeadData, pTrack->pHeadData, pTrack->uHeadSize);
			}
			m_lstTrack.AddTail (pNewTrack);
		}
		else
		{
			if (memcmp (pTrack, pTrackInfo, sizeof (VO_SOURCE2_TRACK_INFO)))
				return VO_ERR_INVALID_ARG;

			pNewTrack = pTrackInfo;
		}

		ppTrackInfo = new VO_SOURCE2_TRACK_INFO *[pStreamInfo->uTrackCount + 1];
		for (VO_U32 j = 0; j < pStreamInfo->uTrackCount; j++)
			ppTrackInfo[j] = pStreamInfo->ppTrackInfo[j];
		ppTrackInfo[pStreamInfo->uTrackCount] = pNewTrack;

		if (pStreamInfo->ppTrackInfo != NULL)
			delete []pStreamInfo->ppTrackInfo;
		
		pStreamInfo->uTrackCount++;
		pStreamInfo->ppTrackInfo = ppTrackInfo;

		Notify (VOCB_PROGINFO_UPDATE_TRACK, nCmd, pNewTrack);
		return VO_ERR_NONE;
	}
	else if (nCmd == VOS2_PROGINFO_DELETE)
	{
		if (uStrmID == VODEF_PROGINFO_FINDALL)
			return DeleteTrack (pTrack->uTrackID);

		if (FindTrack (uStrmID, pTrack->uTrackID, &pTrackInfo) != VO_ERR_NONE)
			return VO_ERR_INVALID_ARG;

		for (VO_U32 i = 0; i < m_progInfo.uStreamCount; i++)
		{
			pStreamInfo = m_progInfo.ppStreamInfo[i];
			if (pStreamInfo == NULL || pStreamInfo->uStreamID != uStrmID)
				continue;

			if (pStreamInfo->uTrackCount > 1)
			{
				ppTrackInfo = new VO_SOURCE2_TRACK_INFO *[pStreamInfo->uTrackCount - 1];
				int nIndex = 0;
				for (VO_U32 j = 0; j < pStreamInfo->uTrackCount; j++)
				{
					if (pStreamInfo->ppTrackInfo[j]->uTrackID != pTrack->uTrackID)
						ppTrackInfo[nIndex++] = pStreamInfo->ppTrackInfo[j];
				}
			}
			delete []pStreamInfo->ppTrackInfo;
			pStreamInfo->uTrackCount--;
			pStreamInfo->ppTrackInfo = ppTrackInfo;

			DeleteTrack (pTrack->uTrackID);
			break;
		}

		return VO_ERR_NONE;
	}
	else if (nCmd == VOS2_PROGINFO_SELECT)
	{
		if (FindTrack (uStrmID, pTrack->uTrackID, &pTrackInfo) != VO_ERR_NONE)
			return VO_ERR_INVALID_ARG;

		VO_SOURCE2_TRACK_INFO * pSelTrack = NULL;
		NODEPOS pos = m_lstTrack.GetHeadPosition ();
		while (pos != NULL)
		{
			pSelTrack = m_lstTrack.GetNext (pos);
			if (pSelTrack != NULL)
			{
				if (pSelTrack->uTrackType == pTrackInfo->uTrackType)
					pSelTrack->uSelInfo = VO_SOURCE2_SELECT_SELECTABLE;

				if (pSelTrack->uTrackID == pTrack->uTrackID)
				{
					pSelTrack->uSelInfo = VO_SOURCE2_SELECT_SELECTED;
					Notify (VOCB_PROGINFO_UPDATE_TRACK, nCmd, pSelTrack);
				}
			}
		}

		return VO_ERR_NONE;
	}
	else if (nCmd == VOS2_PROGINFO_UPDATE)
	{
		if (FindTrack (uStrmID, pTrack->uTrackID, &pTrackInfo) != VO_ERR_NONE)
			return VO_ERR_INVALID_ARG;

		if (pTrackInfo->pHeadData != NULL)
			delete []pTrackInfo->pHeadData;

		memcpy (pTrackInfo, pTrack, sizeof (VO_SOURCE2_TRACK_INFO));
		if (pTrack->uHeadSize > 0)
		{
			pTrackInfo->pHeadData = new VO_BYTE[pTrack->uHeadSize];
			memcpy (pTrackInfo->pHeadData, pTrack->pHeadData, pTrack->uHeadSize);
		}

		Notify (VOCB_PROGINFO_UPDATE_TRACK, nCmd, pTrackInfo);

		return VO_ERR_NONE;
	}

	return VO_ERR_FAILED;
}

VO_U32 CProgInfo::GetStream (VOS2_PROGINFO_GETTYPE nGetType, VO_U32 uValue, VO_SOURCE2_STREAM_INFO * pStream)
{
	CAutoLock lock (&m_lock);

	if (pStream == NULL)
		return VO_ERR_INVALID_ARG;

	if (nGetType == VOS2_PROGINFO_BYINDEX)
	{
		if (uValue >= m_progInfo.uStreamCount)
			return VO_ERR_FAILED;

		if (m_progInfo.ppStreamInfo[uValue] == NULL)
			return VO_ERR_FAILED;

		memcpy (pStream, m_progInfo.ppStreamInfo[uValue], sizeof (VO_SOURCE2_STREAM_INFO));
		pStream->ppTrackInfo = NULL;

		return VO_ERR_NONE;
	}
	else if (nGetType == VOS2_PROGINFO_BYID)
	{
		VO_SOURCE2_STREAM_INFO *	pStreamInfo = NULL;
		if (FindStream (uValue, &pStreamInfo) != VO_ERR_NONE)
			return VO_ERR_FAILED;

		memcpy (pStream, pStreamInfo, sizeof (VO_SOURCE2_STREAM_INFO));
	//	pStream->ppTrackInfo = NULL;

		return VO_ERR_NONE;
	}
	else if (nGetType == VOS2_PROGINFO_BYSELECT)
	{
		if (m_progInfo.uStreamCount <= 0)
			return VO_ERR_FAILED;

		for (VO_U32 i = 0; i < m_progInfo.uStreamCount; i++)
		{
			if (m_progInfo.ppStreamInfo[i]->uSelInfo > 0)
			{
				memcpy (pStream, m_progInfo.ppStreamInfo[i], sizeof (VO_SOURCE2_STREAM_INFO));
			//	pStream->ppTrackInfo = NULL;
				return VO_ERR_NONE;
			}
		}
	}

	return VO_ERR_FAILED;
}

VO_U32 CProgInfo::GetTrack (VOS2_PROGINFO_GETTYPE nGetType, VO_U32 uStrmID, VO_U32 uValue, VO_SOURCE2_TRACK_INFO * pTrack, VO_BOOL bCopyHead)
{
	if (pTrack == NULL)
		return VO_ERR_INVALID_ARG;

	VO_SOURCE2_STREAM_INFO *	pStreamInfo = NULL;
	VO_SOURCE2_TRACK_INFO *		pTrackInfo = NULL;
	VO_SOURCE2_TRACK_INFO *		pTrackFind = NULL;

	if (nGetType == VOS2_PROGINFO_BYINDEX)
	{
		if (uStrmID == VODEF_PROGINFO_FINDALL)
		{
			if (uValue >= m_lstTrack.GetCount ())
				return VO_ERR_FAILED;

			int nIndex = 0;
			NODEPOS pos = m_lstTrack.GetHeadPosition ();
			while (pos != NULL)
			{
				pTrackFind = m_lstTrack.GetNext (pos);
				if (nIndex == uValue)
					break;
				nIndex++;
			}
		}
		else
		{
			if (FindStream (uStrmID, &pStreamInfo) != VO_ERR_NONE)
				return VO_ERR_FAILED;

			if (uValue >= pStreamInfo->uTrackCount)
				return VO_ERR_FAILED;

			if (pStreamInfo->ppTrackInfo[uValue] == NULL)
				return VO_ERR_FAILED;

			pTrackFind = pStreamInfo->ppTrackInfo[uValue];
		}
	}
	else if (nGetType == VOS2_PROGINFO_BYID)
	{
		NODEPOS pos = m_lstTrack.GetHeadPosition ();
		while (pos != NULL)
		{
			pTrackInfo = m_lstTrack.GetNext (pos);
			if (pTrackInfo != NULL && pTrackInfo->uTrackID == uValue)
			{
				pTrackFind = pTrackInfo;
				break;
			}
		}
	}
	else if (nGetType == VOS2_PROGINFO_BYSELECT)
	{
		if (FindStream (uStrmID, &pStreamInfo) != VO_ERR_NONE)
			return VO_ERR_FAILED;

		for (VO_U32 i = 0; i < pStreamInfo->uTrackCount; i++)
		{
			pTrack = pStreamInfo->ppTrackInfo[i];
			if (pTrackInfo != NULL && pStreamInfo->uSelInfo > 0)
			{
				pTrackFind = pTrackInfo;
				break;
			}
		}
	}

	if (pTrackFind == NULL)
		return VO_ERR_FAILED;

	memcpy (pTrack, pTrackFind, sizeof (VO_SOURCE2_TRACK_INFO));
	if (pTrackFind->pHeadData != NULL && bCopyHead)
	{
		pTrack->pHeadData = new VO_BYTE[pTrack->uHeadSize];
		memcpy (pTrack->pHeadData, pTrackFind->pHeadData, pTrack->uHeadSize);
	}
	else
	{
		pTrack->pHeadData = NULL;
		pTrack->uHeadSize = 0;
	}

	return VO_ERR_NONE;
}

VO_U32 CProgInfo::Reset (void)
{
	CAutoLock lock (&m_lock);
	for (VO_U32 i = 0; i < m_progInfo.uStreamCount; i++)
	{
		if (m_progInfo.ppStreamInfo[i] != NULL)
		{
			if (m_progInfo.ppStreamInfo[i]->ppTrackInfo != NULL)
				delete []m_progInfo.ppStreamInfo[i]->ppTrackInfo;
			delete m_progInfo.ppStreamInfo[i];
		}
	}

	if (m_progInfo.ppStreamInfo != NULL)
		delete []m_progInfo.ppStreamInfo;
	m_progInfo.ppStreamInfo = NULL;
	m_progInfo.uStreamCount = 0;

	VO_SOURCE2_TRACK_INFO * pTrack = m_lstTrack.RemoveHead ();
	while (pTrack != NULL)
	{
		if (pTrack->pHeadData != NULL)
			delete []pTrack->pHeadData;
		delete pTrack;
		pTrack = m_lstTrack.RemoveHead ();
	}

	Notify (VOCB_PROGINFO_UPDATE_RESET, 0, NULL);

	return VO_ERR_NONE;
}

VO_U32 CProgInfo::GetParam (VO_U32 uID, VO_PTR pValue)
{
	CAutoLock lock (&m_lock);
	VO_U32 uMaxID = 0;
	switch (uID)
	{
	case VOID_PROGINFO_STREAM_ID:
		if (pValue == NULL)
			return VO_ERR_INVALID_ARG;
		for (VO_U32 i = 0; i < m_progInfo.uStreamCount; i++)
		{
			if (m_progInfo.ppStreamInfo[i] != NULL)
			{
				if (m_progInfo.ppStreamInfo[i]->uStreamID > uMaxID)
					uMaxID = m_progInfo.ppStreamInfo[i]->uStreamID;
			}
		}
		*((VO_U32 *)pValue) = ++uMaxID;
		return VO_ERR_NONE;

	case VOID_PROGINFO_TRACK_ID:
	{
		if (pValue == NULL)
			return VO_ERR_INVALID_ARG;
		VO_SOURCE2_TRACK_INFO * pTrack = NULL;
		NODEPOS pos = m_lstTrack.GetHeadPosition ();
		while (pos != NULL)
		{
			pTrack = m_lstTrack.GetNext (pos);
			if (pTrack != NULL && pTrack->uTrackID > uMaxID)
				uMaxID = pTrack->uTrackID;
		}
		*((VO_U32 *)pValue) = ++uMaxID;
		return VO_ERR_NONE;
	}
	case VOID_PROGINFO_STREAM_NUM:
		*((VO_U32 *)pValue) = m_progInfo.uStreamCount;
		return VO_ERR_NONE;

	default:
		break;
	}

	return VO_ERR_NOT_IMPLEMENT;
}

VO_U32 CProgInfo::SetDefaultStreamSelected (void)
{
	for (VO_U32 i = 0; i < m_progInfo.uStreamCount; i++)
	{
		if (m_progInfo.ppStreamInfo[i]->uSelInfo == VO_SOURCE2_SELECT_RECOMMEND)
		{
			m_progInfo.ppStreamInfo[i]->uSelInfo = VO_SOURCE2_SELECT_SELECTED;
			return VO_ERR_NONE;
		}
	}

	return VO_ERR_FAILED;
}

VO_U32 CProgInfo::GetSelectedStream (void)
{
	for (VO_U32 i = 0; i < m_progInfo.uStreamCount; i++)
	{
		if (m_progInfo.ppStreamInfo[i]->uSelInfo == VO_SOURCE2_SELECT_SELECTED)
			return m_progInfo.ppStreamInfo[i]->uStreamID;
	}

	return -1;
}

VO_U32 CProgInfo::GetSelectedTrack (void)
{
	VO_SOURCE2_TRACK_INFO * pTrack = NULL;
	NODEPOS pos = m_lstTrack.GetHeadPosition ();
	while (pos != NULL)
	{
		pTrack = m_lstTrack.GetNext (pos);
		if (pTrack->uSelInfo == VO_SOURCE2_SELECT_SELECTED)
			return pTrack->uTrackID;
	}

	return -1;
}

VO_U32 CProgInfo::FindStream (VO_U32 uStrmID, VO_SOURCE2_STREAM_INFO ** ppStream)
{
	if (ppStream == NULL)
		return VO_ERR_INVALID_ARG;

	*ppStream = NULL;
	if (m_progInfo.uStreamCount <= 0)
		return VO_ERR_FAILED;

	for (VO_U32 i = 0; i < m_progInfo.uStreamCount; i++)
	{
		if (m_progInfo.ppStreamInfo[i]->uStreamID == uStrmID)
		{
			*ppStream = m_progInfo.ppStreamInfo[i];
			break;
		}
	}

	return *ppStream != NULL ? VO_ERR_NONE : VO_ERR_FAILED;
}

VO_U32 CProgInfo::FindTrack (VO_U32 uStreamID, VO_U32 uTrackID, VO_SOURCE2_TRACK_INFO ** ppTrack)
{
	if (ppTrack == NULL)
		return VO_ERR_INVALID_ARG;
	*ppTrack = NULL;

	if (m_progInfo.uStreamCount <= 0)
		return VO_ERR_FAILED;

	VO_SOURCE2_STREAM_INFO *	pStream = NULL;
	VO_SOURCE2_TRACK_INFO *		pTrack = NULL;
	if (uStreamID != VODEF_PROGINFO_FINDALL)
	{
		if (FindStream (uStreamID, &pStream) != VO_ERR_NONE)
			return VO_ERR_FAILED;

		for (VO_U32 i = 0; i < pStream->uTrackCount; i++)
		{
			pTrack = pStream->ppTrackInfo[i];
			if (pTrack != NULL && pTrack->uTrackID == uTrackID)
			{
				*ppTrack = pTrack;
				return VO_ERR_NONE;
			}
		}

		return VO_ERR_FAILED;
	}

	NODEPOS pos = m_lstTrack.GetHeadPosition ();
	while (pos != NULL)
	{
		pTrack = m_lstTrack.GetNext (pos);
		if (pTrack != NULL && pTrack->uTrackID == uTrackID)
		{
			*ppTrack = pTrack;
			return VO_ERR_NONE;
		}
	}

	return VO_ERR_FAILED;
}

VO_U32 CProgInfo::DeleteTrack (VO_U32 uTrackID)
{
	VO_SOURCE2_STREAM_INFO *	pStream = NULL;
	for (VO_U32 i = 0; i < m_progInfo.uStreamCount; i++)
	{
		pStream = m_progInfo.ppStreamInfo[i];
		if (pStream == NULL)
			continue;

		for (VO_U32 j = 0; j < pStream->uTrackCount; j++)
		{
			if (pStream->ppTrackInfo[j]->uTrackID == uTrackID)
				return VO_ERR_FAILED;
		}
	}

	VO_SOURCE2_TRACK_INFO * pDelTrack = NULL;
	NODEPOS pos = m_lstTrack.GetHeadPosition ();
	while (pos != NULL)
	{
		pDelTrack = m_lstTrack.GetNext (pos);
		if (pDelTrack != NULL)
		{
			if (pDelTrack->uTrackID == uTrackID)
			{
				m_lstTrack.Remove (pDelTrack);

				Notify (VOCB_PROGINFO_UPDATE_TRACK, VOS2_PROGINFO_DELETE, pDelTrack);

				if (pDelTrack->pHeadData != NULL)
					delete []pDelTrack->pHeadData;
				delete pDelTrack;
				return VO_ERR_NONE;
			}
		}
	}

	return VO_ERR_FAILED;
}

VO_U32 CProgInfo::Notify (VO_U32 uID, VO_U32 uStatus, VO_PTR pValue)
{
	VOS2_ProgInfo_Notify * pNotify = NULL;

	NODEPOS pos = m_lstNotify.GetHeadPosition ();
	while (pos != NULL)
	{
		pNotify = m_lstNotify.GetNext (pos);
		if (pNotify != NULL)
		{
			pNotify->fNotify (pNotify->pUserData, uID, uStatus, pValue);
		}
	}

	return VO_ERR_NONE;
}

VO_U32 CProgInfo::DumpInfo (VO_TCHAR * pFile)
{
	yyFile	hFile = yyFileOpen (pFile, YYFILE_WRITE);
	if (hFile == NULL)
		return YY_ERR_FAILED;

	char						szLine[1024];
	VO_SOURCE2_STREAM_INFO *	pStream = NULL;
	VO_SOURCE2_TRACK_INFO *		pTrack = NULL;

	sprintf (szLine, "Program_ID:   %d \r\n", m_progInfo.uProgramID);
	yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
	sprintf (szLine, "Program_Type: %d \r\n", m_progInfo.sProgramType);
	yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
	sprintf (szLine, "Program_Name: %s \r\n", m_progInfo.strProgramName);
	yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
	sprintf (szLine, "Streams Num  :%d \r\n", m_progInfo.uStreamCount);
	yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));

	for (VO_U32 i = 0; i < m_progInfo.uStreamCount; i++)
	{
		if (m_progInfo.ppStreamInfo[i] == NULL)
			continue;

		pStream = m_progInfo.ppStreamInfo[i];
		sprintf (szLine, "\r\nSrmIdx:  %d \r\n", i);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		sprintf (szLine, "ID:      %d \r\n", pStream->uStreamID);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		if (pStream->uSelInfo == VO_SOURCE2_SELECT_SELECTABLE)
			sprintf (szLine, "Sel:     %d Sel-Enable \r\n", pStream->uSelInfo);
		else if (pStream->uSelInfo == VO_SOURCE2_SELECT_RECOMMEND)
			sprintf (szLine, "Sel:     %d Recommand \r\n", pStream->uSelInfo);
		else if (pStream->uSelInfo == VO_SOURCE2_SELECT_SELECTED)
			sprintf (szLine, "Sel:     %d Selected \r\n", pStream->uSelInfo);
		else if (pStream->uSelInfo == VO_SOURCE2_SELECT_DISABLE)
			sprintf (szLine, "Sel:     %d Sel-Disable \r\n", pStream->uSelInfo);
		else if (pStream->uSelInfo == VO_SOURCE2_SELECT_DEFAULT)
			sprintf (szLine, "Sel:     %d Sel-Default \r\n", pStream->uSelInfo);
		else if (pStream->uSelInfo == VO_SOURCE2_SELECT_FORCE)
			sprintf (szLine, "Sel:     %d Sel-Force \r\n", pStream->uSelInfo);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		sprintf (szLine, "Bitrate: %d \r\n", pStream->uBitrate);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		sprintf (szLine, "TrackNum:%d \r\n", pStream->uTrackCount);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));

		for (VO_U32 j = 0; j < pStream->uTrackCount; j++)
		{
			if (pStream->ppTrackInfo[j]->uTrackType == VO_SOURCE2_TT_AUDIO)
				sprintf (szLine, "     ID:  % 6d    Audio\r\n", pStream->ppTrackInfo[j]->uTrackID);
			else if (pStream->ppTrackInfo[j]->uTrackType == VO_SOURCE2_TT_VIDEO)
				sprintf (szLine, "     ID:  % 6d    Video\r\n", pStream->ppTrackInfo[j]->uTrackID);
			else if (pStream->ppTrackInfo[j]->uTrackType == VO_SOURCE2_TT_IMAGE)
				sprintf (szLine, "     ID:  % 6d    Image\r\n", pStream->ppTrackInfo[j]->uTrackID);
			else if (pStream->ppTrackInfo[j]->uTrackType == VO_SOURCE2_TT_STREAM)
				sprintf (szLine, "     ID:  % 6d    Stream\r\n", pStream->ppTrackInfo[j]->uTrackID);
			else if (pStream->ppTrackInfo[j]->uTrackType == VO_SOURCE2_TT_SUBTITLE)
				sprintf (szLine, "     ID:  % 6d    Subtitle\r\n", pStream->ppTrackInfo[j]->uTrackID);
			else
				sprintf (szLine, "     ID:  % 6d    Unknown\r\n", pStream->ppTrackInfo[j]->uTrackID);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		}
	}

	sprintf (szLine, "\r\nTrack Info: Num:  %d \r\n", m_lstTrack.GetCount ());
	yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));

	int nIndex = 0;
	NODEPOS pos = m_lstTrack.GetHeadPosition ();
	while (pos != NULL)
	{
		pTrack = m_lstTrack.GetNext (pos);
		if (pTrack == NULL)
			continue;

		sprintf (szLine, "\r\nTrkIdx: %d \r\n", nIndex);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		sprintf (szLine, "ID:     %d \r\n", pTrack->uTrackID);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		if (pTrack->uSelInfo == VO_SOURCE2_SELECT_SELECTABLE)
			sprintf (szLine, "Sel:    %d Sel-Enable \r\n", pTrack->uSelInfo);
		else if (pTrack->uSelInfo == VO_SOURCE2_SELECT_RECOMMEND)
			sprintf (szLine, "Sel:    %d Recommand \r\n", pTrack->uSelInfo);
		else if (pTrack->uSelInfo == VO_SOURCE2_SELECT_SELECTED)
			sprintf (szLine, "Sel:    %d Selected \r\n", pTrack->uSelInfo);
		else if (pTrack->uSelInfo == VO_SOURCE2_SELECT_DISABLE)
			sprintf (szLine, "Sel:    %d Sel-Disable \r\n", pTrack->uSelInfo);
		else if (pTrack->uSelInfo == VO_SOURCE2_SELECT_DEFAULT)
			sprintf (szLine, "Sel:    %d Sel-Default \r\n", pTrack->uSelInfo);
		else if (pTrack->uSelInfo == VO_SOURCE2_SELECT_FORCE)
			sprintf (szLine, "Sel:    %d Sel-Force \r\n", pTrack->uSelInfo);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		char * pFCC = (char *)pTrack->strFourCC;
		sprintf (szLine, "FCC:    %c%c%c%c ", pFCC[0], pFCC[1], pFCC[2], pFCC[3]);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		if (pTrack->uTrackType == VO_SOURCE2_TT_AUDIO)
			sprintf (szLine, "\r\nType:   %d audio \r\n", pTrack->uTrackType);
		else if (pTrack->uTrackType == VO_SOURCE2_TT_VIDEO)
			sprintf (szLine, "\r\nType:   %d video \r\n", pTrack->uTrackType);
		else if (pTrack->uTrackType == VO_SOURCE2_TT_IMAGE)
			sprintf (szLine, "\r\nType:   %d image \r\n", pTrack->uTrackType);
		else if (pTrack->uTrackType == VO_SOURCE2_TT_STREAM)
			sprintf (szLine, "\r\nType:   %d stream \r\n", pTrack->uTrackType);
		else if (pTrack->uTrackType == VO_SOURCE2_TT_SUBTITLE)
			sprintf (szLine, "\r\nType:   %d subtitle \r\n", pTrack->uTrackType);
		else
			sprintf (szLine, "\r\nType:   %d unknown \r\n", pTrack->uTrackType);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		sprintf (szLine, "Codec:  %d \r\n", pTrack->uCodec);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		sprintf (szLine, "Dur:    %llu \r\n", pTrack->uDuration);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		sprintf (szLine, "Chunks:    %d \r\n", pTrack->uChunkCounts);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		sprintf (szLine, "Bitrate:   %d \r\n", pTrack->uBitrate);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		sprintf (szLine, "HeadLen:   %d \r\n", pTrack->uHeadSize);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		sprintf (szLine, "MuxTrackID:  %d \r\n", pTrack->nMuxTrackID);
		yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));

		if (pTrack->uTrackType == VO_SOURCE2_TT_AUDIO)
		{
			sprintf (szLine, "Audio Info: \r\n");
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
			sprintf (szLine, "Language:   %s\r\n", pTrack->sAudioInfo.chLanguage);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
			sprintf (szLine, "SampleRate: %d\r\n", pTrack->sAudioInfo.sFormat.SampleRate);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
			sprintf (szLine, "Channels:   %d\r\n", pTrack->sAudioInfo.sFormat.Channels);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
			sprintf (szLine, "Bits:       %d\r\n", pTrack->sAudioInfo.sFormat.SampleBits);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		}
		else if (pTrack->uTrackType == VO_SOURCE2_TT_VIDEO)
		{
			sprintf (szLine, "Video Info: \r\n");
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
			sprintf (szLine, "Angle:   %d\r\n", pTrack->sVideoInfo.uAngle);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
			sprintf (szLine, "Desc:    %s\r\n", pTrack->sVideoInfo.strVideoDesc);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
			sprintf (szLine, "Width:   %d\r\n", pTrack->sVideoInfo.sFormat.Width);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
			sprintf (szLine, "Height:  %d\r\n", pTrack->sVideoInfo.sFormat.Height);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		}
		else if (pTrack->uTrackType == VO_SOURCE2_TT_SUBTITLE)
		{
			sprintf (szLine, "SubTitle Info: \r\n");
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
			sprintf (szLine, "Coding:   %d\r\n", pTrack->sSubtitleInfo.uCodingType);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
			sprintf (szLine, "Language: %s\r\n", pTrack->sSubtitleInfo.chLanguage);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
			sprintf (szLine, "Align:    %s\r\n", pTrack->sSubtitleInfo.Align);
			yyFileWrite (hFile, (unsigned char *)szLine, strlen (szLine));
		}
		nIndex++;
	}

	yyFileClose (hFile);

	return YY_ERR_NONE;
}
