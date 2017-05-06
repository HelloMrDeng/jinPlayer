/*******************************************************************************
	File:		CBaseKey.h

	Contains:	the base key header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-20		Fenger			Create file

*******************************************************************************/
#ifndef __CBaseKey_H__
#define __CBaseKey_H__

#include "CBaseObject.h"

#define YYKEY_TEXT	"yyboxfilejinplayer"
#define YYKEY_LEN	18

class CBaseKey : public CBaseObject
{
public:
	static CBaseKey * g_Key;

public:
	CBaseKey(void);
	virtual ~CBaseKey(void);

	virtual bool 	CreateKey (TCHAR * pPassWord);
	virtual bool	IsKeyFile (unsigned char * pBuff, int nSize);
	virtual bool	EncryptData (unsigned char * pBuff, int nSize);
	virtual bool	DecryptData (unsigned char * pBuff, int nSize);

	virtual char *	GetKey (void) {return m_szKey;}
	virtual TCHAR *	GetPW (void) {return m_szPW;}
	virtual TCHAR * GetWKey (void) {return m_wzKey;}
	virtual bool	IsUsed (void) {return strlen (m_szKey) > 0 ? true : false;}

private:
	virtual bool	CreateText (TCHAR * pBuff, int nSize);
	virtual bool	RestoreText (TCHAR * pBuff, int nSize);

protected:
	TCHAR		m_szPW[64];
	char		m_szKey[64];
	TCHAR		m_wzKey[64];

	char		m_szKey1[32];
	char		m_szKey2[32];
	char		m_szKey3[32];
	char		m_szKey4[32];
};

#endif // __CBaseKey_H__
