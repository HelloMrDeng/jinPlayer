/*******************************************************************************
	File:		CBaseObject.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "CBaseObject.h"

int CBaseObject::g_ObjectNum = 0;

CBaseObject::CBaseObject(void)
{
	SetObjectName ("CBaseObject");
	g_ObjectNum++;

	m_llDbgTime = 0;
}

CBaseObject::~CBaseObject(void)
{
	g_ObjectNum--;
}

void CBaseObject::SetObjectName (const char * pObjName)
{
	m_pClassFileName = strrchr (pObjName, '\\');
	if (m_pClassFileName == NULL)
		m_pClassFileName = strrchr (pObjName, '/');
	if (m_pClassFileName != NULL)
	{
		m_pClassFileName += 1;
		if (strlen (m_pClassFileName) >= sizeof (m_szObjName))
			strncpy (m_szObjName, m_pClassFileName, sizeof (m_szObjName) - 2);
		else
			strcpy (m_szObjName, m_pClassFileName);
	}
	else
	{
		strcpy (m_szObjName, pObjName);
	}
}