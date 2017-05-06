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

#include "CMultiPlayer.h"
#include "CLessonInfo.h"

class CNDKPlayer : public CMultiPlayer
{	
public:
	CNDKPlayer();
	virtual ~CNDKPlayer();
	
	virtual int		Init(JavaVM * jvm, JNIEnv * env, jclass clsPlayer, jobject objPlayer, char * pPath);	
	virtual int		Uninit(JNIEnv* env);
	
	virtual int		Open (const char* szUrl);	
	virtual int		GetParam (JNIEnv* env, int nID, jobject pValue);
	virtual int		SetParam (JNIEnv* env, int nID, int nParam, jobject pValue);
	
	virtual int		Run (void);	
	
	CLessonInfo *	GetLessonInfo (void) {return &m_lsnInfo;}
	virtual void	UpdateAudioInfo (JNIEnv* env, int nSampleRate, int nChannels);	

	
protected:
	virtual int		HandleEvent (int nID, void * pV1);
	virtual int		WriteAudio (void);	
	
protected:
	JavaVM *			m_pjVM;
	jclass     			m_pjCls;
	jobject				m_pjObj;
	jmethodID			m_fPostEvent;
	jmethodID			m_fPushAudio;
	
	JNIEnv *			m_pEnvAudio;
	jbyteArray			m_pDataAudio;
	int					m_nBuffSize;
	int					m_nDataSize;
	
	char				m_szPath[256];
	CLessonInfo 		m_lsnInfo;
	
	static	int			AudioPlayStart (void * pParam);
	static	int			AudioPlayStop (void * pParam);
	
};

#endif //__CNDKPlayer_H__
