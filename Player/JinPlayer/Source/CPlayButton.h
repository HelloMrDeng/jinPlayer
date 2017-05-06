/*******************************************************************************
	File:		CPlayButton.h

	Contains:	The play button header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-09		Fenger			Create file

*******************************************************************************/
#ifndef __CPlayButton_H__
#define __CPlayButton_H__

#include "CPlayCtrl.h"

class CPlayButton : public CPlayCtrl
{
public:
	CPlayButton(HINSTANCE hInst);
	virtual ~CPlayButton(void);

	virtual bool	OnDraw (HDC hDC, HBITMAP hBmp, LPBYTE pBuff, RECT * pRect);

};
#endif //__CPlayButton_H__