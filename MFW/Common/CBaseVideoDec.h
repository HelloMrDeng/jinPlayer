/*******************************************************************************
	File:		CBaseVideoDec.h

	Contains:	The Video dec header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseVideoDec_H__
#define __CBaseVideoDec_H__

#include <libavformat/avformat.h>

#include "CBaseObject.h"
#include "CNodeList.h"
#include "CMutexLock.h"

#define	YY_FCC_HVC1		0x31637668
#define	YY_FCC_HEV1		0x31766568
#define	YY_FCC_HAVC		0x31637661
#define	YY_FCC_AVC1		0x31637661


class CBaseVideoDec : public CBaseObject
{
public:
	typedef struct
	{
		long long		llTime;
		long long		llDelay;
		int				nFlag;
		int				nValue;
		unsigned char *	pBuff;
		unsigned int	uSize;
	} YYVDEC_INFO;

public:
	CBaseVideoDec(void * hInst);
	virtual ~CBaseVideoDec(void);

	virtual int		SetDisplay (void * hView, RECT * pRect);

	virtual int		Init (YY_VIDEO_FORMAT * pFmt);
	virtual int		Uninit (void);

	virtual int		SetBuff (YY_BUFFER * pBuff);
	virtual int		GetBuff (YY_BUFFER * pBuff);
	virtual int		RndBuff (YY_BUFFER * pBuff, bool bRend);
	virtual int		RndRest (void);

	virtual int		GetBuffNum (void) {return m_nBuffNum;}
	virtual bool	IsWorking (void) {return false;}

	virtual int		Start (void);
	virtual int		Pause (void);
	virtual int		Stop (void);
	virtual int		Flush (void);

	virtual YY_VIDEO_FORMAT *	GetVideoFormat (void) {return &m_fmtVideo;}

	virtual bool	ConvertHEVCHeadData (unsigned char * pHeadData, int nHeadSize);
	virtual bool	ConvertH264HeadData (unsigned char * pHeadData, int nHeadSize);
	virtual bool	ConvertVideoData (unsigned char * pData, int nSize);
	virtual int		GetHeadData (unsigned char ** ppHead) {*ppHead = m_pHeadData; return m_nHeadSize;}
	virtual int		GetVideoData (unsigned char ** ppData) {*ppData = m_pVideoData; return m_uVideoSize;}

protected:
	virtual int		ReleaseInfoList (void);
	virtual int		ResetInfoList (void);
	virtual int		FillInfo (YY_BUFFER * pBuff);
	virtual int		RestoreInfo (YY_BUFFER * pBuff);

protected:
	void *				m_hInst;
	void *				m_hView;
	RECT				m_rcView;
	YY_BUFFER *			m_pInBuff;
	YY_VIDEO_FORMAT		m_fmtVideo;
	YY_VIDEO_BUFF		m_bufVideo;
	AVPacket			m_pktData;

	CMutexLock			m_mtBuffer;
	unsigned int		m_uBuffFlag;
	unsigned int		m_uCodecTag;

	bool				m_bWaitKeyFrame;
	int					m_nVdoBuffSize;
	bool				m_bDropFrame;
	int					m_nDecCount;

	int					m_nBuffNum;

	CMutexLock					m_mtInfo;
	CObjectList<YYVDEC_INFO>	m_lstFree;
	CObjectList<YYVDEC_INFO>	m_lstFull;

	long long					m_llLastTime;
	long long					m_llFirstTime;
	long long					m_llFrameTime;


	// For HEVC parameter
	int					m_uNalLen;
	int					m_uFrameSize;
	int					m_uNalWord;
	unsigned char *		m_pVideoData;
	int					m_uVideoSize;

	unsigned char *		m_pHeadData;
	int					m_nHeadSize;

};

#endif // __CBaseVideoDec_H__
