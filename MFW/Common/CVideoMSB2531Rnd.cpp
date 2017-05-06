/*******************************************************************************
	File:		CVideoMSB2531Rnd.cpp

	Contains:	The base Video DDraw CD60 render implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CVideoMSB2531Rnd.h"

#include "Pkfuncs.h"

#include "cmmb_dll.h"
#include "columbus_pmu.h"
#include "USystemFunc.h"
#include "UYYDataFunc.h"

#include "yyMonitor.h"
#include "yyConfig.h"
#include "yyLog.h"

#define MSB_NEW_RENDER

#define YUVBUF_TTL_SIZE				0X300000

#define YUV420_BUF_SIZE				0xBC000*2//1*1024*1024
#define YUV422_BUF_SIZE				0xBC000*2//1*1024*1024

#define YUV422_SIZE					0xBC000//800*480*2==0xBB800//512*1024
#define YUV420_SIZE					0xBC000//800*480*2==0xBB800//512*1024

#define YUV422_BUF0_START			0x27D00000
#define YUV422_BUF1_START			(YUV422_BUF0_START+YUV422_SIZE)
#define YUV420_BUF0_START			(YUV422_BUF1_START+YUV422_SIZE)
#define YUV420_BUF1_START			(YUV420_BUF0_START+YUV420_SIZE)

#define COLUMBUS_BASE_REG_PMU_PA	(0x1F007E00)

void *				g_pPMU = NULL;
PREG_PMU_st			g_pPMURegs = NULL;
PHYSICAL_ADDRESS    sPMUPhysicalAddress = { COLUMBUS_BASE_REG_PMU_PA, 0 };
DWORD				g_dwReadPos = 0;

CVideoMSB2531Rnd::CVideoMSB2531Rnd(void * hInst)
	: CBaseVideoRnd (hInst)
	, m_hWnd (NULL)
	, m_pSubTT (NULL)
	, m_hAtscTv (NULL)
	, m_hAtscEvent (NULL)
	, m_dwColorKey (RGB (16, 8, 16))
	, m_bColorKeyEn (1)
	, m_eBufIdx (TYPE420BUF0)
	, m_bNeedUpdateDisp (true)
	, m_pYuv422Buf0 (NULL)
	, m_pYuv422Buf1 (NULL)
	, m_pYuv420Buf0 (NULL)
	, m_pYuv420Buf1 (NULL)
	, m_nVideoWidth (0)
	, m_nVideoHeight (0)
	, m_nYUVWidth (0)
	, m_nYUVHeight (0)
{
	SetObjectName ("CVideoMSB2531Rnd");
	SetRectEmpty (&m_rcDraw);
}

CVideoMSB2531Rnd::~CVideoMSB2531Rnd(void)
{
	Uninit ();
}

int CVideoMSB2531Rnd::SetDisplay (void * hView, RECT * pRect)
{
	CAutoLock lock (&m_mtDraw);
	m_hWnd = (HWND)hView;
	m_hView =hView;

	if (pRect == NULL)
		GetClientRect (m_hWnd, &m_rcView);
	else
		memcpy (&m_rcView, pRect, sizeof (RECT));

	UpdateRenderSize ();

	return YY_ERR_NONE;
}

int CVideoMSB2531Rnd::UpdateDisp (void)
{
	return YY_ERR_NONE;
}

int CVideoMSB2531Rnd::SetSubTTEng (void * pSubTTEng)
{
	m_pSubTTEng = pSubTTEng;

	m_pSubTT = (CSubtitleEngine *)m_pSubTTEng;

	return YY_ERR_NONE;
}

int CVideoMSB2531Rnd::Init (YY_VIDEO_FORMAT * pFmt)
{
	if (pFmt == NULL)
		return YY_ERR_ARG;
	if (m_hWnd == NULL)
		return YY_ERR_STATUS;

	yyDataCloneVideoFormat (&m_fmtVideo, pFmt);

	m_nVideoWidth = (m_fmtVideo.nWidth + 15) & 0XFFF0;
	m_nVideoHeight = (m_fmtVideo.nHeight + 1) & 0XFFFE;

	m_nYUVWidth = m_nVideoWidth;
	m_nYUVHeight = m_nVideoHeight;
#ifndef MSB_NEW_RENDER
	if (m_nYUVWidth > 800)
		m_nYUVWidth = 800;
	if (m_nYUVHeight > 480)
		m_nYUVHeight = 480;
#endif // MSB_NEW_RENDER

	UpdateRenderSize ();

	if (!CreateDevice ())
		return YY_ERR_FAILED;

	return YY_ERR_NONE;
}

int CVideoMSB2531Rnd::Uninit (void)
{
	ReleaseDevice ();

	return YY_ERR_NONE;
}

int CVideoMSB2531Rnd::Render (YY_BUFFER * pBuff)
{
	CBaseVideoRnd::Render (pBuff);
	if (pBuff->pBuff == NULL)
		return YY_ERR_ARG;

	if (!IsWindowVisible (m_hWnd))
		return YY_ERR_NONE;

	GetClientRect (m_hWnd, &m_rcWindow);
	if (m_rcWindow.right - m_rcWindow.left < m_rcView.right - m_rcView.left)
	{
		GetClientRect (m_hWnd, &m_rcView);
		UpdateRenderSize ();
	}

	CAutoLock lock (&m_mtDraw);
	if (m_hAtscTv == NULL)
		return YY_ERR_STATUS;

	YY_VIDEO_BUFF * pVideoBuff = NULL;
	if ((pBuff->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
	{
		AVFrame * pFrmVideo = (AVFrame *)pBuff->pBuff;

		m_bufVideo.pBuff[0] = pFrmVideo->data[0];
		m_bufVideo.pBuff[1] = pFrmVideo->data[1];
		m_bufVideo.pBuff[2] = pFrmVideo->data[2];

		m_bufVideo.nStride[0] = pFrmVideo->linesize[0];
		m_bufVideo.nStride[1] = pFrmVideo->linesize[1];
		m_bufVideo.nStride[2] = pFrmVideo->linesize[2];

		if (pFrmVideo->format == AV_PIX_FMT_YUV420P)
			m_bufVideo.nType = YY_VDT_YUV420_P;
		else if (pFrmVideo->format == AV_PIX_FMT_NV12)
			m_bufVideo.nType = YY_VDT_NV12;
		else if (pFrmVideo->format == AV_PIX_FMT_RGB24)
			m_bufVideo.nType = YY_VDT_RGB24;
		else
			m_bufVideo.nType = YY_VDT_MAX;
		pVideoBuff = &m_bufVideo;
	}
	else if ((pBuff->uFlag & YYBUFF_TYPE_VIDEO) == YYBUFF_TYPE_VIDEO)
	{
		pVideoBuff = (YY_VIDEO_BUFF *)pBuff->pBuff;
	}
	int		nDrawHeight = m_nYUVHeight;
	int		i = 0;
	LPBYTE	pVideo = pVideoBuff->pBuff[0];
	LPBYTE	lpYUV = (LPBYTE)m_pYuv422Buf0;
	if (m_eBufIdx == TYPE420BUF1)
#ifdef MSB_NEW_RENDER
		lpYUV = (LPBYTE)m_pYuv422Buf0 + m_nYUVWidth * m_nYUVHeight * 3 / 2;
#else
		lpYUV = (LPBYTE)m_pYuv422Buf1;
#endif // MSB_NEW_RENDER

	LPBYTE lpSurf = lpYUV;

	if (pVideoBuff->nType == YY_VDT_YUV420_P)
	{
		for (i = 0; i < nDrawHeight; i++)
			memcpy (lpSurf + m_nYUVWidth * i, pVideo + pVideoBuff->nStride[0] * i, m_nYUVWidth);

		lpSurf = lpYUV + m_nYUVWidth * m_nYUVHeight;
		pVideo = pVideoBuff->pBuff[1];
		for (i = 0; i < nDrawHeight / 2; i++)
			memcpy (lpSurf + m_nYUVWidth * i / 2, pVideo + pVideoBuff->nStride[2] * i, m_nYUVWidth / 2);
		lpSurf = lpYUV + (m_nYUVWidth * m_nYUVHeight) * 5 / 4;
		pVideo = pVideoBuff->pBuff[2];
		for (i = 0; i < nDrawHeight / 2; i++)
			memcpy (lpSurf + m_nYUVWidth * i / 2, pVideo + pVideoBuff->nStride[1] * i, m_nYUVWidth / 2);
	}
	else
	{
		pVideoBuff->nType = YY_VDT_YUV420_P;
		pVideoBuff->pBuff[0] = lpYUV;
		pVideoBuff->nStride[0] = m_nVideoWidth;
		pVideoBuff->pBuff[2] = lpYUV + m_nVideoWidth * m_nVideoHeight;
		pVideoBuff->pBuff[1] = lpYUV + (m_nVideoWidth * m_nVideoHeight) * 5 / 4;
		pVideoBuff->nStride[1] = m_nVideoWidth / 2;
		pVideoBuff->nStride[2] = m_nVideoWidth / 2;

		pVideoBuff->nWidth = m_nVideoWidth;
		pVideoBuff->nHeight = m_nVideoHeight;

		if (m_pVideoRCC == NULL)
		{
			m_pVideoRCC = new CFFMpegVideoRCC (m_hInst);
			if (m_pVideoRCC == NULL)
				return YY_ERR_MEMORY;
		}

		m_pVideoRCC->ConvertBuff (pBuff, &m_bufVideo);
	}

	m_bufLogo.nType = YY_VDT_YUV420_P;
	m_bufLogo.pBuff[0] = (unsigned char *)lpYUV;
	m_bufLogo.pBuff[1] = (unsigned char *)lpYUV + m_nYUVWidth * m_nYUVHeight;
	m_bufLogo.pBuff[2] = (unsigned char *)lpYUV + (m_nYUVWidth * m_nYUVHeight) * 5 / 4;
	m_bufLogo.nStride[0] = m_nYUVWidth;
	m_bufLogo.nStride[1] = m_nYUVWidth / 2;
	m_bufLogo.nStride[2] = m_nYUVWidth / 2;

	OverLogo ();

	if (m_bNeedUpdateDisp)
	{
		m_bNeedUpdateDisp = false;
		SetDispSize ();
	}

	if (m_nRndCount <= 1)
		InvalidateRect (m_hWnd, NULL, TRUE);

	SetEventData (m_hAtscEvent, m_eBufIdx);
	SetEvent (m_hAtscEvent);

	if (m_eBufIdx == TYPE420BUF1)
		m_eBufIdx = TYPE420BUF0;
	else
		m_eBufIdx = TYPE420BUF1;

	if (m_pSubTT != NULL)
	{
		HDC hDC = GetDC (m_hWnd);
		m_pSubTT->Draw (hDC, &m_rcDraw, pBuff->llTime, true);
		ReleaseDC (m_hWnd, hDC);
	}

	m_nRndCount++;

	return YY_ERR_NONE;
}

int CVideoMSB2531Rnd::EnableKeyColor (bool bEnable)
{
	if (bEnable)
	{
//		if (m_bColorKeyEn > 0)
//			return YY_ERR_NONE;
		m_bColorKeyEn = 1;
	}
	else
	{
//		if (m_bColorKeyEn == 0)
//			return YY_ERR_NONE;
		m_bColorKeyEn = 0;
	}
	SetColorKey ();
	return YY_ERR_NONE;
}

bool CVideoMSB2531Rnd::UpdateRenderSize (void)
{
	bool bRC = CBaseVideoRnd::UpdateRenderSize ();

	memcpy (&m_rcDest, &m_rcRender, sizeof (RECT));
	ClientToScreen(m_hWnd, (LPPOINT)&m_rcDest.left);
	ClientToScreen(m_hWnd, (LPPOINT)&m_rcDest.right);

	m_rcDest.left = m_rcDest.left & 0XFFFE;
	m_rcDest.top = m_rcDest.top & 0XFFFE;
	m_rcDest.right = m_rcDest.right & 0XFFFE;
	m_rcDest.bottom = m_rcDest.bottom & 0XFFFE;

	m_rcDraw.right = m_rcRender.right - m_rcRender.left;
	m_rcDraw.bottom = m_rcRender.bottom - m_rcRender.top;

	m_bNeedUpdateDisp = true;

	return bRC;
}

bool CVideoMSB2531Rnd::CreateDevice (void)
{
	ReleaseDevice ();

	m_hAtscEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("AtscNotify"));

	m_hAtscTv= CreateFile(
						_T("SCA0:"), 				//LPCTSTR lpFileName,
						GENERIC_READ|GENERIC_WRITE, //DWORD dwDesiredAccess,
						0,							//DWORD dwShareMode,
						NULL, 						//LPSECURITY_ATTRIBUTES lpSecurityAttributes,
						OPEN_EXISTING, 				//DWORD dwCreationDispostion,
						FILE_ATTRIBUTE_NORMAL, 		//DWORD dwFlagsAndAttributes,
						NULL);						//HANDLE hTemplateFile

	if(m_hAtscTv == INVALID_HANDLE_VALUE)
	{
		m_hAtscTv = NULL;
		return false;
	}
	
	DWORD	dwBufIn = 0;
	DWORD	dwBufOut = 0, dwActualOut = 0;
	BOOL bRC = DeviceIoControl(m_hAtscTv,
								IOCTL_JPD_SET_BUFF,
								NULL, 			
								0,
								&dwBufOut,
								sizeof(DWORD),
								&dwActualOut,
								NULL);
	if (bRC)
	{
#ifdef MSB_NEW_RENDER
		m_pYuv422Buf0 = GetVirtualAddr(dwBufOut, YUVBUF_TTL_SIZE);
#else
		m_pYuv422Buf0 = GetVirtualAddr(dwBufOut,YUV422_SIZE);
		m_pYuv422Buf1 = GetVirtualAddr(dwBufOut+YUV422_SIZE,YUV422_SIZE);
		m_pYuv420Buf0 = GetVirtualAddr(dwBufOut+YUV422_BUF_SIZE,YUV420_SIZE);
		m_pYuv420Buf1 = GetVirtualAddr(dwBufOut+YUV422_BUF_SIZE+YUV420_SIZE,YUV420_SIZE);
#endif // MSB_NEW_RENDER
	}
	else
	{
		m_pYuv422Buf0 = GetVirtualAddr(YUV422_BUF0_START, YUVBUF_TTL_SIZE);
//		m_pYuv422Buf0 = GetVirtualAddr(YUV422_BUF0_START,YUV422_SIZE);
//		m_pYuv422Buf1 = GetVirtualAddr(YUV422_BUF1_START,YUV422_SIZE);
//		m_pYuv420Buf0 = GetVirtualAddr(YUV420_BUF0_START,YUV420_SIZE);
//		m_pYuv420Buf1 = GetVirtualAddr(YUV420_BUF1_START,YUV420_SIZE);
	}
	
	g_pPMU = GetVirtualAddr(sPMUPhysicalAddress, sizeof(REG_PMU_st), FALSE);
	if (m_pYuv422Buf0 == NULL)// || m_pYuv422Buf1 == NULL || m_pYuv420Buf0 == NULL || m_pYuv420Buf1 == NULL)
		return false;

	SetSrcSize ();
	SetOutPath ();
	SetBuffIndex ();
	SetColorKey ();

	return true;
}

bool CVideoMSB2531Rnd::ReleaseDevice(void)
{
	if (m_hAtscEvent != NULL)
	{
		CloseHandle (m_hAtscEvent);
		m_hAtscEvent = NULL;
	}

	if (m_hAtscTv != NULL)
	{
		m_bColorKeyEn = 0;
		SetColorKey ();

		CloseHandle (m_hAtscTv);
		m_hAtscTv = NULL;
	}

	if(m_pYuv422Buf0 != NULL)
	{
		VirtualFree(m_pYuv422Buf0, 0, MEM_RELEASE);
		m_pYuv422Buf0 = NULL;
	}

	if(m_pYuv422Buf1!= NULL)
	{
		VirtualFree(m_pYuv422Buf1, 0, MEM_RELEASE);
		m_pYuv422Buf1 = NULL;
	}

	if(m_pYuv420Buf0!= NULL)
	{
		VirtualFree(m_pYuv420Buf0, 0, MEM_RELEASE);
		m_pYuv420Buf0 = NULL;
	}

	if(m_pYuv420Buf1!= NULL)
	{
		VirtualFree(m_pYuv420Buf1, 0, MEM_RELEASE);
		m_pYuv420Buf1 = NULL;
	}

	return TRUE;
}

BOOL CVideoMSB2531Rnd::SetSrcSize (void)
{
	if (m_hAtscTv == NULL)
		return FALSE;

	DWORD	dwBufIn = 0;
	DWORD	dwBufOut = 0, dwActualOut = 0;

	LPBYTE	lpSurf = (LPBYTE)m_pYuv422Buf0;
	if (m_eBufIdx == TYPE420BUF1)
#ifdef MSB_NEW_RENDER
		lpSurf = (LPBYTE)m_pYuv422Buf0 + m_nYUVWidth * m_nYUVHeight * 3 / 2;
#else		
		lpSurf = (LPBYTE)m_pYuv422Buf1;
#endif // MSB_NEW_RENDER

	memset (lpSurf, 0, m_nYUVHeight * m_nYUVWidth);
	lpSurf = lpSurf + m_nYUVWidth * m_nYUVHeight;
	memset (lpSurf, 127, m_nYUVHeight * m_nYUVWidth / 4);
	lpSurf = lpSurf + (m_nYUVWidth * m_nYUVHeight) / 4;
	memset (lpSurf, 127, m_nYUVHeight * m_nYUVWidth / 4);

	//1.high word save V and low word save H.
	dwBufIn = (m_nYUVHeight <<16) | (m_nYUVWidth);
	BOOL bRC = DeviceIoControl(m_hAtscTv,
								IOCTL_JPD_SCALER_SIZE,
								&dwBufIn,
								sizeof(DWORD),
								&dwBufOut,
								sizeof(DWORD),
								&dwActualOut,
								NULL);
	YYLOGI ("SetSrcSize: %d X %d Out: %08X, RC = %d", m_nYUVWidth, m_nYUVHeight, dwActualOut, bRC);
	return bRC;
}

BOOL CVideoMSB2531Rnd::SetOutPath (void)
{
	if (m_hAtscTv == NULL)
		return FALSE;

	DWORD	dwBufIn = 0;
	DWORD	dwBufOut = 0, dwActualOut = 0;

	BOOL bRC = DeviceIoControl(m_hAtscTv,
							IOCTL_JPD_SCALER_PATH,
							&dwBufIn,
							sizeof(DWORD),
							&dwBufOut,
							sizeof(DWORD),
							&dwActualOut,
							NULL);
	YYLOGI ("SetOutPath: %d X %d Out: %08X, RC = %d", m_nYUVWidth, m_nYUVHeight, dwActualOut, bRC);
	return bRC;
}

BOOL CVideoMSB2531Rnd::SetBuffIndex (void)
{
	if (m_hAtscTv == NULL)
		return FALSE;

	DWORD	dwBufIn = 0;
	DWORD	dwBufOut = 0, dwActualOut = 0;

	dwBufIn = m_eBufIdx;
	BOOL bRC = DeviceIoControl(m_hAtscTv,
							IOCTL_JPD_SW2SCALER,
							&dwBufIn,
							sizeof(DWORD),
							&dwBufOut,
							sizeof(DWORD),
							&dwActualOut,
							NULL);
	YYLOGI ("SetBuffIndex: %d X %d Out: %08X, RC = %d", m_nYUVWidth, m_nYUVHeight, dwActualOut, bRC);
	return bRC;
}

BOOL CVideoMSB2531Rnd::SetColorKey (void)
{
//	if (m_hAtscTv == NULL)
		return FALSE;

	DWORD	dwBufIn = 0;
	DWORD	dwBufOut = 0, dwActualOut = 0;

	COLOR_KEY_st	stColorKey;
	stColorKey.u32colorkey = m_dwColorKey;
	stColorKey.u8FlagEn = m_bColorKeyEn;
	BOOL bRC = DeviceIoControl(m_hAtscTv,
							IOCTL_JPD_COLOR_KEY,
							&stColorKey,
							sizeof(COLOR_KEY_st),
							&dwBufOut,
							sizeof(DWORD),
							&dwActualOut,
							NULL);
	YYLOGI ("SetColorKey: Color = %d - Flag: %d Out: %08X, RC = %d", 
					stColorKey.u32colorkey, stColorKey.u8FlagEn, dwActualOut, bRC);
	return bRC;
}

BOOL CVideoMSB2531Rnd::SetDispSize (void)
{
	if (m_hAtscTv == NULL)
		return FALSE;

	Scaler_SetWindow_st	pWinInfo;
	DWORD				dwBytesReturned;

	pWinInfo.SrcType = SCALER_SRC_BT656;
	pWinInfo.bInterace = FALSE;//TRUE;
	pWinInfo.bMEM_422 = TRUE;
	pWinInfo.u16SrcRect_X = 0;
	pWinInfo.u16SrcRect_Y = 0;
	pWinInfo.u16SrcRect_Width = m_nYUVWidth;
	pWinInfo.u16SrcRect_Height = m_nYUVHeight;

	//YUV data format
	pWinInfo.u16Src_Width = m_nYUVWidth;
	pWinInfo.u16Src_Height = m_nYUVHeight;

	pWinInfo.u16DispRect_X = 0;//m_rcDest.left;
	pWinInfo.u16DispRect_Y = 0;//m_rcDest.top;
	pWinInfo.u16DispRect_Width = 800;//m_rcDest.right - m_rcDest.left;
	pWinInfo.u16DispRect_Height = 480;//m_rcDest.bottom - m_rcDest.top;
//	pWinInfo.u16DispRect_Width = (GetSystemMetrics(SM_CXSCREEN)-pWinInfo.u16DispRect_X*2);//760;
//	pWinInfo.u16DispRect_Height = (GetSystemMetrics(SM_CYSCREEN)-pWinInfo.u16DispRect_Y*2);//460;

	BOOL bRC = DeviceIoControl (m_hAtscTv, IOCTL_SCALER_SETWINDOW_CMMB,
							&pWinInfo, sizeof(Scaler_SetWindow_st),
							NULL, 0, &dwBytesReturned, 0);

	YYLOGI ("SetDispSize: %d X %d Out: %d %d %d %d, RC = %d", m_nYUVWidth, m_nYUVHeight, 
			pWinInfo.u16DispRect_X, pWinInfo.u16DispRect_Y, 
			pWinInfo.u16DispRect_Width, pWinInfo.u16DispRect_Height, bRC);

	return bRC;
}

volatile LPVOID CVideoMSB2531Rnd::GetVirtualAddr(DWORD dwPhyBaseAddress, DWORD dwSize)
{
	volatile LPVOID pVirtual = NULL;
	VIRTUAL_COPY_EX_DATA vced;

	if(dwPhyBaseAddress & 0xFFF)
		return NULL;

	//vced.dwPhysAddr = dwPhyBaseAddress>>8;
    vced.dwPhysAddr = dwPhyBaseAddress;
	pVirtual = VirtualAlloc(0,dwSize,MEM_RESERVE,PAGE_NOACCESS);
	vced.pvDestMem = pVirtual;
	vced.dwSize = dwSize;
    vced.dwFlag = PAGE_READWRITE|PAGE_PHYSICAL|PAGE_NOCACHE;

	if(KernelIoControl(IOCTL_VIRTUAL_COPY_EX,&vced, sizeof(vced), NULL, NULL, NULL))
		return pVirtual;
	else
		return NULL;
}

volatile LPVOID CVideoMSB2531Rnd::GetVirtualAddr(PHYSICAL_ADDRESS pa, DWORD size, BOOL cacheEnable)
{
    VOID *pAddress = NULL;
    ULONGLONG sourcePA;
    U32 sourceSize, offset;
    //BOOL rc;
    VIRTUAL_COPY_EX_DATA vced;

    // Page align source and adjust size to compensate
    sourcePA = pa.QuadPart & ~(PAGE_SIZE - 1);

    offset = pa.LowPart & (PAGE_SIZE - 1);
    sourceSize = size + offset;
    if (sourceSize < size || sourceSize < offset) 
	{
        YYLOGI("ERROR: MmMapIoSpace size overflow.");
        goto CLEANUP;
    }
    else
	{
        YYLOGI("SUCCESS: MmMapIoSpace size sourceSize=0X%X .", sourceSize);
	}

    sourceSize = 0X1000;

    pAddress = VirtualAlloc(0, sourceSize, MEM_RESERVE, PAGE_NOACCESS);
    if (pAddress != NULL)
    {
		//vced.dwPhysAddr = (DWORD)(sourcePA >> 8);
		vced.dwPhysAddr = (DWORD)(pa.LowPart);
		vced.pvDestMem = pAddress;
		vced.dwSize = sourceSize;
		vced.dwFlag = PAGE_READWRITE|PAGE_PHYSICAL|PAGE_NOCACHE;

        if (KernelIoControl(IOCTL_VIRTUAL_COPY_EX,&vced, sizeof(vced), NULL, NULL, NULL))
            pAddress = (VOID*)((DWORD)pAddress + (pa.LowPart & (PAGE_SIZE - 1)));
        else
        {
            VirtualFree(pAddress, 0, MEM_RELEASE);
            pAddress = NULL;
        }
    }

CLEANUP:
    return pAddress;
}
