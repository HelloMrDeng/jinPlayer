/*******************************************************************************
	File:		CBaseTCP.h

	Contains:	the data source class of all objects.

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseTCP_H__
#define __CBaseTCP_H__

#include "CBaseIO.h"

class CBaseTCP : public CBaseIO
{
public:
	CBaseTCP(void);
	virtual ~CBaseTCP(void);

	virtual int 		open (URLContext *h, const TCHAR *filename, int flags);
	virtual int 		read(URLContext *h, unsigned char *buf, int size);
	virtual int 		write(URLContext *h, const unsigned char *buf, int size);
	virtual int			close(URLContext *h);
	virtual int 		get_handle(URLContext *h);
    virtual int			shutdown (URLContext *h, int flags);
};

#endif // __CBaseTCP_H__
