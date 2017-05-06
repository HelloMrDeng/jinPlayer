/*******************************************************************************
	File:		UStringFunc.cpp

	Contains:	The utility for string implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#include "UStringFunc.h"

#include "yyDefine.h"

#pragma warning (disable : 4996)

bool yyChangeFileExtName (char * pFileName, bool bUP)
{
	char *	pExtChar = strrchr (pFileName, '.');
	if (pExtChar == NULL)
		return false;

	int	nExtLen = strlen (pExtChar);
	for (int i = 0; i < nExtLen; i++)
	{
		if (bUP)
		{
			if (*(pExtChar + i) >= 'a' && *(pExtChar + i) <= 'z')
				*(pExtChar + i) -= 'a' - 'A';
		}
		else
		{
			if (*(pExtChar + i) >= 'A' && *(pExtChar + i) <= 'Z')
				*(pExtChar + i) += 'a' - 'A';
		}
	}

	return true;
}

bool yyStringChange (char * pString, bool bUP)
{
	int	nLen = strlen (pString);
	for (int i = 0; i < nLen; i++)
	{
		if (bUP)
		{
			if (*(pString + i) >= 'a' && *(pString + i) <= 'z')
				*(pString + i) -= 'a' - 'A';
		}
		else
		{
			if (*(pString + i) >= 'A' && *(pString + i) <= 'Z')
				*(pString + i) += 'a' - 'A';
		}
	}

	return true;
}

bool yyCheckTextUTF8 (char * pText)
{
	int score = 0;
	int i, rawtextlen = 0;
	int goodbytes = 0, asciibytes = 0;
	// Maybe also use UTF8 Byte Order Mark: EF BB BF
	// Check to see if characters fit into acceptable ranges
	rawtextlen = strlen (pText);
	for (i = 0; i < rawtextlen; i++) 
	{
		if ((pText[i] & 0x7F) == pText[i]) 
		{ 
			// 最高位是0的ASCII字符
			asciibytes++;
			// Ignore ASCII, can throw off count
		} 
		else if (-64 <= pText[i] && pText[i] <= -33
				//-0x40~-0x21
				&& // Two bytes
				i + 1 < rawtextlen && -128 <= pText[i + 1]
				&& pText[i + 1] <= -65) 
		{
				goodbytes += 2;
				i++;
		} else if (-32 <= pText[i]
					&& pText[i] <= -17
					&& // Three bytes
					i + 2 < rawtextlen && -128 <= pText[i + 1]
					&& pText[i + 1] <= -65 && -128 <= pText[i + 2]
					&& pText[i + 2] <= -65) 
		{
			goodbytes += 3;
			i += 2;
		}
	}

	if (asciibytes == rawtextlen) 
	{
		return false;
	}
	score = 100 * goodbytes / (rawtextlen - asciibytes);
	// If not above 98, reduce to zero to prevent coincidental matches
	// Allows for some (few) bad formed sequences
	if (score > 98)
	{
		return true;
	} 
	else if (score > 95 && goodbytes > 30) 
	{
		return true;
	}
	else 
	{
		return false;
	}
}

YY_PROT_TYPE yyGetProtType (const TCHAR * pSource)
{
	YY_PROT_TYPE nType = YY_PROT_FILE;

	TCHAR * pProtocol = _tcsstr ((TCHAR *)pSource, _T("://"));
	if (pProtocol == NULL)
		return nType;

	if (pProtocol - pSource > 10)
		return nType;

	TCHAR szProtocolName[32];
	memset (szProtocolName, 0, sizeof (szProtocolName));
	_tcsncpy (szProtocolName, pSource, pProtocol - pSource + 3);
	pProtocol = szProtocolName;

	int	nPtclLen = _tcslen (szProtocolName);
	for (int i = 0; i < nPtclLen; i++)
	{
		if (*(pProtocol + i) >= _T('A') && *(pProtocol + i) <= _T('Z'))
			*(pProtocol + i) += _T('a') - _T('A');
	}

	if (_tcsstr (pProtocol, _T("http")) == pProtocol)
		return YY_PROT_HTTP;
	else if (_tcsstr (pProtocol, _T("rtsp")) == pProtocol)
		return YY_PROT_RTSP;
	else if (_tcsstr (pProtocol, _T("mms")) == pProtocol)
		return YY_PROT_MMS;
	else if (_tcsstr (pProtocol, _T("ftp")) == pProtocol)
		return YY_PROT_FTP;
	else if (_tcsstr (pProtocol, _T("pdp")) == pProtocol)
		return YY_PROT_PDP;
	else if (_tcsstr (pProtocol, _T("file:pdp")) == pProtocol)
		return YY_PROT_PDP;
	return nType;
}

void yyURLSplit ( char *proto, int proto_size,
                  char *authorization, int authorization_size,
                  char *hostname, int hostname_size,
                  int *port_ptr,
                  char *path, int path_size,
                  const char *url)
{
	const char *p, *ls, *ls2, *at, *at2, *col, *brk;

	if (port_ptr)               *port_ptr = 80;
	if (proto_size > 0)         proto[0] = 0;
	if (authorization_size > 0) authorization[0] = 0;
	if (hostname_size > 0)      hostname[0] = 0;
	if (path_size > 0)          path[0] = 0;

	// parse protocol 
	if ((p = strchr(url, ':'))) 
	{
		strncpy(proto, url, YYMIN(proto_size, p + 1 - url));
		p++; // skip ':' 
		if (*p == '/') p++;
		if (*p == '/') p++;
	} 
	else 
	{
		// no protocol means plain filename 
		strncpy(path, url, path_size);
		return;
	}

	// separate path from hostname 
	ls = strchr(p, '/');
	ls2 = strchr(p, '?');
	if(!ls)
		ls = ls2;
	else if (ls && ls2)
		ls = YYMIN(ls, ls2);
	if(ls)
		strncpy(path, ls, path_size);
	else
		ls = &p[strlen(p)]; // XXX

	// the rest is hostname, use that to parse auth/port 
	if (ls != p)
	{
		// authorization (user[:pass]@hostname)
		at2 = p;
		while ((at = strchr(p, '@')) && at < ls)
		{
			strncpy(authorization, at2, YYMIN(authorization_size, at + 1 - at2));
			p = at + 1; // skip '@' 
		}

		if (*p == '[' && (brk = strchr(p, ']')) && brk < ls) 
		{
			// [host]:port 
			strncpy(hostname, p + 1, YYMIN(hostname_size, brk - p));
			if (brk[1] == ':' && port_ptr)
				*port_ptr = atoi(brk + 2);
		} 
		else if ((col = strchr(p, ':')) && col < ls) 
		{
			strncpy (hostname, p, YYMIN(col + 1 - p, hostname_size));
			if (port_ptr)
				*port_ptr = atoi(col + 1);
		}
		else
			strncpy (hostname, p, YYMIN(ls - p, hostname_size));
	}
}

int yyFindInfoTag (char *arg, int arg_size, const char *tag1, const char *info)
{
	const char *p;
	char		tag[128], *q;

	p = info;
	if (*p == '?')
		p++;
	for(;;) 
	{
		q = tag;
		while (*p != '\0' && *p != '=' && *p != '&') 
		{
			if ((q - tag) < sizeof(tag) - 1)
				*q++ = *p;
			p++;
		}
		*q = '\0';
		q = arg;
		if (*p == '=') 
		{
			p++;
			while (*p != '&' && *p != '\0')
			{
				if ((q - arg) < arg_size - 1)
				{
					if (*p == '+')
						*q++ = ' ';
					else
						*q++ = *p;
				}
			p++;
			}
		}

		*q = '\0';
		if (!strcmp(tag, tag1))
			return 1;
		if (*p != '&')
			break;
		p++;
	}

	return 0;
}

int yyTextReadLine (TCHAR * pText, int nTextLen, TCHAR * pLine, int nSize)
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
