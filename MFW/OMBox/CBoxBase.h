/*******************************************************************************
	File:		CBoxBase.h

	Contains:	the base box header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-29		Fenger			Create file

*******************************************************************************/
#ifndef __CBoxBase_H__
#define __CBoxBase_H__

#include "yyData.h"

#include "CBaseObject.h"
#include "CMutexLock.h"
#include "CBaseClock.h"

typedef enum  {
	OMB_TYPE_BASE		= 0,
	OMB_TYPE_SOURCE		= 10,
	OMB_TYPE_FILTER		= 20,
	OMB_TYPE_RENDER		= 30,
	OMB_TYPE_RND_EXT	= 31,
	OMB_TYPE_MAX		= 0X7FFFFFFF
} OMBOX_TYPE;

typedef enum  {
	OMB_STATUS_INIT		= 0,
	OMB_STATUS_RUN		= 1,
	OMB_STATUS_PAUSE	= 2,
	OMB_STATUS_STOP		= 3,
	OMB_STATUS_MAX		= 0X7FFFFFFF
} OMBOX_STATUS;

class CBoxBase : public CBaseObject
{
public:
	CBoxBase(void * hInst);
	virtual ~CBoxBase(void);

	virtual void			SetNotifyFunc (YYMediaNotifyEvent pFunc, void * pUserData);

	virtual int				SetDisplay (void * hView, RECT * pRect);
	virtual int				SetDecMode (int nMode);
	virtual int				SetSource (CBoxBase * pSource);
	virtual CBoxBase *		GetSource (void) {return m_pBoxSource;}

	virtual int				GetStreamCount (YYMediaType nType);
	virtual int				GetStreamPlay (YYMediaType nType);
	virtual int				SetStreamPlay (YYMediaType nType, int nIndex);
	virtual int				GetDuration (void);

	virtual int				Start (void);
	virtual int				Pause (void);
	virtual int				Stop (void);

	virtual int				ReadBuffer (YY_BUFFER * pBuffer, bool bWait);
	virtual int				RendBuffer (YY_BUFFER * pBuffer, bool bRend);

	virtual int				Convert (YY_BUFFER * pInBuff, YY_BUFFER * pOutBuff, RECT * pZoom);

	virtual int				SetPos (int nPos, bool bSeek);
	virtual int				SetSeekMode (int nSeekMode);

	virtual void			SetClock (CBaseClock * pClock);
	virtual CBaseClock *	GetClock (void);
	virtual char *			GetName (void) {return m_szBoxName;}
	virtual OMBOX_TYPE		GetType (void) {return m_nBoxType;}

	virtual int 			SetParam (int nID, void * pParam);
	virtual int				GetParam (int nID, void * pParam);

	virtual YY_AUDIO_FORMAT *	GetAudioFormat (void);
	virtual YY_VIDEO_FORMAT *	GetVideoFormat (void);

protected:
	void *				m_hInst;
	char				m_szBoxName[32];
	OMBOX_TYPE			m_nBoxType;
	OMBOX_STATUS		m_nStatus;
	YYMediaNotifyEvent	m_pNotifyFunc;
	void *				m_pUserData;
	CBaseClock *		m_pClock;
	void *				m_hView;
	RECT				m_rcView;
	int					m_nDecMode;
	CBoxBase *			m_pBoxSource;

	int					m_nAudioStreamNum;
	int					m_nVideoStreamNum;
	int					m_nAudioStreamPlay;
	int					m_nVideoStreamPlay;

	YY_AUDIO_FORMAT *	m_pFmtAudio;
	YY_VIDEO_FORMAT *	m_pFmtVideo;

	YY_BUFFER *			m_pBaseBuffer;
	YY_BUFFER *			m_pCurrBuffer;

	long long			m_llSeekPos;
	int					m_nSeekMode;

	long long			m_llDbgLastTime;
};

#endif // __CBoxBase_H__
