/*******************************************************************************
	File:		CBaseIOHook.h

	Contains:	the data source class of all objects.

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseIOHook_H__
#define __CBaseIOHook_H__

#include "CBaseObject.h"
#include "CBaseUtils.h"

#include "CBaseIO.h"

class CBaseIOHook : public CBaseIO
{
public:
	CBaseIOHook(void);
	virtual ~CBaseIOHook(void);

	virtual int 		open (URLContext *h, const TCHAR *filename, int flags);
	virtual int 		open2 (URLContext *h, const TCHAR *filename, int flags, AVDictionary **options);
	virtual int 		read(URLContext *h, unsigned char *buf, int size);
	virtual int 		write(URLContext *h, const unsigned char *buf, int size);
	virtual int64_t		seek(URLContext *h, int64_t pos, int whence);
	virtual int			close(URLContext *h);
    virtual int			read_pause (URLContext *h, int pause);
    virtual int64_t		read_seek (URLContext *h, int stream_index, int64_t timestamp, int flags);
	virtual int 		get_handle(URLContext *h);
    virtual int			get_multi_file_handle (URLContext *h, int **handles, int *numhandles);
    virtual int			shutdown (URLContext *h, int flags);
	virtual int 		check(URLContext *h, int mask);

protected:
	URLProtocol *		m_pURLPrtc;
};

#endif // __CBaseIOHook_H__
