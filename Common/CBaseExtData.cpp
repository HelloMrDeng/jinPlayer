/*******************************************************************************
	File:		CBaseExtData.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "CBaseExtData.h"

#include "USystemFunc.h"
#include "UFFMpegFunc.h"

#include "yyLog.h"

YY_READ_EXT_DATA *	CBaseExtData::g_pExtData = NULL;

CBaseExtData::CBaseExtData(void)
	: CBaseIO ()
	, m_llSeekPos (-1)
	, m_llTimePos (-1)
	, m_llReadPos (0)
{
	memset (&m_bufRead, 0, sizeof (m_bufRead));
}

CBaseExtData::~CBaseExtData(void)
{

}

int CBaseExtData::open (URLContext *h, const TCHAR * filename, int flags)
{
	if (g_pExtData == NULL)
		return -1;

	return 0;
}

int CBaseExtData::read (URLContext *h, unsigned char *buf, int size)
{	
	if (g_pExtData == NULL)
		return -1;

	m_bufRead.nType = YY_MEDIA_Data;
	m_bufRead.uSize = size;
	m_bufRead.pBuff = buf;
	if (m_llTimePos != m_llSeekPos)
		m_bufRead.llTime = m_llSeekPos;
	else 
		m_bufRead.llTime = -1;
	m_llTimePos = m_llSeekPos;
	int nRC = g_pExtData->pRead (g_pExtData->pUser, &m_bufRead);

	if (nRC == YY_ERR_FINISH)
		return -1;

	m_llReadPos += m_bufRead.uSize;

	return m_bufRead.uSize;
}

int64_t CBaseExtData::seek (URLContext *h, int64_t pos, int whence)
{		
	if (g_pExtData == NULL)
		return -1;

	if (whence == AVSEEK_SIZE)
		return g_pExtData->llSize;
	else if (whence == SEEK_SET)
		m_llSeekPos = pos;
	else if (whence == SEEK_CUR)
		m_llSeekPos = m_llReadPos + pos;
	else if (whence == SEEK_END)
		m_llSeekPos = g_pExtData->llSize - pos;

	m_llReadPos = m_llSeekPos;

	return m_llSeekPos;
}

int CBaseExtData::close (URLContext *h)
{	
	if (g_pExtData == NULL)
		return -1;

	m_bufRead.nType = YY_MEDIA_Data;
	m_bufRead.uSize = 0;
	m_bufRead.pBuff = NULL;
	m_bufRead.llTime = -1;
	g_pExtData->pRead (g_pExtData->pUser, &m_bufRead);

	return 0;
}

int CBaseExtData::get_handle (URLContext *h)
{	
	return (int)g_pExtData;
}

int CBaseExtData::check (URLContext *h, int mask)
{	
	if (g_pExtData == NULL)
		return -1;

	return 0;
}

