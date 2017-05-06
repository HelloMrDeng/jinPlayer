/*******************************************************************************
	File:		CNDKPlayer.h

	Contains:	yy NDK player header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
#ifndef __CNDKPlayer_H__
#define __CNDKPlayer_H__

#include <jni.h>

#include "CNativeWndRender.h"

#include "CBoxBase.h"
#include "CMutexLock.h"
#include "CNodeList.h"

#include "yyMediaEng.h"

int 	getIntegerValue(JNIEnv * env, jobject value);
void 	callbackEvent(JNIEnv* env, jobject obj, int nId, void * pParam);
int 	updateVideoSize(JNIEnv* env, jobject obj, int nWidth, int nHeight);
int 	updateAudioFormat(JNIEnv* env, jobject thiz, int nSampleRate, int nChannels);

class CNDKPlayer
{
public:
	typedef struct YYNDK_EVENT
	{
		int		nID;
		void *	pV1;
		void *	pV2;
	} YYNDK_EVENT;
	
public:
	CNDKPlayer();
	virtual ~CNDKPlayer();
	
	virtual int		Init(char * pPath);
	virtual int		Uninit();
	
	virtual int		SetView(JNIEnv* env, jobject pView);
	virtual jobject	GetView();	
	
	virtual int		Open (const char* szUrl);
	virtual int 	OpenExt (int nExtData, int nFlag);
	virtual int		GetOpenStatus (void) {return m_nOpenStatus;}
	
	virtual int		Play (void);
	virtual int		Pause (void); 
	virtual int		Stop (void); 
		
	virtual long	GetDuration();

	virtual int		GetPos (int * pCurPos);
	virtual int		SetPos (int nCurPos);	

	virtual int		GetVideoData(JNIEnv* env, jobject obj, unsigned char * pData, int nStride);
	virtual int		GetVideoSize (int * nWidth, int * nHeight);
	
	virtual int		GetAudioData(JNIEnv* env, jobject obj, unsigned char ** ppData, int nSize);
	virtual int		GetAudioFormat(int* nSampleRate,int* nChannels);

	virtual int		GetEventStatus (JNIEnv* env, jobject obj);
	
	virtual int		GetParam (JNIEnv* env, int nID, jobject pValue);
	virtual int		SetParam (JNIEnv* env, int nID, jobject pValue);

	void	SetJavaVM(JavaVM  * jvm, jobject envobj);
	void* 	GetJavaObj();
	
public:
	static	void NotifyEvent (void * pUserData, int nID, void * pV1);
	
	int 		jniEventCallBack (int nId, void * pParam);	
	
protected:
	virtual int		FillInfo (void);
	
protected:
	JavaVM *			m_pjVM;
	jobject				m_pjObj;
	
	jobject				m_pView;	
	CNativeWndRender *	m_pRndWnd;

	void *				m_hDllBase;
	void *				m_hDllMedia;
	YYMEDIAINIT			m_fInit;
	YYMEDIACREATE		m_fCreate;
	YYMEDIADESTROY		m_fDestroy;
	YYMEDIAUNINIT		m_fUninit;
	void *				m_hMediaEng;	
	YYM_Player *		m_pPlayer;	
	
	CBoxBase *			m_pBoxVideo;
	YY_VIDEO_FORMAT		m_fmtVideo;		
	YY_BUFFER			m_bufVideo;
	
	YY_VIDEO_BUFF		m_bufVideoRnd;
	YY_BUFFER			m_bufVideoCvt;
	
	int					m_nVideoRndCount;
	int					m_nVideoRndATime;
	int					m_nVideoLastSysTime;
	int					m_nVideoLastBufTime;
			
	CBoxBase *			m_pBoxAudio;
	YY_AUDIO_FORMAT		m_fmtAudio;		
	YY_BUFFER			m_bufAudio;
	YY_BUFFER			m_bufAudioCvt;	
	int					m_nAudioOffsetTime;
	
	
	CMutexLock					m_mtEvent;
	CObjectList<YYNDK_EVENT>	m_lstFreeEvent;
	CObjectList<YYNDK_EVENT>	m_lstFullEvent;	
	
	int							m_nOpenStatus;
	bool						m_bEOS;
	
	bool						m_bAudioEOS;
	bool						m_bVideoEOS;
};

#endif //__CNDKPlayer_H__
