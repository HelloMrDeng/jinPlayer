/*******************************************************************************
	File:		URLEFunc.cpp

	Contains:	The utility for library implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#include "string.h"

#include "URLEFunc.h"

int yyRLEEncodeBmp (char * pBMP, int nW, int nH, char ** ppRLE)
{
	char * pBuffR = new char[nW * nH * 8];
	char * pBuffG = pBuffR + nW * nH * 2;
	char * pBuffB = pBuffR + nW * nH * 4;
	char * pBuffA = pBuffR + nW * nH * 6;
	char * pR = pBuffR;
	char * pG = pBuffG;
	char * pB = pBuffB;
	char * pA = pBuffA;
	char * pRGBA = pBMP;

	char cR = pRGBA[0];
	char cG = pRGBA[1];
	char cB = pRGBA[2];
	char cA = pRGBA[3];
	char nR = 1;
	char nG = 1;
	char nB = 1;
	char nA = 1;
	for (int h = 0; h < nH; h++)
	{
		for (int w = 0; w < nW; w++)
		{
			// Red
			if (*pRGBA == cR && nR < 127)
			{
				nR++;
			}
			else
			{
				*pR++ = nR;
				*pR++ = cR;
				cR = *pRGBA;
				nR = 1;
			}
			pRGBA++;
			// Blue
			if (*pRGBA == cG && nG < 127)
			{
				nG++;
			}
			else
			{
				*pG++ = nG;
				*pG++ = cG;
				cG = *pRGBA;
				nG = 1;
			}
			pRGBA++;
			// Green
			if (*pRGBA == cB && nB < 127)
			{
				nB++;
			}
			else
			{
				*pB++ = nB;
				*pB++ = cB;
				cB = *pRGBA;
				nB = 1;
			}
			pRGBA++;

			//Alpha
			if (*pRGBA == cA && nA < 127)
			{
				nA++;
			}
			else
			{
				*pA++ = nA;
				*pA++ = cA;
				cA = *pRGBA;
				nA = 1;
			}		
			pRGBA++;
		}
	}
	if (nR > 1)
	{
		*pR++ = nR-1;
		*pR++ = cR;
	}
	if (nG > 1)
	{
		*pG++ = nG - 1;
		*pG++ = cG;
	}
	if (nB > 1)
	{
		*pB++ = nB - 1;
		*pB++ = cB;
	}
	if (nA > 1)
	{
		*pA++ = nA - 1;
		*pA++ = cA;
	}

	int nSize = (pR - pBuffR) + (pG - pBuffG) + (pB - pBuffB) + (pA - pBuffA);
	memcpy (pR, pBuffG, pG - pBuffG);
	pR = pR + (pG - pBuffG);
	memcpy (pR, pBuffB, pB - pBuffB);
	pR = pR + (pB - pBuffB);
	memcpy (pR, pBuffA, pA - pBuffA);
	
//	yyRLEDecodeBmp (pBuffR, nW, nH, pBMP);
	if (ppRLE == NULL)
		delete []pBuffR;
	else
		*ppRLE = pBuffR;

	return nSize;
}

int yyRLEDecodeBmp (char * pRLE, int nW, int nH, char * pBMP)
{
	int		i = 0;
	int		nFrame = nW * nH;
	int		nSize = 0;
	char	cNum = 0;
	char *	pR = pBMP;
	while (nSize < nFrame)
	{
		cNum = *pRLE++;
		nSize += cNum;
		for (i = 0; i < cNum; i++)
		{
			*pR = *pRLE;
			pR += 4;
		}
		pRLE++;
	}
	nSize = 0;
	pR = pBMP + 1;
	while (nSize < nFrame)
	{
		cNum = *pRLE++;
		nSize += cNum;
		for (i = 0; i < cNum; i++)
		{
			*pR = *pRLE;
			pR += 4;
		}
		pRLE++;
	}
	nSize = 0;
	pR = pBMP + 2;
	while (nSize < nFrame)
	{
		cNum = *pRLE++;
		nSize += cNum;
		for (i = 0; i < cNum; i++)
		{
			*pR = *pRLE;
			pR += 4;
		}
		pRLE++;
	}
	nSize = 0;
	pR = pBMP + 3;
	while (nSize < nFrame)
	{
		cNum = *pRLE++;
		nSize += cNum;
		for (i = 0; i < cNum; i++)
		{
			*pR = *pRLE;
			pR += 4;
		}
		pRLE++;
	}

	return nFrame * 4;
}
