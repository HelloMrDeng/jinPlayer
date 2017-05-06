#include "CMediaCodecRnd.h"

#include <dlfcn.h>
#include <sys/system_properties.h>

#include "UJNIFunc.h"
#include "yyLog.h"

#define JAVA_VIDEO_FALG		"m_nVideoFlag"
#define JAVA_VIDEO_TIME		"m_lVideoTime"
#define JAVA_VIDEO_WIDTH	"m_nVideoWidth"
#define JAVA_VIDEO_HEIGHT	"m_nVideoHeight"

CMediaCodecRnd::CMediaCodecRnd(YYM_Player * pPlayer)
	: CBaseObject ()
	, m_pPlayer (pPlayer)
{
	SetObjectName ("CMediaCodecRnd");

	memset (&m_bufVideo, 0, sizeof (m_bufVideo));
	m_bufVideo.nType = YY_MEDIA_Video;

	memset (&m_bufAudio, 0, sizeof (m_bufAudio));
	m_bufAudio.nType = YY_MEDIA_Audio;
}

CMediaCodecRnd::~CMediaCodecRnd()
{
}

int CMediaCodecRnd::ReadVideoBuff (JNIEnv* env, jobject obj, jbyteArray data)
{
	int 			nRC = 0;
	unsigned long 	lSize  = 0;
	jbyte* 			pBuff = 0;
	
	if (data == NULL)
		return -1;
	lSize = env->GetArrayLength(data);
	pBuff = (jbyte*)env->GetByteArrayElements(data, 0);
	if (pBuff == NULL)
		return -1;

	m_bufVideo.uFlag = 0;
	m_bufVideo.llTime = 0;
	m_bufVideo.uSize = 0;
	m_bufVideo.pBuff = (unsigned char *)pBuff;
	m_bufVideo.uBuffSize = lSize;
	nRC = m_pPlayer->GetParam (m_pPlayer, YYPLAY_PID_ReadData, &m_bufVideo);
	if (nRC != YY_ERR_NONE)
	{
		if (nRC == YY_ERR_FINISH)
			yyJNISetIntegerValue (env, obj, JAVA_VIDEO_FALG, YYBUFF_EOS);
				
		env->ReleaseByteArrayElements(data, pBuff, 0);	
		return -1;
	}
	if (m_bufVideo.uFlag & YYBUFF_NEW_FORMAT)
		yyJNISetIntegerValue (env, obj, JAVA_VIDEO_FALG, YYBUFF_NEW_FORMAT);
	if (m_bufVideo.uFlag & YYBUFF_EOS)
		yyJNISetIntegerValue (env, obj, JAVA_VIDEO_FALG, YYBUFF_EOS);
	if (m_bufVideo.uFlag & YYBUFF_NEW_POS)
		yyJNISetIntegerValue (env, obj, JAVA_VIDEO_FALG, YYBUFF_NEW_POS);
	if (m_bufVideo.uFlag & YYBUFF_KEY_FRAME)
		yyJNISetIntegerValue (env, obj, JAVA_VIDEO_FALG, YYBUFF_KEY_FRAME);
		
	if (m_bufVideo.uFlag & YYBUFF_NEW_FORMAT)
	{
		YY_VIDEO_FORMAT * pFmtVideo = (YY_VIDEO_FORMAT *)m_bufVideo.pFormat;
		if (pFmtVideo != NULL)
		{	
			YYLOGI ("Video size is % 8d X %d", pFmtVideo->nWidth, pFmtVideo->nHeight);
			yyJNISetIntegerValue (env, obj, JAVA_VIDEO_WIDTH, pFmtVideo->nWidth);
			yyJNISetIntegerValue (env, obj, JAVA_VIDEO_HEIGHT, pFmtVideo->nHeight);
		}
	}		
	
	yyJNISetIntegerValue (env, obj, JAVA_VIDEO_TIME, (int)m_bufVideo.llTime);
				
	env->ReleaseByteArrayElements(data, pBuff, 0);	

	return m_bufVideo.uSize;
}

int CMediaCodecRnd::ReadAudioBuff (JNIEnv* env, jobject obj, jbyteArray data)
{
	return 0;
}
