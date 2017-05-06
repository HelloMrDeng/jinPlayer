/*******************************************************************************
	File:		CBaseFile.h

	Contains:	the data source class of all objects.

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseFile_H__
#define __CBaseFile_H__

#include "CBaseObject.h"
#include "CBaseUtils.h"
#include "CMutexLock.h"

#include "CBaseIO.h"

class CBaseFile : public CBaseIO
{
public:
	CBaseFile(void);
	virtual ~CBaseFile(void);

	virtual int 		open (URLContext *h, const TCHAR *filename, int flags);
	virtual int 		read(URLContext *h, unsigned char *buf, int size);
	virtual int 		write(URLContext *h, unsigned char *buf, int size);
	virtual int64_t		seek(URLContext *h, int64_t pos, int whence);
	virtual int			close(URLContext *h);
	virtual int 		get_handle(URLContext *h);
	virtual int 		check(URLContext *h, int mask);
	virtual long long	GetSize (void) {return m_llFileSize;}

protected:
#ifdef _OS_WIN32
	HANDLE				m_hFile;
#elif defined _OS_LINUX
	FILE *				m_hFile;
#endif // _OS_WIN32
	int					m_nFD;
	bool				m_bKeyFile;
	int					m_nKeyLen;
	
	long long			m_llFileSize;
	long long			m_llFilePos;
	unsigned char *		m_pBuffer;
	int					m_nBuffSize;

	int					m_nReadTimes;

	int					m_nBitrate;
	int					m_nStartTime;

public:
	static bool			g_bFileError;
	static long long	g_llFileSize;
	static long long	g_llFileSeek;
	static char			g_szKey[32];
};

#endif // __CBaseFile_H__
