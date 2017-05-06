/*******************************************************************************
	File:		CSubtitleAss.cpp

	Contains:	subtitle srt implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-17		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#include "CSubtitleAss.h"

#include "UStringFunc.h"
#include "stdio.h"

#include "yyConfig.h"
#include "yyLog.h"

CSubtitleAss::CSubtitleAss(void)
	: CSubtitleBase ()
{
	SetObjectName ("CSubtitleAss");
}

CSubtitleAss::~CSubtitleAss(void)
{
}

int CSubtitleAss::Parse (CBaseFile * pFile)
{
	int nRC = CSubtitleBase::Parse (pFile);
	if (nRC != YY_ERR_NONE)
		return nRC;

	ReleaseItems ();

	bool	bEventField = false;
	int		nIdxStart = 1, nIdxEnd = 2, nIdxText = 9;
	TCHAR * pDot = NULL;
	TCHAR * pText = NULL;
	TCHAR	szText[1024];
	int		nIndex = 0;

	CSubtitleItem *	pItem = NULL;
	int		nTextIndex = 0;
	int		nHour, nMin, nSec, nMS;
	long long	llStart = -1, llEnd = -1;

	TCHAR	szLine[1024];
	int		nLineLen = 0;
	int		nRestLen = m_nBuffSize;
	TCHAR * pPos = m_pFileBuff;
	while (pPos - m_pFileBuff < m_nBuffSize)
	{
		memset (szLine, 0, sizeof (szLine));
		nLineLen = ReadLine (pPos, nRestLen, szLine, 1024);
		nRestLen -= nLineLen;
		if (nLineLen <= 0)
		{
			pPos++;
		}
		else
		{
			pPos += nLineLen;

			if (!_tcscmp (szLine, _T("[Events]")))
			{
				bEventField = true;
				continue;
			}
			if (!bEventField)
				continue;
			if (szLine[0] == _T('['))
				break;

			if (!_tcsncmp (szLine, _T("Format:"), 7))
			{
				szLine[6] = _T(',');

				pDot = szLine;
				pDot = _tcschr (pDot, _T(','));
				while (pDot != NULL)
				{
					nIndex++;
					pDot++;
					while (*pDot == _T(' '))
						pDot++;
					
					if (!_tcsncmp (pDot, _T("Start"), 5))
						nIdxStart = nIndex;
					else if (!_tcsncmp (pDot, _T("End"), 3))
						nIdxEnd = nIndex;
					else if (!_tcsncmp (pDot, _T("Text"), 4))
						nIdxText = nIndex;

					pDot = _tcschr (pDot, _T(','));
				}
			}
			else if (!_tcsncmp (szLine, _T("Dialogue:"), 9))
			{
				szLine[8] = _T(',');
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
					pItem = new CSubtitleItem ();
					m_lstTemp.AddTail (pItem);
					nTextIndex++;

					if (nTextIndex > 500)
						nTextIndex = nTextIndex;

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
							pItem->AddText (pText, llStart, llEnd, nTextIndex);
							pText = pTemp;
						}

						pTemp++;
					}

					pItem->AddText (pText, llStart, llEnd, nTextIndex);
				}

			}
		}
	}

	int nCount = m_lstTemp.GetCount ();
	if (nCount <= 0)
		return YY_ERR_FAILED;

	CSubtitleItem ** ppItems = new CSubtitleItem *[nCount];
	NODEPOS pos = m_lstTemp.GetHeadPosition ();
	int i = 0;
	for (i = 0; i < nCount; i++)
		ppItems[i] = m_lstTemp.GetNext (pos);

	qsort(ppItems, nCount, sizeof(CSubtitleItem *), compare_starttime);
	
	for (i = 0; i < nCount; i++)
		m_lstText.AddTail (ppItems[i]);

	delete []ppItems;

/*
	pos = m_lstText.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstText.GetNext (pos);
		_stprintf (szLine, _T("%01d:%02d:%02d.%03d % 8d\r\n"), (int)(pItem->m_llStart / 3600000), 
						((int)pItem->m_llStart % 3600000) / 60000, ((int)pItem->m_llStart % 60000) / 1000, 
						(int)pItem->m_llStart % 1000, (int)pItem->m_llStart);

		OutputDebugString (szLine);
	}
*/
	CheckTimeStamp ();

	return YY_ERR_NONE;
}

