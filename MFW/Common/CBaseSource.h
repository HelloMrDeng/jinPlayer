/*******************************************************************************
	File:		CBaseSource.h

	Contains:	The base source header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseSource_H__
#define __CBaseSource_H__

#include "CBaseObject.h"

#include "yyThumbnail.h"
#include "yyMediaPlayer.h"
#include "yyneonFunc.h"

#include "UStringFunc.h"

class CBaseSource : public CBaseObject
{
public:
	CBaseSource(void * hInst);
	virtual ~CBaseSource(void);

	virtual int		Open (const TCHAR * pSource, int nType);
	virtual int		Close (void);
	virtual int		ForceClose (void);
	virtual int		GetStreamCount (YYMediaType nType);
	virtual int		GetStreamPlay (YYMediaType nType);
	virtual int		SetStreamPlay (YYMediaType nType, int nStream);
	virtual int		EnableSubTT (bool bEnable);

	virtual int		Start (void);
	virtual int		Pause (void);
	virtual int		Stop (void);

	virtual int		ReadData (YY_BUFFER * pBuff);

	virtual int		SetPos (long long llPos);
	virtual int		GetPos (void);
	virtual int		GetDuration (void);
	virtual bool	IsEOF (void) {return m_bEOF;}

	virtual int 	SetParam (int nID, void * pParam);
	virtual int		GetParam (int nID, void * pParam);

	virtual YY_AUDIO_FORMAT *	GetAudioFormat (void) {return &m_fmtAudio;}
	virtual YY_VIDEO_FORMAT *	GetVideoFormat (void) {return &m_fmtVideo;}
	virtual YY_SUBTT_FORMAT *	GetSubTTFormat (void) {return &m_fmtSubTT;}
	virtual int					GetMediaInfo (TCHAR * pInfo, int nSize);
	virtual int					FillMediaInfo (YYINFO_Thumbnail * pInfo);
	virtual char *				GetSourceName (void) {return m_szSourceName;}

protected:
	virtual int			CheckConfigLimit (int nCodec, int nWidth, int nHeight);
	virtual void		ResetParam (int nLevel);

protected:
	void *				m_hInst;
	bool				m_bSubTTEnable;
	char				m_szSourceName[1024];
	TCHAR				m_szSource[1024];
	int					m_nSourceType;
	bool				m_bForceClosed;
	YY_PROT_TYPE		m_nProtType;
	long long			m_llDuration;
	long long			m_llDurAudio;
	long long			m_llDurVideo;
	int					m_nAudioStreamNum;
	int					m_nVideoStreamNum;
	int					m_nSubTTStreamNum;
	int					m_nAudioStreamPlay;
	int					m_nVideoStreamPlay;
	int					m_nSubTTStreamPlay;

	YY_AUDIO_FORMAT		m_fmtAudio;
	YY_VIDEO_FORMAT		m_fmtVideo;
	YY_SUBTT_FORMAT		m_fmtSubTT;

	long long			m_llSeekPos;
	bool				m_bVideoNewPos;
	bool				m_bAudioNewPos;
	bool				m_bSubTTNewPos;

	bool				m_bEOF;
	bool				m_bEOA;
	bool				m_bEOV;

};

#endif // __CBaseSource_H__
