/*******************************************************************************
	File:		CVVVideoOMXDec.h

	Contains:	the vo video dec wrap header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-15		Fenger			Create file

*******************************************************************************/
#ifndef __CVVVideoOMXDec_H__
#define __CVVVideoOMXDec_H__

#include "CVVVideoDec.h"
#include "voIOMXDec.h"

class CVVVideoOMXDec : public CVVVideoDec
{
public:
	CVVVideoOMXDec(void * hInst);
	virtual ~CVVVideoOMXDec(void);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);
	
	virtual int		SetBuff (YY_BUFFER * pBuff);	
	virtual int		GetBuff (YY_BUFFER * pBuff);
	virtual int		RndBuff (YY_BUFFER * pBuff, bool bRend);
	virtual int		RndRest (void);
     
	virtual bool	IsWorking (void);
	 
protected:
	virtual int		SetHeadData (YY_VIDEO_FORMAT * pFmt);
	virtual int		SetDecParam (YY_VIDEO_FORMAT * pFmt);

	virtual int		SetInputData (VO_CODECBUFFER * pInData);
	
	virtual int		FillInfo (YY_BUFFER * pBuff);
	virtual int		RestoreInfo (YY_BUFFER * pBuff);
	
	VO_VIDEO_FRAMETYPE 	GetH264FrameType(unsigned char * buffer , int size);

protected:
	char			m_szVer[64];	
	char			m_szBP[64];	
	void *			m_hDllOMX;
	
	
	VO_CODECBUFFER	m_dataInput;
	bool			m_bEOS;
	bool			m_bCheckFrameType;

	int				m_nDgbIndex;
	long long		m_llDbgLastTime;
};

#endif // __CVVVideoOMXDec_H__
