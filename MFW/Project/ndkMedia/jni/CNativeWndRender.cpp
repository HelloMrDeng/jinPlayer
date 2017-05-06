#include "CNativeWndRender.h"

#include <dlfcn.h>
#include <sys/system_properties.h>

#include "yyLog.h"

CNativeWndRender::CNativeWndRender()
	: CBaseObject ()
	, m_hAndroidDll (NULL)
	, m_pNativeWnd (NULL)		
	, m_fANativeWindow_fromSurface (NULL)
	, m_fANativeWindow_release (NULL)
	, m_fANativeWindow_setBuffersGeometry (NULL)
	, m_fANativeWindow_lock (NULL)
	, m_fANativeWindow_unlockAndPost (NULL)
	, m_nFormat (-1)
	, m_nWidth (0)
	, m_nHeight (0)
	, m_nDispColor (0)

{
	SetObjectName ("CNativeWndRender");
		
	m_hAndroidDll = dlopen("libandroid.so", RTLD_NOW);
	
	if (m_hAndroidDll != NULL) 
	{
		m_fANativeWindow_fromSurface = (ANativeWindow_fromSurface_t)dlsym(m_hAndroidDll, "ANativeWindow_fromSurface");
		m_fANativeWindow_release = (ANativeWindow_release_t)dlsym(m_hAndroidDll, "ANativeWindow_release");
		m_fANativeWindow_setBuffersGeometry = (ANativeWindow_setBuffersGeometry_t) dlsym(m_hAndroidDll, "ANativeWindow_setBuffersGeometry");
		m_fANativeWindow_lock = (ANativeWindow_lock_t) dlsym(m_hAndroidDll, "ANativeWindow_lock");
		m_fANativeWindow_unlockAndPost = (ANativeWindow_unlockAndPost_t)dlsym(m_hAndroidDll, "ANativeWindow_unlockAndPost");
		if (!m_fANativeWindow_fromSurface || !m_fANativeWindow_release || !m_fANativeWindow_setBuffersGeometry
			|| !m_fANativeWindow_lock || !m_fANativeWindow_unlockAndPost)
		{
			dlclose (m_hAndroidDll);
			m_hAndroidDll = NULL;
			return;
		}
	}
	else
	{
		YYLOGE ("The libandroid.so could not be loaded!");
	}
	
	memset (&m_buffer, 0, sizeof (ANativeWindow_Buffer));
	
	__system_property_get ("ro.build.version.release", m_szVer);	
	__system_property_get ("ro.board.platform", m_szBoardPlatform);		
//	YYLOGI ("Platform: %s", m_szBoardPlatform);
}

CNativeWndRender::~CNativeWndRender()
{
	if (m_pNativeWnd != NULL)
		m_fANativeWindow_release(m_pNativeWnd);
	if (m_hAndroidDll != NULL)
		dlclose (m_hAndroidDll);
}

bool CNativeWndRender::Init(JNIEnv* env, jobject surface) 
{
	if (m_fANativeWindow_fromSurface)
		m_pNativeWnd = (ANativeWindow *)m_fANativeWindow_fromSurface(env, surface);
	
	if (m_pNativeWnd == NULL)
		YYLOGE ("CNativeWndRender::init %p", m_pNativeWnd);
		
	m_nFormat = -1;
	
	return m_pNativeWnd != NULL;
}

void CNativeWndRender::SetColor(int nColorFormat)
{
	m_nDispColor = nColorFormat;
	m_nFormat = -1;
}
	
int CNativeWndRender::Lock(int nW, int nH, YY_VIDEO_BUFF * pBuff)
{
	if (!m_pNativeWnd) 
		return -1;
	
	int nRC = -1;
	if (m_nFormat == -1)
	{
		YYLOGI ("Version is %s. m_nDispColor is %d", m_szVer, m_nDispColor);
		YYLOGI ("Platform is %s", m_szBoardPlatform);
		
		if (strstr (m_szVer, "2.") == m_szVer)
			m_nFormat = WINDOW_FORMAT_RGBA_8888;
		else
			m_nFormat = WINDOW_FORMAT_YUV_YV12;	
		if (m_nFormat == WINDOW_FORMAT_YUV_YV12)
		{			
			//if (strlen (m_szBoardPlatform) <= 1)
			//	m_nFormat = WINDOW_FORMAT_RGBA_8888;
			if (!strcmp (m_szBoardPlatform, "exynos4"))
				m_nFormat = WINDOW_FORMAT_RGBA_8888;	
		}

		if (m_nDispColor > 0)
			m_nFormat = WINDOW_FORMAT_RGBA_8888;					
													
		nRC = m_fANativeWindow_setBuffersGeometry(m_pNativeWnd, nW, nH, m_nFormat);	
		if (nRC == 0)	
		{
			memset (&m_buffer, 0, sizeof (ANativeWindow_Buffer));			
			nRC = m_fANativeWindow_lock(m_pNativeWnd, (void*)&m_buffer, NULL);	
		}			
					
		if (nRC != 0 || m_buffer.bits == NULL) 	
		{		
			YYLOGW ("Loack window buffer failed!");	
			m_nFormat = WINDOW_FORMAT_RGBA_8888;
			nRC = m_fANativeWindow_setBuffersGeometry(m_pNativeWnd, nW, nH, m_nFormat);
			if (nRC != 0)
				return nRC;	
				
			nRC = m_fANativeWindow_lock(m_pNativeWnd, (void*)&m_buffer, NULL);					
		}	
		m_nWidth = nW;
		m_nHeight = nH;		
		YYLOGI ("Color Format %08X, Size % 6d X % 6d", m_nFormat, m_nWidth, m_nHeight);		
	}
	else if (nW != m_nWidth || nH != m_nHeight)
	{
		nRC = m_fANativeWindow_setBuffersGeometry(m_pNativeWnd, nW, nH, m_nFormat);		
		nRC = m_fANativeWindow_lock(m_pNativeWnd, (void*)&m_buffer, NULL);					
		m_nWidth = nW;
		m_nHeight = nH;		
		YYLOGI ("Color Format %08X, Size % 6d X % 6d", m_nFormat, m_nWidth, m_nHeight);			
	}
		
	if (nRC != 0)
		nRC = m_fANativeWindow_lock(m_pNativeWnd, (void*)&m_buffer, NULL);		
				
	if (nRC == 0) 
	{
//		YYLOGI ("Buffer Info: %d X %d,   %d  Buffer %p",  m_buffer.width, m_buffer.height, m_buffer.stride, m_buffer.bits);	
		if (m_nFormat == WINDOW_FORMAT_RGBA_8888)
		{	
			pBuff->nType = YY_VDT_ARGB;
			pBuff->pBuff[0] = (unsigned char*)m_buffer.bits;
		 	pBuff->nStride[0] = m_buffer.stride * 4;	
		}
		else if (m_nFormat == WINDOW_FORMAT_YUV_YV12)
		{	
			pBuff->nType = YY_VDT_YUV420_P;
			pBuff->pBuff[0] = (unsigned char*)m_buffer.bits;
			pBuff->pBuff[1] = (unsigned char*)m_buffer.bits + m_buffer.stride  * m_buffer.height;
			pBuff->pBuff[2] = (unsigned char*)m_buffer.bits + (m_buffer.stride  * m_buffer.height) * 5 / 4;						
		 	pBuff->nStride[0] = m_buffer.stride;	
		 	pBuff->nStride[1] = m_buffer.stride / 2;
		 	pBuff->nStride[2] = m_buffer.stride / 2;		 			 	
		}		
//		memset (m_buffer.bits, 100, m_buffer.stride * 200);
//		memset (m_buffer.bits + m_buffer.stride * 200, 200, m_buffer.stride * 200);		
	}	
	else
	{
		YYLOGI ("Lock window buffer failed! return %08X", nRC);
	}	
	return nRC;
}

int CNativeWndRender::Unlock()
{
	if (!m_pNativeWnd) 
		return -1;
		
	int nRC = m_fANativeWindow_unlockAndPost(m_pNativeWnd);
	
//	YYLOGI ("Unlock returnn %d", nRC);
		
	return nRC;
}
