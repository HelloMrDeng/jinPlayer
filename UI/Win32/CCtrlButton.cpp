/*******************************************************************************
	File:		CCtrlButton.cpp

	Contains:	The button control implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-28		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "CCtrlButton.h"

#include "USystemFunc.h"
#include "UBitmapFunc.h"

#include "yyLog.h"

#pragma warning (disable : 4996)

CCtrlButton::CCtrlButton(TCHAR * pPath)
	: CCtrlBase (pPath)
{
	SetObjectName ("CCtrlButton");
}

CCtrlButton::~CCtrlButton(void)
{
}
