/*******************************************************************************
	File:		CBaseIOHook.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "CBaseIOHook.h"

#include "USystemFunc.h"
#include "UFFMpegFunc.h"

#include "yyLog.h"

CBaseIOHook::CBaseIOHook(void)
	: CBaseIO ()
{
	m_pURLPrtc = (URLProtocol *)g_ioFileExt.priv_data_class;
}

CBaseIOHook::~CBaseIOHook(void)
{

}

int CBaseIOHook::open (URLContext *h, const TCHAR * filename, int flags)
{
#ifdef _OS_WIN32	
	char szFile[2048];
	memset (szFile, 0, sizeof (szFile));
	WideCharToMultiByte (CP_ACP, 0, filename, -1, szFile, sizeof (szFile), NULL, NULL);
	h->filename = szFile;
	int nRC = m_pURLPrtc->url_open (h, (const char *)szFile, flags);

	return nRC;
#else
	return m_pURLPrtc->url_open (h, (const char *)filename, flags);
#endif // _OS_WIN32
}

int CBaseIOHook::open2 (URLContext *h, const TCHAR *filename, int flags, AVDictionary **options)
{
	return m_pURLPrtc->url_open2 (h, (const char *)filename, flags, options);
}

int CBaseIOHook::read (URLContext *h, unsigned char *buf, int size)
{	
	return m_pURLPrtc->url_read (h, buf, size);
}

int CBaseIOHook::write(URLContext *h, const unsigned char *buf, int size)
{	
	return m_pURLPrtc->url_write (h, buf, size);
}

int64_t CBaseIOHook::seek (URLContext *h, int64_t pos, int whence)
{		
	return m_pURLPrtc->url_seek (h, pos, whence);
}

int CBaseIOHook::close (URLContext *h)
{	
	return m_pURLPrtc->url_close (h);
}

int CBaseIOHook::read_pause (URLContext *h, int pause)
{
	return m_pURLPrtc->url_read_pause (h, pause);
}

int64_t CBaseIOHook::read_seek (URLContext *h, int stream_index, int64_t timestamp, int flags)
{
	return m_pURLPrtc->url_read_seek (h, stream_index, timestamp, flags);
}

int CBaseIOHook::get_handle (URLContext *h)
{	
	return m_pURLPrtc->url_get_file_handle (h);
}

int CBaseIOHook::get_multi_file_handle (URLContext *h, int **handles, int *numhandles)
{
	return m_pURLPrtc->url_get_multi_file_handle (h, handles, numhandles);
}

int CBaseIOHook::shutdown (URLContext *h, int flags)
{
	return m_pURLPrtc->url_shutdown (h, flags);
}

int CBaseIOHook::check (URLContext *h, int mask)
{	
	return m_pURLPrtc->url_check (h, mask);
}

