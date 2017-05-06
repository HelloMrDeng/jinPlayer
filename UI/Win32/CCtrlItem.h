/*******************************************************************************
	File:		CCtrlItem.h

	Contains:	The base control item header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-27		Fenger			Create file

*******************************************************************************/
#ifndef __CCtrlItem_H__
#define __CCtrlItem_H__

#include "windows.h"

class CCtrlItem
{
public:
	CCtrlItem(void);
	virtual ~CCtrlItem(void);

public:
	int			m_nID;
	RECT		m_rcPos;
	TCHAR		m_szBmpFile[1024];
	int			m_nBmpNum;
	COLORREF	m_clrTP;
};

#endif // __CCtrlItem_H__
