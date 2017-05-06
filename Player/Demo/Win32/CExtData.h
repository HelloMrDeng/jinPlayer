/*******************************************************************************
	File:		CExtData.h

	Contains:	The ext render player header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-21		Fenger			Create file

*******************************************************************************/
#ifndef __CExtData_H__
#define __CExtData_H__

#include "CBaseObject.h"

#include "yyData.h"

class CExtData : public CBaseObject
{
public:
	CExtData(void * hInst);
	virtual ~CExtData(void);

	YY_READ_EXT_DATA *	GetExtData (YYExtDataType nType);
	static	int			ReadExtData (void * pUser, YY_BUFFER * pData);

protected:
	virtual int		ReadMuxData (YY_BUFFER * pData);
	virtual int		ReadVideoData (YY_BUFFER * pData);
	virtual int		ReadAudioData (YY_BUFFER * pData);

protected:
	void *				m_hInst;
	
	HANDLE				m_hFile;
	long long			m_llSize;

	int					m_nReadTimes;
	int					m_nReadFiles;
	int					m_nReadDataSize;

	HANDLE				m_hFileVideo;
	unsigned char *		m_pBuffVideo;
	unsigned char *		m_pVideoPos;
	int					m_nSizeVideo;
	YY_VIDEO_FORMAT		m_fmtVideo;
	int					m_nVideoCount;

	HANDLE				m_hFileAudio;
	unsigned char *		m_pBuffAudio;
	unsigned char *		m_pAudioPos;
	int					m_nSizeAudio;
	YY_AUDIO_FORMAT		m_fmtAudio;
	int					m_nAudioCount;


	YY_READ_EXT_DATA	m_extData;
};

#endif // __CExtData_H__
