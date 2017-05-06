/*******************************************************************************
	File:		CTestFFMpeg.cpp

	Contains:	The ext render player implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "Psapi.h"

#include "CTestFFMpeg.h"

#include "CFFMpegSource.h"


#include "CBaseUtils.h"
#include "yyLog.h"

CTestFFMpeg::CTestFFMpeg(void * hInst)
	: m_hInst (hInst)
	, m_pFmtCtx (NULL)
{
	CBaseUtils::FillExtIOFunc (&m_ioFileExt);
}

CTestFFMpeg::~CTestFFMpeg(void)
{

}

void CTestFFMpeg::Test (void)
{
	int nRC = 0;

	MEMORYSTATUS	memInfo;
	memset (&memInfo, 0, sizeof (memInfo));

#ifdef _OS_WINPC
	PROCESS_MEMORY_COUNTERS memProc;
	memset (&memProc, 0, sizeof (memProc));

	HANDLE hProc = GetCurrentProcess ();
#endif// _OS_WINPC

	while (true)
	{
		GlobalMemoryStatus (&memInfo);
		TCHAR szMemInfo[128];
		DWORD dwUsed = (memInfo.dwTotalPhys - memInfo.dwAvailPhys) / 1024;
		if (dwUsed < 1000000)
			_stprintf (szMemInfo, _T("Mem: % 3d,%03dK / % 8dK\r\n"),
						(dwUsed % 1000000) / 1000, dwUsed % 1000, memInfo.dwTotalPhys / 1024);
		else
			_stprintf (szMemInfo, _T("Mem: %d,%03d,%03dK / % 8dK\r\n"),
						dwUsed / 1000000, (dwUsed % 1000000) / 1000, dwUsed % 1000, memInfo.dwTotalPhys / 1024);
//		::OutputDebugString (szMemInfo);

#ifdef _OS_WINPC
		GetProcessMemoryInfo  (hProc, &memProc, sizeof (memProc));
		YYLOGI ("Mem: %d", memProc.WorkingSetSize);
#endif // _OS_WINPC

//		TestReadOnly ();

		TestFFMpegSource ();


	}

}

void CTestFFMpeg::TestFFMpegSource (void)
{
	int nRC = 0;

	YY_BUFFER buff;
	memset (&buff, 0, sizeof (buff));
	buff.nType = YY_MEDIA_Audio;

	CFFMpegSource * pSource = new CFFMpegSource (NULL);

	TCHAR szSource[256];
#ifdef _OS_WINCE
	_tcscpy (szSource, _T("\\SDMMC\\MaxTek\\MP3-5\\MP3\\032. Æíµ».MP3"));
#else
	_tcscpy (szSource, _T("O:\\Data\\Works\\Customers\\MaxTek\\MP3-5\\MP3\\032. Æíµ».MP3"));
#endif // _OS_WINCE
	pSource->Open (szSource, 0);
	pSource->Start ();
	while (true)
	{
		nRC = pSource->ReadData (&buff);
		if (nRC == YY_ERR_FINISH)
			break;
		Sleep (1);
	}
	pSource->Stop ();

	delete pSource;
}


void CTestFFMpeg::TestReadOnly (void)
{
	int nRC = 0;

	TCHAR szSource[256];
	_tcscpy (szSource, _T("\\SDMMC\\MaxTek\\MP3-5\\MP3\\032. Æíµ».MP3"));

	char szFileName[1024];
	memset (szFileName, 0, sizeof (szFileName));
	sprintf (szFileName, "ExtIO:%08X", &m_ioFileExt);
	char * pFileName = szFileName + strlen (szFileName);
#ifdef _UNICODE
	CBaseUtils::ConvertDataToBase64 ((const unsigned char *)szSource, _tcslen (szSource) * sizeof (TCHAR),
										pFileName, sizeof (szFileName) - strlen (szFileName));
#else
	strcat (szFileName, pFile);
#endif // UNICODE

	// open input file, and allocate format context
	nRC = avformat_open_input(&m_pFmtCtx, szFileName, NULL, NULL);

    nRC = avformat_find_stream_info(m_pFmtCtx, NULL);

	AVPacket	avPacket;

	av_init_packet (&avPacket);

	while (nRC >= 0)
	{
		nRC = av_read_frame (m_pFmtCtx, &avPacket);
		if (nRC < 0)
			break;

		av_free_packet (&avPacket);
	}

	avformat_close_input (&m_pFmtCtx);
}
