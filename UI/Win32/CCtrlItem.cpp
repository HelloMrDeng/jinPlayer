/*******************************************************************************
	File:		CCtrlItem.cpp

	Contains:	The control item implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-27		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <tchar.h>

#include "CCtrlItem.h"

#include "yyLog.h"

#pragma warning (disable : 4996)

CCtrlItem::CCtrlItem(void)
{
	m_nID = 0;
	SetRectEmpty (&m_rcPos);
	_tcscpy (m_szBmpFile, _T(""));
	m_nBmpNum = 1;
	m_clrTP = RGB (10, 10, 10);
}

CCtrlItem::~CCtrlItem(void)
{
}

