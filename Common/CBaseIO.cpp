/*******************************************************************************
	File:		CBaseIO.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#ifdef _OS_LINUX
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif // _OS_LINUX

#include "CBaseIO.h"

#include "UThreadFunc.h"
#include "USystemFunc.h"
#include "yyLog.h"

CBaseIO::CBaseIO(void)
	: CBaseObject ()
{
	SetObjectName ("CBaseIO");
}

CBaseIO::~CBaseIO(void)
{
}

int CBaseIO::open (URLContext *h, const TCHAR * filename, int flags)
{
	return -1;
}

int CBaseIO::open2 (URLContext *h, const TCHAR *filename, int flags, AVDictionary **options)
{
	return -1;
}

int CBaseIO::read (URLContext *h, unsigned char *buf, int size)
{	
	return -1;
}

int CBaseIO::write(URLContext *h, const unsigned char *buf, int size)
{	
	return 0;
}

int64_t CBaseIO::seek (URLContext *h, int64_t pos, int whence)
{		
	return -1;
}

int CBaseIO::close (URLContext *h)
{	
	return 0;
}

int CBaseIO::read_pause (URLContext *h, int pause)
{
	return -1;
}

int64_t CBaseIO::read_seek (URLContext *h, int stream_index, int64_t timestamp, int flags)
{
	return -1;
}

int CBaseIO::get_handle (URLContext *h)
{	
	return (int)this;
}

int CBaseIO::get_multi_file_handle (URLContext *h, int **handles, int *numhandles)
{
	return -1;
}

int CBaseIO::shutdown (URLContext *h, int flags)
{
	return 0;
}

int CBaseIO::check (URLContext *h, int mask)
{	
	return 0;
}

