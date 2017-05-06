/*******************************************************************************
	File:		CLicenseCheck.h

	Contains:	the license check class.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-09		Fenger			Create file

*******************************************************************************/

#ifndef __CLicenseCheck_H__
#define __CLicenseCheck_H__

#include "CBaseObject.h"

#ifdef _OS_WIN32
#define	WM_YYRR_CHKLCS	WM_USER + 765
#define	WM_YYYF_CHKLCS	WM_USER + 871
#define	WM_YYSM_CHKLCS	WM_USER + 811
#define	WM_YYHS_CHKLCS	WM_USER + 919
#endif // _OS_WIN32

#define	YY_LCS_V1	20050422
#define	YY_LCS_V2	20010805
#define	YY_LCS_V3	20121113

#define YY_MAX_CUSTOMER_NUM		128

class CLicenseCheck : public CBaseObject
{
public:
	static int yyVerifyExtLicenseText (void * pUserData, char * pText, int nSize);
	static CLicenseCheck *	m_pLcsChk;

public:
	int			m_nLcsStatus1;

public:
	CLicenseCheck(void);
	virtual ~CLicenseCheck(void);

	virtual void	CheckLicense (void);
	virtual void	SetView (void * hView);
	virtual char *	GetCMUUID (void) {return m_szUUID;}

public:
	int			m_nLcsStatus2;

protected:
	void *		m_hView;
	bool		m_bChecked;
	char		m_szLcsText[256];

	int			m_nCustomerNum;
	char *		m_szCustomer[YY_MAX_CUSTOMER_NUM];
	int			m_nMsgID[YY_MAX_CUSTOMER_NUM];
	char		m_szUUID[256];

public:
	int			m_nLcsStatus3;

};

#endif // __CLicenseCheck_H__

