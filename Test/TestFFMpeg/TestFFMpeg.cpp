// TestFFMpeg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "CFFMpegTest.h"

#include "CBaseUtils.h"

#include "UFFMpegFunc.h"

int _tmain(int argc, _TCHAR* argv[])
{
	yyInitFFMpeg ();


	CFFMpegTest * pFFT = new CFFMpegTest (NULL);

	pFFT->Open (_T("\\SDMMC\\MaxTek\\MP3-5\\MP3\\032. Æíµ».MP3"), 0);



	yyFreeFFMpeg ();

	return 0;
}

