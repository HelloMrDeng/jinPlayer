/*******************************************************************************
	File:		CSubtitleSmi.cpp

	Contains:	subtitle srt implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-17		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#include "CSubtitleSmi.h"

#include "stdio.h"
#include "UStringFunc.h"

#include "yyConfig.h"
#include "yyLog.h"

CSubtitleSmi::CSubtitleSmi(void)
	: CSubtitleBase ()
{
	SetObjectName ("CSubtitleSmi");
}

CSubtitleSmi::~CSubtitleSmi(void)
{
}

int CSubtitleSmi::Parse (CBaseFile * pFile)
{
	ReleaseItems ();

	if (pFile == NULL)
		return YY_ERR_FAILED;

	m_nFileSize = pFile->GetSize ();
	if (m_nFileSize <= 32)
		return YY_ERR_FAILED;

	char *	pFileBuff = new char[m_nFileSize];
	if (pFile->read (NULL, (unsigned char *)pFileBuff, m_nFileSize) != m_nFileSize)
		return YY_ERR_FAILED;

	if (strncmp (pFileBuff, "<SAMI>", 6))
	{
		delete []pFileBuff;
		return YY_ERR_FAILED;
	}

	CSubtitleItem *	pItem = NULL;
	bool	bBodyFind = false;
	bool	bSyncLine = false;
	char *	pBegin, * pEnd;
	int		nStartTime = 0;

	TCHAR	wzText[1024];
	char	szText[1024];
	char	szLine[1024];
	int		nLineLen = 0;
	int		nRestLen = m_nFileSize;
	char *	pPos = pFileBuff;
	while (pPos - pFileBuff < m_nFileSize)
	{
		memset (szLine, 0, sizeof (szLine));
		nLineLen = CharReadLine (pPos, nRestLen, szLine, 1024);
		nRestLen -= nLineLen;
		if (nLineLen <= 0)
		{
			pPos++;
		}
		else
		{
			pPos += nLineLen;
			if (!bBodyFind)
			{
				if (!strncmp (szLine, "<BODY>", 6))
					bBodyFind = true;
				continue;
			}
			if (!strncmp (szLine, "</BODY>", 7))
				break;

			if (!strncmp (szLine, "<SYNC", 5))
			{
				bSyncLine = true;
				nStartTime = -1;
				pBegin = strstr (szLine, "=");
				if (pBegin != NULL)
				{
					pBegin++;
					pEnd = strstr (pBegin, ">");
					if (pEnd != NULL)
						*pEnd = 0;
					nStartTime = atoi (pBegin);
				}
			}
			else
			{
				if (bSyncLine)
				{
					bSyncLine = false;
					if (pItem != NULL)
						pItem->m_llEnd = nStartTime;

					pItem = new CSubtitleItem ();
					m_lstTemp.AddTail (pItem);
					pItem->m_llStart = nStartTime;
				}

				memset (szText, 0, sizeof (szText));

				char * pTemp = szLine;
				char * pText = szLine;
				while (pTemp != NULL)
				{
					if (*pTemp == ('<'))
					{
						if (pTemp > pText)
							strncat (szText, pText, pTemp - pText);

						while (pTemp != NULL)
						{
							pTemp++;
							if (*pTemp == ('>'))
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

				if (strlen (pText) > 0)
					strcat (szText, pText);
#ifdef _OS_WIN32
				// add the text in item
				memset (wzText, 0, sizeof (wzText));
				MultiByteToWideChar (CP_ACP, 0, szText, -1, wzText, sizeof (wzText));
				if (pItem != NULL)
					pItem->AddText (wzText);
#endif // _OS_WIN32					
			}
		}
	}
	delete []pFileBuff;

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

	CheckTimeStamp ();
/*
	pos = m_lstText.GetHeadPosition ();
	while (pos != NULL)
	{
		pItem = m_lstText.GetNext (pos);
		_stprintf (wzText, _T("%01d:%02d:%02d.%03d % 8d\r\n"), (int)(pItem->m_llStart / 3600000), 
						((int)pItem->m_llStart % 3600000) / 60000, ((int)pItem->m_llStart % 60000) / 1000, 
						(int)pItem->m_llStart % 1000, (int)pItem->m_llStart);

		OutputDebugString (wzText);
	}
*/
	return YY_ERR_NONE;
}

