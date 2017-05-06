/*******************************************************************************
	File:		CWndMem.cpp

	Contains:	Window slide pos implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-28		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CWndMem.h"
#include "CBaseUtils.h"

#pragma warning (disable : 4996)

#define		TIMER_UPDATE_INFO	101

CWndMem::CWndMem(HINSTANCE hInst)
	: CWndBase (hInst)
	, m_hPenLine (NULL)
	, m_hPenInfo (NULL)
	, m_hBrushBG (NULL)
	, m_hDCBmp (NULL)
	, m_hBmpMem (NULL)
	, m_hBmpCPU (NULL)
	, m_pBmpMemBuff (NULL)
	, m_pBmpCPUBuff (NULL)
	, m_nPrevCPULoad (0)
	, m_pBmpBGData (NULL)
	, m_nStep (0)
{
	memset (&m_memInfo, 0, sizeof (m_memInfo));
}

CWndMem::~CWndMem(void)
{
	if (m_hPenLine != NULL)
		DeleteObject (m_hPenLine);
	m_hPenLine = NULL;
	if (m_hPenInfo != NULL)
		DeleteObject (m_hPenInfo);
	m_hPenInfo = NULL;
	if (m_hBrushBG != NULL)
		DeleteObject (m_hBrushBG);
	m_hBrushBG = NULL;

	if (m_hBmpMem != NULL)
		DeleteObject (m_hBmpMem);
	m_hBmpMem = NULL;
	if (m_hBmpCPU != NULL)
		DeleteObject (m_hBmpCPU);
	m_hBmpCPU = NULL;
	if (m_hDCBmp != NULL)
		DeleteDC (m_hDCBmp);
	m_hDCBmp = NULL;

	if (m_pBmpBGData != NULL)
		delete []m_pBmpBGData;
	m_pBmpBGData = NULL;
}

bool CWndMem::CreateWnd (HWND hParent, RECT rcView, COLORREF clrBG)
{
	if (!CWndBase::CreateWnd (hParent, rcView, clrBG))
		return false;

	m_hPenLine = ::CreatePen (PS_SOLID, 1, RGB (0, 125, 0));
	m_hPenInfo = ::CreatePen (PS_SOLID, 2, RGB (222, 222, 0));
	m_hBrushBG = ::CreateSolidBrush (RGB (5, 5, 5));

	CreateBGBmp (3);

	SetTimer (m_hWnd, TIMER_UPDATE_INFO, 500, NULL);

	return true;
}

LRESULT CWndMem::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rcWnd;
	if (hwnd != NULL)
		GetClientRect (hwnd, &rcWnd);

	switch (uMsg)
	{
	case WM_TIMER:
		if (wParam == TIMER_UPDATE_INFO)
		{
			UpdateBmpInfo ();
			return S_OK;
		}
		break;

	case WM_PAINT:
	{
		RECT rcMem;
		PAINTSTRUCT ps;		
		HDC hdc = BeginPaint(hwnd, &ps);

		GetClientRect (m_hWnd, &rcMem);
		TCHAR szMemInfo[128];
		DWORD dwUsed = (m_memInfo.dwTotalPhys - m_memInfo.dwAvailPhys) / 1024;
		if (dwUsed < 1000000)
			_stprintf (szMemInfo, _T("Mem: % 3d,%03dK / % 8dK"),
						(dwUsed % 1000000) / 1000, dwUsed % 1000, m_memInfo.dwTotalPhys / 1024);
		else
			_stprintf (szMemInfo, _T("Mem: %d,%03d,%03dK / % 8dK"),
						dwUsed / 1000000, (dwUsed % 1000000) / 1000, dwUsed % 1000, m_memInfo.dwTotalPhys / 1024);

		RECT rcText;
		SetRect (&rcText, 0, 0, rcWnd.right, 20);
		//SetBkMode (hdc, TRANSPARENT);
		SetBkColor (hdc, RGB (200, 200, 200));
		SetTextColor (hdc, RGB (5, 5, 5));
		DrawText (hdc, szMemInfo, _tcslen (szMemInfo), &rcText, 0);

		_stprintf (szMemInfo, _T("CPU: % 6d %%"), m_nPrevCPULoad);
		SetRect (&rcText, 0, rcWnd.bottom / 2, rcWnd.right, rcWnd.bottom / 2 + 20);
		DrawText (hdc, szMemInfo, _tcslen (szMemInfo), &rcText, 0);


		SelectObject (m_hDCBmp, m_hBmpMem);
		BitBlt (hdc, 0, 20, m_nBmpWidth / 2, m_nBmpHeight + 20, m_hDCBmp, m_nStep % (m_nBmpWidth / 2), 0, SRCCOPY);

		SelectObject (m_hDCBmp, m_hBmpCPU);
		BitBlt (hdc, 0, rcWnd.bottom / 2 + 20, m_nBmpWidth / 2, rcWnd.bottom, m_hDCBmp, m_nStep % (m_nBmpWidth / 2), 0, SRCCOPY);

		EndPaint(hwnd, &ps);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	default:
		break;
	}

	return	CWndBase::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

bool CWndMem::UpdateBmpInfo (void)
{
	DWORD dwMemUsed1 = m_memInfo.dwTotalPhys - m_memInfo.dwAvailPhys;
	GlobalMemoryStatus (&m_memInfo);
	DWORD dwMemUsed2 = m_memInfo.dwTotalPhys - m_memInfo.dwAvailPhys;
	int nX = (m_nStep % (m_nBmpWidth / 2)) + (m_nBmpWidth / 2);
	if (nX == m_nBmpWidth / 2)
		nX = 0;
	else
		nX--;
	int nY = m_nBmpHeight - m_nBmpHeight * (dwMemUsed1 / 1024) / (m_memInfo.dwTotalPhys / 1024);
	if (nY == m_nBmpHeight)
		nY = m_nBmpHeight - m_nBmpHeight * (dwMemUsed2 / 1024) / (m_memInfo.dwTotalPhys / 1024);

	SelectObject (m_hDCBmp, m_hPenInfo);
	SelectObject (m_hDCBmp, m_hBmpMem);
	MoveToEx (m_hDCBmp, nX, nY, NULL);
	nY = m_nBmpHeight - m_nBmpHeight * (dwMemUsed2 / 1024) / (m_memInfo.dwTotalPhys / 1024);
	LineTo (m_hDCBmp, nX + 1, nY);

	SelectObject (m_hDCBmp, m_hBmpCPU);
	nY = m_nBmpHeight - m_nBmpHeight * m_nPrevCPULoad / 100;
	MoveToEx (m_hDCBmp, nX, nY, NULL);
	m_nPrevCPULoad = CBaseUtils::GetCPULoad ();
	nY = m_nBmpHeight - m_nBmpHeight * m_nPrevCPULoad / 100;
	LineTo (m_hDCBmp, nX + 1, nY);

	::InvalidateRect (m_hWnd, NULL, FALSE);

	m_nStep++;
	if (m_nStep % (m_nBmpWidth / 2) == 0)
	{
		int i = 0;
		int nStride = m_nBmpWidth * 4;
		for (i = 0; i < m_nBmpHeight; i++)
		{
			memcpy (m_pBmpMemBuff + i * nStride, m_pBmpMemBuff + i * nStride + m_nBmpWidth * 2, m_nBmpWidth * 2);
			memcpy (m_pBmpMemBuff + i * nStride + m_nBmpWidth * 2, m_pBmpBGData + i * m_nBmpWidth * 2, m_nBmpWidth * 2);
		}
		for (i = 0; i < m_nBmpHeight; i++)
		{
			memcpy (m_pBmpCPUBuff + i * nStride, m_pBmpCPUBuff + i * nStride + m_nBmpWidth * 2, m_nBmpWidth * 2);
			memcpy (m_pBmpCPUBuff + i * nStride + m_nBmpWidth * 2, m_pBmpBGData + i * m_nBmpWidth * 2, m_nBmpWidth * 2);
		}
	}

	return true;
}

bool CWndMem::CreateBGBmp (int nType)
{
	int i = 0;
	RECT rcWnd;
	RECT rcBmp;
	GetClientRect (m_hWnd, &rcWnd);
	m_nBmpWidth = (rcWnd.right) / 12 * 12 * 2;
	m_nBmpHeight = (rcWnd.bottom / 2 - 20) / 12 * 12;

	if (nType == 1)
		SetRect (&rcBmp, 0, 0, m_nBmpWidth / 2, m_nBmpHeight); 
	else if (nType == 2)
		SetRect (&rcBmp, m_nBmpWidth / 2, 0, m_nBmpWidth, m_nBmpHeight); 
	else 
		SetRect (&rcBmp, 0, 0, m_nBmpWidth, m_nBmpHeight); 

	if (m_hDCBmp == NULL)
	{
		HDC hDCWnd = GetDC (m_hWnd);
		m_hDCBmp = CreateCompatibleDC (hDCWnd);
		ReleaseDC (m_hWnd, hDCWnd);
	}

	if (m_hBmpMem == NULL)
	{
		BITMAPINFO * pBmpInfo = new BITMAPINFO ();
		pBmpInfo->bmiHeader.biSize			= sizeof (BITMAPINFOHEADER);
		pBmpInfo->bmiHeader.biWidth			= m_nBmpWidth;
		pBmpInfo->bmiHeader.biHeight		= -m_nBmpHeight;
		pBmpInfo->bmiHeader.biBitCount		= 32;
		pBmpInfo->bmiHeader.biCompression	= BI_RGB;
		pBmpInfo->bmiHeader.biXPelsPerMeter	= 0;
		pBmpInfo->bmiHeader.biYPelsPerMeter	= 0;
		pBmpInfo->bmiHeader.biPlanes		= 1;
		pBmpInfo->bmiHeader.biSizeImage	= m_nBmpWidth * m_nBmpHeight * 4;
		m_hBmpMem = CreateDIBSection(m_hDCBmp , pBmpInfo, DIB_RGB_COLORS , (void **)&m_pBmpMemBuff, NULL , 0);
		if (m_hBmpCPU == NULL)
			m_hBmpCPU = CreateDIBSection(m_hDCBmp , pBmpInfo, DIB_RGB_COLORS , (void **)&m_pBmpCPUBuff, NULL , 0);
		delete pBmpInfo;
	}
	if (m_hBmpMem == NULL)
		return false;

	SelectObject (m_hDCBmp, m_hPenLine);
	SelectObject (m_hDCBmp, m_hBmpMem);
	FillRect (m_hDCBmp, &rcBmp, m_hBrushBG);
	for (i = 0; i < m_nBmpHeight; i += 12)
	{
		MoveToEx (m_hDCBmp, rcBmp.left, i, NULL);
		LineTo (m_hDCBmp, rcBmp.right, i);
	}
	for (i = rcBmp.left; i < rcBmp.right; i += 12)
	{
		MoveToEx (m_hDCBmp, i, 0, NULL);
		LineTo (m_hDCBmp, i, m_nBmpHeight);
	}

	if (m_pBmpBGData == NULL)
		m_pBmpBGData = new BYTE[m_nBmpWidth * m_nBmpHeight * 2];
	int nStride = m_nBmpWidth * 4;
	for (i = 0; i < m_nBmpHeight; i++)
		memcpy (m_pBmpBGData + i * m_nBmpWidth * 2, m_pBmpMemBuff + i * nStride, m_nBmpWidth * 2);

	if (m_hBmpCPU == NULL)
		return false;

	SelectObject (m_hDCBmp, m_hBmpCPU);
	FillRect (m_hDCBmp, &rcBmp, m_hBrushBG);
	for (i = 0; i < m_nBmpHeight; i += 12)
	{
		MoveToEx (m_hDCBmp, rcBmp.left, i, NULL);
		LineTo (m_hDCBmp, rcBmp.right, i);
	}
	for (i = rcBmp.left; i < rcBmp.right; i += 12)
	{
		MoveToEx (m_hDCBmp, i, 0, NULL);
		LineTo (m_hDCBmp, i, m_nBmpHeight);
	}

	return true;
}

