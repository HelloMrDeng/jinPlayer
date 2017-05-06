/*******************************************************************************
	File:		CDataConvert.h

	Contains:	The data convert header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-20		Fenger			Create file

*******************************************************************************/
#ifndef __CDataConvert_H__
#define __CDataConvert_H__

#include "CBaseObject.h"
#include "CMutexLock.h"

#include "CFFMpegVideoRCC.h"
#include "CFFMpegAudioRSM.h"
#include "CVideoRCCThread.h"

#include "yyData.h"

class CDataConvert : public CBaseObject
{
public:
	CDataConvert(void * hInst);
	virtual ~CDataConvert(void);

	virtual int		Convert (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff, RECT * pZoom);
	virtual void	SetSpeed (float fSpeed);

protected:
	virtual int		ConvertAudio (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff);
	virtual int		ConvertVideo (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff, RECT * pZoom);
	virtual	int		OverLogo (YY_VIDEO_BUFF * pVideoBuff);

protected:
	void *				m_hInst;
	CMutexLock			m_mtConvert;
	CFFMpegVideoRCC *	m_pVideoRCC;
	CFFMpegAudioRSM *	m_pFFMpegASM;

	//For multi thread video color conversion
	int					m_nCPUNum;
	CVideoRCCThread **	m_ppVRCC;
	YY_BUFFER *			m_pInBuff;
	AVFrame *			m_pAVFrame;
	YY_VIDEO_BUFF *		m_pVideoBuff;

	float				m_fSpeed;
	
	int					m_nLicenseStatus;
};

#endif // __CDataConvert_H__
