/*******************************************************************************
	File:		CBaseUtils.h

	Contains:	the base utils class of all objects.

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-08		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseUtils_H__
#define __CBaseUtils_H__

#include "CBaseObject.h"

#include <libavformat/avformat.h>
#include <libavformat/url.h>

class CBaseUtils : public CBaseObject
{
public:
	CBaseUtils(void);
	virtual ~CBaseUtils(void);

public:
	typedef struct FileContext {
		const void *	pClass;
		void *			hFile;
		int				trunc;
	} FileContext;

	static int		FillExtIOFunc (URLProtocol * pProt);
	static int		FreeExtIOFunc (URLProtocol * pProt);

	// ext file IO functions
	static int 		yy_extio_open (URLContext *h, const char *filename, int flags);
    static int		yy_extio_open2 (URLContext *h, const char *url, int flags, AVDictionary **options);
	static int 		yy_extio_read (URLContext *h, unsigned char *buf, int size);
	static int 		yy_extio_write (URLContext *h, const unsigned char *buf, int size);
	static int64_t	yy_extio_seek (URLContext *h, int64_t pos, int whence);
	static int		yy_extio_close (URLContext *h);
    static int		yy_extio_read_pause (URLContext *h, int pause);
    static int64_t	yy_extio_read_seek (URLContext *h, int stream_index, int64_t timestamp, int flags);
	static int 		yy_extio_get_handle (URLContext *h);
    static int		yy_extio_get_multi_file_handle (URLContext *h, int **handles, int *numhandles);
    static int		yy_extio_shutdown (URLContext *h, int flags);
	static int 		yy_extio_check (URLContext *h, int mask);

	static int		ConvertDataToBase64 (unsigned char * pData, int nDataSize, char * pOutText, int nOutSize);
	static int		ConvertBase64ToData (char * pBase64, unsigned char * pOutData, int nOutSize);
};

#endif // __CBaseUtils_H__
