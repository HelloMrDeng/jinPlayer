/*******************************************************************************
	File:		CNativeWndRender.h

	Contains:	yy NDK player native window render header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
#ifndef __CNativeWndRender_H__
#define __CNativeWndRender_H__

#include "./android/native_window_jni.h"
#include "./android/native_window.h"

#include "yyData.h"
#include "CBaseObject.h"


//    HAL_PIXEL_FORMAT_YV12   = 0x32315659, // YCrCb 4:2:0 Planar
//    HAL_PIXEL_FORMAT_YCbCr_422_SP       = 0x10, // NV16
//    HAL_PIXEL_FORMAT_YCrCb_420_SP       = 0x11, // NV21
//    HAL_PIXEL_FORMAT_YCbCr_422_I        = 0x14, // YUY2

#define	WINDOW_FORMAT_YUV_YV12		0x32315659
#define	WINDOW_FORMAT_YUV_NV16		0x10
#define	WINDOW_FORMAT_YUV_NV12		0x11
#define	WINDOW_FORMAT_YUV_YUY2		0x14
	
class CNativeWndRender : public CBaseObject
{
private:
	typedef void* (*ANativeWindow_fromSurface_t)		(JNIEnv *env, jobject surface);
  	typedef void (*ANativeWindow_release_t)				(void *window);
  	typedef int (*ANativeWindow_setBuffersGeometry_t)	(void *window, int width, int height, int format);
  	typedef int (* ANativeWindow_lock_t)				(void *window, void *outBuffer, void *inOutDirtyBounds);
  	typedef int (* ANativeWindow_unlockAndPost_t)		(void *window);
  
public:
	CNativeWndRender ();
	virtual ~CNativeWndRender (void);
	
	bool 		Init(JNIEnv* env, jobject surface);
	void		SetColor(int nColorFormat);
	int 		Lock(int nW, int nH, YY_VIDEO_BUFF * pBuff);	
	int 		Unlock();
	
	ANativeWindow *		GetWindow (void) {return m_pNativeWnd;}
	
protected:
	void * 								m_hAndroidDll;	
	ANativeWindow *						m_pNativeWnd;
	
	ANativeWindow_fromSurface_t			m_fANativeWindow_fromSurface;
	ANativeWindow_release_t 			m_fANativeWindow_release;
	ANativeWindow_setBuffersGeometry_t 	m_fANativeWindow_setBuffersGeometry;
	ANativeWindow_lock_t 				m_fANativeWindow_lock;
	ANativeWindow_unlockAndPost_t 		m_fANativeWindow_unlockAndPost;
	
	int									m_nFormat;
	int									m_nWidth;
	int									m_nHeight;
	ANativeWindow_Buffer 				m_buffer;
	
	int									m_nDispColor;
	
	char								m_szVer[64];	
	char								m_szBoardPlatform[64];
};

#endif //__CNativeWndRender_H__
