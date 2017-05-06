/*******************************************************************************
	File:		CSubtitleBase.cpp

	Contains:	subtitle base implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-03		Fenger			Create file

*******************************************************************************/
#include "CSubtitleBase.h"

#include "UStringFunc.h"
#include "UFileFunc.h"

#include "yyConfig.h"
#include "yyLog.h"

#ifdef _OS_NDK
#include "iconv.h"
#endif // _OS_NDK

CSubtitleBase::CSubtitleBase(void)
	: CBaseObject ()
	, m_nCharset (0)
	, m_nTrackNum (0)
	, m_posItem (NULL)
	, m_pPosItem (NULL)
	, m_nFileSize (0)
	, m_pFileBuff (NULL)
	, m_nBuffSize (0)
	, m_hTextFont (NULL)
	, m_nFontSize (-1)
	, m_nFontColor (-1)
{
	SetObjectName ("CSubtitleBase");
}

CSubtitleBase::~CSubtitleBase(void)
{
	if (m_pFileBuff != NULL)
		delete []m_pFileBuff;
	m_pFileBuff = NULL;

	ReleaseItems ();

#ifdef _OS_WIN32
	if (m_hTextFont != NULL)
		DeleteObject (m_hTextFont);
	m_hTextFont = NULL;
#endif // _OS_WIN32
}

int CSubtitleBase::Parse (CBaseFile * pFile)
{
	if (pFile == NULL)
		return YY_ERR_FAILED;

	m_nFileSize = pFile->GetSize ();
	if (m_nFileSize <= 32)
	{
		YYLOGE ("The file size %d was too small!", m_nFileSize);
		return YY_ERR_FAILED;
	}

	char *	pFileBuff = new char[m_nFileSize];
	if (pFile->read (NULL, (unsigned char *)pFileBuff, m_nFileSize) != m_nFileSize)
	{	
		YYLOGE ("It can not read the data frrom file!");
		return YY_ERR_FAILED;
	}

	if (m_pFileBuff != NULL)
		delete []m_pFileBuff;

//	if (pFileBuff[0] == 0XFF && pFileBuff[1] == 0XFE)
	if (pFileBuff[0] == -1 && pFileBuff[1] == -2)
	{
		YYLOGI ("It is UNICODE text file!");
		m_nCharset = 1;
#ifdef _OS_WIN32
		m_nBuffSize = (m_nFileSize - 2) / 2;
		m_pFileBuff = new TCHAR[m_nBuffSize];
		memcpy (m_pFileBuff, pFileBuff + 2, m_nBuffSize * 2);
#elif defined _OS_NDK
		iconv_t hConv = iconv_open ("UTF-8", "UTF-16LE");
		if (hConv == NULL)
			return YY_ERR_FAILED;
		m_nBuffSize = (m_nFileSize - 2) * 3;	
		m_pFileBuff = new TCHAR[m_nBuffSize];
		memset (m_pFileBuff,  0, m_nBuffSize);
		
		size_t 	nFileSize = m_nFileSize - 2;
		char * 	pFileData = pFileBuff + 2;	
		size_t	nBuffize = m_nBuffSize;
		char * 	pBuffeData = m_pFileBuff;	
		size_t err = iconv (hConv, &pFileData, &nFileSize, &pBuffeData, &nBuffize);
//		m_nBuffSize = strlen (m_pFileBuff);
		m_nBuffSize = m_nBuffSize - nBuffize;
		iconv_close (hConv);
#endif // _OS_#@
	}
	else
	{
		YYLOGI ("It is normal text file!");
		m_nCharset = 2;
#ifdef _OS_WIN32
		m_nBuffSize = m_nFileSize + 1;
		m_pFileBuff = new TCHAR[m_nBuffSize];
		memset (m_pFileBuff, 0, m_nBuffSize * sizeof (TCHAR));
		MultiByteToWideChar (CP_ACP, 0, pFileBuff, -1, m_pFileBuff, m_nBuffSize);
		m_nBuffSize = _tcslen (m_pFileBuff);
#else
		m_nBuffSize = m_nFileSize;
		m_pFileBuff = new TCHAR[m_nBuffSize];
		memcpy (m_pFileBuff, pFileBuff, m_nBuffSize);	
#endif // _OS_WIN32		
	}

	delete []pFileBuff;

	return YY_ERR_NONE;
}

int CSubtitleBase::SetPos (int nPos)
{
	return YY_ERR_NONE;
}

CSubtitleItem *	CSubtitleBase::GetItem (long long llPos)
{
	if (m_pPosItem == NULL)
	{
		if (m_posItem == NULL)
			m_posItem = m_lstText.GetHeadPosition ();
		m_pPosItem = m_lstText.GetNext (m_posItem);
		if (m_pPosItem == NULL)
			return NULL;
	}

	if (m_pPosItem->m_llStart <= llPos && llPos <= m_pPosItem->m_llEnd)
		return m_pPosItem;
	else if (m_pPosItem->m_llPrevEnd < llPos && llPos < m_pPosItem->m_llStart)
		return NULL;

	while (llPos > m_pPosItem->m_llEnd)
	{
		m_pPosItem = m_lstText.GetNext (m_posItem);
		if (m_pPosItem == NULL)
			return NULL;

		if (m_pPosItem->m_llStart <= llPos && llPos <= m_pPosItem->m_llEnd)
			return m_pPosItem;
		else if (m_pPosItem->m_llPrevEnd < llPos && llPos < m_pPosItem->m_llStart)
			return NULL;
	}

	m_posItem = m_lstText.GetHeadPosition ();
	m_pPosItem = m_lstText.GetNext (m_posItem);
	while (m_pPosItem != NULL)
	{
		if (m_pPosItem->m_llStart <= llPos && llPos <= m_pPosItem->m_llEnd)
			return m_pPosItem;
		else if (m_pPosItem->m_llPrevEnd < llPos && llPos < m_pPosItem->m_llStart)
			return NULL;

		m_pPosItem = m_lstText.GetNext (m_posItem);
	}

	return NULL;
}

int CSubtitleBase::GetFontSize (void)
{
	return m_nFontSize;
}

int CSubtitleBase::GetFontColor (void)
{
	return m_nFontColor;
}

HFONT CSubtitleBase::GetFontHandle (void)
{
	return m_hTextFont;
}

int CSubtitleBase::CreateTxtFont (void)
{
#ifdef _OS_WIN32
	TCHAR	szFontName[256];
	memset (szFontName, 0, sizeof (szFontName));
	HDC hdc = GetDC (NULL);
	GetTextFace (hdc, 256, szFontName);
	ReleaseDC (NULL, hdc);

    LOGFONT lf; 

	lf.lfHeight = -m_nFontSize;
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
//	_tcscpy (lf.lfFaceName, _T("Courier New"));
	_tcscpy (lf.lfFaceName, szFontName);

	if (m_hTextFont != NULL)
		DeleteObject (m_hTextFont);
    m_hTextFont = CreateFontIndirect(&lf); 
#endif // _OS_WIN32
	return YY_ERR_NONE;
}

int CSubtitleBase::CheckTimeStamp (void)
{
	NODEPOS pos = m_lstText.GetHeadPosition ();
	CSubtitleItem * pItem1 = m_lstText.GetNext (pos);
	CSubtitleItem * pItem2 = NULL;

	if (pItem1 != NULL)
		pItem1->m_llPrevEnd = 0;

	while (pItem1 != NULL)
	{
		pItem2 = m_lstText.GetNext (pos);
		if (pItem2 == NULL)
			break;

		if (pItem2->m_llStart < 0 && pItem2->m_nLineNum > 0)
		{
			if (pItem1->m_nLineNum > 0 && pItem1->m_llEnd > 0)
				pItem2->m_llStart = pItem1->m_llEnd;
		}

		if ((pItem1->m_llEnd < 0 || pItem1->m_llEnd > pItem2->m_llStart) && pItem1->m_nLineNum > 0)
		{
			if (pItem2->m_nLineNum > 0 && pItem2->m_llStart > 0)
				pItem1->m_llEnd = pItem2->m_llStart;
		}

		pItem2->m_llPrevEnd = pItem1->m_llEnd;
		if (pItem2->m_llPrevEnd == -1)
			pItem2->m_llPrevEnd = pItem2->m_llPrevEnd;

		pItem1 = pItem2;
	}

	return YY_ERR_NONE;
}

int CSubtitleBase::ReleaseItems (void)
{
	CSubtitleItem * pItem = m_lstText.RemoveHead ();
	while (pItem != NULL)
	{
		delete pItem;
		pItem = m_lstText.RemoveHead ();
	}
	return 0;
}

int CSubtitleBase::ReadLine (TCHAR * pText, int nTextLen, TCHAR * pLine, int nSize)
{
	TCHAR *	pFind = pText;
	int		nLineLen = 0;

	_tcscpy (pLine, _T(""));
	while (*pFind == _T('\r') || *pFind == _T('\n'))
	{
		if (pFind - pText >= nTextLen)
			return -1;
		pFind++;
	}
	if (pFind != pText)
	{
		nLineLen = pFind - pText;
		return nLineLen;
	}

	pText = pFind;
	while (pFind - pText < nTextLen)
	{
		if (*pFind == _T('\r') || *pFind == _T('\n'))
			break;
		pFind++;
	}

	if (pFind - pText < nSize)
	{
		*pFind++ = 0;
		if (*pFind == _T('\r') || *pFind == _T('\n'))
			*pFind++ = 0;
		nLineLen = pFind - pText;
		
		while (*pText == _T(' '))
			pText++;
		while (*(pText + _tcslen (pText) - 1) == _T(' '))
			*(pText + _tcslen (pText) - 1) = 0;

		_tcscpy (pLine, pText);
		return nLineLen;
	}
	else
	{
		return -1;
	}
}

int CSubtitleBase::CharReadLine (char * pText, int nTextLen, char * pLine, int nSize)
{
	char *	pFind = pText;
	int		nLineLen = 0;

	strcpy (pLine, (""));
	while (*pFind == ('\r') || *pFind == ('\n'))
	{
		if (pFind - pText >= nTextLen)
			return -1;
		pFind++;
	}
	if (pFind != pText)
	{
		nLineLen = pFind - pText;
		return nLineLen;
	}

	pText = pFind;
	while (pFind - pText < nTextLen)
	{
		if (*pFind == ('\r') || *pFind == ('\n'))
			break;
		pFind++;
	}

	if (pFind - pText < nSize)
	{
		*pFind++ = 0;
		if (*pFind == ('\r') || *pFind == ('\n'))
			*pFind++ = 0;
		nLineLen = pFind - pText;
		
		while (*pText == (' '))
			pText++;
		while (*(pText + strlen (pText) - 1) == (' '))
			*(pText + strlen (pText) - 1) = 0;

		strcpy (pLine, pText);
		return nLineLen;
	}
	else
	{
		return -1;
	}
}

#ifdef _OS_WIN32
int __cdecl CSubtitleBase::compare_starttime (const void *arg1, const void *arg2)
#else
int CSubtitleBase::compare_starttime (const void *arg1, const void *arg2)
#endif // _OS_WIN32
{
	CSubtitleItem * pItem1 = *(CSubtitleItem **)arg1;
	CSubtitleItem * pItem2 = *(CSubtitleItem **)arg2;

	return (int)(pItem1->m_llStart - pItem2->m_llStart);
}
