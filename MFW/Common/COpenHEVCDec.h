/*******************************************************************************
	File:		COpenHEVCDec.h

	Contains:	the vo video dec wrap header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __COpenHEVCDec_H__
#define __COpenHEVCDec_H__

#include "CBaseVideoDec.h"
#include "yyHEVCDec.h"

class COpenHEVCDec : public CBaseVideoDec
{
public:
	COpenHEVCDec(void * hInst);
	virtual ~COpenHEVCDec(void);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		Flush (void);

	virtual int		SetBuff (YY_BUFFER * pBuff);
	virtual int		GetBuff (YY_BUFFER * pBuff);

protected:
	void *							m_hDll;
	OpenHevc_Handle					m_hDec;

	LIBOPENHEVCINIT					m_fInit;
	LIBOPENHEVCSTARTDECODER			m_fStart;
	LIBOPENHEVCDECODE				m_fDec;
	LIBOPENHEVCGETPICTUREINFO		m_fGetPicInfo;
	LIBOPENHEVCCOPYEXTRADATA		m_fCopyExtData;
	LIBOPENHEVCGETPICTURESIZE2		m_fGetPicSize2;
	LIBOPENHEVCGETOUTPUT			m_fGetOutput;
	LIBOPENHEVCGETOUTPUTCPY			m_fGetOutputCopy;
	LIBOPENHEVCSETCHECKMD5			m_fSetCheckMD5;
	LIBOPENHEVCSETDEBUGMODE			m_fSetDebugMode;
	LIBOPENHEVCSETTEMPORALLAYER_ID	m_fSetTempLayer;
	LIBOPENHEVCSETNOCROPPING		m_fSetNoCrop;
	LIBOPENHEVCSETACTIVEDECODERS	m_fSetActiveDec;
	LIBOPENHEVCCLOSE				m_fClose;
	LIBOPENHEVCFLUSH				m_fFlush;
	LIBOPENHEVCVERSION				m_fGetVer;

	OpenHevc_Frame					m_frmVideo;
	OpenHevc_Frame_cpy				m_frmCopy;

	long long						m_llNewPos;
};

#endif // __COpenHEVCDec_H__
