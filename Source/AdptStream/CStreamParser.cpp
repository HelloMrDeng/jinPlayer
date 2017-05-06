/*******************************************************************************
	File:		CStreamParser.cpp

	Contains:	stream parser implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#include "CStreamParser.h"

#include "voAdaptiveStreamHLS.h"

CStreamParser::CStreamParser (VO_U32 adaptivestream_type)
	: CBaseObject ()
	, m_hParser (NULL)
	, m_pProgInfoFunc (NULL)
{
	SetObjectName ("CStreamParser");

	memset( &m_fAPI , 0 , sizeof(VO_ADAPTIVESTREAM_PARSER_API) );
	
	switch( adaptivestream_type )
	{
	case VO_ADAPTIVESTREAMPARSER_STREAMTYPE_HLS:
		yyGetHLSParserFunc (&m_fAPI);
		break;
	case VO_ADAPTIVESTREAMPARSER_STREAMTYPE_ISS:
		break;
	case VO_ADAPTIVESTREAMPARSER_STREAMTYPE_DASH:
		break;
	default:
		break;
	}
}

CStreamParser::~CStreamParser(void)
{
}

VO_U32 CStreamParser::Init ( VO_ADAPTIVESTREAM_PLAYLISTDATA * pData , VO_SOURCE2_EVENTCALLBACK * pCallback )
{
	CAutoLock lock (&m_mtFunc);
	if (m_fAPI.Init)
	{	
		VO_ADAPTIVESTREAMPARSER_INITPARAM initParam;
		initParam.uFlag = 0;
		initParam.pInitParam = NULL;
		initParam.strWorkPath =  NULL;
		return m_fAPI.Init( &m_hParser , pData , pCallback,&initParam );
	}
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::UnInit ()
{
	CAutoLock lock (&m_mtFunc);
	if (m_fAPI.UnInit)
		return m_fAPI.UnInit( m_hParser );
	else
		return VO_RET_SOURCE2_OK;
}

VO_U32 CStreamParser::Open ()
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.Open)
		return m_fAPI.Open( m_hParser );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::Close ()
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.Close)
		return m_fAPI.Close( m_hParser );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::Start ()
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.Start)
		return m_fAPI.Start( m_hParser );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::Stop ()
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.Stop)
		return m_fAPI.Stop( m_hParser );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::Update ( VO_ADAPTIVESTREAM_PLAYLISTDATA * pData )
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.Update)
		return m_fAPI.Update( m_hParser , pData );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::GetChunk ( VO_SOURCE2_ADAPTIVESTREAMING_CHUNKTYPE uID ,  VO_ADAPTIVESTREAMPARSER_CHUNK **ppChunk )
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.GetChunk)
		return m_fAPI.GetChunk( m_hParser , uID , ppChunk );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::Seek ( VO_U64 * pTimeStamp, VO_ADAPTIVESTREAMPARSER_SEEKMODE sSeekMode )
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.Seek)
		return m_fAPI.Seek( m_hParser , pTimeStamp, sSeekMode );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::GetDuration ( VO_U64 * pDuration)
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.GetDuration)
		return m_fAPI.GetDuration( m_hParser , pDuration );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::GetProgramCount ( VO_U32 *pProgramCount )
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.GetProgramCount)
		return m_fAPI.GetProgramCount( m_hParser , pProgramCount );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::GetProgramInfo ( VO_U32 uProgram , _PROGRAM_INFO  **ppProgramInfo )
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.GetProgramInfo)
		return m_fAPI.GetProgramInfo( m_hParser , uProgram , ppProgramInfo );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::GetCurTrackInfo ( VO_SOURCE2_TRACK_TYPE sTrackType , _TRACK_INFO ** ppTrackInfo )
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.GetCurTrackInfo)
		return m_fAPI.GetCurTrackInfo( m_hParser , sTrackType , ppTrackInfo );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::SelectProgram ( VO_U32 uProgram)
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.SelectProgram)
		return m_fAPI.SelectProgram( m_hParser , uProgram );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::SelectStream ( VO_U32 uStream)
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.SelectStream)
	{
		VO_U32 nRC = m_fAPI.SelectStream( m_hParser , uStream,  VO_SOURCE2_ADAPTIVESTREAMING_CHUNKPOS_PRESENT);
		if (nRC == VO_RET_SOURCE2_OK && m_pProgInfoFunc != NULL)
		{
			VO_SOURCE2_STREAM_INFO stmInfo;
			stmInfo.uStreamID = uStream;
			m_pProgInfoFunc->SetStream (m_pProgInfoFunc->hHandle, &stmInfo, VOS2_PROGINFO_SELECT);
		}
		return nRC;
	}
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::SelectTrack ( VO_U32 uTrack, VO_SOURCE2_TRACK_TYPE sTrackType)
{
	CAutoLock lock (&m_mtFunc);
	if (!m_fAPI.SelectTrack)
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
		
	VO_U32 uRC = m_fAPI.SelectTrack( m_hParser , uTrack, sTrackType );
	if (uRC == VO_RET_SOURCE2_OK && m_pProgInfoFunc != NULL)
	{
		VO_SOURCE2_TRACK_INFO trkInfo;
		trkInfo.uTrackID = uTrack;
		m_pProgInfoFunc->SetTrack (m_pProgInfoFunc->hHandle, -1, &trkInfo, VOS2_PROGINFO_SELECT);
	}
	return uRC;
}

VO_U32 CStreamParser::GetDRMInfo ( VO_SOURCE2_DRM_INFO **ppDRMInfo )
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.GetDRMInfo)
		return m_fAPI.GetDRMInfo( m_hParser , ppDRMInfo );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::GetParam ( VO_U32 nParamID, VO_PTR pParam )
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.GetParam)
		return m_fAPI.GetParam( m_hParser , nParamID , pParam );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_U32 CStreamParser::SetParam ( VO_U32 nParamID, VO_PTR pParam )
{
	CAutoLock lock (&m_mtFunc);

	if (m_fAPI.SetParam)
		return m_fAPI.SetParam( m_hParser , nParamID , pParam );
	else
		return VO_RET_SOURCE2_FORMATUNSUPPORT;
}

VO_VOID	CStreamParser::SetProgInfoFunc (VOS2_ProgramInfo_Func * pFunc)
{
	CAutoLock lock (&m_mtFunc);

	m_pProgInfoFunc = pFunc;
}