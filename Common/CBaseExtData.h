/*******************************************************************************
	File:		CBaseExtData.h

	Contains:	the data source class of all objects.

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseExtData_H__
#define __CBaseExtData_H__

#include "CBaseObject.h"
#include "CBaseUtils.h"

#include "CBaseIO.h"

class CBaseExtData : public CBaseIO
{
public:
	CBaseExtData(void);
	virtual ~CBaseExtData(void);

	virtual int 		open (URLContext *h, const TCHAR *filename, int flags);
	virtual int 		read(URLContext *h, unsigned char *buf, int size);
	virtual int64_t		seek(URLContext *h, int64_t pos, int whence);
	virtual int			close(URLContext *h);
	virtual int 		get_handle(URLContext *h);
	virtual int 		check(URLContext *h, int mask);

protected:
	YY_BUFFER			m_bufRead;
	long long			m_llSeekPos;
	long long			m_llTimePos;
	long long			m_llReadPos;

public:
	static YY_READ_EXT_DATA *	g_pExtData;
};

#endif // __CBaseExtData_H__
