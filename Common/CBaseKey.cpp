/*******************************************************************************
	File:		CBaseKey.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "CBaseKey.h"

#include "CBaseUtils.H"
#include "USystemFunc.h"
#include "UStringFunc.h"

#include "yyLog.h"

CBaseKey * CBaseKey::g_Key = NULL;

CBaseKey::CBaseKey(void)
	: CBaseObject ()
{
	SetObjectName ("CBaseKey");

	memset (m_szKey1, 0, sizeof (m_szKey1));
	strcpy (m_szKey1, "0420XJY*(@F");
	memset (m_szKey2, 0, sizeof (m_szKey2));
	strcpy (m_szKey2, "IBOXafsdjlf");
	memset (m_szKey3, 0, sizeof (m_szKey3));
	strcpy (m_szKey3, "F&*D)SDFSsfsd");
	memset (m_szKey4, 0, sizeof (m_szKey4));
	strcpy (m_szKey4, "&**F^SFHKJFHasd");

	memset (m_szPW, 0, sizeof (m_szPW));
	memset (m_szKey, 0, sizeof (m_szKey));
	memset (m_wzKey, 0, sizeof (m_wzKey));

	g_Key = this;
}

CBaseKey::~CBaseKey(void)
{
	g_Key = NULL;
}

bool CBaseKey::CreateKey (TCHAR * pPassWord)
{
	if (pPassWord == NULL)
		return false;

	int				nSize = sizeof (m_szKey);
	unsigned char * pSource = new unsigned char[nSize];
	char *			pTarget = new char[nSize];
	unsigned char * pData = (unsigned char *)pPassWord;
	int				nLeng = _tcslen (pPassWord) * sizeof (TCHAR);

	int i = 0;
	int j = 0;
	memset (pSource, 0, nSize);
	for (i = 0; i <nLeng; i++) 
	{
		pSource[i] = pData[i];
		for (j = 5; j < strlen (m_szKey3); j++) 
			pSource[i] = pSource[i] ^ m_szKey3[j];
		for (j = 5; j >= 0; j--) 
			pSource[i] = pSource[i] ^ m_szKey1[j];
	}

	memset (pTarget, 0, nSize);
	CBaseUtils::ConvertDataToBase64 (pSource, nLeng, pTarget, nSize);

	memset (m_szKey, 0, sizeof (m_szKey));
	strcpy (m_szKey, pTarget);
	memset (m_wzKey, 0, sizeof (m_wzKey));
#ifdef _OS_WIN32
	MultiByteToWideChar (CP_ACP, 0, (char *)pTarget, -1, m_wzKey, sizeof (m_wzKey));
#else
	strcpy ((char *)m_wzKey, (char *)pTarget);
#endif // _OS_WIN32

	delete []pSource;
	delete []pTarget;

	_tcscpy (m_szPW, pPassWord);

	return true;
}

bool CBaseKey::IsKeyFile (unsigned char * pBuff, int nSize)
{
	if (nSize < YYKEY_LEN)
		return false;

	if (!strncmp ((char *)pBuff, YYKEY_TEXT, YYKEY_LEN))
		return true;
	else
		return false;
}

bool CBaseKey::EncryptData (unsigned char * pBuff, int nSize)
{
	int nKeyLen = strlen (m_szKey);
	if (nKeyLen <= 0)
		return true;

	for (int i = 0; i < nSize; i++)
	{
		for (int j = 0; j < nKeyLen; j+=3)
		{
			pBuff[i] = pBuff[i] ^ m_szKey[j];
		}
	}
	return true;
}

bool CBaseKey::DecryptData (unsigned char * pBuff, int nSize)
{
	int nKeyLen = strlen (m_szKey);
	int i, j;
	for (i = 0; i < nSize; i++)
	{
		for (j = 0; j < nKeyLen; j+=3)
		{
			pBuff[i] = pBuff[i] ^ m_szKey[j];
		}
	}
	return true;
}





bool CBaseKey::CreateText (TCHAR * pBuff, int nSize)
{
	unsigned char * pSource = new unsigned char[nSize];
	char *			pTarget = new char[nSize];
	unsigned char * pData = (unsigned char *)pBuff;
	int				nLeng = _tcslen (pBuff) * sizeof (TCHAR);

	int i = 0;
	int j = 0;

	memset (pSource, 0, nSize);
	for (i = 0; i <nLeng; i++) {
		for (j = 5; j < strlen (m_szKey3); j++) 
			pSource[i] = pData[i] ^ m_szKey3[j];
		for (j = 5; j >= 0; j--) 
			pSource[i] = pData[i] ^ m_szKey1[j];
	}

	memset (pTarget, 0, nSize);
	CBaseUtils::ConvertDataToBase64 (pSource, nLeng, pTarget, nSize);

	memset (pBuff, 0, nSize);
#ifdef _OS_WIN32
	MultiByteToWideChar (CP_ACP, 0, (char *)pTarget, -1, pBuff, nSize);
#else
	strcpy ((char *)pBuff, (char *)pTarget);
#endif // _OS_WIN32

	delete []pSource;
	delete []pTarget;

	return true;
}

bool CBaseKey::RestoreText (TCHAR * pBuff, int nSize)
{
	unsigned char * pSource = new unsigned char[nSize];
	char *			pTarget = new char[nSize];
	unsigned char * pData = (unsigned char *)pBuff;
	int				nLeng = _tcslen (pBuff) * sizeof (TCHAR);

	int i = 0;
	int j = 0;

	memset (pSource, 0, nSize);
	memset (pTarget, 0, nSize);

#ifdef _OS_WIN32
	WideCharToMultiByte (CP_ACP, 0, pBuff, -1, pTarget, nSize, NULL, NULL);
#else
	strcpy ((char *)pTarget, pBuff);
#endif // _OS_WIN32

	memset (pSource, 0, nSize);
	nLeng = CBaseUtils::ConvertBase64ToData (pTarget, pSource, nSize);

	memset (pBuff, 0, nSize);
	for (i = 0; i <nLeng; i++) 
	{
		pData[i] = pSource[i];
		for (j = 5; j < strlen (m_szKey3); j++) 
			pData[i] = pData[i] ^ m_szKey3[j];
		for (j = 5; j >= 0; j--) 
			pData[i] = pData[i] ^ m_szKey1[j];
	}

	delete []pSource;
	delete []pTarget;

	return true;
}

