/*******************************************************************************
	File:		CProgInfo.h

	Contains:	The program info implement header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __CProgInfo_H__
#define __CProgInfo_H__

#include "voSource2_ProgramInfo.h"

#include "CBaseObject.h"
#include "CMutexLock.h"
#include "CNodeList.h"

class CProgInfo
{
public:
	CProgInfo (void);
	virtual ~CProgInfo(void);

	VO_U32 SetProgType (VO_SOURCE2_PROGRAM_TYPE nType);
	VO_U32 RegNotify (VOS2_ProgInfo_Notify * pNotify, VO_BOOL bReg);
	VO_U32 SetStream (VO_SOURCE2_STREAM_INFO * pStream, VOS2_PROGINFO_COMMAND nCmd);
	VO_U32 SetTrack (VO_U32 uStrmID, VO_SOURCE2_TRACK_INFO * pTrack, VOS2_PROGINFO_COMMAND nCmd);
	VO_U32 GetStream (VOS2_PROGINFO_GETTYPE nGetType, VO_U32 uValue, VO_SOURCE2_STREAM_INFO * pStream);
	VO_U32 GetTrack (VOS2_PROGINFO_GETTYPE nGetType, VO_U32 uStrmID, VO_U32 uValue, VO_SOURCE2_TRACK_INFO * pTrack, VO_BOOL bCopyHead);
	VO_U32 Reset (void);
	VO_U32 GetParam (VO_U32 uID, VO_PTR pValue);
	VO_U32 DumpInfo (VO_TCHAR * pFile);

	virtual VO_U32		GetSelectedStream (void);
	virtual VO_U32		GetSelectedTrack (void);

	virtual VO_U32		SetDefaultStreamSelected (void);

	virtual VOS2_ProgramInfo_Func *		GetFunc (void) {return &m_func;}
	virtual VO_SOURCE2_PROGRAM_INFO *	GetProg (void) {return &m_progInfo;}

protected:
	virtual VO_U32	FindStream (VO_U32 uStrmID, VO_SOURCE2_STREAM_INFO ** ppStream);
	virtual VO_U32	FindTrack (VO_U32 uStreamID, VO_U32 uTrackID, VO_SOURCE2_TRACK_INFO ** ppTrack);
	virtual VO_U32	DeleteTrack (VO_U32 uTrackID);

	virtual VO_U32	Notify (VO_U32 uID, VO_U32 uStatus, VO_PTR pValue);

protected:
	CMutexLock							m_lock;
	VO_SOURCE2_PROGRAM_INFO				m_progInfo;
	CObjectList <VO_SOURCE2_TRACK_INFO>	m_lstTrack;
	VOS2_ProgramInfo_Func				m_func;
	CObjectList <VOS2_ProgInfo_Notify>	m_lstNotify;
};

#ifdef _VONAMESPACE
}
#endif

#endif // __CProgInfo_H__
