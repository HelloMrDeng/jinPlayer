/*******************************************************************************
	File:		CExtSource.h

	Contains:	The ext source header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#ifndef __CExtSource_H__
#define __CExtSource_H__

#include "CBaseObject.h"
#include "UFileFunc.h"
#include "yyData.h"

class CExtSource : public CBaseObject
{
public:
	CExtSource(void * hInst);
	virtual ~CExtSource(void);

	YY_READ_EXT_DATA *	GetExtData (YYExtDataType nType, TCHAR * pSource);
	static	int			ReadExtData (void * pUser, YY_BUFFER * pData);

protected:
	virtual int			ReadMuxData (YY_BUFFER * pData);

protected:
	void *				m_hInst;	
	yyFile				m_hFile;
	long long			m_llSize;
	YY_READ_EXT_DATA	m_extData;
	TCHAR				m_szSource[1024];

	bool				m_bKeyFile;
	int					m_nKeyLen;
	unsigned char *		m_pBuff;
	unsigned int		m_nSize;
};

#endif // __CExtSource_H__
