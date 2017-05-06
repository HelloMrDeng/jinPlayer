/*******************************************************************************
	File:		CStreamParser.h

	Contains:	The adaption streaming wrap header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __CStreamParser_H__
#define __CStreamParser_H__

#include "CBaseObject.h"
#include "CMutexLock.h"

#include "voAdaptiveStreamParser.h"
#include "voSource2_ProgramInfo.h"

class CStreamParser : public CBaseObject
{
public:
	CStreamParser( VO_U32 adaptivestream_type);
	virtual ~CStreamParser(void);

	virtual VO_U32 Init ( VO_ADAPTIVESTREAM_PLAYLISTDATA * pData , VO_SOURCE2_EVENTCALLBACK * pCallback );
	virtual VO_U32 UnInit ();
	virtual VO_U32 Open ();
	virtual VO_U32 Close ();
	virtual VO_U32 Start ();
	virtual VO_U32 Stop ();
	virtual VO_U32 Update ( VO_ADAPTIVESTREAM_PLAYLISTDATA * pData );
	virtual VO_U32 GetChunk ( VO_SOURCE2_ADAPTIVESTREAMING_CHUNKTYPE uID ,  VO_ADAPTIVESTREAMPARSER_CHUNK **ppChunk );
	virtual VO_U32 Seek ( VO_U64 * pTimeStamp, VO_ADAPTIVESTREAMPARSER_SEEKMODE sSeekMode );
	virtual VO_U32 GetDuration ( VO_U64 * pDuration);
	virtual VO_U32 GetProgramCount ( VO_U32 *pProgramCount );
	virtual VO_U32 GetProgramInfo ( VO_U32 uProgram , _PROGRAM_INFO  **ppProgramInfo );
	virtual VO_U32 GetCurTrackInfo ( VO_SOURCE2_TRACK_TYPE sTrackType , _TRACK_INFO ** ppTrackInfo );
	virtual VO_U32 SelectProgram ( VO_U32 uProgram);
	virtual VO_U32 SelectStream ( VO_U32 uStream);
	virtual VO_U32 SelectTrack ( VO_U32 uTrack, VO_SOURCE2_TRACK_TYPE sTrackType);
	virtual VO_U32 GetDRMInfo ( VO_SOURCE2_DRM_INFO **ppDRMInfo );
	virtual VO_U32 GetParam ( VO_U32 nParamID, VO_PTR pParam );
	virtual VO_U32 SetParam ( VO_U32 nParamID, VO_PTR pParam );

	virtual VO_VOID	SetProgInfoFunc (VOS2_ProgramInfo_Func * pFunc);

protected:
	VO_ADAPTIVESTREAM_PARSER_API	m_fAPI;
	VO_HANDLE						m_hParser;

	CMutexLock						m_mtFunc;
	VOS2_ProgramInfo_Func *			m_pProgInfoFunc;
};


#endif
