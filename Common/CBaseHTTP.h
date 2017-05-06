/*******************************************************************************
	File:		CBaseHTTP.h

	Contains:	the data source class of all objects.

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseHTTP_H__
#define __CBaseHTTP_H__

#include "CBaseObject.h"
#include "CBaseUtils.h"
#include "CMutexLock.h"

#include "CBaseIO.h"

#include "voSource2_IO.h"

class CBaseHTTP : public CBaseIO
{
public:
	CBaseHTTP(void);
	virtual ~CBaseHTTP(void);

	virtual int 		open (URLContext *h, const TCHAR *filename, int flags);
	virtual int 		read(URLContext *h, unsigned char *buf, int size);
	virtual int 		write(URLContext *h, const unsigned char *buf, int size);
	virtual int64_t		seek(URLContext *h, int64_t pos, int whence);
	virtual int			close(URLContext *h);
	virtual int 		get_handle(URLContext *h);
	virtual int 		check(URLContext *h, int mask);
	virtual long long	GetSize (void) {return m_llFileSize;}
    virtual int			shutdown (URLContext *h, int flags);

	static	VO_U32		ioOpenNotify (VO_PTR pUserData , VO_U32 uID , VO_U32 uError , VO_PBYTE pBuf , VO_U32 uSizeDone);
	static	VO_U32		ioImplNotify (VO_PTR hHandle , VO_U32 uID , VO_PTR pParam1 , VO_PTR pParam2);

protected:
	void *				m_hDll;
	VO_SOURCE2_IO_API	m_fAPI;
	VO_HANDLE			m_hHandle;

	long long			m_llFileSize;


	VO_SOURCE2_IO_ASYNC_CALLBACK	m_sOpenCB;
	VO_SOURCE2_IO_HTTPCALLBACK		m_sImplCB;
};

#endif // __CBaseHTTP_H__
