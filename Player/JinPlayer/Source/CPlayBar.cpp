/*******************************************************************************
	File:		CPlayBar.cpp

	Contains:	The play bar implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-09		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "tchar.h"

#include "CPlayBar.h"
#include "CPlayButton.h"
#include "CPlaySlider.h"

#include "CWndPlay.h"
#include "CRegMng.h"
#include "USystemFunc.h"
#include "UBitmapFunc.h"
#include "URGB32Rotate.h"
#include "RPlayerDef.h"
#include "yyLog.h"
#include <libavformat/avformat.h>

#pragma warning (disable : 4996)

CPlayBar::CPlayBar(HINSTANCE hInst, CWndPlay * pWndPlay)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_pWndPlay (pWndPlay)
	, m_hWnd (NULL)
	, m_bShow (false)
	, m_hViewDC (NULL)
	, m_hViewBmp (NULL)
	, m_pViewBuff (NULL)
	, m_hBmpVideo (NULL)
	, m_pBmpVBuff (NULL)
	, m_hRottBmp (NULL)
	, m_pRottBuff (NULL)
	, m_bRottCopy (false)
	, m_nLeft (10)
	, m_nTop (0)
	, m_nHeight (0)
	, m_nArc (5)
	, m_nTransParent (60)
	, m_nVideoW (0)
	, m_nVideoH (0)
	, m_pVCC (NULL)
	, m_hPenZoom (NULL)
	, m_hBrhZoom (NULL)
	, m_hPenSel (NULL)
	, m_pMedia (NULL)
	, m_nVolume (100)
	, m_bVideoEOS (false)
	, m_bAudioOnly (false)
	, m_bSeeking (false)
	, m_nSeekPos (0)
	, m_nRotate (0)
	, m_pConfig (NULL)
	, m_pSldAudio (NULL)
	, m_pSldPos (NULL)
	, m_pBtnPlay (NULL)
	, m_pBtnPause (NULL)
	, m_pBtnFull (NULL)
	, m_pBtnNormal (NULL)
{
	SetObjectName ("CPlayBar");
	m_dcbVideoRnd.funcCB = VideoExtRender;
	m_dcbVideoRnd.userData = this;

	memset (&m_buffConv, 0, sizeof (m_buffConv));
	memset (&m_buffData, 0, sizeof (m_buffData));
	memset (&m_buffVideo, 0, sizeof (m_buffVideo));
	m_buffConv.pTarget =  &m_buffData;
	m_buffData.nType = YY_MEDIA_Video;
	m_buffData.pBuff = (unsigned char *)&m_buffVideo;
	m_buffData.uFlag = YYBUFF_TYPE_VIDEO;
	m_buffData.uSize = sizeof (YY_VIDEO_BUFF);
	m_buffVideo.nType = YY_VDT_RGBA;

	_tcscpy (m_szTextDur, _T("00:00:00"));
	m_nVolume = CRegMng::g_pRegMng->GetIntValue (_T("AudioVolume"), m_nVolume);
}	

CPlayBar::~CPlayBar(void)
{
	CPlayCtrl * pCtrl = m_lstCtrl.RemoveHead ();
	while (pCtrl != NULL)
	{
		delete pCtrl;
		pCtrl = m_lstCtrl.RemoveHead ();
	}
	YY_DEL_P (m_pConfig);
	YY_DEL_P (m_pVCC);
	if (m_hViewDC != NULL)
		DeleteDC (m_hViewDC);
	if (m_hViewBmp != NULL)
		DeleteObject (m_hViewBmp);
	if (m_hBmpVideo != NULL)
		DeleteObject (m_hBmpVideo);
	if (m_hRottBmp != NULL)
		DeleteObject (m_hRottBmp);
	if (m_hPenZoom != NULL)
		DeleteObject (m_hPenZoom);
	if (m_hBrhZoom != NULL)
		DeleteObject (m_hBrhZoom);
	if (m_hPenSel != NULL)
		DeleteObject (m_hPenSel);
	CRegMng::g_pRegMng->SetIntValue (_T("AudioVolume"), m_nVolume);
}

bool CPlayBar::Create (HWND hWnd, CMediaEngine * pMedia)
{
	m_hWnd = hWnd;
	m_pMedia = pMedia;

	TCHAR szPath[1024];
	yyGetAppPath (m_hInst, szPath, sizeof (szPath));
	_tcscat (szPath, _T("jpres\\jPlayer.skn"));
	LoadConfig (szPath);

	HDC hWinDC = GetDC (m_hWnd);
	m_hViewDC = CreateCompatibleDC (hWinDC);
	SetBkMode (m_hViewDC, TRANSPARENT);
	SetTextColor (m_hViewDC, RGB (215, 215, 215));
	ReleaseDC (m_hWnd, hWinDC);

	SetTimer (m_hWnd, WT_BAR_UI_Update, 500, NULL);

	return true;
}

bool CPlayBar::Show (bool bShow)
{
	if (m_bShow == bShow)
		return true;
	m_bShow = bShow;
	if(m_bShow)
	{
		m_pMedia->SetParam (YYPLAY_PID_VideoExtRnd, &m_dcbVideoRnd);
	}
	else
	{
		m_pMedia->SetParam (YYPLAY_PID_VideoExtRnd, NULL);
		if (m_bVideoEOS)
			InvalidateRect (m_hWnd, NULL, TRUE);
	}

	return true;
}

void CPlayBar::SetPos (bool bForward, int nStep)
{
	if (m_bSeeking)
		return;
	int nDur = m_pMedia->GetDur ();
	if (nStep == 0)
	{
		nStep = nDur / 200;
		if (nStep > 40000)
			nStep = 40000;
		if (nStep < 20000)
			nStep = 20000;
	}
	int nPos = m_pMedia->GetPos ();
	if (bForward)
		nPos += nStep;
	else
		nPos -= nStep;
	if (nPos < 0)
		nPos = 0;
	else if (nPos > nDur)
		nPos = nDur - 1000;

	m_nSeekPos = nPos;
	m_pSldPos->SetPos (m_nSeekPos);
	m_bSeeking = true;
	if (m_bShow)
		InvalidateRect (m_hWnd, NULL, FALSE);
	m_pMedia->SetPos (nPos);
}

void CPlayBar::SetVolume (int nStep)
{
	m_nVolume += nStep;
	if (m_nVolume > 100)
		m_nVolume = 100;
	if (m_nVolume < 0)
		m_nVolume = 0;
	m_pMedia->SetVolume (m_nVolume);
	if (m_pSldAudio != NULL)
		m_pSldAudio->SetPos (m_nVolume);
	if (m_bShow)
		InvalidateRect (m_hWnd, NULL, FALSE);
}

void CPlayBar::SetZoomRect (RECT * pZoom)
{
	CAutoLock lock (&m_mtDraw);
	m_buffConv.pZoom = pZoom;

	if (m_pMedia->GetStatus () == YY_PLAY_Pause)
	{
		if (!UpdateVideoBitmap ())
			return;

		YY_BUFFER * pVideo = NULL;
		int nRC = m_pMedia->GetParam (YYPLAY_PID_VideoData, &pVideo);
		if (nRC != YY_ERR_NONE)
			return;

		if (m_nRotate == 0 || m_nRotate == 180)
		{
			m_buffVideo.nWidth = m_rcBmpVideo.right - m_rcBmpVideo.left;
			m_buffVideo.nHeight = m_rcBmpVideo.bottom - m_rcBmpVideo.top;
		}
		else
		{
			m_buffVideo.nWidth = m_rcBmpVideo.bottom - m_rcBmpVideo.top;
			m_buffVideo.nHeight = m_rcBmpVideo.right - m_rcBmpVideo.left;
		}
		m_buffVideo.pBuff[0] = m_pBmpVBuff;
		m_buffVideo.nStride[0] = m_buffVideo.nWidth * 4;
		m_buffConv.pSource = pVideo;
		m_pMedia->GetParam (YYPLAY_PID_ConvertData, &m_buffConv);

		LPBYTE pViewBuff = m_pViewBuff + m_rcBmpVideo.top * m_rcView.right * 4 + m_rcBmpVideo.left * 4;
		if (m_nRotate == 0)
			yyRGB32Rotate00 (m_pBmpVBuff, m_buffVideo.nWidth, m_buffVideo.nHeight, m_buffVideo.nWidth * 4, pViewBuff, m_rcView.right * 4);
		else if (m_nRotate == 90)
			yyRGB32Rotate90 (m_pBmpVBuff, m_buffVideo.nWidth, m_buffVideo.nHeight, m_buffVideo.nWidth * 4, pViewBuff, m_rcView.right * 4);
		else if (m_nRotate == 180)
			yyRGB32Rotate180 (m_pBmpVBuff, m_buffVideo.nWidth, m_buffVideo.nHeight, m_buffVideo.nWidth * 4, pViewBuff, m_rcView.right * 4);
		else if (m_nRotate == 270)
			yyRGB32Rotate270 (m_pBmpVBuff, m_buffVideo.nWidth, m_buffVideo.nHeight, m_buffVideo.nWidth * 4, pViewBuff, m_rcView.right * 4);
	}
}

void CPlayBar::HandleEvent (int nID, void * pV1)
{
	switch (nID)
	{
	case YY_EV_Open_Complete:
	case YY_EV_Play_Duration:
	{
		if (nID == YY_EV_Open_Complete)
		{
			m_bVideoEOS = false;
			m_pMedia->SetVolume (m_nVolume);
			if (m_pSldAudio != NULL)
				m_pSldAudio->SetPos (m_nVolume);
			YY_VIDEO_FORMAT * pFmtVideo = NULL;
			m_pMedia->GetParam (YYPLAY_PID_Fmt_Video, &pFmtVideo);
			if (pFmtVideo == NULL)
			{
				m_bAudioOnly = true;
				m_bVideoEOS = true;
				UpdateControl (true);
			}
			else
			{
				m_bAudioOnly = false;
				m_bVideoEOS = false;
				UpdateControl (false);
				m_nVideoW = pFmtVideo->nWidth;
			}
			m_nSeekPos = 0;
			m_bSeeking = false;
			m_nRotate = 0;
			if (m_hBmpVideo != NULL)
			{
				DeleteObject (m_hBmpVideo);
				m_hBmpVideo = NULL;
			}
			if(m_bShow)
				m_pMedia->SetParam (YYPLAY_PID_VideoExtRnd, &m_dcbVideoRnd);
		}
		int nDur = m_pMedia->GetDur ();
		if (m_pSldPos != NULL)
			m_pSldPos->SetRange (0, nDur);
		nDur = nDur / 1000;
		_stprintf (m_szTextDur, _T("%02d:%02d:%02d"), nDur / 3600, (nDur % 3600) / 60, nDur % 60);
		break;
	}
	case YY_EV_Play_Status:
		if (m_pMedia->GetStatus () == YY_PLAY_Pause)
		{
			if (m_pBtnPlay != NULL)
				m_pBtnPlay->Show (true);
			if (m_pBtnPause != NULL)
				m_pBtnPause->Show (false);
		}
		else 
		{
			if (m_pBtnPlay != NULL)
				m_pBtnPlay->Show (false);
			if (m_pBtnPause != NULL)
				m_pBtnPause->Show (true);
		}
		break;

	case YY_EV_Seek_Complete:
	case YY_EV_Seek_Failed:
		m_bSeeking = false;
		break;

	case YY_EV_Video_EOS:
		m_bVideoEOS = true;
		UpdateControl (true);
		break;

	default:
		break;
	}
}

LRESULT	CPlayBar::MsgProc (HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!m_bShow && uMsg != WM_SIZE)
	{
		if (uMsg == WM_PAINT)
		{
			if (!m_bVideoEOS)
				return S_FALSE;
		}
		else
			return S_FALSE;
	}

	LRESULT		lRC = S_FALSE;
	CPlayCtrl * pCtrl = NULL;
	NODEPOS pos = m_lstCtrl.GetHeadPosition ();
	while (pos != NULL)
	{
		pCtrl = m_lstCtrl.GetNext (pos);
		lRC = pCtrl->MsgProc (hWnd, uMsg, wParam, lParam);
		if (lRC != S_FALSE)
			return lRC;
	}

	switch (uMsg)
	{
	case WM_COMMAND:
		return OnCommand (uMsg, wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown (uMsg, wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp (uMsg, wParam, lParam);
	case WT_PLAY_HScroll:
		return OnHScroll (uMsg, wParam, lParam);
	case WM_SIZE:
		return OnSize (uMsg, wParam, lParam);
	case WM_TIMER:
		return OnTimer (uMsg, wParam, lParam);
	case WM_ERASEBKGND:
		return OnEraseBG (uMsg, wParam, lParam);
	case WM_PAINT:
		return OnPaint (uMsg, wParam, lParam);
	default:
		break;
	}
	return S_FALSE;
}

LRESULT	CPlayBar::OnCommand (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	switch (wmId)
	{
	case ID_BUTTON_PLAY:
		m_pMedia->Run ();
		return S_OK;
	case ID_BUTTON_PAUSE:
		m_pMedia->Pause ();
		return S_OK;
	case ID_BUTTON_BF:
	{
		SetPos (false, 20000);
		return S_OK;
	}
	case ID_BUTTON_FF:
	{
		SetPos (true, 20000);
		return S_OK;
	}
	case ID_BUTTON_FULL:
	case ID_BUTTON_NORMAL:
		m_pWndPlay->ShowFullScreen ();
		return S_OK;
	case ID_BUTTON_LIST:
		SendMessage (m_hWnd, WM_PLAY_Close, 0, 0);
		return S_OK;

	case ID_BUTTON_MUTE:
		m_pMedia->SetVolume (0);
		if (m_pSldAudio != NULL)
			m_pSldAudio->SetPos (0);
		return S_OK;
	case ID_BUTTON_AUDIO:
		m_nVolume += 15;
		if (m_nVolume > 100)
			m_nVolume = 100;
		m_pMedia->SetVolume (m_nVolume);
		if (m_pSldAudio != NULL)
			m_pSldAudio->SetPos (m_nVolume);
		return S_OK;

	default:
		break;
	}
	return S_FALSE;
}

LRESULT	CPlayBar::OnLButtonDown (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pWndPlay->IsZoomSelect ())
		return S_FALSE;

	POINT pt;
	pt.x = LOWORD (lParam);
	pt.y = HIWORD (lParam);

	if (PtInRect (&m_rcBar, pt))
		return S_OK;
	else
		return S_FALSE;
}

LRESULT	CPlayBar::OnLButtonUp (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pWndPlay->IsZoomSelect ())
		return S_FALSE;

	POINT pt;
	pt.x = LOWORD (lParam);
	pt.y = HIWORD (lParam);

	if (PtInRect (&m_rcBar, pt))
		return S_OK;
	else
		return S_FALSE;
}

LRESULT	CPlayBar::OnHScroll (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (lParam == (LPARAM)m_pSldAudio)
	{
		m_nVolume = m_pSldAudio->GetPos ();
		m_pMedia->SetVolume (m_nVolume);
		return S_OK;
	}
	else if (lParam == (LPARAM)m_pSldPos)
	{
		int nPos = m_pSldPos->GetPos ();
		m_bSeeking = true;
		m_nSeekPos = nPos;
		InvalidateRect (m_hWnd, NULL, FALSE);
		m_pMedia->SetPos (m_nSeekPos);
		return S_OK;
	}
	return S_FALSE;
}

LRESULT	CPlayBar::OnTimer (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WT_BAR_UI_Update)
	{
		if (m_pMedia->GetStatus () != YY_PLAY_Run || m_bVideoEOS)
			InvalidateRect (m_hWnd, NULL, FALSE);
		return S_OK;
	}
	return S_FALSE;
}

LRESULT	CPlayBar::OnSize (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CAutoLock lock (&m_mtDraw);
	GetClientRect (m_hWnd, &m_rcView);

	if (m_rcView.right == GetSystemMetrics (SM_CXSCREEN) && m_rcView.bottom == GetSystemMetrics (SM_CYSCREEN))
	{
		if (m_pBtnFull != NULL)
			m_pBtnFull->Show (false);
		if (m_pBtnNormal != NULL)
			m_pBtnNormal->Show (true);
	}
	else
	{
		if (m_pBtnFull != NULL)
			m_pBtnFull->Show (true);
		if (m_pBtnNormal != NULL)
			m_pBtnNormal->Show (false);
	}

	UpdateViewBitmap ();
	int nTop = m_rcView.bottom - (YYPOS_RIGHTB - m_nTop);
	SetRect (&m_rcBar, m_nLeft, nTop, m_rcView.right - m_nLeft, nTop + m_nHeight);

	return S_FALSE;
}

LRESULT	CPlayBar::OnEraseBG (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return S_FALSE;
}

LRESULT	CPlayBar::OnPaint (UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CAutoLock lock (&m_mtDraw);
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(m_hWnd, &ps);
	RenderVideo (hDC, NULL);
	EndPaint(m_hWnd, &ps);
	return S_FALSE;
}

int CPlayBar::VideoExtRender (void * pUserData, YY_BUFFER * pData)
{
	CPlayBar * pPlayBar = (CPlayBar *)pUserData;
	if (pData == NULL)
		return YY_ERR_ARG;

	if (pPlayBar->m_pMedia->GetStatus () == YY_PLAY_Run)
	{
//		if (pPlayBar->m_bSeeking)
//			return YY_ERR_FAILED;
		if (pPlayBar->m_hBmpVideo != NULL && pPlayBar->m_mtDraw.IsLock ())
			return YY_ERR_NONE;
	}

	return pPlayBar->RenderVideo (NULL, pData);
}

int CPlayBar::RenderVideo (HDC hDC, YY_BUFFER * pData)
{
	CAutoLock lock (&m_mtDraw);
	if (m_rcView.right <= 16 || m_rcView.bottom <= 16)
		return YY_ERR_NONE;

	UpdateViewBitmap ();

	if (pData != NULL && (pData->uFlag & YYBUFF_NEW_FORMAT) == YYBUFF_NEW_FORMAT)
	{
		YY_VIDEO_FORMAT * pFmt = (YY_VIDEO_FORMAT *)pData->pFormat;
		SetRect (m_buffConv.pZoom, 0, 0, pFmt->nWidth, pFmt->nHeight);
	}
	if (pData != NULL)
	{
		if ((pData->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
		{
			AVFrame * pFrmVideo = (AVFrame *)pData->pBuff;
			m_nVideoW = pFrmVideo->width;
			m_nVideoH = pFrmVideo->height;
		}
		else if ((pData->uFlag & YYBUFF_TYPE_VIDEO) == YYBUFF_TYPE_VIDEO)
		{
			YY_VIDEO_BUFF * pBufVideo = (YY_VIDEO_BUFF *)pData->pBuff;
			m_nVideoW = pBufVideo->nWidth;
			m_nVideoH = pBufVideo->nHeight;
		}

		if (!UpdateVideoBitmap ())
			return YY_ERR_NONE;

		if (m_nRotate == 0 || m_nRotate == 180)
		{
			m_buffVideo.nWidth = m_rcBmpVideo.right - m_rcBmpVideo.left;
			m_buffVideo.nHeight = m_rcBmpVideo.bottom - m_rcBmpVideo.top;
		}
		else
		{
			m_buffVideo.nWidth = m_rcBmpVideo.bottom - m_rcBmpVideo.top;
			m_buffVideo.nHeight = m_rcBmpVideo.right - m_rcBmpVideo.left;
		}
		m_buffVideo.pBuff[0] = m_pBmpVBuff;
		m_buffVideo.nStride[0] = m_buffVideo.nWidth * 4;
		m_buffConv.pSource = pData;
		m_pMedia->GetParam (YYPLAY_PID_ConvertData, &m_buffConv);

		LPBYTE pViewBuff = m_pViewBuff + m_rcBmpVideo.top * m_rcView.right * 4 + m_rcBmpVideo.left * 4;
		if (m_nRotate == 0)
			yyRGB32Rotate00 (m_pBmpVBuff, m_buffVideo.nWidth, m_buffVideo.nHeight, m_buffVideo.nWidth * 4, pViewBuff, m_rcView.right * 4);
		else if (m_nRotate == 90)
			yyRGB32Rotate90 (m_pBmpVBuff, m_buffVideo.nWidth, m_buffVideo.nHeight, m_buffVideo.nWidth * 4, pViewBuff, m_rcView.right * 4);
		else if (m_nRotate == 180)
			yyRGB32Rotate180 (m_pBmpVBuff, m_buffVideo.nWidth, m_buffVideo.nHeight, m_buffVideo.nWidth * 4, pViewBuff, m_rcView.right * 4);
		else if (m_nRotate == 270)
			yyRGB32Rotate270 (m_pBmpVBuff, m_buffVideo.nWidth, m_buffVideo.nHeight, m_buffVideo.nWidth * 4, pViewBuff, m_rcView.right * 4);
		m_bRottCopy = false;
	}
	else
	{
		ResizeVideoBitmap ();
	}

	if (m_bShow)
	{
		int nPos = m_pMedia->GetPos ();
		if (m_pSldPos != NULL)
			m_pSldPos->SetPos (nPos);
		nPos = nPos / 1000;
		_stprintf (m_szTextPos, _T("%02d:%02d:%02d"), nPos / 3600, (nPos % 3600) / 60, nPos % 60);
		OverlayBmpVideo ();
	}

	HDC hWinDC = hDC;
	if (hWinDC == NULL)
		hWinDC = GetDC (m_hWnd);
	HBITMAP hOld = (HBITMAP)SelectObject (m_hViewDC, m_hViewBmp);
	if (m_bShow)
	{
		RECT	rcText;
		RECT *	pRectPos = m_pSldPos->GetDrawRect ();
		SetRect (&rcText, pRectPos->left + 24, pRectPos->bottom, pRectPos->left + 150, pRectPos->bottom + 40);
		DrawText (m_hViewDC, m_szTextPos, _tcslen (m_szTextPos), &rcText, DT_LEFT);
		SetRect (&rcText, pRectPos->right - 160, pRectPos->bottom, pRectPos->right - 24, pRectPos->bottom + 40);
		DrawText (m_hViewDC, m_szTextDur, _tcslen (m_szTextDur), &rcText, DT_RIGHT);
	}
	BitBlt (hWinDC, m_rcView.left, m_rcView.top, m_rcView.right - m_rcView.left, m_rcView.bottom - m_rcView.top, m_hViewDC, 0, 0, SRCCOPY);
	if (m_bShow)
	{
		ShowZoomArea (hWinDC);
		ShowZoomSelect (hWinDC);
	}
	SelectObject (m_hViewDC, hOld);
	if (hDC == NULL)
		ReleaseDC (m_hWnd, hWinDC);

	return YY_ERR_NONE;
}

bool CPlayBar::UpdateViewBitmap (void)
{
	BITMAP bmpInfo;
	memset (&bmpInfo, 0, sizeof (BITMAP));
	if (m_hViewBmp != NULL)
		GetObject (m_hViewBmp, sizeof (BITMAP), &bmpInfo);
	if (bmpInfo.bmWidth != m_rcView.right || bmpInfo.bmHeight != m_rcView.bottom)
	{
		DeleteObject (m_hViewBmp);
		m_hViewBmp = NULL;
	}
	if (m_hViewBmp == NULL)
		m_hViewBmp = yyBmpCreate (m_hViewDC, m_rcView.right, m_rcView.bottom, &m_pViewBuff, 0);
	else
		memset (m_pViewBuff, 0, m_rcView.right * m_rcView.bottom * 4);

	return true;
}

bool CPlayBar::UpdateVideoBitmap (void)
{
	BITMAP bmpInfo;
	memset (&bmpInfo, 0, sizeof (BITMAP));
	if (m_hBmpVideo != NULL)
		GetObject (m_hBmpVideo, sizeof (BITMAP), &bmpInfo);
	int nRC = m_pMedia->GetParam (YYPLAY_PID_RenderArea, &m_rcBmpVideo);
	if (nRC != YY_ERR_NONE)
		return false;
	if (m_rcBmpVideo.right - m_rcBmpVideo.left <= 16 || m_rcBmpVideo.bottom - m_rcBmpVideo.top <= 16)
		return false;
	m_rcBmpVideo.left = m_rcBmpVideo.left & ~1;
	m_rcBmpVideo.right = m_rcBmpVideo.right & ~1;
	if (m_rcBmpVideo.right > m_rcView.right)
		m_rcBmpVideo.right = m_rcView.right;
	if (m_rcBmpVideo.bottom > m_rcView.bottom)
		m_rcBmpVideo.bottom = m_rcView.bottom;
	int nRndW = m_rcBmpVideo.right - m_rcBmpVideo.left;
	int nRndH = m_rcBmpVideo.bottom - m_rcBmpVideo.top;
	if (m_hBmpVideo != NULL && (bmpInfo.bmWidth != nRndW || bmpInfo.bmHeight != nRndH))
	{
		DeleteObject (m_hBmpVideo);
		m_hBmpVideo = NULL;
	}
	if (m_hBmpVideo == NULL)
		m_hBmpVideo = yyBmpCreate (m_hViewDC, nRndW, nRndH, &m_pBmpVBuff, 0);
	return true;
}

bool CPlayBar::ResizeVideoBitmap (void)
{
	if (m_hBmpVideo == NULL)
		return false;

	RECT rcRnd;
	m_pMedia->GetParam (YYPLAY_PID_RenderArea, &rcRnd);
	rcRnd.left = rcRnd.left & ~1;
	rcRnd.right = rcRnd.right & ~1;
	if (rcRnd.right > m_rcView.right)
		rcRnd.right = m_rcView.right;
	if (rcRnd.bottom > m_rcView.bottom)
		rcRnd.bottom = m_rcView.bottom;
	HDC hDC = CreateCompatibleDC (m_hViewDC);
	HBITMAP hView = (HBITMAP)SelectObject (m_hViewDC, m_hViewBmp);
	HBITMAP hVideo = NULL;
	if (m_nRotate == 0)
	{
		hVideo = (HBITMAP)SelectObject (hDC, m_hBmpVideo);
	}
	else
	{
		int nRndW = m_rcBmpVideo.right - m_rcBmpVideo.left;
		int nRndH = m_rcBmpVideo.bottom - m_rcBmpVideo.top;
		BITMAP bmpInfo;
		memset (&bmpInfo, 0, sizeof (BITMAP));
		if (m_hRottBmp != NULL)
		{
			GetObject (m_hRottBmp, sizeof (BITMAP), &bmpInfo);
			if (bmpInfo.bmWidth != nRndW || bmpInfo.bmHeight != nRndH)
			{
				DeleteObject (m_hRottBmp);
				m_hRottBmp = NULL;
			}
		}
		if (m_hRottBmp == NULL)
			m_hRottBmp = yyBmpCreate (m_hViewDC, nRndW, nRndH, &m_pRottBuff, 0);
		if (!m_bRottCopy)
		{
			if (m_nRotate == 90)
				yyRGB32Rotate90 (m_pBmpVBuff, nRndH, nRndW, nRndH * 4, m_pRottBuff, nRndW * 4);
			else if (m_nRotate == 180)
				yyRGB32Rotate180 (m_pBmpVBuff, nRndW, nRndH, nRndW * 4, m_pRottBuff, nRndW * 4);
			else if (m_nRotate == 270)
				yyRGB32Rotate270 (m_pBmpVBuff, nRndH, nRndW, nRndH * 4, m_pRottBuff, nRndW * 4);

			m_bRottCopy = true;
		}
		hVideo = (HBITMAP)SelectObject (hDC, m_hRottBmp);
	}
	StretchBlt (m_hViewDC, rcRnd.left, rcRnd.top, rcRnd.right - rcRnd.left, rcRnd.bottom - rcRnd.top, 
				hDC, 0, 0, m_rcBmpVideo.right - m_rcBmpVideo.left, m_rcBmpVideo.bottom - m_rcBmpVideo.top, SRCCOPY);
	SelectObject (hDC, hVideo);
	SelectObject (m_hViewDC, hView);
	DeleteDC (hDC);

	return true;
}

bool CPlayBar::OverlayBmpVideo (void)
{
	int nTop = m_rcView.bottom - (YYPOS_RIGHTB - m_nTop);
	SetRect (&m_rcBar, m_nLeft, nTop, m_rcView.right - m_nLeft, nTop + m_nHeight);
	unsigned char * pRGB = m_pViewBuff + (m_rcView.right * 4 * nTop);
	for (int h = 0; h < m_nHeight; h++)
	{
		for (int w = 0; w < m_rcView.right; w++)
		{
			if (w < m_nLeft || w > m_rcView.right - m_nLeft)
			{
				pRGB += 4;
				continue;
			}
			if (h < m_nArc)
			{
				if (w < m_nLeft + m_nArc - h)
				{
					pRGB += 4;
					continue;
				}
				else if (w > m_rcView.right - m_nLeft - m_nArc + h)
				{
					pRGB += 4;
					continue;
				}
			}
			else if (h > m_nHeight - m_nArc)
			{
				if (w < m_nLeft + m_nArc - (m_nHeight - h))
				{
					pRGB += 4;
					continue;
				}
				else if (w > m_rcView.right - m_nLeft - m_nArc + (m_nHeight - h))
				{
					pRGB += 4;
					continue;
				}
			}
			*pRGB = *pRGB++ * m_nTransParent / 100;
			*pRGB = *pRGB++ * m_nTransParent / 100;
			*pRGB = *pRGB++ * m_nTransParent / 100;
			pRGB++;
		}
	}

	CPlayCtrl * pCtrl = NULL;
	NODEPOS pos = m_lstCtrl.GetHeadPosition ();
	while (pos != NULL)
	{
		pCtrl = m_lstCtrl.GetNext (pos);
		pCtrl->OnDraw (m_hViewDC, m_hViewBmp, m_pViewBuff, &m_rcView);
	}

	return true;
}

bool CPlayBar::ShowZoomArea (HDC hDC)
{
	if (m_bAudioOnly || m_nVideoW == m_buffConv.pZoom->right - m_buffConv.pZoom->left)
		return false;

	int nDen = 3;
	int nW = m_nVideoW / nDen;
	while (nW > m_rcView.right / 5)
		nW = m_nVideoW / ++nDen;
	int nH = m_nVideoH / nDen;
	if (m_hPenZoom == NULL)
		m_hPenZoom = CreatePen (PS_SOLID, 4, RGB (200, 200, 200));
	HPEN hOld = (HPEN)SelectObject (hDC, m_hPenZoom);
	MoveToEx (hDC, m_rcView.right - nW, 2, NULL);
	LineTo (hDC, m_rcView.right - 2, 2);
	LineTo (hDC, m_rcView.right - 2, nH);
	LineTo (hDC, m_rcView.right - nW, nH);
	LineTo (hDC, m_rcView.right - nW, 2);
	SelectObject (hDC, hOld);
	if (m_hBrhZoom == NULL)
		m_hBrhZoom = CreateSolidBrush (RGB (215, 215, 215));
	RECT rcZoom;
	memcpy (&rcZoom, m_buffConv.pZoom, sizeof (RECT));
	SetRect (&rcZoom, rcZoom.left / nDen, rcZoom.top / nDen, rcZoom.right / nDen, rcZoom.bottom / nDen);
	rcZoom.left = rcZoom.left + m_rcView.right - nW;
	rcZoom.right = rcZoom.right + m_rcView.right - nW;
	FillRect (hDC, &rcZoom, m_hBrhZoom);

	return true;
}

bool CPlayBar::ShowZoomSelect (HDC hDC)
{
	RECT rcSel;
	if (!m_pWndPlay->GetZoomSelect (&rcSel))
		return false;

	if (m_hPenSel == NULL)
		m_hPenSel = CreatePen (PS_DOT, 2, RGB (200, 200, 200));
	HPEN hOld = (HPEN)SelectObject (hDC, m_hPenSel);
	MoveToEx (hDC, rcSel.left, rcSel.top, NULL);
	LineTo (hDC, rcSel.right, rcSel.top);
	LineTo (hDC, rcSel.right, rcSel.bottom);
	LineTo (hDC, rcSel.left, rcSel.bottom);
	LineTo (hDC, rcSel.left, rcSel.top);
	SelectObject (hDC, hOld);

	return true;
}

bool CPlayBar::LoadConfig (TCHAR * pCfgFile)
{
	if (m_pConfig == NULL)
		m_pConfig = new CBaseConfig ();
	if (!m_pConfig->Open (pCfgFile))
		return false;

	m_nLeft = m_pConfig->GetItemValue ("Bar", "Left", m_nLeft);
	m_nTop = m_pConfig->GetItemValue ("Bar", "Top", 60);
	m_nHeight = m_pConfig->GetItemValue ("Bar", "Height", 60);
	m_nArc = m_pConfig->GetItemValue ("Bar", "Arc", m_nArc);
	m_nTransParent = m_pConfig->GetItemValue ("Bar", "TransParent", m_nTransParent);
	char * pSectType = NULL;
	CCfgSect * pSect = m_pConfig->GetFirstSect ();
	while (pSect != NULL)
	{
		pSectType = m_pConfig->GetItemText (pSect->m_pName, "Type", NULL);
		if (pSectType != NULL)
			AddControl (pSect->m_pName);
		pSect = pSect->m_pNext;
	}

	return true;
}

bool CPlayBar::AddControl (char * pName)
{
	char * pType = m_pConfig->GetItemText (pName, "Type", NULL);
	if (pType == NULL)
		return false;

	if (strcmp (pType, "Button") && strcmp (pType, "Slider"))
		return false;

	CPlayCtrl * pCtrl = NULL;
	if (!strcmp (pType, "Button"))
		 pCtrl = new CPlayButton (m_hInst);
	else if (!strcmp (pType, "Slider"))
		 pCtrl = new CPlaySlider (m_hInst);
	else
		return false;

	pCtrl->Create (m_hWnd, m_pConfig, pName);

	if (!strcmp (pName, "BtnPlay"))
		m_pBtnPlay = (CPlayButton *)pCtrl;
	else if (!strcmp (pName, "BtnPause"))
		m_pBtnPause = (CPlayButton *)pCtrl;
	else if (!strcmp (pName, "BtnFull"))
		m_pBtnFull = (CPlayButton *)pCtrl;
	else if (!strcmp (pName, "BtnNormal"))
		m_pBtnNormal = (CPlayButton *)pCtrl;
	else if (!strcmp (pName, "SldAudio"))
		m_pSldAudio = (CPlaySlider *)pCtrl;
	else if (!strcmp (pName, "SldPos"))
		m_pSldPos = (CPlaySlider *)pCtrl;

	m_lstCtrl.AddTail (pCtrl);

	return true;
}

bool CPlayBar::UpdateControl (bool bUpdate)
{
	CPlayCtrl * pCtrl = NULL;
	NODEPOS pos = m_lstCtrl.GetHeadPosition ();
	while (pos != NULL)
	{
		pCtrl = m_lstCtrl.GetNext (pos);
		pCtrl->NeedUpdate (bUpdate);
	}
	return true;
}
