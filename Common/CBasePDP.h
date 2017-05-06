/*******************************************************************************
	File:		CBasePDP.h

	Contains:	the data source class of all objects.

	Written by:	Fenger King

	Change History (most recent first):
	2015-03-14		Fenger			Create file

*******************************************************************************/
#ifndef __CBasePDP_H__
#define __CBasePDP_H__

#include "CBaseObject.h"
#include "CBaseUtils.h"
#include "CMutexLock.h"

#include "CBaseIO.h"

#include "UThreadFunc.h"

class CSourceIO;

class CBasePDP : public CBaseIO
{
public:
	CBasePDP(void);
	virtual ~CBasePDP(void);

	virtual int 		open (URLContext *h, const TCHAR *filename, int flags);
	virtual int 		read(URLContext *h, unsigned char *buf, int size);
	virtual int 		write(URLContext *h, unsigned char *buf, int size);
	virtual int64_t		seek(URLContext *h, int64_t pos, int whence);
	virtual int			close(URLContext *h);
	virtual int 		get_handle(URLContext *h);
	virtual int 		check(URLContext *h, int mask);
	virtual long long	GetSize (void) {return m_llFileSize;}

protected:	
	CSourceIO *			m_pIO;

	long long			m_llFileSize;
	unsigned char *		m_pBuffer;
	int					m_nBuffRead;
	int					m_nBuffDown;

	yyThreadHandle		m_hThread;
	YYPLAY_STATUS		m_nStatus;
	CMutexLock			m_mtBuff;

protected:
	static	int			DownLoadProc (void * pParam);
	virtual int			DownLoadLoop (void);

public:
	static TCHAR		g_szPDPFile[1024];

};

#endif // __CBasePDP_H__
