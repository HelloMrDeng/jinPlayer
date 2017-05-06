/*******************************************************************************
	File:		CSubtitleSrt.cpp

	Contains:	subtitle srt implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-17		Fenger			Create file

*******************************************************************************/
#include "stdio.h"

#include "CSubtitleSrt.h"

#include "UStringFunc.h"

#include "yyConfig.h"
#include "yyLog.h"

CSubtitleSrt::CSubtitleSrt(void)
	: CSubtitleBase ()
{
	SetObjectName ("CSubtitleSrt");
}

CSubtitleSrt::~CSubtitleSrt(void)
{
}

int CSubtitleSrt::Parse (CBaseFile * pFile)
{
	int nRC = CSubtitleBase::Parse (pFile);
	if (nRC != YY_ERR_NONE)
		return nRC;

	ReleaseItems ();

	CSubtitleItem *	pItem = NULL;
	int		nTextIndex = -1;
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
			if (_tcslen (szLine) <= 0)
			{
				nTextIndex = -1;
				llStart = -1;

				if (pItem != NULL)
				{					
					if (pItem->GetLineNum () > 0)
						m_lstTemp.AddTail (pItem);
					else
						delete pItem;
					pItem = NULL;
				}
				continue;
			}
			
			if (nTextIndex < 0)
			{
				_stscanf (szLine, _T("%d"), &nTextIndex);
				pItem = new CSubtitleItem ();
			}
			else if (llStart < 0)
			{
				_stscanf (szLine, _T("%02d:%02d:%02d,%03d"), &nHour, &nMin, &nSec, &nMS);
				llStart = nHour * 3600000 + nMin * 60000 + nSec * 1000 + nMS;
				llEnd = -1;
				TCHAR * pEnd = _tcsstr (szLine, _T("-->"));
				if (pEnd != NULL)
				{
					pEnd += 3;
					while (*pEnd == _T(' '))
						pEnd++;
					_stscanf (pEnd, _T("%02d:%02d:%02d,%03d"), &nHour, &nMin, &nSec, &nMS);
					llEnd = nHour * 3600000 + nMin * 60000 + nSec * 1000 + nMS;
				}
			}
			else
			{
				if (llStart < 0 || llEnd < 0)
					llStart = llStart;

				if (pItem != NULL)
					pItem->AddText (szLine, llStart, llEnd, nTextIndex);
			}
		}
	}

	int nCount = m_lstTemp.GetCount ();
	if (nCount <= 0)
	{
		YYLOGE ("There was not content in file!");
		return YY_ERR_FAILED;
	}

	CSubtitleItem ** ppItems = new CSubtitleItem *[nCount];
	NODEPOS pos = m_lstTemp.GetHeadPosition ();
	int i = 0;
	for (i = 0; i < nCount; i++)
		ppItems[i] = m_lstTemp.GetNext (pos);

	qsort (ppItems, nCount, sizeof(CSubtitleItem *), compare_starttime);

	for (i = 0; i < nCount; i++)
		m_lstText.AddTail (ppItems[i]);

	delete []ppItems;

	CheckTimeStamp ();

	return YY_ERR_NONE;
}
