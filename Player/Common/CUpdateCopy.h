/*******************************************************************************
	File:		CUpdateCopy.h

	Contains:	The ext source header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-19		Fenger			Create file

*******************************************************************************/
#ifndef __CUpdateCopy_H__
#define __CUpdateCopy_H__
#include "windows.h"

class CUpdateCopy
{
public:
	CUpdateCopy(void * hInst);
	virtual ~CUpdateCopy(void);

	bool	UpdateFiles (void);

protected:
	bool	CopyFiles (TCHAR * pSource, TCHAR * pTarget);

protected:
	void *	m_hInst;

};

#endif // __CUpdateCopy_H__
