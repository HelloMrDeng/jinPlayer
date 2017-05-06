/*******************************************************************************
	File:		CSubtitleFFMpeg.cpp

	Contains:	subtitle srt implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-17		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#include "CSubtitleFFMpeg.h"

#include "UStringFunc.h"
#include "stdio.h"

#include "yyConfig.h"
#include "yyLog.h"

CSubtitleFFMpeg::CSubtitleFFMpeg(void)
	: CSubtitleBase ()
	, m_pSrc (NULL)
	, m_pDec (NULL)
	, m_pAVSub (NULL)
	, m_llStart (-1)
	, m_llEnd (-1)
	, m_pSubItem (NULL)
{
	SetObjectName ("CSubtitleFFMpeg");
	memset (&m_bufSrc, 0, sizeof (m_bufSrc));
	m_bufSrc.nType = YY_MEDIA_SubTT;
	memset (&m_bufDec, 0, sizeof (m_bufDec));
	m_bufDec.nType = YY_MEDIA_SubTT;
}

CSubtitleFFMpeg::~CSubtitleFFMpeg(void)
{
	YY_DEL_P (m_pDec);
}

int CSubtitleFFMpeg::Parse (CBaseSource * pSource)
{
	m_nCharset = 0;
	if (pSource == NULL)
		return YY_ERR_FAILED;
	if (pSource->GetStreamCount (YY_MEDIA_SubTT) <= 0)
		return YY_ERR_FAILED;

	m_pSrc = pSource;
	YY_SUBTT_FORMAT * pFmt = m_pSrc->GetSubTTFormat ();
	if (pFmt == NULL)
		return YY_ERR_FAILED;
	YY_DEL_P (m_pDec);
	m_pDec = new CFFMpegSubTTDec (NULL);
	if (m_pDec->Init (pFmt) != YY_ERR_NONE)
		return YY_ERR_FAILED;

	return YY_ERR_NONE;
}

int CSubtitleFFMpeg::SetPos (int nPos)
{
	CAutoLock lock (&m_mtSeek);
	m_subItem1.Reset ();
	m_subItem1.Reset ();
	m_pSubItem = NULL;
	return YY_ERR_NONE;
}

CSubtitleItem *	CSubtitleFFMpeg::GetItem (long long llTime)
{
	CAutoLock lock (&m_mtSeek);
	if (m_pSubItem != NULL)
	{
		m_pSubItem->GetTime (&m_llStart, &m_llEnd);
		if (llTime >= m_llStart && llTime <= m_llEnd)
			return m_pSubItem;
		if (llTime < m_llStart)
			return NULL;
	}

	m_bufSrc.uFlag = 0;
	m_bufSrc.llTime = 0;
	int nRC = m_pSrc->ReadData (&m_bufSrc);
	if (nRC != YY_ERR_NONE)
		return NULL;

	nRC = m_pDec->SetBuff (&m_bufSrc);
	nRC = m_pDec->GetBuff (&m_bufDec);
	if (nRC != YY_ERR_NONE)
		return NULL;

	m_pAVSub = (AVSubtitle *)m_bufDec.pBuff;
	if (m_pAVSub->format == 0) // graphics
		return NULL;

	if (m_pSubItem != &m_subItem1)
		m_pSubItem = &m_subItem1;
	else
		m_pSubItem = &m_subItem2;
	m_pSubItem->Reset ();
	AVSubtitleRect *	pSub = NULL;
	for (int i = 0; i < m_pAVSub->num_rects; i++)
	{
		pSub = m_pAVSub->rects[i];
		if (pSub->type == SUBTITLE_TEXT)
		{
#ifdef _OS_WIN32
			memset (m_szText, 0, sizeof (m_szText));
			MultiByteToWideChar (CP_ACP, 0, pSub->text, -1, m_szText, sizeof (m_szText));
			m_pSubItem->AddText (m_szText, m_pAVSub->start_display_time, m_pAVSub->end_display_time);
#else
			m_pSubItem->AddText (pSub->text, m_pAVSub->start_display_time, m_pAVSub->end_display_time);
#endif // _OS_WIN32
		}
		else if (pSub->type == SUBTITLE_ASS)
		{
			ParseAss (pSub->ass);
		}
		else
		{
			return NULL;
		}
	}
	
	m_pSubItem->GetTime (&m_llStart, &m_llEnd);
	if (llTime >= m_llStart && llTime <= m_llEnd)
		return m_pSubItem;
	else
		return NULL;
}

int CSubtitleFFMpeg::ParseAss (char * pAssText)
{
	int		nIndex = 0;
	int		nTextIndex = 0;
	int		nIdxStart = 1, nIdxEnd = 2, nIdxText = 9;
	int		nHour, nMin, nSec, nMS;
	long long llStart = -1, llEnd = -1;
	TCHAR * pDot = NULL;
	TCHAR * pText = NULL;
	TCHAR	szText[1024];
	TCHAR	szLine[1024];
	memset (szLine, 0, sizeof (szLine));
#ifdef _OS_WIN32
	MultiByteToWideChar (CP_UTF8, 0, pAssText, -1, szLine, sizeof (szLine));
#else
	strcpy (szLine, pAssText);
#endif // _OS_WIN32
	if (!_tcsncmp (szLine, _T("Dialogue:"), 9))
	{
//		szLine[8] = _T(',');
		pText = NULL;
		nIndex = 0;

		pDot = szLine;
		pDot = _tcschr (pDot, _T(','));
		while (pDot != NULL)
		{
			nIndex++;
			pDot++;
			while (*pDot == _T(' '))
				pDot++;

			if (nIndex == nIdxStart)
			{
				_stscanf (pDot, _T("%01d:%02d:%02d.%02d"), &nHour, &nMin, &nSec, &nMS);
				llStart = nHour * 3600000 + nMin * 60000 + nSec * 1000 + nMS * 10;
			}
			else if (nIndex == nIdxEnd)
			{
				_stscanf (pDot, _T("%01d:%02d:%02d.%02d"), &nHour, &nMin, &nSec, &nMS);
				llEnd = nHour * 3600000 + nMin * 60000 + nSec * 1000 + nMS * 10;
			}
			else if (nIndex == nIdxText)
			{
				pText = pDot;
			}
			
			pDot = _tcschr (pDot, _T(','));
		}

		if (pText != NULL && llEnd > 0 && llStart > 0)
		{
			nTextIndex++;
			memset (szText, 0, sizeof (szText));
			TCHAR * pTemp = pText;
			while (pTemp != NULL)
			{
				if (*pTemp == _T('{'))
				{
					if (pTemp > pText)
						_tcsncat (szText, pText, pTemp - pText);

					while (pTemp != NULL)
					{
						pTemp++;
						if (*pTemp == _T('}'))
						{
							pText = pTemp + 1;
							break;
						}
					}
				}

				pTemp++;
				if (*pTemp == 0)
					break;
			}

			_tcscat (szText, pText);

			memset (szLine, 0, sizeof (szLine));
			_tcscpy (szLine, szText);
			pTemp = szLine;
			pText = szLine;
			int nLen = _tcslen (szLine);
			while (pTemp != NULL)
			{
				if (pTemp - szLine >= nLen)
					break;

				if (!_tcsncmp (pTemp, _T("\\N"), 2) || 
					!_tcsncmp (pTemp, _T("\\n"), 2) ||
					!_tcsncmp (pTemp, _T("\\h"), 2))
				{
					*pTemp = 0;
					pTemp += 2;
					m_pSubItem->AddText (pText, llStart, llEnd, nTextIndex);
					pText = pTemp;
				}

				pTemp++;
			}
			m_pSubItem->AddText (pText, llStart, llEnd, nTextIndex);
		}
	}
	else
	{
		return YY_ERR_FAILED;
	}

	return YY_ERR_NONE;
}