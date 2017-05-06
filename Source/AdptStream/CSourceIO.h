/*******************************************************************************
	File:		CSourceIO.h

	Contains:	The source IO header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-02-24		Fenger			Create file

*******************************************************************************/
#ifndef __CSourceIO_H__
#define __CSourceIO_H__

#include "voIndex.h"
#include "voSource2_IO.h"
#include "voSSL.h"

#include "CBaseObject.h"
#include "CMutexLock.h"

class CSourceIO : public CBaseObject
{
public:
	CSourceIO (void);
	virtual ~CSourceIO (void);

	virtual int		Init (const char * pURL, int nFlag);
	virtual int		Open (void);
	virtual int		Read (VO_PBYTE	pData, VO_U32 nSize, int * pRead);
	virtual int		Seek (VO_S64 llPos);
	virtual VO_U64	GetSize (void) {return m_llFileSize;}
	virtual VO_U32	SetParam (VO_U32 uParamID , VO_PTR pParam);
	virtual int		Close (void);

	VO_SOURCE2_IO_API * GetIO (void) {return &m_fAPI;}

	static	VO_U32		ioOpenNotify (VO_PTR pUserData , VO_U32 uID , VO_U32 uError , VO_PBYTE pBuf , VO_U32 uSizeDone);
	static	VO_U32		ioImplNotify (VO_PTR hHandle , VO_U32 uID , VO_PTR pParam1 , VO_PTR pParam2);

protected:
	virtual bool		LoadSSL (void);
	virtual bool		FreeSSL (void);

protected:
	VO_SOURCE2_IO_API				m_fAPI;
	VO_SOURCE2_IO_ASYNC_CALLBACK	m_sOpenCB;
	VO_SOURCE2_IO_HTTPCALLBACK		m_sImplCB;

	CMutexLock						m_mtIO;
	char							m_szURL[2048];
	VO_U64							m_llFileSize;
	bool							m_bExitRead;

	// Open SSL 
	void *					m_hSSLDll;
	vosslapi				m_sSSLApi;
};

#endif
