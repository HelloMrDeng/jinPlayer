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
#include "CMediaCodecRnd.h"

#include "yyMediaEng.h"
#include "CBaseObject.h"

typedef struct {
    uint32_t    width;
    uint32_t    height;
    uint32_t    stride;
    int32_t     format;
    uint32_t    flags; // 0 for now
} AndroidBitmapInfo;

typedef int (* bitmap_getInfo)(JNIEnv* env, jobject jbitmap, AndroidBitmapInfo* info);
typedef int (* bitmap_lockPixels)(JNIEnv* env, jobject jbitmap, void** addrPtr);
typedef int (* bitmap_unlockPixels)(JNIEnv* env, jobject jbitmap);

class CNDKPlayer : public CBaseObject
{	
public:
	CNDKPlayer();
	virtual ~CNDKPlayer();
	
	virtual int		Init(JavaVM * jvm, JNIEnv * env, jclass clsPlayer, jobject objPlayer, char * pPath);	
	virtual int		Uninit(JNIEnv* env);
	
	virtual int		GetThumb (JNIEnv* env, jobject obj, const char * pFile, int nWidth, int nHeight, jobject objBitmap);
	
	virtual int		SetView(JNIEnv* env, jobject pView);
	
	virtual int		Open (const char* szUrl);
	virtual int 	OpenExt (int nExtData, int nFlag);
	
	virtual int		Play (void);
	virtual int		Pause (void); 
	virtual int		Stop (void); 
		
	virtual long	GetDuration();

	virtual int		GetPos (int * pCurPos);
	virtual int		SetPos (int nCurPos);	
	
	virtual int 	ReadVideoBuff (JNIEnv* env, jobject obj, jbyteArray data);
	virtual int 	ReadAudioBuff (JNIEnv* env, jobject obj, jbyteArray data);
	
	virtual int		GetParam (JNIEnv* env, int nID, jobject pValue);
	virtual int		SetParam (JNIEnv* env, int nID, int nParam, jobject pValue);

	virtual void	UpdateVideoSize (JNIEnv* env, YY_VIDEO_FORMAT * pFmtVideo);
	virtual void	UpdateAudioInfo (JNIEnv* env, int nSampleRate, int nChannels);	
	
public:
	static	void 	NotifyEvent (void * pUserData, int nID, void * pV1);
	static 	int	 	MediaDataCB (void * pUser, YY_BUFFER * pData);
	
protected:
	virtual void	HandleEvent (int nID, void * pV1);
	virtual int		RenderVideo (YY_BUFFER * pData);
	virtual int		RenderAudio (YY_BUFFER * pData);
	virtual int		RenderSubTT (YY_BUFFER * pData);
	

	virtual void 	FadeIn (void * pData, int nSize);
	virtual int		LoadBmpFunc (void);
	
protected:
	JavaVM *			m_pjVM;
	jclass     			m_pjCls;
	jobject				m_pjObj;
	jmethodID			m_fPostEvent;
	jmethodID			m_fPushAudio;
	jmethodID			m_fPushVideo;
	jmethodID			m_fPushSubTT;
	jmethodID			m_fPushTTByte;
	bool				m_bEventDone;
		
	jobject				m_pView;	
	CNativeWndRender *	m_pRndWnd;
	CMediaCodecRnd *	m_RndCodec;

	void *				m_hDllBase;
	void *				m_hDllMedia;
	YYMEDIAINIT			m_fInit;
	YYMEDIACREATE		m_fCreate;
	YYMEDIADESTROY		m_fDestroy;
	YYMEDIAUNINIT		m_fUninit;
	void *				m_hMediaEng;	
	YYM_Player *		m_pPlayer;	
	
	YY_DATACB			m_cbData;
	
	JNIEnv *			m_pEnvAudio;
	YY_BUFFER_CONVERT	m_dataConvertAudio;	
	YY_AUDIO_FORMAT		m_fmtAudio;	
	jbyteArray			m_pDataAudio;
	int					m_nBuffSize;
	int					m_nDataSize;
	YY_BUFFER 			m_bufAudioRnd;	
	int					m_nAudioRndCount;
	int					m_nLatency;
	
	YY_VIDEO_FORMAT		m_fmtVideo;	
	YY_VIDEO_BUFF		m_bufVideoWnd;
	YY_BUFFER 			m_bufVideoRnd;
	YY_BUFFER_CONVERT	m_dataConvertVideo;		
		
	jobject				m_jBmpVideo;	
	bool				m_bDrawBmp;
	AndroidBitmapInfo	m_bmpInfo;
	bool				m_bBmpShow;
	int					m_nBmpTime;
	
	int					m_nVideoRndCount;
	int					m_nVideoRndATime;
	int					m_nVideoLastSysTime;
	int					m_nVideoLastBufTime;	
	int					m_nVideoDispColor;
	
	int					m_nEnbSubTT;
	JNIEnv *			m_pEnvSubTT;
		
	// Android bitmap render
	void*					m_hGraphics;
	bitmap_getInfo 			m_fGetInfo;
	bitmap_lockPixels 		m_fLockPixels;
	bitmap_unlockPixels		m_fUnlockPixels;	
	
	YYINFO_Thumbnail		m_sThumb;
};

#endif //__CNDKPlayer_H__
