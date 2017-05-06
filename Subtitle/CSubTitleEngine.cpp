/*******************************************************************************
	File:		CSubtitleEngine.cpp

	Contains:	subtitle engine implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-03		Fenger			Create file

*******************************************************************************/
#include "CSubtitleEngine.h"

#include "CSubtitleSrt.h"
#include "CSubtitleAss.h"
#include "CSubtitleSmi.h"
#include "CSubtitleFFMpeg.h"

#include "CBaseFile.h"
#include "UStringFunc.h"
#include "USystemFunc.h"

#include "yyConfig.h"
#include "yyLog.h"

CSubtitleEngine::CSubtitleEngine(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_pMediaSrc (NULL)
	, m_pClock (NULL)
	, m_hView (NULL)
	, m_nEnable (0)
	, m_pExtRnd (NULL)
	, m_pExtDraw (NULL)
	, m_hRndThread (NULL)
	, m_bWorking (false)
	, m_pSource (NULL)
	, m_pDrawItem (NULL)
	, m_llLastTime (-1)
	, m_hMemDC (NULL)
	, m_hBmpText (NULL)
	, m_pBmpBuff (NULL)
	, m_nBmpWidth (0)
	, m_nBmpHeight (0)
	, m_hBmpOld (NULL)
#ifdef _OS_WINPC
	, m_nTxtSize (28)
#else
	, m_nTxtSize (28)
#endif // _OS_WINPC
	, m_nTxtColor (0XFFFFFF00)
	, m_hTxtFont (NULL)
	, m_nExtSize (0)
	, m_nExtColor (-1)
	, m_hExtFont (NULL)
	, m_nBkgColor (10)
	, m_bFontChanged (false)
{
	SetObjectName ("CSubtitleEngine");
	memset (&m_rcView, 0, sizeof (m_rcView));
	memset (&m_rcBmp, 0, sizeof (m_rcBmp));
	memset (&m_extBuff, 0, sizeof (m_extBuff));
	m_extBuff.uBuffSize = 4096;
	m_extBuff.pBuff = new unsigned char[m_extBuff.uBuffSize];
	memset (m_extBuff.pBuff, 0, m_extBuff.uBuffSize);
}

CSubtitleEngine::~CSubtitleEngine(void)
{
	Close ();
	YY_DEL_A (m_extBuff.pBuff);
#ifdef _OS_WIN32	
	ReleaseBMP ();
	if (m_hTxtFont != NULL)
		DeleteObject (m_hTxtFont);
	m_hTxtFont = NULL;
#endif // _OS_WIN32	
}

int CSubtitleEngine::SetClock (CBaseClock * pClock)
{
	m_pClock = pClock;
	return YY_ERR_NONE;
}

int CSubtitleEngine::Open (const TCHAR * pSource)
{
	if (pSource == NULL)
		return YY_ERR_ARG;
	TCHAR * pDot = _tcsrchr ((TCHAR*)pSource, _T('.'));
	if (pDot == NULL)
		return YY_ERR_SOURCE;

	CBaseFile * pFile = new CBaseFile ();
	TCHAR szFile[1024];
	_tcscpy (szFile, pSource);
	pDot = _tcsrchr (szFile, _T('.'));
	*pDot = 0;
	_tcscat (szFile, _T(".srt"));
	if (pFile->open (NULL, szFile, AVIO_FLAG_READ) >= 0)
	{
		m_pSource = new CSubtitleSrt ();
	}
	else
	{
		_tcscpy (szFile, pSource);
		_tcscat (szFile, _T(".srt"));
		if (pFile->open (NULL, szFile, AVIO_FLAG_READ) >= 0)
			m_pSource = new CSubtitleSrt ();
	}
	if (m_pSource == NULL)
	{
		_tcscpy (szFile, pSource);
		pDot = _tcsrchr (szFile, _T('.'));
		*pDot = 0;
		_tcscat (szFile, _T(".ass"));
		if (pFile->open (NULL, szFile, AVIO_FLAG_READ) >= 0)
		{
			m_pSource = new CSubtitleAss ();
		}
		else
		{
			_tcscpy (szFile, pSource);
			_tcscat (szFile, _T(".ass"));
			if (pFile->open (NULL, szFile, AVIO_FLAG_READ) >= 0)
				m_pSource = new CSubtitleAss ();
		}
	}
	if (m_pSource == NULL)
	{
		_tcscpy (szFile, pSource);
		pDot = _tcsrchr (szFile, _T('.'));
		*pDot = 0;
		_tcscat (szFile, _T(".ssa"));
		if (pFile->open (NULL, szFile, AVIO_FLAG_READ) >= 0)
		{
			m_pSource = new CSubtitleAss ();
		}
		else
		{
			_tcscpy (szFile, pSource);
			_tcscat (szFile, _T(".ssa"));
			if (pFile->open (NULL, szFile, AVIO_FLAG_READ) >= 0)
				m_pSource = new CSubtitleAss ();
		}
	}
	if (m_pSource == NULL)
	{
		_tcscpy (szFile, pSource);
		pDot = _tcsrchr (szFile, _T('.'));
		*pDot = 0;
		_tcscat (szFile, _T(".smi"));
		if (pFile->open (NULL, szFile, AVIO_FLAG_READ) >= 0)
		{
			m_pSource = new CSubtitleSmi ();
		}
		else
		{
			_tcscpy (szFile, pSource);
			_tcscat (szFile, _T(".smi"));
			if (pFile->open (NULL, szFile, AVIO_FLAG_READ) >= 0)
				m_pSource = new CSubtitleSmi ();
		}
	}
	int nRC = YY_ERR_IMPLEMENT;
	if (m_pSource != NULL)
	{
		if (m_pSource->Parse (pFile) == YY_ERR_NONE)
			nRC = YY_ERR_NONE;
		else
			nRC = YY_ERR_FAILED;
	}
	if (pFile != NULL)
	{
		pFile->close (NULL);
		delete pFile;
	}
	if (nRC != YY_ERR_NONE)
	{
		delete m_pSource;
		m_pSource = new CSubtitleFFMpeg ();
		nRC = ((CSubtitleFFMpeg *)m_pSource)->Parse (m_pMediaSrc);
	}
	
	return nRC;
}

int CSubtitleEngine::Close (void)
{
	Stop ();

	if (m_pSource != NULL)
		delete m_pSource;
	m_pSource = NULL;

	return YY_ERR_NONE;
}

int CSubtitleEngine::SetPos (int nPos)
{
	CAutoLock lock (&m_mtDraw);
	if (m_pSource != NULL)
		m_pSource->SetPos (nPos);
#ifdef _OS_WIN32
	 if (m_pDrawItem != NULL && m_pBmpBuff != NULL)
	{
		memset (m_pBmpBuff, m_nBkgColor, m_nBmpWidth * m_nBmpHeight * 4);
		HDC hDC = GetDC ((HWND)m_hView);
		RECT rcView;
		GetClientRect ((HWND)m_hView, &rcView);
		BitBlt (hDC, rcView.left, rcView.bottom - m_nBmpHeight, m_nBmpWidth, rcView.bottom, m_hMemDC, 0, 0, SRCCOPY);
		ReleaseDC ((HWND)m_hView, hDC);
	}
#endif // _OS_WIN32
	return YY_ERR_NONE;
}

int CSubtitleEngine::Enable (int nEnable)
{
	m_nEnable = nEnable;
	return 0;
}

int CSubtitleEngine::SetExtRnd (YY_DATACB * pDataCB)
{
	if (m_pExtRnd = pDataCB)
		return YY_ERR_NONE;

	CAutoLock lock (&m_mtDraw);
	m_pExtRnd = pDataCB;

	return YY_ERR_NONE;
}

int CSubtitleEngine::SetExtDraw (YYSUB_ExtDraw * pExtDraw)
{
	if (m_pExtDraw = pExtDraw)
		return YY_ERR_NONE;

	CAutoLock lock (&m_mtDraw);
	m_pExtDraw = pExtDraw;

	return YY_ERR_NONE;
}

int CSubtitleEngine::SetView (void * hView)
{
	if (m_hView == hView)
		return YY_ERR_NONE;

	m_hView = hView;

#ifdef _OS_WIN32
	if (m_hTxtFont == NULL)
	{
		HDC hDC = GetDC ((HWND)m_hView);
		CreateTxtFont (hDC);
		ReleaseDC ((HWND)m_hView, hDC);
	}
#endif // _OS_WIN32

	return YY_ERR_NONE;
}

int CSubtitleEngine::SetFontSize (int nSize)
{
	CAutoLock lock (&m_mtDraw);
	if (m_nExtSize == nSize)
		return YY_ERR_NONE;

	m_nExtSize = nSize;
	m_bFontChanged = true;

	return 0;
}

int CSubtitleEngine::SetFontColor (int nColor)
{
	CAutoLock lock (&m_mtDraw);
	if (m_nExtColor == nColor)
		return YY_ERR_NONE;
	
	m_nExtColor = nColor;

	return YY_ERR_NONE;
}

int CSubtitleEngine::SetBackColor (int nColor)
{
	CAutoLock lock (&m_mtDraw);
	if (m_nBkgColor == nColor)
		return YY_ERR_NONE;
	
	m_nBkgColor = nColor;

	return YY_ERR_NONE;
}

int CSubtitleEngine::SetFontHandle (void * hFont)
{
	CAutoLock lock (&m_mtDraw);
	if (m_hExtFont == (HFONT)hFont)
		return YY_ERR_NONE;

	m_hExtFont = (HFONT)hFont;

	return YY_ERR_NONE;
}

int	CSubtitleEngine::Start (void)
{
	if (m_pSource == NULL)
		return YY_ERR_NONE;
	m_status = YY_PLAY_Run;
	if (m_hView != NULL)
	{
		if (m_hRndThread == NULL)
		{
			int nID = 0;
			yyThreadCreate (&m_hRndThread, &nID, RenderProc, this, 0);
		}
	}
	return YY_ERR_NONE;
}

int CSubtitleEngine::Pause (void)
{
	if (m_pSource == NULL)
		return YY_ERR_NONE;
	m_status = YY_PLAY_Pause;
	while (m_bWorking)
		yySleep (10000);
	return YY_ERR_NONE;
}

int	CSubtitleEngine::Stop (void)
{
	if (m_pSource == NULL)
		return YY_ERR_NONE;
	m_status = YY_PLAY_Stop;
	int nTryTimes = 0;
	while (m_hRndThread != NULL)
	{
		nTryTimes++;
		if (nTryTimes > 200)
			break;
		yySleep (10000);
	}
	return YY_ERR_NONE;
}

int CSubtitleEngine::SetParam (int nID, void * pParam)
{
	return YY_ERR_PARAMID;
}

int CSubtitleEngine::GetParam (int nID, void * pParam)
{
	return YY_ERR_PARAMID;
}

int CSubtitleEngine::GetCharset (void)
{
	if (m_pSource == NULL)
		return 0;
	return m_pSource->GetCharset ();
}

int CSubtitleEngine::GetItemText (long long llTime, YY_BUFFER * pTextBuff)
{
	if (pTextBuff == NULL)
		return YY_ERR_ARG;

	CSubtitleItem * pItem = m_pSource->GetItem (llTime);
	if (pItem == NULL|| pItem == m_pDrawItem)
		return YY_ERR_RETRY;
		
	m_pDrawItem = pItem;

	pTextBuff->nType = YY_MEDIA_SubTT;
	pTextBuff->llTime = pItem->m_llStart;
	pTextBuff->nValue = (int)(pItem->m_llEnd - pItem->m_llStart);
	if (pTextBuff->nValue <= 0)
		pTextBuff->nValue = 1;
	if (m_pClock != NULL)
		pTextBuff->llDelay = llTime - m_pClock->GetTime ();

	pTextBuff->uFlag = YYBUFF_TYPE_TEXT;
	TCHAR * pText = (TCHAR *)pTextBuff->pBuff;
	memset (pTextBuff->pBuff, 0, pTextBuff->uSize);
	int		nRestSize = pTextBuff->uSize;
	for (int i = 0; i < pItem->GetLineNum (); i++)
	{
		if (nRestSize < (_tcslen (pItem->GetText (i)) + 4) * sizeof (TCHAR))
			break;

		if (i == 0)
		{
			_tcscpy (pText, pItem->GetText (i));
			_tcscat (pText, _T("\r\n"));
		}
		else
		{
			_tcscat (pText, pItem->GetText (i));
			_tcscat (pText, _T("\r\n"));
		}

		nRestSize = pTextBuff->uSize - (_tcslen (pText) * sizeof (TCHAR));
	}
	pTextBuff->uSize = _tcslen (pText);
	m_llLastTime = pItem->m_llEnd;
	
	//YYLOGI ("SubTT Time: %d text: %s  ********", (int)llTime, pText);
	return YY_ERR_NONE;
}

int CSubtitleEngine::Draw (HDC hDC, RECT * pView, long long llTime, bool bOverlay)
{
	CAutoLock lock (&m_mtDraw);
	if (m_pSource == NULL || pView == NULL)
		return YY_ERR_STATUS;
#ifdef _OS_WIN32
	if (!bOverlay)
	{
		if (m_nEnable == 0)
			return YY_ERR_NONE;

		CSubtitleItem * pItem = m_pSource->GetItem (llTime);
		if (pItem == NULL)
			return YY_ERR_NONE;

		DrawItem (pItem, hDC, pView);

		return YY_ERR_NONE;
	}

	if (memcmp (&m_rcView, pView, sizeof (RECT)))
	{
		memcpy (&m_rcView, pView, sizeof (RECT));
		ReleaseBMP ();
	}

	if (m_hBmpText == NULL)
	{
		m_nBmpWidth = pView->right - pView->left;
		m_nBmpHeight = (pView->bottom - pView->top) / 2;
		m_hBmpText = CreateBMP (hDC, m_nBmpWidth, m_nBmpHeight, &m_pBmpBuff);
		if (m_hBmpText != NULL)
			m_hBmpOld = (HBITMAP) SelectObject (m_hMemDC, m_hBmpText);
		if (m_pBmpBuff != NULL)
			memset (m_pBmpBuff, m_nBkgColor, m_nBmpWidth * m_nBmpHeight * 4);
	}
	if (m_hBmpText == NULL || m_pBmpBuff == NULL)
		return YY_ERR_FAILED;

	if (m_nEnable <= 0)
	{
		if (m_nEnable == 0)
		{
			memset (m_pBmpBuff, m_nBkgColor, m_nBmpWidth * m_nBmpHeight * 4);
			BitBlt (hDC, pView->left, pView->bottom - m_nBmpHeight, m_nBmpWidth, pView->bottom, 
					m_hMemDC, 0, 0, SRCCOPY);
		}
		m_nEnable = -1;
		return YY_ERR_NONE;
	}

	CSubtitleItem * pItem = m_pSource->GetItem (llTime);

	if (pItem != m_pDrawItem)
	{
		m_pDrawItem = pItem;
		memset (m_pBmpBuff, m_nBkgColor, m_nBmpWidth * m_nBmpHeight * 4);

		if (m_pDrawItem != NULL)
			DrawItem (m_pDrawItem, m_hMemDC, &m_rcBmp);

		BitBlt (hDC, pView->left, pView->bottom - m_nBmpHeight, m_nBmpWidth, pView->bottom, 
				m_hMemDC, 0, 0, SRCCOPY);
	}
#endif // _OS_WIN32
	return YY_ERR_NONE;
}

#ifdef _OS_WIN32
int CSubtitleEngine::DrawItem (CSubtitleItem * pItem, HDC hDC, RECT * pRect)
{
	int nTxtLine = pItem->GetLineNum ();
	if (nTxtLine <= 0)
		return YY_ERR_FAILED;

	if (m_nExtColor > 0)
		SetTextColor (hDC, m_nExtColor);
	else if (pItem->m_nTxtColor > 0)
		SetTextColor (hDC, pItem->m_nTxtColor);
	else if (m_pSource->GetFontColor () > 0)
		SetTextColor (hDC, m_pSource->GetFontColor ());
	else
		SetTextColor (hDC, m_nTxtColor);

	HFONT hOldFont = NULL;
	if (m_hExtFont != NULL)
	{
		hOldFont = (HFONT) SelectObject (hDC, m_hExtFont);
	}
	else
	{
		if (m_hTxtFont == NULL || m_bFontChanged)
			CreateTxtFont (hDC);
		hOldFont = (HFONT) SelectObject (hDC, m_hTxtFont);
	}

	TCHAR * pText = pItem->GetText (0);
	SIZE szTxt;
	GetTextExtentPoint (hDC, pText, _tcslen (pText), &szTxt);

	SetBkMode (hDC, TRANSPARENT);

	RECT rcDraw;
	for (int i = nTxtLine; i > 0; i--)
	{
		pText = pItem->GetText (i-1);
		SetRect (&rcDraw, pRect->left, pRect->bottom - ((szTxt.cy + 8) * i), 
					pRect->right, pRect->bottom - ((szTxt.cy + 8) * (i - 1)));
		DrawText (hDC, pText, _tcslen (pText), &rcDraw, DT_CENTER | DT_BOTTOM);
	}

	if (hOldFont != NULL)
		SelectObject (hDC, hOldFont);

	return YY_ERR_NONE;
}

HBITMAP CSubtitleEngine::CreateBMP (HDC hDC, int nW, int nH, LPBYTE * pBmpBuff)
{
	int nBmpSize = sizeof(BITMAPINFOHEADER);
	BITMAPINFO * pBmpInfo = new BITMAPINFO ();
	pBmpInfo->bmiHeader.biSize			= nBmpSize;
	pBmpInfo->bmiHeader.biWidth			= nW;
	pBmpInfo->bmiHeader.biHeight		= -nH;
	pBmpInfo->bmiHeader.biBitCount		= (WORD)32;
	pBmpInfo->bmiHeader.biCompression	= BI_RGB;
	pBmpInfo->bmiHeader.biXPelsPerMeter	= 0;
	pBmpInfo->bmiHeader.biYPelsPerMeter	= 0;
	pBmpInfo->bmiHeader.biPlanes		= 1;

	int nStride = ((pBmpInfo->bmiHeader.biWidth * pBmpInfo->bmiHeader.biBitCount / 8) + 3) & ~3;
	pBmpInfo->bmiHeader.biSizeImage	= nStride * nH;

	if (m_hMemDC == NULL)
		m_hMemDC = CreateCompatibleDC (hDC);

	SetBkMode (m_hMemDC, TRANSPARENT);
	HBITMAP hBitmap = CreateDIBSection(m_hMemDC , pBmpInfo , DIB_RGB_COLORS , (void **)pBmpBuff, NULL , 0);;

	delete pBmpInfo;

	m_rcBmp.right = m_nBmpWidth;
	m_rcBmp.bottom = m_nBmpHeight;

	return hBitmap;
}

void CSubtitleEngine::ReleaseBMP (void)
{
	if (m_hMemDC != NULL)
	{
		if (m_hBmpOld != NULL)
			SelectObject (m_hMemDC, m_hBmpOld);
		DeleteDC (m_hMemDC);
		m_hMemDC = NULL;
	}

	if (m_hBmpText != NULL)
	{
		DeleteObject (m_hBmpText);
		m_hBmpText = NULL;
	}
}

int CSubtitleEngine::CreateTxtFont (HDC hDC)
{
	TCHAR	szFontName[256];
	memset (szFontName, 0, sizeof (szFontName));
	GetTextFace (hDC, 256, szFontName);

    LOGFONT lf; 

	if (m_nExtSize > 0)
		lf.lfHeight = -m_nExtSize;
	else
		lf.lfHeight = -m_nTxtSize;
	lf.lfWidth = 0;
	lf.lfEscapement	= 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_MEDIUM;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = GB2312_CHARSET;
	lf.lfOutPrecision = 1;
	lf.lfClipPrecision = 2;
	lf.lfQuality = 1;
	lf.lfPitchAndFamily	= 49;
#ifdef _OS_WINPC
	_tcscpy (lf.lfFaceName, _T("Courier New"));
#else
	_tcscpy (lf.lfFaceName, szFontName);
#endif // _OS_WINPC
	if (m_hTxtFont != NULL)
		DeleteObject (m_hTxtFont);
    m_hTxtFont = CreateFontIndirect(&lf); 

	return YY_ERR_NONE;
}
#endif // _OS_WIN32

int CSubtitleEngine::RenderProc (void * pParam)
{
	CSubtitleEngine * pEngine = (CSubtitleEngine *) pParam;
	yyThreadSetPriority (yyThreadGetCurHandle (), YY_THREAD_PRIORITY_NORMAL);

	return pEngine->RenderLoop ();
}

int CSubtitleEngine::RenderLoop (void)
{
	int nRC = YY_ERR_NONE;
	while (m_status == YY_PLAY_Run || m_status == YY_PLAY_Pause)
	{
		m_bWorking = false;
		if (m_status != YY_PLAY_Run)
		{
			yySleep (5000);
			continue;
		}
		yySleep (30000);
		CAutoLock lock (&m_mtDraw);
		m_bWorking = true;
		if (m_pExtRnd != NULL)
		{
			m_extBuff.uSize = m_extBuff.uBuffSize;
			long long llPlay = m_pClock->GetTime ();
			if (GetItemText (llPlay, &m_extBuff) == YY_ERR_NONE)
				m_pExtRnd->funcCB (m_pExtRnd->userData, &m_extBuff);
			else
			{
				if (m_llLastTime  > 0 && llPlay >= m_llLastTime)
				{
					m_extBuff.nValue = -1;	
					m_pExtRnd->funcCB (m_pExtRnd->userData, &m_extBuff);
					m_llLastTime = -1;
				}
			}
			continue;
		}
		else if (m_pExtDraw != NULL)
		{
			Draw (m_pExtDraw->hDC, m_pExtDraw->pRect, m_pExtDraw->llTime, m_pExtDraw->bOverLay);
			continue;
		}
#ifdef _OS_WIN32
		HDC hDC = GetDC ((HWND)m_hView);
		RECT rcRnd;
		GetClientRect ((HWND)m_hView, &rcRnd);
		if (m_pClock != NULL)
			Draw (hDC, &rcRnd, m_pClock->GetTime (), true);
		ReleaseDC ((HWND)m_hView, hDC);
#endif // _OS_WIN32
	}

	if (m_pExtRnd != NULL && m_pExtRnd->funcCB != NULL)
	{
		m_extBuff.uFlag = YYBUFF_EOS;
		m_pExtRnd->funcCB (m_pExtRnd->userData, &m_extBuff);
	}
	m_hRndThread = NULL;
	return 0;
}

