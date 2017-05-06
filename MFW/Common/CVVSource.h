/*******************************************************************************
	File:		CVVSource.h

	Contains:	The image file source header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-06-02		Fenger			Create file

*******************************************************************************/
#ifndef __CVVSource_H__
#define __CVVSource_H__

#include "CBaseSource.h"
#include <libavformat/avformat.h>

#include "UFileFunc.h"

class CVVSource : public CBaseSource
{
public:
	CVVSource(void * hInst);
	virtual ~CVVSource(void);

	virtual int		Open (const TCHAR * pSource, int nType);
	virtual int		Close (void);

	virtual int		ReadData (YY_BUFFER * pBuff);

protected:
	YY_READ_EXT_DATA *		m_pExtSrc;
	yyFile					m_hFile;
	AVCodecID				m_nCodecID;
	AVPacket				m_pktData;
	int						m_nStartTime;
	int						m_nSampleTime;
};

#endif // __CVVSource_H__
