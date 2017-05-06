/*******************************************************************************
	File:		CExtData.cpp

	Contains:	The ext data read implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "CExtData.h"

CExtData::CExtData(void)
	: m_hFile (NULL)
	, m_llSize (0)
{
	memset (&m_extData, 0, sizeof (m_extData));
}

CExtData::~CExtData(void)
{
	if (m_hFile != NULL)
		fclose (m_hFile);
	m_hFile = NULL;
}

YY_READ_EXT_DATA * CExtData::GetExtData (YYExtDataType nType)
{
	if (m_hFile == NULL)
	{
		m_hFile = fopen ("/sdcard/Bang/GeniaTeck/Dump_mpeg.ts", "rb");		
		if (m_hFile != NULL)
		{
			fseeko (m_hFile, 0LL, SEEK_END);
			m_llSize = ftello (m_hFile);
			fseeko (m_hFile, 0, SEEK_SET);		
		}
	}

	m_extData.pUser = this;
	m_extData.pRead = ReadExtData;
	m_extData.nType = nType;
	m_extData.llSize = m_llSize;
	strcpy (m_extData.szName, "file:\\yyextdata.ts");

	return &m_extData;
}

int CExtData::ReadExtData (void * pUser, YY_BUFFER * pData)
{
	CExtData * pExtData = (CExtData *)pUser;
	if (pData->nType == YY_MEDIA_Data)
		return pExtData->ReadMuxData (pData);
	else if (pData->nType == YY_MEDIA_Video)
		return pExtData->ReadVideoData (pData);
	else if (pData->nType == YY_MEDIA_Audio)
		return pExtData->ReadAudioData (pData);
	else
		return -1;
}

int CExtData::ReadMuxData (YY_BUFFER * pData)
{
	if (m_hFile == NULL)
	{
		pData->uSize = 0;
		return YY_ERR_FAILED;
	}

	if (pData->llTime >= 0)
		fseeko (m_hFile, pData->llTime, SEEK_SET);

	int nRead = fread (pData->pBuff, 1, pData->uSize, m_hFile);
	pData->uSize = nRead;

	return 0;
}

int CExtData::ReadVideoData (YY_BUFFER * pData)
{
	return 0;
}

int CExtData::ReadAudioData (YY_BUFFER * pData)
{
	return 0;
}

