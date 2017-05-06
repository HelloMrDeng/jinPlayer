/*******************************************************************************
	File:		CExtData.h

	Contains:	The ext render player header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-21		Fenger			Create file

*******************************************************************************/
#ifndef __CExtData_H__
#define __CExtData_H__

#include "./Include/yyData.h"

class CExtData
{
public:
	CExtData(void );
	virtual ~CExtData(void);

	YY_READ_EXT_DATA *	GetExtData (YYExtDataType nType);
	static	int			ReadExtData (void * pUser, YY_BUFFER * pData);

protected:
	virtual int		ReadMuxData (YY_BUFFER * pData);
	virtual int		ReadVideoData (YY_BUFFER * pData);
	virtual int		ReadAudioData (YY_BUFFER * pData);

protected:
	FILE *				m_hFile;
	long long			m_llSize;

	YY_READ_EXT_DATA	m_extData;
};

#endif // __CExtData_H__
