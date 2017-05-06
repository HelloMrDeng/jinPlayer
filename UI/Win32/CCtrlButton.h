/*******************************************************************************
	File:		CCtrlButton.h

	Contains:	The button control header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-28		Fenger			Create file

*******************************************************************************/
#ifndef __CCtrlButton_H__
#define __CCtrlButton_H__
#include "windows.h"

#include "CCtrlBase.h"

class CCtrlButton : public CCtrlBase
{
public:
	CCtrlButton(TCHAR * pPath);
	virtual ~CCtrlButton(void);

protected:

};

#endif // __CCtrlButton_H__
