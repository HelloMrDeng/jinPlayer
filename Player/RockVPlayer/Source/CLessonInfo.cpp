/*******************************************************************************
	File:		CLessonInfo.cpp

	Contains:	Mutex lock implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-10-23		Fenger			Create file

*******************************************************************************/
#include "CLessonInfo.h"

#include "UStringFunc.h"
#include "yyLog.h"

CLessonInfo::CLessonInfo(void)
	: m_nBeatPM (60)
	, m_nBeatPC (4)
	, m_nChapTime (4000)
	, m_nItemNum (0)
	, m_ppItem (NULL)
	, m_nItemSel (0)
	, m_nRptNum (0)
	, m_ppRepeat (NULL)
	, m_nRptSel (-1)
{
	SetObjectName ("CLessonInfo");
	memset (m_szPath, 0, sizeof (m_szPath));
	memset (m_szPrevFile, 0, sizeof (m_szPrevFile));
}

CLessonInfo::~CLessonInfo(void)
{
	Close ();
}

CLsnRepeat * CLessonInfo::GetRepeatInfo (int nIndex)
{
	if (m_ppRepeat == NULL)
		return NULL;
	if (nIndex < 0 || nIndex >= m_nRptNum)
		return NULL;
	return m_ppRepeat[nIndex];
}

CLsnRepeat * CLessonInfo::GetRepeat (void)
{
	if (m_ppRepeat == NULL || m_nRptSel < 0)
		return NULL;
	return m_ppRepeat[m_nRptSel];
}

bool CLessonInfo::SetRepeatSel (int nSel)
{
	if (m_ppRepeat == NULL)
		return false;
	if (nSel >= m_nRptNum)
		return false;
	m_nRptSel = nSel;
	return true;
}

CLsnItem * CLessonInfo::GetItemInfo (int nIndex)
{
	if (m_ppItem == NULL)
		return NULL;
	if (nIndex < 0 || nIndex >= m_nItemNum)
		return NULL;
	return m_ppItem[nIndex];
}

CLsnItem * CLessonInfo::GetItem (void)
{
	if (m_ppItem == NULL)
		return NULL;
	if (m_nItemSel < 0 || m_nItemSel >= m_nItemNum)
		return NULL;
	return m_ppItem[m_nItemSel];
}

bool CLessonInfo::SetItemSel (int nSel)
{
	if (m_ppItem == NULL)
		return false;
	if (nSel < 0 || nSel >= m_nItemNum)
		return false;
	m_nItemSel = nSel;
	return true;
}

void CLessonInfo::Close (void)
{
	if (m_ppItem != NULL)
	{
		for (int i = 0; i < m_nItemNum; i++)
			YY_DEL_P (m_ppItem[i]);
		delete []m_ppItem;
	}
	m_ppItem = NULL;
	m_nItemNum = 0;
	m_nItemSel = 0;

	if (m_ppRepeat != NULL)
	{
		for (int i = 0; i < m_nRptNum; i++)
			YY_DEL_P (m_ppRepeat[i]);
		delete []m_ppRepeat;
	}
	m_ppRepeat = NULL;
	m_nRptNum = 0;
	m_nRptSel = -1;
}

bool CLessonInfo::Open (TCHAR * pFile)
{
	Close ();
		
	if (!m_cfgFile.Open (pFile))
	{
		YYLOGE ("Open config file failed!");
		return false;
	}

	char	szItem[64];
	char *	pItem = NULL;
	m_nRptNum = m_cfgFile.GetItemValue ("Repeat", "Count", 0);
	if (m_nRptNum > 0)
	{
		m_ppRepeat = new CLsnRepeat*[m_nRptNum];
		for (int i = 0; i < m_nRptNum; i++)
		{
			m_ppRepeat[i] = new CLsnRepeat ();
			sprintf (szItem, "RS%d", i+1);
			m_ppRepeat[i]->m_nStart = m_cfgFile.GetItemValue ("Repeat", szItem, 0) * 1000;
			sprintf (szItem, "RE%d", i+1);
			m_ppRepeat[i]->m_nEnd = m_cfgFile.GetItemValue ("Repeat", szItem, 0) * 1000;
		}
	}
	pItem = m_cfgFile.GetItemText ("Lesson", "PrevFile");
#ifdef _OS_WIN32
	MultiByteToWideChar (CP_ACP, 0, pItem, -1, m_szPrevFile, sizeof (m_szPrevFile));
#else
	strcpy (m_szPrevFile, pItem);
#endif // _OS_WIN32
	m_nBeatPM = m_cfgFile.GetItemValue ("Lesson", "BeatPM", 60);
	m_nBeatPC = m_cfgFile.GetItemValue ("Lesson", "BeatPC", 4);
	m_nChapTime = 60000 / m_nBeatPM * m_nBeatPC;
	m_nItemNum = m_cfgFile.GetItemValue ("Lesson", "ItemNum", 0);
	m_nItemSel = m_cfgFile.GetItemValue ("Lesson", "ItemSel", 0);
	if (m_nItemNum > 0)
	{
		m_ppItem = new CLsnItem*[m_nItemNum];
		for (int i = 0; i < m_nItemNum; i++)
		{
			m_ppItem[i] = new CLsnItem ();
			sprintf (szItem, "Item%d", i+1);
			pItem = m_cfgFile.GetItemText (szItem, "Name");
#ifdef _OS_WIN32
			MultiByteToWideChar (CP_ACP, 0, pItem, -1, m_ppItem[i]->m_szName, sizeof (m_ppItem[i]->m_szName));
#else
			strcpy (m_ppItem[i]->m_szName, pItem);
#endif // _OS_WIN32
			int nTrack = m_cfgFile.GetItemValue (szItem, "FileNum", 0);
			if (nTrack > 0)
			{
				char szTrack[64];
				m_ppItem[i]->m_nTrackNum = nTrack;
				m_ppItem[i]->m_ppTrack = new CLsnTrack*[nTrack];
				for (int j = 0; j < nTrack; j++)
				{
					m_ppItem[i]->m_ppTrack[j] = new CLsnTrack ();
					sprintf (szTrack, "Name%d", j+1);
					pItem = m_cfgFile.GetItemText (szItem, szTrack);
#ifdef _OS_WIN32
					MultiByteToWideChar (CP_ACP, 0, pItem, -1, m_ppItem[i]->m_ppTrack[j]->m_szName, sizeof (m_ppItem[i]->m_ppTrack[j]->m_szName));
#else
					strcpy (m_ppItem[i]->m_ppTrack[j]->m_szName, pItem);
#endif // _OS_WIN32
					sprintf (szTrack, "File%d", j+1);
					pItem = m_cfgFile.GetItemText (szItem, szTrack);
#ifdef _OS_WIN32
					MultiByteToWideChar (CP_ACP, 0, pItem, -1, m_ppItem[i]->m_ppTrack[j]->m_szFile, sizeof (m_ppItem[i]->m_ppTrack[j]->m_szFile));
#else
					strcpy (m_ppItem[i]->m_ppTrack[j]->m_szFile, pItem);
#endif // _OS_WIN32
					sprintf (szTrack, "Volume%d", j+1);
					m_ppItem[i]->m_ppTrack[j]->m_nVolume = m_cfgFile.GetItemValue (szItem, szTrack, 100);
					sprintf (szTrack, "Type%d", j+1);
					m_ppItem[i]->m_ppTrack[j]->m_nType = m_cfgFile.GetItemValue (szItem, szTrack, 0);
					sprintf (szTrack, "Enable%d", j+1);
					m_ppItem[i]->m_ppTrack[j]->m_bEnable = m_cfgFile.GetItemValue (szItem, szTrack, 1) > 0;
				}
			}
		}
	}

	TCHAR * pPos = _tcsrchr (pFile, _T('\\'));
	if (pPos == NULL)
		pPos = (TCHAR *)_tcsrchr (pFile, _T('/'));
	if (pPos != NULL)
		_tcsncpy (m_szPath, pFile, pPos - pFile + 1);

	return true;
}