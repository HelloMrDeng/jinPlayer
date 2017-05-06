/*******************************************************************************
	File:		CBaseUtils.cpp

	Contains:	base utils implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-08		Fenger			Create file

*******************************************************************************/
#include "CBaseUtils.h"
#include "CBaseFile.h"
#include "CBasePDP.h"
#include "CBaseExtData.h"
#include "CBaseIOHook.h"

#include "USystemFunc.h"
#include "UStringFunc.h"

#include "yyLog.h"

static CBaseUtils	g_baseUtils;

CBaseUtils::CBaseUtils(void)
	: CBaseObject ()
{
	SetObjectName ("CBaseUtils");
}

CBaseUtils::~CBaseUtils(void)
{
}

int CBaseUtils::FillExtIOFunc (URLProtocol * pProt)
{
	if (pProt == NULL)
		return -1;

	memset (pProt, 0, sizeof (pProt));
	pProt->url_open						= yy_extio_open;
	pProt->url_open2					= NULL;//yy_extio_open2;
	pProt->url_read						= yy_extio_read;
	pProt->url_write					= yy_extio_write;
	pProt->url_seek						= yy_extio_seek;
	pProt->url_close					= yy_extio_close;
	pProt->url_read_pause				= NULL;//yy_extio_read_pause;
	pProt->url_read_seek				= NULL;//yy_extio_read_seek;
	pProt->url_get_file_handle			= yy_extio_get_handle;
	pProt->url_get_multi_file_handle	= NULL;//yy_extio_get_multi_file_handle;
	pProt->url_shutdown					= yy_extio_shutdown;
	pProt->url_check					= yy_extio_check;

	pProt->priv_data_size = sizeof (URLProtocol);
	pProt->priv_data_class = (const AVClass *)malloc (pProt->priv_data_size);
	memset ((void *)pProt->priv_data_class, 0, pProt->priv_data_size);

	return 0;
}

int CBaseUtils::FreeExtIOFunc (URLProtocol * pProt)
{
	if (pProt->priv_data_size > 0 && pProt->priv_data_class != NULL)
	{
		free ((void *)pProt->priv_data_class);
		pProt->priv_data_size = 0;
		pProt->priv_data_class = NULL;
	}

	return 0;
}

int CBaseUtils::yy_extio_open(URLContext *h, const char *filename, int flags)
{
	FileContext * pCtx = (FileContext *)h->priv_data;

	if (strstr (filename, "file:") == filename)
		filename += 5;

	TCHAR szFileName[1024];
	memset (szFileName, 0, sizeof (szFileName));

#ifdef _UNICODE
	ConvertBase64ToData ((char *)filename, (unsigned char *)szFileName, sizeof (szFileName) * sizeof (TCHAR));
#else
	strcpy (szFileName, filename);
#endif // _UNICODE

	CBaseIO	* pIO = NULL;

	if (_tcsstr (szFileName, _T("\\yyextdata")) != NULL)
	{
		if (CBaseExtData::g_pExtData->llSize == 0)
			h->is_streamed = true;
		else
			h->is_streamed = false;
		pIO = new CBaseExtData ();
	}
	else
	{
		YY_PROT_TYPE nType = yyGetProtType (szFileName);
		if (nType == YY_PROT_FILE)
		{
			h->is_streamed = false;
			pIO = new CBaseFile ();
		}	
#ifndef _OS_WINCE
		else if (nType == YY_PROT_PDP)
		{
			h->is_streamed = true;
			pIO = new CBasePDP ();
		}	
#endif // _OS_WINCE
		else if (nType == YY_PROT_HTTP)
		{
			h->is_streamed = true;
//			pIO = new CBaseHTTP ();
		}
	}

	int nRC = pIO->open (h, szFileName, flags);
	pCtx->hFile = pIO;

	return nRC;
}

int CBaseUtils::yy_extio_open2 (URLContext *h, const char *url, int flags, AVDictionary **options)
{
	return yy_extio_open (h, url, flags);
}

int CBaseUtils::yy_extio_read(URLContext *h, unsigned char *buf, int size)
{
	FileContext * pCtx = (FileContext *)h->priv_data;
	if (pCtx == NULL || pCtx->hFile == NULL)
		return -1;

	CBaseIO * pIO = (CBaseIO *)pCtx->hFile;

	return pIO->read (h, buf, size);
}

int CBaseUtils::yy_extio_write(URLContext *h, const unsigned char *buf, int size)
{
	FileContext * pCtx = (FileContext *)h->priv_data;
	if (pCtx == NULL || pCtx->hFile == NULL)
		return -1;

	CBaseIO * pIO = (CBaseIO *)pCtx->hFile;

	return pIO->write (h, buf, size);
}

// XXX: use llseek 
int64_t CBaseUtils::yy_extio_seek(URLContext *h, int64_t pos, int whence)
{
	FileContext * pCtx = (FileContext *)h->priv_data;
	if (pCtx == NULL || pCtx->hFile == NULL)
		return -1;

	CBaseIO * pIO = (CBaseIO *)pCtx->hFile;

	return pIO->seek (h, pos, whence);
}

int CBaseUtils::yy_extio_close(URLContext *h)
{
	FileContext * pCtx = (FileContext *)h->priv_data;
	if (pCtx == NULL || pCtx->hFile == NULL)
		return -1;

	CBaseIO * pIO = (CBaseIO *)pCtx->hFile;
	pIO->close (h);
	delete pIO;

	return 0;
}

int CBaseUtils::yy_extio_read_pause (URLContext *h, int pause)
{
	FileContext * pCtx = (FileContext *)h->priv_data;
	if (pCtx == NULL || pCtx->hFile == NULL)
		return -1;

	CBaseIO * pIO = (CBaseIO *)pCtx->hFile;

	return pIO->read_pause (h, pause);
}

int64_t	CBaseUtils::yy_extio_read_seek (URLContext *h, int stream_index, int64_t timestamp, int flags)
{
	FileContext * pCtx = (FileContext *)h->priv_data;
	if (pCtx == NULL || pCtx->hFile == NULL)
		return -1;

	CBaseIO * pIO = (CBaseIO *)pCtx->hFile;

	return pIO->read_seek (h, stream_index, timestamp, flags);
}

int CBaseUtils::yy_extio_get_handle(URLContext *h)
{
	FileContext * pCtx = (FileContext *)h->priv_data;
	if (pCtx == NULL || pCtx->hFile == NULL)
		return -1;

	CBaseIO * pIO = (CBaseIO *)pCtx->hFile;

	return pIO->get_handle (h);
}

int CBaseUtils::yy_extio_get_multi_file_handle (URLContext *h, int **handles, int *numhandles)
{
	FileContext * pCtx = (FileContext *)h->priv_data;
	if (pCtx == NULL || pCtx->hFile == NULL)
		return -1;

	CBaseIO * pIO = (CBaseIO *)pCtx->hFile;

	return pIO->get_multi_file_handle (h, handles, numhandles);
}

int CBaseUtils::yy_extio_shutdown (URLContext *h, int flags)
{
	FileContext * pCtx = (FileContext *)h->priv_data;
	if (pCtx == NULL || pCtx->hFile == NULL)
		return -1;

	CBaseIO * pIO = (CBaseIO *)pCtx->hFile;

	return pIO->shutdown (h, flags);
}

int CBaseUtils::yy_extio_check(URLContext *h, int mask)
{
	// assume the file always exist.
	FileContext * pCtx = (FileContext *)h->priv_data;
	if (pCtx == NULL || pCtx->hFile == NULL)
		return -1;

	CBaseIO * pIO = (CBaseIO *)pCtx->hFile;

	return pIO->check (h, mask);
}

int CBaseUtils::ConvertDataToBase64 (unsigned char * pData, int nDataSize, char * pOutText, int nOutSize)
{
	const char szBase64EncTable[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   
	int nCharSize = nDataSize * 4 / 3 + 64;
	char * pBase64Text = new char[nCharSize];
	memset (pBase64Text, 0, nCharSize);

	TCHAR * pExt = _tcsrchr ((TCHAR *)pData, _T('.'));
	char	szExt[32];
	memset (szExt, 0, sizeof (szExt));

	if (pExt != NULL)
	{
		nDataSize = (pExt - (TCHAR *)pData) * 2;
#ifdef _UNICODE
		WideCharToMultiByte (CP_ACP, 0, pExt, -1, szExt, sizeof (szExt), NULL, NULL);
#endif // _UNICODE
	}

	unsigned char	cTmp[4] = {0};
	int				nLineLen = 0;
	int				nDestPos = 0;
	for(int i = 0; i < nDataSize / 3; i++)
	{
		cTmp[1] = *pData++;
		cTmp[2] = *pData++;
		cTmp[3] = *pData++;
		pBase64Text[nDestPos++] = szBase64EncTable[cTmp[1] >> 2];
		pBase64Text[nDestPos++] = szBase64EncTable[((cTmp[1] << 4) | (cTmp[2] >> 4)) & 0x3F];
		pBase64Text[nDestPos++] = szBase64EncTable[((cTmp[2] << 2) | (cTmp[3] >> 6)) & 0x3F];
		pBase64Text[nDestPos++] = szBase64EncTable[cTmp[3] & 0x3F];

		if(nLineLen+=4,nLineLen==76) 
		{
			pBase64Text[nDestPos++] ='\r';
			pBase64Text[nDestPos++] ='\n';
			nLineLen=0;
		}
	}

	//The rest data
	int nMod = nDataSize % 3;
	if(nMod==1)
	{
		cTmp[1] = *pData++;
		pBase64Text[nDestPos++] = szBase64EncTable[(cTmp[1] & 0xFC) >> 2];
		pBase64Text[nDestPos++] = szBase64EncTable[((cTmp[1] & 0x03) << 4)];
		pBase64Text[nDestPos++] = '=';
		pBase64Text[nDestPos++] = '=';
	}
	else if(nMod==2)
	{
		cTmp[1] = *pData++;
		cTmp[2] = *pData++;
		pBase64Text[nDestPos++] = szBase64EncTable[(cTmp[1] & 0xFC) >> 2];
		pBase64Text[nDestPos++] = szBase64EncTable[((cTmp[1] & 0x03) << 4) | ((cTmp[2] & 0xF0) >> 4)];
		pBase64Text[nDestPos++] = szBase64EncTable[((cTmp[2] & 0x0F) << 2)];
		pBase64Text[nDestPos++] = '=';
	}
	if (pExt != NULL)
		strcat (pBase64Text, szExt);

	if (pOutText != NULL && nOutSize >= nDestPos)
		strcpy (pOutText, pBase64Text);
	else
		nDestPos = -1;

	delete []pBase64Text;
	return nDestPos;
}

int CBaseUtils::ConvertBase64ToData (char * pBase64, unsigned char * pOutData, int nOutSize)
{
    //Base64 decoder table
	const char szBase64DecTable[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		62, // '+'
		0, 0, 0,
		63, // '/'
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
		0, 0, 0, 0, 0, 0, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
		13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
		0, 0, 0, 0, 0, 0,
		26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
		39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
	};

	char *	pDataBuff = (char *)pBase64;
	int		nDataSize = strlen (pBase64);
	int		nOutBytes = 0;

	unsigned char * pDestData = new unsigned char[nDataSize * 3];
	memset (pDestData, 0, nDataSize * 3);
	char *	pDesBuff = (char *)pDestData;

	char *	pExt = (char *)strrchr (pBase64, '.');
	if (pExt != NULL)
		nDataSize = (pExt - pDataBuff);

	int		nValue;
	int		i= 0;
	while (i < nDataSize)
	{
		if (*pDataBuff != '\r' && *pDataBuff!='\n')
		{
			nValue = szBase64DecTable[*pDataBuff++] << 18;
			nValue += szBase64DecTable[*pDataBuff++] << 12;
			*pDesBuff++ = (nValue & 0x00FF0000) >> 16;
			nOutBytes++;

			if (*pDataBuff != '=')
			{
				nValue += szBase64DecTable[*pDataBuff++] << 6;
				*pDesBuff++ = (nValue & 0x0000FF00) >> 8;
				nOutBytes++;
				if (*pDataBuff != '=')
				{
					nValue += szBase64DecTable[*pDataBuff++];
					*pDesBuff++ =nValue & 0x000000FF;
					nOutBytes++;
				}
			}
			i += 4;
		}
		else// new line skip
		{
			pDataBuff++;
			i++;
		}
	}

	if (pExt != NULL)
	{
		while (*pExt != 0)
		{
			*pDesBuff++ = *pExt++;
			nOutBytes++;
#ifdef _UNICODE
			pDesBuff++;
			nOutBytes++;
#endif // _UNICODE
		}
	}

	if (pOutData != NULL && nOutSize >= nOutBytes)
		memcpy ((void *)pOutData, (void *)pDestData, nOutBytes);
	else
		nOutBytes = -1;

	delete []pDestData;
	return nOutBytes;
}

