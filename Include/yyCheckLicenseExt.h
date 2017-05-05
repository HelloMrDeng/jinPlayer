/*******************************************************************************
	File:		yyCheckLicenseExt.h

	Contains:	yy player check license ext header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-09		Fenger			Create file

*******************************************************************************/
#ifndef __yyCheckLicenseExt_H__
#define __yyCheckLicenseExt_H__

// Call back function of player verify license text
typedef int (* yyVerifyLicenseText) (void * pUserData, char * pText, int nSize);

// Check ext license status
typedef int (* YYCHECKEXTLICENSE) (char * pText, int nSize, yyVerifyLicenseText fVerify, void * pUserData);
int yyCheckExtLicense (char * pText, int nSize, yyVerifyLicenseText fVerify, void * pUserData)
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


#endif // __yyCheckLicenseExt_H__
