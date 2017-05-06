/*******************************************************************************
	File:		CLicenseCheck.cpp

	Contains:	the license check implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-09		Fenger			Create file

*******************************************************************************/
#include "CLicenseCheck.h"

#include "yyCheckLicenseExt.h"

#ifdef _OS_WINCE
#include "pkfuncs.h"
#endif // _OS_WINCE

#include "yyLog.h"

CLicenseCheck *	CLicenseCheck::m_pLcsChk = NULL;

static int CheckDefaultLicense (char * pText, int nSize, yyVerifyLicenseText fVerify, void * pUserData)
{
	if (pText == NULL || nSize <= 0)
		return -2;

	char	szExtLicenseKey[32];
	char *	pExtLicenseText = new char[nSize+1];

	strcpy (pExtLicenseText, pText);
	strcpy (szExtLicenseKey, "yyMediaEngine");
	int nKeyLen = strlen (szExtLicenseKey);

	for (int i = 0; i < nSize; i++)
	{
		for (int j = 0; j < nKeyLen; j++)
		{
			pExtLicenseText[i] = pExtLicenseText[i] ^ szExtLicenseKey[j];
		}
	}

	if (fVerify != NULL)
		fVerify (pUserData, pExtLicenseText, nSize);

	delete []pExtLicenseText;

	return 0;
}

CLicenseCheck::CLicenseCheck(void)
	: CBaseObject ()
	, m_hView (NULL)
	, m_bChecked (false)
	, m_nLcsStatus1 (0)
	, m_nLcsStatus2 (0)
	, m_nLcsStatus3 (0)
{
	SetObjectName ("LicenseCheck");

	strcpy (m_szLcsText, "yyMediaPlayerSDK");
	memset (&m_szUUID, 0, sizeof (m_szUUID));

	m_nCustomerNum = 4;
	for (int i = 0; i < m_nCustomerNum; i++)
		m_szCustomer[i] = new char[64];


	// Road Rover
	strcpy (m_szCustomer[0], "6ARR");
	
	// Yuan Feng	
	strcpy (m_szCustomer[1], "NAUY");	

	// Sino Embed	
	strcpy (m_szCustomer[2], "SNPL");	

	// HangSheng	
	strcpy (m_szCustomer[3], "HGSH");

#ifdef _OS_WIN32
	m_nMsgID[0] = WM_YYRR_CHKLCS;
	m_nMsgID[1] = WM_YYYF_CHKLCS;
	m_nMsgID[2] = WM_YYSM_CHKLCS;
	m_nMsgID[3] = WM_YYHS_CHKLCS;
#endif // _OS_WIN32	

#ifdef _OS_WINCE
	DWORD	dwBytesReturned;
	BOOL	bRetVal = KernelIoControl (IOCTL_HAL_GET_UUID, NULL, 0, m_szUUID,
										 sizeof (m_szUUID), &dwBytesReturned);

	TCHAR szWinTxt[256];
	GetWindowText ((HWND)m_hView, szWinTxt, sizeof (szWinTxt) / sizeof (TCHAR));
	if (!_tcscmp (szWinTxt, _T("MP4_OSD")))
		strcpy (m_szUUID, "6ARR");
#else
	m_szUUID[0] = '6';
	m_szUUID[1] = 'A';
	m_szUUID[2] = 'R';
	m_szUUID[3] = 'R';
//	m_uuidDevice.Data1 = '6ARR';
#endif // WINCE
	YYLOGI ("The UUID is: %02X %02X %02X %02X, Text: %c%c%c%c  Num: %d ", 
				m_szUUID[0], m_szUUID[1], m_szUUID[2], m_szUUID[3],
				m_szUUID[0], m_szUUID[1], m_szUUID[2], m_szUUID[3], m_nCustomerNum);

	m_pLcsChk = this;
}

CLicenseCheck::~CLicenseCheck(void)
{
	for (int i = 0; i < m_nCustomerNum; i++)
		delete []m_szCustomer[i];

	m_pLcsChk = NULL;
}

int CLicenseCheck::yyVerifyExtLicenseText (void * pUserData, char * pText, int nSize)
{
	char szLcsText[1024];
	strcpy (szLcsText, pText);
	char	szExtLicenseKey[32];	
	strcpy (szExtLicenseKey, "yyMediaEngine");
	int nKeyLen = strlen (szExtLicenseKey);

	for (int i = 0; i < nSize; i++)
	{
		for (int j = 0; j < nKeyLen; j++)
		{
			szLcsText[i] = szLcsText[i] ^ szExtLicenseKey[j];
		}
	}

	CLicenseCheck * pLcsCheck = (CLicenseCheck *)pUserData;
	if (!strcmp (szLcsText, pLcsCheck->m_szLcsText))
	{
		pLcsCheck->m_nLcsStatus1 = YY_LCS_V1;
		pLcsCheck->m_nLcsStatus2 = YY_LCS_V2;
		pLcsCheck->m_nLcsStatus3 = YY_LCS_V3;

		YYLOGT (pLcsCheck->m_szObjName, "Check License Passed!");
	}
	else
	{
		YYLOGT (pLcsCheck->m_szCustomer, "Check License Failed!");
	}

	return 0;
}

void CLicenseCheck::CheckLicense (void)
{
	if (m_nLcsStatus1 == YY_LCS_V1 && m_nLcsStatus2 == YY_LCS_V2 && m_nLcsStatus3 == YY_LCS_V3)
		return;

	if (m_bChecked || m_hView == NULL)
		return;

	m_bChecked = true;

#ifdef _OS_WIN32
	YYCHECKEXTLICENSE pLcsChk = NULL;
#ifdef _CPU_PRIMA2
/*
	TCHAR szWinText[256];
	memset (szWinText, 0, sizeof (szWinText));
	GetWindowText ((HWND)m_hView, szWinText, sizeof (szWinText));
	char szLogText[256];
	memset (szLogText, 0, sizeof (szLogText));
	WideCharToMultiByte (CP_ACP, 0, szWinText, -1, szLogText, sizeof (szLogText), NULL, NULL);
	YYLOGI ("The video window text is %s", szLogText);
	if (!_tcscmp (szWinText, _T("HSAEVideoPlayer")) || !_tcscmp (szWinText, _T("HSAEMusicPlayer")))
*/
	HWND hWnd = FindWindow (NULL, _T("HSAEVideoPlayer"));
	if (hWnd == NULL)
		hWnd = FindWindow (NULL, _T("HSAEMusicPlayer"));
	YYLOGI ("The player windows is %p", hWnd);
	if (hWnd != NULL)
	{
		pLcsChk = (YYCHECKEXTLICENSE)SendMessage ((HWND)m_hView, WM_YYHS_CHKLCS, 0, 0);
		YYLOGI ("Send the message to Window 0x%08X return 0X%08X", m_hView, pLcsChk);
		if (pLcsChk == NULL)
			pLcsChk = CheckDefaultLicense;
	}
#endif // _CPU_PRIMA2
#ifdef _CPU_MSB2531
	HWND hWnd = FindWindow (NULL, _T("FYT-Media"));
	YYLOGI ("The player windows is %p", hWnd);
	if (hWnd != NULL)
		pLcsChk = CheckDefaultLicense;
#endif // _CPU_PRIMA2
	if (pLcsChk == NULL)
	{
		for (int i = 0; i < m_nCustomerNum; i++)
		{
			if (!memcmp (m_szUUID, m_szCustomer[i], strlen (m_szCustomer[i])))
			{
				pLcsChk = (YYCHECKEXTLICENSE)SendMessage ((HWND)m_hView, m_nMsgID[i], 0, 0);
				YYLOGI ("Send the message to Window 0x%08X return 0X%08X", m_hView, pLcsChk);
				break;
			}
		}
	}

	if (pLcsChk != NULL)
	{
		pLcsChk (m_szLcsText, strlen (m_szLcsText), yyVerifyExtLicenseText, this);
	}
#endif // _OS_WIN32

	return;
}

void CLicenseCheck::SetView (void * hView)
{
	m_hView = hView;
	return;
}