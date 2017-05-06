/*******************************************************************************
	File:		CListRes.h

	Contains:	The list resourc header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#ifndef __CListRes_H__
#define __CListRes_H__

#include "CBaseObject.h"

class CListRes : public CBaseObject
{
public:
	CListRes(HINSTANCE hInst);
	virtual ~CListRes(void);

protected:
	HINSTANCE		m_hInst;

public:
	HBITMAP			m_hBmpExit;
	HBITMAP			m_hBmpHome;
	HBITMAP			m_hBmpFolder;
	HBITMAP			m_hBmpVideo;
	HBITMAP			m_hBmpAudio;
	HBITMAP			m_hBmpImage;
	HBITMAP			m_hBmpNewFile;
	HBITMAP			m_hBmpNewFolder;
};
#endif //__CListRes_H__