/*******************************************************************************
	File:		CBmpFile.h

	Contains:	the bitmap file header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-27		Fenger			Create file

*******************************************************************************/
#ifndef __CBmpFile_H__
#define __CBmpFile_H__
#include "windows.h"
#include "CBaseObject.h"

#define MAX_BITMAPS 16

class CBmpFile : public CBaseObject
{
public:
	CBmpFile(void);
	virtual ~CBmpFile(void);

	virtual HBITMAP	GetBmpHandle (int nIndex, LPBYTE * ppBmpBuff);
	virtual int		ReadBmpFile (HDC hDC, TCHAR * pFile, int nNum);

	virtual int		GetWidth (void) {return m_lWidth;}
	virtual int		GetHeight (void) {return m_lHeight;}
	virtual int		GetNum (void) {return m_nBitmaps;}

protected:
	virtual	void	ReleaseData (void);

protected:
	int				m_nBitmaps;
	int				m_lWidth;
	int				m_lHeight;
	LPBYTE			m_pData[MAX_BITMAPS];
	HBITMAP			m_hBitmap[MAX_BITMAPS];
};

#endif // __CBmpFile_H__
