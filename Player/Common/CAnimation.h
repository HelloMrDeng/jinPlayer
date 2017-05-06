/*******************************************************************************
	File:		CAnimation.h

	Contains:	The player ui animation header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CAnimation_H__
#define __CAnimation_H__

#include "CBaseObject.h"

typedef enum {
	ANMT_None		= 0,
	ANMT_Shrink		= 1,
	ANMT_Expand		= 2,
	ANMT_DoorOpen	= 3,
	ANMT_DoorClose	= 4,
	ANMT_Transition	= 5,
}ANMT_TYPE;

class CAnimation : public CBaseObject
{
public:
	CAnimation(HINSTANCE hInst);
	virtual ~CAnimation(void);

	virtual bool SetBackBmp (HBITMAP hBmp, LPBYTE pBuff, RECT * pRect);
	virtual bool SetForeBmp (HBITMAP hBmp, LPBYTE pBuff, RECT * pRect, int nTime);
	virtual bool SetRndWnd (HWND hWnd, RECT * pRCSrc, RECT * pRCTgt);
	virtual bool Show (ANMT_TYPE nType);

protected:
	virtual bool Expand (void);
	virtual bool Shrink (void);
	virtual bool DoorOpen (void);
	virtual bool DoorClose (void);
	virtual bool Transition (void);
	virtual bool ShowNone (void);

protected:
	HINSTANCE		m_hInst;
	HBITMAP			m_hBmpBack;
	LPBYTE			m_pBuffBack;
	RECT			m_rcBmpBack;
	HBITMAP			m_hBmpFore;
	LPBYTE			m_pBuffFore;
	RECT			m_rcBmpFore;
	int				m_nTime;
	HBITMAP			m_hBmpNew;
	HWND			m_hWnd;
	RECT			m_rcRndSrc;
	RECT			m_rcRndTgt;
};
#endif //__CAnimation_H__