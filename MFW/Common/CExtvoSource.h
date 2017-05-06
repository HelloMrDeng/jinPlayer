/*******************************************************************************
	File:		CExtvoSource.h

	Contains:	The ext vo source header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-01-24		Fenger			Create file

*******************************************************************************/
#ifndef __CExtvoSource_H__
#define __CExtvoSource_H__

#include "CBaseSource.h"

#include "voParser.h"

class CExtvoSource : public CBaseSource
{
public:
	CExtvoSource(void * hInst);
	virtual ~CExtvoSource(void);

	virtual int		Open (const TCHAR * pSource, int nType);
	virtual int		Close (void);
	virtual int		ForceClose (void);

	virtual int		Start (void);
	virtual int		Stop (void);

	virtual int		ReadData (YY_BUFFER * pBuff);

	static void		demuxCallback (VO_PARSER_OUTPUT_BUFFER* pData);

protected:
	void *					m_hModule;
	VO_PARSER_API			m_fAPI;
	VO_PTR					m_hDemux;
	YY_READ_EXT_DATA		m_extData;

	YY_BUFFER				m_bufRead;
	unsigned char *			m_pReadBuff;
	int						m_nReadSize;

	VO_PARSER_INPUT_BUFFER	m_bufDemux;

};

#endif // __CExtvoSource_H__
