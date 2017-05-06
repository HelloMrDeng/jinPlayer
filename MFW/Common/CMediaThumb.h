/*******************************************************************************
	File:		CMediaThumb.h

	Contains:	the media engine header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-24		Fenger			Create file

*******************************************************************************/

#ifndef __CMediaThumb_H__
#define __CMediaThumb_H__

#include "CBaseSource.h"
#include "CBaseVideoDec.h"
#include "CFFMpegVideoRCC.h"

#include "yyThumbnail.h"

class CMediaThumb : public CBaseObject
{
public:
	CMediaThumb(void * hInst);
	virtual ~CMediaThumb(void);

	virtual HBITMAP	GetThumb (const TCHAR * pFile, YYINFO_Thumbnail * pThumbInfo);
	virtual bool	Cancel (void);

protected:
	virtual HBITMAP	CreateResBMP (int nW, int nH);
	virtual void	CloseMedia (void);
	virtual bool	IsBlackVideo (YY_BUFFER * pVideo);

protected:
	void *				m_hInst;
	YYINFO_Thumbnail	m_thumbInfo;
	bool				m_bCancel;

	CBaseSource *		m_pMedia;
	TCHAR				m_szFile[1024];
	YY_BUFFER 			m_buffMedia;

	CBaseVideoDec *		m_pDec;
	YY_BUFFER 			m_buffVideo;

	CFFMpegVideoRCC *	m_pVideoRCC;
	YY_VIDEO_BUFF		m_buffThumb;

	HDC					m_hMemDC;
	unsigned char *		m_pBmpInfo;
    unsigned char *		m_pBmpBuff;
	int					m_nPixelBits;
	int					m_nRndStride;
};

#endif // __CMediaThumb_H__
