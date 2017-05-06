/*******************************************************************************
	File:		CVVAudioDec.h

	Contains:	The vo audio dec wrap header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __CVVAudioDec_H__
#define __CVVAudioDec_H__

#include "CBaseAudioDec.h"

#include <libavformat/avformat.h>
#include "voAudio.h"

class CVVAudioDec : public CBaseAudioDec
{
public:
	CVVAudioDec(void * hInst);
	virtual ~CVVAudioDec(void);

	virtual int		Init (YY_AUDIO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Flush (void);

	virtual int		SetBuff (YY_BUFFER * pBuff);
	virtual int		GetBuff (YY_BUFFER * pBuff);

protected:
	VO_AUDIO_CODECAPI	m_fAPI;
	VO_HANDLE			m_hDec;

	VO_CODECBUFFER		m_Input;
	VO_CODECBUFFER		m_Output;
	VO_AUDIO_OUTPUTINFO	m_OutputInfo;

	unsigned char *		m_pOutBuff;
	int					m_nOutSize;
};

#endif // __CVVAudioDec_H__
