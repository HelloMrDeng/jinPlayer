/*******************************************************************************
	File:		CMediaCodecRnd.h

	Contains:	yy NDK player native window render header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-09-23		Fenger			Create file

*******************************************************************************/
#ifndef __CMediaCodecRnd_H__
#define __CMediaCodecRnd_H__
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <jni.h>

#include "yyData.h"
#include "CBaseObject.h"

#include "yyMediaPlayer.h"

	
class CMediaCodecRnd : public CBaseObject
{  
public:
	CMediaCodecRnd (YYM_Player * pPlayer);
	virtual ~CMediaCodecRnd (void);
	
	virtual int 	ReadVideoBuff (JNIEnv* env, jobject obj, jbyteArray data);
	virtual int 	ReadAudioBuff (JNIEnv* env, jobject obj, jbyteArray data);

 
protected:
	YYM_Player *	m_pPlayer;

	YY_BUFFER		m_bufVideo;
	YY_BUFFER		m_bufAudio;
};

#endif //__CMediaCodecRnd_H__
