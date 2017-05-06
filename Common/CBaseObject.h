/*******************************************************************************
	File:		CBaseObject.h

	Contains:	the base class of all objects.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/

#ifndef __CBaseObject_H__
#define __CBaseObject_H__

#ifdef _OS_WIN32
#include "windows.h"
#include "tchar.h"
#endif // _OS_WIN32

#include "stdio.h"
#include "string.h"
#include "yyData.h"

#ifdef WIN_CE
#define _TFUNC _T
#else
#define _TFUNC 
#endif //WIN_CE

#pragma warning (disable : 4996)

#define	YY_DEL_P(p)   \
	if (p != NULL) {  \
		delete p;     \
		p = NULL; }   \

#define	YY_DEL_A(a)   \
	if (a != NULL) {  \
		delete []a;   \
		a = NULL; }   \

#define	YY_REL_P(a)   \
	if (a != NULL) {  \
		a->Release(); \
		a = NULL; }   \

class CBaseObject
{
public:
	CBaseObject(void);
	virtual ~CBaseObject(void);

protected:
	virtual void	SetObjectName (const char * pObjName);

protected:
	char			m_szObjName[64];
	const char *	m_pClassFileName;
	long long		m_llDbgTime;

public:
	static int	g_ObjectNum;

};

#endif // __CBaseObject_H__
